#include "project_info.h"

#include "drag_and_drop.h"
#include "rendering/views/explore_view.h"
#include "rendering/views/default_view.h"
#include "rendering/ui/widgets/util.h"

Rendering::ProjectInfo::ProjectInfo() {
	enter_shortcut_.keys = { KEY_ENTER };
	enter_shortcut_.name = "enter";
	enter_shortcut_.callback = [this] {
		confirm_ = true;
	};
}

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto project = project_manager_.getCurrentProject();
	ImGui::Begin("Project information");

	if (project != nullptr) {
		if (!is_set_ && !project->getSaveFile().empty()) {
			is_set_ = true;
			std::cout << "Set dataset" << std::endl;
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
							ImGui::SetNextTreeNodeOpen(false);
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
									ImGui::Text("%s", ::core::parse_dicom_id(dicom->getId()).first.c_str());
									ImGui::EndDragDropSource();
								}

								if (ImGui::BeginPopupContextItem()) {
									if (ImGui::Selectable("Remove dicom from group")) {
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
				for (auto& seg : project->getSegmentations()) {
					auto& color = seg->getMaskColor();
					ImGui::PushStyleColor(ImGuiCol_Header, seg->getMaskColor());
					bool leaf = ImGui::TreeNodeEx(&seg, ImGuiTreeNodeFlags_Framed, "%s", seg->getName().c_str());
					ImGui::PopStyleColor();
					if (leaf) {
						ImGui::Text("Overlay color (mask color):"); ImGui::SameLine();
						ImGui::ColorButton("Overlay color (with alpha)", color, ImGuiColorEditFlags_AlphaPreviewHalf);
						if (ImGui::BeginPopupContextItem("Change color", 0)) {
							static float edit_col[4] = { color.x, color.y, color.z, color.w };
							ImGui::ColorPicker4("color", edit_col, ImGuiColorEditFlags_AlphaPreviewHalf);
							ImVec4 new_color = { edit_col[0], edit_col[1], edit_col[2], edit_col[3] };
							if (new_color.x != color.x || new_color.y != color.y || new_color.z != color.z || new_color.w != color.w) {
								seg->setMaskColor(edit_col);
								EventQueue::getInstance().post(Event_ptr(new ::core::segmentation::ReloadSegmentationEvent()));
								project->saveSegmentations();
							}
							ImGui::EndPopup();
						}
						ImGui::TreePop();
					}
				}
				Widgets::NewLine(5.f);
				if (ImGui::Button("Create segmentation")) {
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
