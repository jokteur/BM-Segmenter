#pragma once

#include <memory>
#include <tempo.h>

struct UIState {
    bool read_only = false;
    long long int imID = 100000000;

    // Fonts
    Tempo::FontID font_regular;
    Tempo::FontID font_italic;
    Tempo::FontID font_bold;
    Tempo::FontID font_title;

    // Nav bar
    enum Panel { LOAN, MANAGEMENT, STATE, PROPERTIES, STATS };
    Panel active_panel = LOAN;
};
typedef std::shared_ptr<UIState> UIState_ptr;