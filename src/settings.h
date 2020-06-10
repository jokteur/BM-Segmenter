#ifndef BM_SEGMENTER_SETTINGS_H
#define BM_SEGMENTER_SETTINGS_H

#include "imgui.h"
#include <string>
#include <list>

#define STRING2(X) #X
#define STRING(X) STRING2(X)
#define PROJECT_EXTENSION ml_prj

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
    std::string filesave_;

    std::list<std::string> recent_projects_;

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

    /**
     * Sets the theme of the app (light or dark)
     * @param theme
     */
    void setStyle(Theme theme);

    /**
     *
     * @param size
     */
    void setUIsize(int size);

    /**
     * Sets the scale parameter of the UI
     * @param scale scale parameter (in %)
     */
    void setScale(float scale);

    /**
     * Sets the file save for where settings will be saved
     * @param filename
     */
    void setFileSave(std::string filename) {
        filesave_ = filename;
    }

    void addRecentFile(std::string filename);

    /**
     * Saves all the settings to the current save file
     * Use setFileSave to set the current save file
     */
    void saveSettings();

    /**
     * Loads the setting from a file (in toml format)
     * If an error occurs, the toml library launches an exception
     * By default, if there is no file under `filename`, then a settings file is created with default values
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

    /**
     * @return returns current size of the ui
     */
    int &getUIsize() { return ui_size_; }

    /**
     * @return returns the current theme
     */
    int getCurrentTheme() { return current_theme_; }

    /**
     * @return returns the recent files (projects) saved in the settings
     */
    std::list<std::string>& getRecentFiles() { return recent_projects_; }
};


#endif //BM_SEGMENTER_SETTINGS_H
