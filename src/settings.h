#ifndef BM_SEGMENTER_SETTINGS_H
#define BM_SEGMENTER_SETTINGS_H

#include "imgui.h"

class Settings {
private:
    int current_theme = SETTINGS_LIGHT_THEME;
    int last_theme = SETTINGS_LIGHT_THEME;

    float ui_size = 1.;
    float last_ui_size = 1.;

    Settings() {}
public:
    enum Theme {SETTINGS_LIGHT_THEME, SETTINGS_DARK_THEME};
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    Settings(Settings const &) = delete;
    void operator=(Settings const &) = delete;

    void setTheme(Theme theme);
    void setUIsize(float size);

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static Settings& getInstance () {
        static Settings instance;
        return instance;
    }

    float getUIsize() {return ui_size; }
    int getCurrentTheme() { return current_theme; }
};


#endif //BM_SEGMENTER_SETTINGS_H
