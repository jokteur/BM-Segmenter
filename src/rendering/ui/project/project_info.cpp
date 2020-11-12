#include "project_info.h"

void Rendering::ProjectInfo::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto project = project_manager_.getCurrentProject();
	ImGui::Begin("Project information");

	if (project != nullptr) {
		ImGui::Text("Title: %s", project->getName().c_str());
		ImGui::Text("Description:\n%s", project->getDescription().c_str());
		ImGui::Separator();
	}
	ImGui::End();
}
