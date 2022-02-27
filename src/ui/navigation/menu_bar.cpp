#include "menu_bar.h"
#include "ui/utils.h"
#include "misc/cpp/imgui_stdlib.h"

#include "IconsMaterialDesign.h"

#include "../translations/translate.h"

void MenuBar::FrameUpdate() {
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu("Files")) {
        ImGui::MenuItem(ICON_MD_NOTE_ADD " New project");
        ImGui::MenuItem(ICON_MD_FOLDER_OPEN " Open project");
        ImGui::MenuItem(TXT("hello world %s", "hello"));
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Settings")) {
        if (ImGui::BeginMenu(ICON_MD_LANGUAGE " Language")) {
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    float cursor_pos = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor_pos + SCALED_PX(10));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 5));

    ImGui::SetNextItemWidth(SCALED_PX(190.f));
    ImGui::InputTextWithHint(ICON_MD_SEARCH, "Search for something to do", &m_search);

    ImGui::BeginTabBar("Nav bar");

    if (ImGui::BeginTabItem(ICON_MD_DOWNLOAD " Import")) {
        m_ui_state->active_panel = UIState::IMPORT;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(ICON_MD_APPS " Dataset")) {
        m_ui_state->active_panel = UIState::DATASET;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(ICON_MD_APP_REGISTRATION " Segmentation")) {
        m_ui_state->active_panel = UIState::SEGMENTATION;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(ICON_MD_STRAIGHTEN " Measurements")) {
        m_ui_state->active_panel = UIState::MEASUREMENTS;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(ICON_MD_AUTO_FIX_NORMAL " Machine learning")) {
        m_ui_state->active_panel = UIState::ML;
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::EndMenuBar();

    ImGui::PopStyleVar();
}

void MenuBar::BeforeFrameUpdate() {

}