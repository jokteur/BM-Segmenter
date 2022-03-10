#pragma once

#include <memory>
#include <tempo.h>

#include "translations/translate.h"
#include "search/search.h"
#include "search/widgets.h"

struct UIState {
    bool read_only = false;
    long long int imID = 100000000;
    float scaling = 1.f;

    // Fonts
    Tempo::FontID font_regular;
    Tempo::FontID font_italic;
    Tempo::FontID font_bold;
    Tempo::FontID font_title;
    Tempo::FontID icons_regular;

    // Languages
    Language language = LANG_EN;
    Translator babel_fr = build_FR();
    Translator babel_it = build_IT();
    Translator babel_default;
    Translator* babel_current = &this->babel_default;

    // Widget search
    Search::Universe search;

    // Nav bar
    enum Panel { IMPORT, DATASET, SEGMENTATION, MEASUREMENTS, ML };
    Panel active_panel = DATASET;
};
typedef std::shared_ptr<UIState> UIState_ptr;