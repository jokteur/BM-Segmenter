#include "settings.h"

#include <toml.hpp>
#include <iostream>
#include <exception>


class FailedToLoadSettingsFile: public std::exception
{
    std::string error_msg_;
public:
    FailedToLoadSettingsFile(std::string error_msg) : error_msg_(error_msg) {}
    virtual const char* what() const throw()
    {
        return error_msg_.c_str();
    }
};

/*
 * Settings implentations
 */

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

    const auto user_appearance = toml::find(settings, "user_appearance");

    const auto uisize = toml::find<int> (user_appearance, "ui_size");
    if (uisize < 50 || uisize > 300) {
        auto error = toml::format_error("[error] ui size should be between 50 and 300",
                                        user_appearance.at("ui_size"), "correct ui size needed here");
        throw FailedToLoadSettingsFile(error);
    }
    ui_size_ = uisize;

    const auto theme = toml::find<std::string> (user_appearance, "theme");
    if (theme != "dark" && theme != "light") {
        auto error = toml::format_error("[error] theme should be only \"dark\" or \"light\"",
                                        user_appearance.at("theme"), "correct theme name needed here");
        throw FailedToLoadSettingsFile(error);
    }
    if (theme == "dark") {
        setStyle(SETTINGS_DARK_THEME);
    }
    else if (theme == "light") {
        setStyle(SETTINGS_LIGHT_THEME);
    }

    const auto project = toml::find(settings, "project_settings");
    const auto recent_files = toml::find(project, "recent");
    recent_projects_ = toml::get<std::vector<std::string>>(recent_files);
}
