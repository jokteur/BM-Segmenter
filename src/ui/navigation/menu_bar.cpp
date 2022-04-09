#include "menu_bar.h"
#include "ui/utils.h"


#include "IconsMaterialDesign.h"

#include "ui/translations/translate.h"

void MenuBar::BuildSymbols() {
    m_ui_state->search.Register("MenuBar_Files");
    m_ui_state->search.Register("MenuBar_NewProject", "MenuBar_Files", { TXT("New project"), TXT("Create project") });
    m_ui_state->search.Register("MenuBar_OpenProject", "MenuBar_Files", { TXT("Open project") });

    m_ui_state->search.Register("MenuBar_Settings");
    m_ui_state->search.Register("MenuBar_Language", "MenuBar_Settings", { TXT("Change language"), TXT("Set display language") });
}

void MenuBar::FrameUpdate() {
    ImGui::BeginMenuBar();
    if (Widgets::BeginMenu(S_PRE, "MenuBar_Files", TXT("Files"))) {
        Widgets::MenuItem(S_PRE, "MenuBar_NewProject", TXT(ICON_MD_NOTE_ADD " New project"));
        Widgets::MenuItem(S_PRE, "MenuBar_OpenProject", TXT(ICON_MD_FOLDER_OPEN " Open project"));
        Widgets::EndMenu();
    }

    if (Widgets::BeginMenu(S_PRE, "MenuBar_Settings", TXT("Settings"))) {
        if (Widgets::BeginMenu(S_PRE, "MenuBar_Language", TXT(ICON_MD_LANGUAGE " Language"))) {
            if (ImGui::MenuItem("English")) {
                m_ui_state->babel_current = &m_ui_state->babel_default;
            }
            if (ImGui::MenuItem("Français")) {
                m_ui_state->babel_current = &m_ui_state->babel_fr;
            }
            if (ImGui::MenuItem("Italiano")) {
                m_ui_state->babel_current = &m_ui_state->babel_it;
            }
            Widgets::EndMenu();
        }
        Widgets::EndMenu();
    }

    float cursor_pos = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursor_pos + SCALED_PX(10));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 5));

    m_search_bar.FrameUpdate();

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

    m_search_bar.DrawSearchResults();
}

void MenuBar::BeforeFrameUpdate() {

}