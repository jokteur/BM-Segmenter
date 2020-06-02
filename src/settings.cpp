#include "settings.h"

void Settings::setTheme(Theme theme) {
    current_theme = theme;
    if (current_theme == SETTINGS_LIGHT_THEME) {
        ImGui::StyleColorsLight();
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
