#pragma once

#include <tempo.h>
#include "state.h"

#include "IconsMaterialDesign.h"

void buildFonts(UIState_ptr state) {
    state->font_regular = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 18).value();
    state->font_italic = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Italic.ttf", 18).value();
    state->font_bold = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Bold.ttf", 18).value();
    state->font_title = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 30).value();

    // merge in icons
    std::vector< ImWchar> icons_ranges = {
        static_cast<ImWchar16>(ICON_MIN_MD),
        static_cast<ImWchar16>(ICON_MAX_MD),
        static_cast<ImWchar16>(0) };
    ImFontConfig icons_config;
    // icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = ImVec2(0, 4.f);
    icons_config.MergeMode = true;
    Tempo::AddIconsToFont(state->font_regular,
        "fonts/Icons/material-design-icons/MaterialIcons-Regular.ttf", icons_config, icons_ranges
    );
}