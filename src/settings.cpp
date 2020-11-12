#include "settings.h"

#include <toml.hpp>
#include <iostream>
#include <exception>
#include <fstream>


class SettingsError: public std::exception
{
    std::string error_msg_;
public:
    SettingsError(std::string error_msg) : error_msg_(error_msg) {}
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

    // Define all the colors
    colors_.disabled_text = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
}


void Settings::defineDarkStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    dark_ = style;

    // Define all the colors
    colors_.disabled_text = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
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
    saveSettings();
}

void Settings::setUIsize(int size) {
    if (size > 0.) {
        ui_size_ = size;
        saveSettings();
    }
}

void Settings::setScale(float scale) {
    current_scale_ = scale;
    setStyle(current_theme_);
}

void Settings::saveSettings() {
    if (filesave_.empty()) {
        filesave_ = "settings.toml";
    }
    std::ofstream file(filesave_, std::ios::trunc);

    file << "[user_appearance]" << std::endl;

    const toml::value user_appearance{
            {"ui_size", ui_size_},
            {"theme", (current_theme_ == SETTINGS_LIGHT_THEME) ? "light" : "dark"}};

    file << user_appearance << std::endl;

    file << "[project_settings]" << std::endl;
    const toml::value recent_files{
            {"recent", recent_projects_}};
    file << recent_files << std::endl;
}

void Settings::loadSettings(std::string filename) {
    std::ifstream file(filename, std::ios_base::binary);
    if (!file) {
        filesave_ = filename;
        saveSettings();
        return;
    }
    filesave_ = filename;

    const auto settings =  toml::parse(file);

    // User appearance settings
    const auto user_appearance = toml::find(settings, "user_appearance");
    const auto uisize = toml::find<int> (user_appearance, "ui_size");
    if (uisize < 50 || uisize > 300) {
        auto error = toml::format_error("[error] ui size should be between 50 and 300",
                                        user_appearance.at("ui_size"), "correct ui size needed here");
        throw SettingsError(error);
    }
    ui_size_ = uisize;

    const auto theme = toml::find<std::string> (user_appearance, "theme");
    if (theme != "dark" && theme != "light") {
        auto error = toml::format_error("[error] theme should be only \"dark\" or \"light\"",
                                        user_appearance.at("theme"), "correct theme name needed here");
        throw SettingsError(error);
    }
    if (theme == "dark") {
        setStyle(SETTINGS_DARK_THEME);
    }
    else if (theme == "light") {
        setStyle(SETTINGS_LIGHT_THEME);
    }

    // Projects settings
    const auto project = toml::find(settings, "project_settings");
    const auto recent_files = toml::find(project, "recent");
    const auto file_list = toml::get<std::vector<std::string>>(recent_files);
    for(auto& name : file_list) {
        recent_projects_.push_back(name);
    }
}

void Settings::addRecentFile(std::string filename) {
    for (auto it = recent_projects_.begin(), end = recent_projects_.end(); it != end; it++) {
        // If the file already exists, erase it and let it push to the top
        if (*it == filename)  {
            recent_projects_.erase(it);
            break;
        }
    }
    recent_projects_.push_back(filename);
    saveSettings();
}

void Settings::removeRecentFile(std::string filename) {
    for (auto it = recent_projects_.begin(), end = recent_projects_.end(); it != end; it++) {
        // If the file already exists, erase it and let it push to the top
        if (*it == filename) {
            recent_projects_.erase(it);
            break;
        }
    }
}
