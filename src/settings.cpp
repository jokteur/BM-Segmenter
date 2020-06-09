#include "settings.h"

#include <toml.hpp>
#include <iostream>

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

void Settings::saveSettings() {

}

void Settings::loadSettings(std::string filename) {
    const auto settings =  toml::parse(filename);

    const auto user_appearance = toml::find(settings, "user appearance");

    const auto uisize = toml::find<int> (user_appearance, "ui size");
    if (uisize < 50 || uisize > 300) {
        std::cerr << toml::format_error("[error] value should be between 50 and 300",
                                        user_appearance.at("ui size"), "correct ui size need here")
                  << std::endl;
    }

    const auto theme = toml::find<std::string> (user_appearance, "theme");

    const auto project = toml::find(settings, "project settings");
    const auto recent_files = toml::find(project, "recent");
    const auto recent_files_list = toml::get<std::vector<std::string>>(recent_files);
}
