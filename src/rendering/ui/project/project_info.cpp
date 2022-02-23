#include "project_info.h"

#include "drag_and_drop.h"
#include "rendering/views/explore_view.h"
#include "rendering/views/default_view.h"
#include "rendering/ui/widgets/util.h"
#include "rendering/animation_util.h"

#include "log.h"

void Rendering::ProjectInfo::build_seg_colors(std::shared_ptr<::core::project::Project> project) {
	for (auto color_ptr : colors_) {
		delete[] color_ptr;
	}

	colors_.clear();

	for (auto& seg : project->getSegmentations()) {
		auto& color = seg->getMaskColor();
		float* color_ = new float[4];
		color_[0] = color.x;
		color_[1] = color.y;
		color_[2] = color.z;
		color_[3] = color.w;
		colors_.push_back(color_);
	}
}

void Rendering::ProjectInfo::send_reload_seg_event(std::shared_ptr<::core::project::Project> project) {
	auto time_since_last_evt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_reload_evt_);

	if (time_since_last_evt.count() > 500 && color_changed_) {
		last_reload_evt_ = std::chrono::system_clock::now();

		jobFct job = [=](float& progress, bool& abort) -> std::shared_ptr<JobResult> {
			project->saveSegmentations();
			EventQueue::getInstance().post(Event_ptr(new ::core::segmentation::ReloadSegmentationEvent()));
			return std::make_shared<JobResult>();
		};
		JobScheduler::getInstance().addJob("reload_segmentation", job);
		color_changed_ = false;
	}
}

Rendering::ProjectInfo::ProjectInfo() {
	enter_shortcut_.keys = { KEY_ENTER };
	enter_shortcut_.name = "enter";
	enter_shortcut_.callback = [this] {
		confirm_ = true;
	};
}

Rendering::ProjectInfo::~ProjectInfo() {
	for (auto color_ptr : colors_) {
		delete[] color_ptr;
	}
}

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto project = project_manager_.getCurrentProject();
	ImGui::Begin("Project information");

	if (project != nullptr) {

		// We don't want to spam the reload events
		// This function throttles the events that are sended
		send_reload_seg_event(project);

		if (!is_set_ && !project->getSaveFile().empty()) {
			is_set_ = true;
			BM_DEBUG("Set project and load segmentations");
			auto err = project->getDataset().load(project->getSaveFile());
			if (!err.empty()) {
				show_error_modal("Error when opening the project", "An error occured when opening the project's dataset.", err.c_str());
				EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<DefaultView>())));
			}
			err = project->loadSegmentations();
			if (!err.empty()) {
				show_error_modal("Error when opening the segmentations", "An error occured when opening the project's segmentations.", err.c_str());
				EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<DefaultView>())));
			}
		}

		if (is_set_) {
			auto& dataset = project->getDataset();
			{
				ImGui::Text("Title: %s", project->getName().c_str());
				ImGui::Text("Description:\n%s", project->getDescription().c_str());
				ImGui::Separator();
			}

			// Show groups
			{
				if (!dataset.getDicoms().empty()) {
					ImGui::Text("%d unique dicoms in the project.", dataset.getDicoms().size());
					Widgets::NewLine(5.f);
					ImGui::Text("Groups:");
					Widgets::NewLine(5.f);
					for (auto& group : dataset.getGroups()) {
						if (set_tree_closed_) {
							ImGui::SetNextItemOpen(false);
						}
						bool node = ImGui::TreeNodeEx((void*)&group, ImGuiTreeNodeFlags_Framed, "%s: %d dicoms", group.getName().c_str(), group.getDicoms().size());
						if (ImGui::BeginDragDropTarget()) {
							if (ImGui::AcceptDragDropPayload("_DICOM_PAYLOAD")) {
								auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
								auto dicom = drag_and_drop.returnData();
								group.addDicom(dicom);
								std::string err = dataset.save(project->getRoot());
								if (!err.empty()) {
									show_error_modal("Failed to save dataset", err);
								}
							}
							ImGui::EndDragDropTarget();
						}
						if (node) {
							for (auto& dicom : group.getOrderedDicoms()) {
								std::string bullet_text;
								bool leaf;
								if (dicom->getPaths().size() == 1)
									leaf = ImGui::TreeNodeEx(&dicom, ImGuiTreeNodeFlags_Bullet, "%s", ::core::parse_dicom_id(dicom->getId()).first.c_str());
								else
									leaf = ImGui::TreeNodeEx(&dicom, ImGuiTreeNodeFlags_Bullet, "%s (%d images)", ::core::parse_dicom_id(dicom->getId()).first.c_str(), dicom->getPaths().size());

								if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
									auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
									drag_and_drop.giveData(dicom);
									int a = 0; // Dummy int
									ImGui::SetDragDropPayload("_DICOM_PAYLOAD", &a, sizeof(a));
									ImGui::Text("%s", dicom->getIdPair().first.c_str());
									ImGui::EndDragDropSource();
								}

								if (ImGui::BeginPopupContextItem()) {
									if (ImGui::Selectable("Remove dicom from group")) {
										BM_DEBUG("Remove dicom " + dicom->getIdPair().first + "from group " + group.getName());
										group.removeDicom(dicom);
										dataset.save(project->getSaveFile());
									}
									ImGui::EndPopup();
								}
								if (leaf)
									ImGui::TreePop();
							}
							ImGui::TreePop();
						}
					}
					set_tree_closed_ = false;
				}
				Widgets::NewLine(5.f);
				if (ImGui::Button("Import data to project")) {
					if (project->getSaveFile().empty()) {
						show_error_modal("Error", "You can not import data to a project without saving the project first.\n"
							"Please save by going to Files -> Save or use Ctrl+S");
					}
					else {
						BM_DEBUG("Set import view");
						EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ExploreView>())));
					}
				}
				Widgets::NewLine(5.f);
				ImGui::Separator();
			}

			// Segmentation menu
			{
				Widgets::NewLine(5.f);
				ImGui::Text("Segmentations:");
				Widgets::NewLine(5.f);
				int i = 0;

				if (project->getSegmentations().size() != colors_.size()) {
					build_seg_colors(project);
				}

				for (auto& seg : project->getSegmentations()) {
					auto& color = seg->getMaskColor();
					ImGui::PushStyleColor(ImGuiCol_Header, seg->getMaskColor());
					bool open = ImGui::CollapsingHeader(seg->getName().c_str());
					ImGui::PopStyleColor();
					if (open) {
						// Segmentation color
						auto col = colors_[i];

						// Models
						ImGui::Text("Models");
						if (ImGui::Button("New model")) {
							BM_DEBUG("New model modal");
							push_animation();
							new_model_.showModal(seg);
						}

						ImGui::Separator();
						ImGui::Text("Color:");
						ImGui::ColorPicker4(("Color for " + seg->getName()).c_str(), col, ImGuiColorEditFlags_AlphaPreviewHalf);
						if (col[0] != color.x || col[1] != color.y || col[2] != color.z || col[3] != color.w) {
							seg->setMaskColor(ImVec4(col[0], col[1], col[2], col[3]));
							color_changed_ = true;
						}
					}
					i++;
				}
				Widgets::NewLine(7.f);
				if (ImGui::Button("Create segmentation")) {
					BM_DEBUG("New segmentation modal");
					push_animation();
					new_segmentation_.showModal(project);
				}
				Widgets::NewLine(5.f);
				ImGui::Separator();
			}

			// Users
			{
				users_ = project->getUsers();
				Widgets::NewLine(5.f);
				ImGui::Text("Users");
				Widgets::NewLine(5.f);

				for (auto& user : users_) {
					ImGui::BulletText("%s", user.c_str());
				}
				
				ImGui::Button("Add user");
				if (ImGui::BeginPopupContextItem("Create user", 0)) {
					ImGui::InputText("Name of user", &current_user_);
					KeyboardShortCut::addTempShortcut(enter_shortcut_);
					KeyboardShortCut::ignoreNormalShortcuts();

					if ((ImGui::Button("OK") || confirm_) && !current_user_.empty()) {
						BM_DEBUG("Create user");
						project->setCurrentUser(current_user_);
						current_user_ = "";
						ImGui::CloseCurrentPopup();
						project_manager_.saveProjectToFile(project, project->getSaveFile());
						confirm_ = false;
					}
					ImGui::EndPopup();
				}
			}
		}
	}
	ImGui::End();
}
