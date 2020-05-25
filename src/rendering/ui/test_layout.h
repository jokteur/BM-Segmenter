#ifndef BM_SEGMENTER_TEST_LAYOUT_H
#define BM_SEGMENTER_TEST_LAYOUT_H

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    class MyLayout : public AbstractLayout {
    private:
        bool visible_ = false;

        void FileMenu() {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open project", "..."))
                {

                }
                if (ImGui::MenuItem("Save project", ""))
                {

                }
                if (ImGui::MenuItem("Close project", ""))
                {

                }
                ImGui::EndMenu();
            }
        }
    public:
        MyLayout() {
        }

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;


            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("Menu", &visible_, window_flags);
            ImGui::PopStyleVar();
            ImGui::PopStyleVar(2);

            if (ImGui::BeginMenuBar()) {
                FileMenu();

                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
    };
}

#endif //BM_SEGMENTER_TEST_LAYOUT_H
