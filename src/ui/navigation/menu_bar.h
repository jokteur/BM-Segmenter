#pragma once

#include "ui/drawable.h"

class MenuBar : public Drawable {
private:
    std::string m_search;
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};