#include "dataset_view.h"


Rendering::DatasetView::DatasetView() {
	auto& project = project_manager_.getCurrentProject();
}

Rendering::DatasetView::~DatasetView() {
}

void Rendering::DatasetView::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto& project = project_manager_.getCurrentProject();
	ImGui::Begin("Dataset overview");

	if (project != nullptr) {
		if (project->getDataset().getDicoms().size() != cases_.size()) {
			cases_ = project->getDataset().getDicoms();
		}
		for (auto& case_ : cases_) {
			ImGui::BulletText("%s", case_->getId().c_str());
			case_->loadCase(0);
			break;
		}
	}
	ImGui::End();
}
