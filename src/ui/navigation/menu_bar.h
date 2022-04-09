#pragma once

#include "ui/drawable.h"
#include "search_bar.h"

int SearchCallback(ImGuiInputTextCallbackData* data);

class MenuBar : public Drawable {
private:
    SearchBar m_search_bar;
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state), m_search_bar(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
    void BuildSymbols() override;

    friend int SearchCallback(ImGuiInputTextCallbackData* data);
};