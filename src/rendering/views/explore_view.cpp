#include "explore_view.h"

#include "imgui_internal.h"
#include "ui/dockspace.h"

Rendering::ExploreView::ExploreView() {
    EventQueue::getInstance().post(Event_ptr(new Event("menu/change_title/Import data to project")));
    drawables_.push_back(explore_);
    drawables_.push_back(preview_);
    //drawables_.push_back(dicom_);

//    auto fct = [=] (ImGuiID* main_id) {
//        ImGuiID id = ImGui::DockBuilderSplitNode(*main_id, ImGuiDir_Left, 0.9f, nullptr, main_id);
//        explore_->setDockID(id);
//    };
//    EventQueue::getInstance().post(Event_ptr (new SetDockSpaceFctEvent(fct)));
}