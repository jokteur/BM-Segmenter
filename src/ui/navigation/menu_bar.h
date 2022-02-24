#pragma once

#include "ui/drawable.h"

class MenuBar : public Drawable {
private:
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};