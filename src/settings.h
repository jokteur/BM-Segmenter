#ifndef BM_SEGMENTER_SETTINGS_H
#define BM_SEGMENTER_SETTINGS_H

#include "imgui.h"
#include <string>
#include <vector>

class Settings {
public:
    enum Theme {SETTINGS_LIGHT_THEME, SETTINGS_DARK_THEME};
private:
    Theme current_theme_ = SETTINGS_LIGHT_THEME;

    ImGuiStyle light_;
    ImGuiStyle dark_;

    // In percentage
    int ui_size_ = 100;

    // For saving the settings to the disk
    std::string filesave;

    std::vector<std::string> recent_projects_;

    float current_scale_ = 1.f;

    void defineLightStyle();
    void defineDarkStyle();

    Settings() {
        defineLightStyle();
        defineDarkStyle();
    }
public:
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    Settings(Settings const &) = delete;
    void operator=(Settings const &) = delete;

    void setStyle(Theme theme);
    void setUIsize(int size);

    void setScale(float scale);

    void setFileSave(std::string filename) {
        filesave = filename;
    }

    void saveSettings();
    /**
     * Loads the setting from a file (in toml format)
     * If an error occurs, the toml library launches an exception
     * @param filename
     */
    void loadSettings(std::string filename);

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static Settings& getInstance () {
        static Settings instance;
        return instance;
    }

    int &getUIsize() { return ui_size_; }
    int getCurrentTheme() { return current_theme_; }
};


#endif //BM_SEGMENTER_SETTINGS_H
