#pragma once

#include <memory>
#include <tempo.h>

#include "translations/translate.h"

struct UIState {
    bool read_only = false;
    long long int imID = 100000000;
    float scaling = 1.f;

    // Fonts
    Tempo::FontID font_regular;
    Tempo::FontID font_italic;
    Tempo::FontID font_bold;
    Tempo::FontID font_title;

    // Nav bar
    enum Panel { IMPORT, DATASET, SEGMENTATION, MEASUREMENTS, ML };
    Panel active_panel = DATASET;
};
typedef std::shared_ptr<UIState> UIState_ptr;