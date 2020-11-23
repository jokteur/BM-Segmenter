#include "project_info.h"

#include "drag_and_drop.h"
#include "rendering/views/explore_view.h"
#include "rendering/views/default_view.h"

Rendering::ProjectInfo::ProjectInfo() {
	auto& project = project_manager_.getCurrentProject();
}

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto& project = project_manager_.getCurrentProject();
	ImGui::Begin("Project information");

	if (project != nullptr) {
		if (!is_set_) {
			if (!project->getSaveFile().empty()) {
				is_set_ = true;
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
		}

		if (is_set_) {
			auto& dataset = project->getDataset();
			ImGui::Text("Title: %s", project->getName().c_str());
			ImGui::Text("Description:\n%s", project->getDescription().c_str());
			ImGui::Separator();
			if (!dataset.getDicoms().empty()) {
				ImGui::Text("%d unique dicoms in the project.", dataset.getDicoms().size());
				ImGui::Text("Groups:");
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
			if (ImGui::Button("Import data to project")) {
				if (project->getSaveFile().empty()) {
					show_error_modal("Error", "You can not import data to a project without saving the project first.\n"
						"Please save by going to Files -> Save or use Ctrl+S");
				}
				else {
					EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ExploreView>())));
				}
			}
			ImGui::Separator();
			ImGui::Text("Segmentations");
			project->getSegmentations();
			for (auto& seg : project->getSegmentations()) {
				bool leaf = ImGui::TreeNodeEx(&seg, ImGuiTreeNodeFlags_Framed, "%s", seg->getName().c_str());
				if (leaf) {
					ImGui::TreePop();
				}
			}
			if (ImGui::Button("Create segmentation")) {
				new_segmentation_.showModal(project);
			}
		}
	}
	ImGui::End();
}
