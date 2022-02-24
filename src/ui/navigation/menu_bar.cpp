#include "menu_bar.h"
#include "ui/utils.h"

void MenuBar::FrameUpdate() {
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("Files")) {
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        ImGui::EndMenu();
    }

    float cursor_pos = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor_pos + SCALED_PX(10));

    ImGui::BeginTabBar("Nav bar");

    if (ImGui::BeginTabItem("Import")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Dataset")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Segmentation")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Measurements")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Machine learning")) {
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::EndMenuBar();
}

void MenuBar::BeforeFrameUpdate() {

}