#include "dockspace.h"
#include "imgui_internal.h"

Rendering::Dockspace::Dockspace() {
    dockspace_flags_ = ImGuiDockNodeFlags_None;
    window_flags_ = ImGuiWindowFlags_NoDocking;

    window_flags_ |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags_ |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    // ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;

    listener_.filter = "set_dock_fct";
    listener_.callback = [=] (Event_ptr event) {
        rebuild_ = true;
        build_dock_fct_ = reinterpret_cast<SetDockSpaceFctEvent*>(event.get())->getFct();
    };
    EventQueue::getInstance().subscribe(&listener_);
}

void Rendering::Dockspace::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    viewport->GetCenter();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    //ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &open_, window_flags_);
    ImGui::PopStyleVar();


    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        dockspace_id_ = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id_);
//        ImGui::DockBuilderRemoveNode(dockspace_id_); // Clear out existing layout
//        ImGui::DockBuilderAddNode(dockspace_id_, ImGuiDockNodeFlags_DockSpace);
//        ImGui::DockBuilderSetNodeSize(dockspace_id_, ImGui::GetContentRegionAvail());

//        if (rebuild_) {
//            build_dock_fct_(&dockspace_id_);
//            rebuild_ = false;
//        }
    }
    ImGui::PopStyleVar(2);
    menu_bar_.ImGuiDraw(window, parent_dimension);
    ImGui::End();

}

Rendering::Dockspace::~Dockspace() {
    EventQueue::getInstance().unsubscribe(&listener_);
}
