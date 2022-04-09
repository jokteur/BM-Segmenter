#pragma once

#include "ui/drawable.h"

int SearchCallback(ImGuiInputTextCallbackData* data);

class SearchBar : public Drawable {
private:
    std::string m_search;
    bool m_show = false;

    ImVec2 m_pos;
    float m_search_width;
    int m_num_results = 0;

    std::map<int, std::vector<Search::Result>> m_search_results;
public:
    SearchBar(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    friend int SearchCallback(ImGuiInputTextCallbackData* data);

    void DrawSearchResults();
    void SetWindowShow(bool show) { m_show = show; }
};