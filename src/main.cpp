#include "python/py_api.h"

#include <iostream>

#include "application.h"
#include "rendering/gui.h"
#include "GLFWwindow_handler.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const char* keyName = glfwGetKeyName(key, 0);
    std::cout << "KEY : " << keyName << std::endl;
}

int main(int, char**)
{
    auto *python = &PyAPI::Handler::getInstance();

    python->testFct();

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
