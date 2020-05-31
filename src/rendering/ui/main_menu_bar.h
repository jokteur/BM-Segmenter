#ifndef BM_SEGMENTER_MAIN_MENU_BAR_H
#define BM_SEGMENTER_MAIN_MENU_BAR_H

#include "nfd.h"
#include "rendering/drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

#include "core/project/project_manager.h"

namespace Rendering {
    class MainMenuBar : public AbstractLayout {
    private:
        int counter_ = 0;
        nfdchar_t *outPath = NULL;
        ProjectManager& project_manager_;


        void file_menu() {
            if (ImGui::MenuItem("New project", "Ctrl+Shift+N")) {
                ImGui::OpenPopup("Stacked 1");
                if (ImGui::BeginPopupModal("Stacked 1", NULL, ImGuiWindowFlags_MenuBar)) {
                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    project_manager_.newProject("Test", "b");
                }
            }
            if (ImGui::MenuItem("Open project", "Ctrl+O")) {
                open_file();
            }
            if (ImGui::BeginMenu("Open Recent"))
            {
                ImGui::MenuItem("prj1.c");
                ImGui::MenuItem("prj2.inl");
                ImGui::MenuItem("prj3.h");
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {}


        }
        static void edit_menu() {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
        }

        void projects_menu() {
            if (project_manager_.getNumProjects() > 0) {
                for(auto &prj : project_manager_) {
                    bool is_active = project_manager_.getCurrentProject() == prj;
                    if (ImGui::MenuItem(prj->getName().c_str(), "", is_active)) {}
                }
            }
            else
                ImGui::MenuItem("No opened project", "", false, false);
        }

        void open_file() {
            nfdresult_t result = NFD_OpenDialog( "ml_prj", NULL, &outPath );
            if (result == NFD_OKAY) {
                // Do something
            }
        }
    public:
        MainMenuBar() : project_manager_(ProjectManager::getInstance()) {

        }


        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {
            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    file_menu();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    edit_menu();
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Projects"))
                {
                    projects_menu();
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }
    };
}

#endif //BM_SEGMENTER_MAIN_MENU_BAR_H
