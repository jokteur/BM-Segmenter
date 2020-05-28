#include <iostream>

#include "application.h"
#include "rendering/gui.h"
#include "GLFWwindow_handler.h"

int main(int, char**)
{
    GLFWwindowHandler::focus_all = true;

    // Test a simple initialization with an empty window
    Rendering::Application app("TestApp", 1280, 720);

    app.addImGuiFlags(ImGuiConfigFlags_ViewportsEnable
    | ImGuiConfigFlags_DockingEnable
    | ImGuiConfigFlags_NavEnableKeyboard);

    app.init();

    Rendering::GUI gui(app);

    app.loop();
    return 0;
}
