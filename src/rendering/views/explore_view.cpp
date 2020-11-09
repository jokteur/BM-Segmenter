#include "explore_view.h"

#include "imgui_internal.h"
#include "ui/dockspace.h"

Rendering::ExploreView::ExploreView() {
//    ImGui::DockBuilderDockWindow("Find dicoms in folder", left);
    drawables_.push_back(explore_);
    drawables_.push_back(preview_);
    drawables_.push_back(dicom_);

    auto fct = [=] (ImGuiID* main_id) {
        ImGuiID id = ImGui::DockBuilderSplitNode(*main_id, ImGuiDir_Left, 0.9f, nullptr, main_id);
        explore_->setDockID(id);
    };
    EventQueue::getInstance().post(Event_ptr (new SetDockSpaceFctEvent(fct)));
}

void Rendering::ExploreView::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
//    dock_id_ = Dockspace::getInstance().getDockSpaceID();
//    std::cout << "hello" << std::endl;
//    ImGuiID id = ImGui::DockBuilderSplitNode(dock_id_, ImGuiDir_Left, 0.9f, nullptr, &dock_id_);
//    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_, ImGuiDir_Left, 0.20f, nullptr, &dock_id_);
//    explore_->setDockID(left);
}