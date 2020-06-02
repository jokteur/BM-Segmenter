#include "gui.h"

#include "rendering/ui/shortcuts_list.h"

Rendering::GUI::GUI(Rendering::Application &app) {
    //app.getMainWindow().addDrawable(&menu_bar_);

    Shortcuts::init_global_shortcuts();

    app.getMainWindow().addDrawable(&dockspace_);
    app.getMainWindow().addDrawable(&window_);
    app.getMainWindow().addDrawable(&modals_);
    app.getMainWindow().addDrawable(&file_);
}
