#include "gui.h"

Rendering::GUI::GUI(Rendering::Application &app) {
    //app.getMainWindow().addDrawable(&menu_bar_);
    app.getMainWindow().addDrawable(&dockspace_);
    app.getMainWindow().addDrawable(&window_);
    app.getMainWindow().addDrawable(&file_);
}
