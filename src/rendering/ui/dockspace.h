#ifndef BM_SEGMENTER_DOCKSPACE_H
#define BM_SEGMENTER_DOCKSPACE_H

#include "nfd.h"

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

#include "main_menu_bar.h"

namespace Rendering {
    class Dockspace : public AbstractLayout {
    private:
        int counter_ = 0;
        bool open_ = true;
        MainMenuBar menu_bar_;
        ImGuiDockNodeFlags dockspace_flags_;
        ImGuiWindowFlags window_flags_;
    public:
        Dockspace() {
            dockspace_flags_ = ImGuiDockNodeFlags_None;
            window_flags_ = ImGuiWindowFlags_NoDocking;

            window_flags_ |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags_ |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            // ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
        }

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkPos());
            ImGui::SetNextWindowSize(viewport->GetWorkSize());
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            // window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", &open_, window_flags_);
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            // DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("Dockspace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags_);
            }

            menu_bar_.ImGuiDraw(window, parent_dimension);
        }
    };
}

#endif //BM_SEGMENTER_DOCKSPACE_H
