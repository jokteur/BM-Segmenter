#pragma once

#include <string>
#include <imgui.h>

#include "search.h"

#define S_PRE m_ui_state->search

namespace Widgets {
    bool Begin(Search::Universe& search, const std::string& id, const std::string& name, bool* p_open = (bool*)0, ImGuiWindowFlags flags = 0);
    void End();

    bool BeginChild();
    void EndChild();

    bool BeginMenu(Search::Universe& search, const std::string& id, const std::string& label, bool enabled = true);
    void EndMenu();

    bool MenuItem(Search::Universe& search, const std::string& id, const std::string& label, const char* shortcut = (const char*)0, bool selected = false, bool enabled = true);
}