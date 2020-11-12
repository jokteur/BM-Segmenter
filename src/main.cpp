#include "python/py_api.h"
#include "python/init_python.h"

#include <iostream>

#include "rendering/gui.h"
#include "GLFWwindow_handler.h"
#include "settings.h"

int main(int, char**)
{
    PyAPI::Handler::getInstance();

    GLFWwindowHandler::focus_all = true;

    // Test a simple initialization with an empty window
    Rendering::Application app("ML image segmentation", 1280, 720);

    app.addImGuiFlags(ImGuiConfigFlags_ViewportsEnable
    | ImGuiConfigFlags_DockingEnable
    | ImGuiConfigFlags_NavEnableKeyboard);

    app.init();
    PyAPI::init();

    Rendering::GUI::getInstance().init(app);

    app.loop();
    return 0;
}
