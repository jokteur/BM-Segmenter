#include "project_info.h"

#include "rendering/views/explore_view.h"

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto project = project_manager_.getCurrentProject();
	ImGui::Begin("Project information");

	if (project != nullptr) {
		ImGui::Text("Title: %s", project->getName().c_str());
		ImGui::Text("Description:\n%s", project->getDescription().c_str());
		ImGui::Separator();
		if (ImGui::Button("Import data to project")) {
			EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ExploreView>())));
		}
	}
	ImGui::End();
}
