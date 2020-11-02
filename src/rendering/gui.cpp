#include "gui.h"

#include "rendering/ui/shortcuts_list.h"
#include "rendering/ui/modales/error_message.h"
#include "settings.h"
#include "toml.hpp"

Rendering::GUI::GUI(Rendering::Application &app) {
    //app.getMainWindow().addDrawable(&menu_bar_);

    Shortcuts::init_global_shortcuts();

    try {
        Settings::getInstance().loadSettings("settings.toml");
    }
    catch (std::exception &e) {
        show_error_modal("Load setting error",
                         "An error occured when loading settings file (settings.toml)\n"
                         "Default settings have been applied",
                         e.what());
    }

    app.getMainWindow().addDrawable(&dockspace_);
//    app.getMainWindow().addDrawable(&window_);
//    app.getMainWindow().addDrawable(&dicomViewer_2);
    app.getMainWindow().addDrawable(&preview_);
    app.getMainWindow().addDrawable(&modals_);
    app.getMainWindow().addDrawable(&dicomViewer_);
    app.getMainWindow().addDrawable(&exploreFolder_);
}
