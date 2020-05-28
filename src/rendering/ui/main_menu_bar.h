#ifndef BM_SEGMENTER_MAIN_MENU_BAR_H
#define BM_SEGMENTER_MAIN_MENU_BAR_H


#include "nfd.h"

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    class MainMenuBar : public AbstractLayout {
    private:
        int counter_ = 0;
        nfdchar_t *outPath = NULL;

        void file_menu() {
            if (ImGui::MenuItem("New project", "Ctrl+Shift+N")) {}
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
        void edit_menu() {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
        }

        void open_file() {
            nfdresult_t result = NFD_OpenDialog( "ml_prj", NULL, &outPath );
            if (result == NFD_OKAY) {
                // Do something
            }
        }
    public:
        MainMenuBar() = default;

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
                ImGui::EndMainMenuBar();
            }
        }
    };
}

#endif //BM_SEGMENTER_MAIN_MENU_BAR_H
