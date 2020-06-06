#include "settings.h"


void Settings::defineLightStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    // Popup Bg
    style.Colors[ImGuiCol_PopupBg] = ImColor(240, 240, 240, 255);
    light_ = style;
}


void Settings::defineDarkStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    dark_ = style;
}

void Settings::setStyle(Theme theme) {
    current_theme_ = theme;
    ImGuiStyle &style = ImGui::GetStyle();
    if (current_theme_ == SETTINGS_LIGHT_THEME) {
        style = light_;
    }
    else {
        style = dark_;
    }
    style.ScaleAllSizes(current_scale_);
}

void Settings::setUIsize(int size) {
    if (size > 0.) {
        ui_size_ = size;
    }
}

void Settings::setScale(float scale) {
    current_scale_ = scale;
    setStyle(current_theme_);
}