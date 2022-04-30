#pragma once

#include <tempo.h>

#include "state.h"
#include "navigation/menu_bar.h"

// Drawable and widgets

using std::chrono::duration_cast;
using std::chrono::milliseconds;
typedef std::chrono::high_resolution_clock pclock;
typedef std::chrono::time_point<std::chrono::steady_clock> tp;

class MainApp : public Tempo::App {
private:
    bool m_open = false;

    std::string m_open_error;

    std::shared_ptr<MenuBar> m_menu_bar;

    tp last;
    bool open_popup = false;

    std::shared_ptr<UIState> m_ui_state = std::make_shared<UIState>();
    std::string my_string;

public:
    MainApp();
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;

    void BuildSymbols();

    void AfterLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};