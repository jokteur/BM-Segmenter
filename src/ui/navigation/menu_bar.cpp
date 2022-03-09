#include "menu_bar.h"
#include "ui/utils.h"
#include "misc/cpp/imgui_stdlib.h"

#include "IconsMaterialDesign.h"

#include "ui/translations/translate.h"

void MenuBar::FrameUpdate() {
    ImGui::BeginMenuBar();
    if (ImGui::BeginMenu(TXT("Files"))) {
        ImGui::MenuItem(TXT(ICON_MD_NOTE_ADD " New project"));
        ImGui::MenuItem(TXT(ICON_MD_FOLDER_OPEN " Open project %s", (m_ui_state) ? "yes" : "no"));
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(TXT("Settings"))) {
        if (ImGui::BeginMenu(TXT(ICON_MD_LANGUAGE " Language"))) {
            if (ImGui::MenuItem("English")) {
                m_ui_state->babel_current = &m_ui_state->babel_default;
            }
            if (ImGui::MenuItem("Français")) {
                m_ui_state->babel_current = &m_ui_state->babel_fr;
            }
            if (ImGui::MenuItem("Italiano")) {
                m_ui_state->babel_current = &m_ui_state->babel_it;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    float cursor_pos = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor_pos + SCALED_PX(10));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 5));

    ImGui::SetNextItemWidth(SCALED_PX(190.f));
    ImGui::InputTextWithHint(ICON_MD_SEARCH, TXT("Search for something to do"), &m_search);

    ImGui::BeginTabBar("Nav bar");

    if (ImGui::BeginTabItem(TXT(ICON_MD_APPS " Dataset"))) {
        m_ui_state->active_panel = UIState::DATASET;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(TXT(ICON_MD_DOWNLOAD " Import"))) {
        m_ui_state->active_panel = UIState::IMPORT;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(TXT(ICON_MD_APP_REGISTRATION " Segmentation"))) {
        m_ui_state->active_panel = UIState::SEGMENTATION;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(TXT(ICON_MD_STRAIGHTEN " Measurements"))) {
        m_ui_state->active_panel = UIState::MEASUREMENTS;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(TXT(ICON_MD_AUTO_FIX_NORMAL " Machine learning"))) {
        m_ui_state->active_panel = UIState::ML;
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::EndMenuBar();

    ImGui::PopStyleVar();
}

void MenuBar::BeforeFrameUpdate() {

}