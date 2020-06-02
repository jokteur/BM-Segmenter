#include "settings.h"

void Settings::setTheme(Theme theme) {
    current_theme = theme;
    if (current_theme == SETTINGS_LIGHT_THEME) {
        ImGui::StyleColorsLight();
        ImGuiStyle& style = ImGui::GetStyle();
        // Popup Bg
        style.Colors[4] = ImColor(240, 240, 240, 255);
    }
    else {
        ImGui::StyleColorsDark();
    }
}

void Settings::setUIsize(float size) {
    if (size > 0.) {
        ui_size = size;
    }
}
