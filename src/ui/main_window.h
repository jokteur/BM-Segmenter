#pragma once

#include <tempo.h>

#include "state.h"
#include "navigation/menu_bar.h"

// Drawable and widgets

class MainApp : public Tempo::App {
private:
    bool m_open = false;

    std::string m_open_error;

    std::shared_ptr<MenuBar> m_menu_bar;

    tp last;
    bool open_popup = false;

    std::shared_ptr<UIState> m_ui_state = std::make_shared<UIState>();

public:
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void BuildSymbols();

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};