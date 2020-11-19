#include "project_info.h"

#include "rendering/views/explore_view.h"
#include "rendering/views/default_view.h"

Rendering::ProjectInfo::ProjectInfo() {
	auto &project = project_manager_.getCurrentProject();
}

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto &project = project_manager_.getCurrentProject();
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
			}
		}

		if (is_set_) {
			auto& dataset = project->getDataset();
			ImGui::Text("Title: %s", project->getName().c_str());
			ImGui::Text("Description:\n%s", project->getDescription().c_str());
			ImGui::Separator();
			if (!dataset.getDicoms().empty()) {
				ImGui::Text("%d dicoms in the project.", dataset.getDicoms().size());
				for (auto& group : dataset.getGroups()) {
					ImGui::BulletText("%s: %d dicoms", group.getName().c_str(), group.getDicoms().size());
				}
			}
			if (!project->getDataset().getGroups().empty()) {
				ImGui::Text("%s sdffd", project->getDataset().getGroups()[0].getName().c_str());
			}
			if (ImGui::Button("SDFSDF") && !project->getDataset().getDicoms().empty()) {
					project->getDataset().getDicoms()[0]->loadCase(0);
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
		}
	}
	ImGui::End();
}
