#include "search_bar.h"

#include <chrono>
#include "IconsMaterialDesign.h"

#include "misc/cpp/imgui_stdlib.h"
#include "ui/utils.h"
#include "ui/translations/translate.h"

using namespace std::chrono;
int SearchCallback(ImGuiInputTextCallbackData* data) {
    SearchBar* search_bar = (SearchBar*)data->UserData;

    Search::Universe& search = search_bar->m_ui_state->search;

    search_bar->m_search_results = search.getSearchResults(data->Buf, 20.);
    int num = 0;
    for (auto pair : search_bar->m_search_results) {
        for (auto result : pair.second) {
            num++;
        }
    }
    search_bar->m_num_results = num;
    search_bar->m_show = true;
    Tempo::PollUntil(100);
    return 0;
}

void SearchBar::FrameUpdate() {
    std::string search_txt = TXT("Search for something to do");
    m_search_width = ImGui::CalcTextSize(search_txt.c_str()).x + SCALED_PX(20.f);

    m_pos = ImGui::GetCursorPos();

    ImGui::SetNextItemWidth(m_search_width);
    ImGui::InputTextWithHint(
        ICON_MD_SEARCH, search_txt.c_str(),
        &m_search,
        ImGuiInputTextFlags_CallbackEdit,
        SearchCallback,
        (void*)this
    );


    ImGuiViewport* viewport = ImGui::GetWindowViewport();
    m_pos.x += viewport->WorkPos.x;
    m_pos.y += viewport->WorkPos.y;
}

void SearchBar::DrawSearchResults() {
    if (!m_show || m_search.empty()) {
        return;
    }

    auto& style = ImGui::GetStyle();
    float item_height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2.0f;
    float window_height = item_height * (float)MIN(MAX(1, m_num_results), 5) + style.FramePadding.y * 2.0f;
    ImVec2 window_pos(m_pos.x, m_pos.y + item_height);

    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(ImVec2(m_search_width * 1.5f, window_height));
    ImGui::SetNextWindowBgAlpha(0.9f);

    ImGui::Begin("#Search results", &m_show,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoFocusOnAppearing);
    for (auto it = m_search_results.rbegin();it != m_search_results.rend();it++) {
        for (auto result : it->second) {
            if (ImGui::Selectable(("    > " + result.description).c_str())) {
                m_ui_state->search.Highlight(result.widget_name);
                m_show = false;
                m_search = "";
            }
        }
    }
    if (m_search_results.empty())
        ImGui::Text(TXT("No result was found"));

    /* Only hide the window if the user clicks outside of the results window */
    bool is_mouse_inside_window = ImGui::IsMouseHoveringRect(
        window_pos,
        ImVec2(window_pos.x + m_search_width * 1.5f, window_pos.y + window_height)
    );
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
        (!is_mouse_inside_window && (
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)
            || ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            )
        ) {
        m_show = false;
        m_search = "";
    }

    ImGui::End();
}

void SearchBar::BeforeFrameUpdate() {

}