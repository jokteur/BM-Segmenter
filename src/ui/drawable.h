#pragma once

#include <memory>

#include "state.h"

// using namespace core;

class Drawable {
protected:
    std::shared_ptr<UIState> m_ui_state;
public:
    Drawable(std::shared_ptr<UIState> ui_state) : m_ui_state(ui_state) {}
    virtual ~Drawable() {}

    virtual void FrameUpdate() {}
    virtual void BeforeFrameUpdate() {}
};