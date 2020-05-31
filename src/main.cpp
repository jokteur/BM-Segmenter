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
    GLFWwindowHandler::focus_all = true;

    Shortcut shortcut = {
            .keys = {GLFW_KEY_N, KEY_CTRL},
            .description = "New project",
    };
    Shortcut shortcut1 = {
            .keys = {GLFW_KEY_N, KEY_CTRL, KEY_SHIFT},
            .description = "New file",
    };

    Listener listener{.filter="shortcuts/global/*",
            .callback = [] (Event_ptr &event) {
                std::cout << "Shortcut :" << event.get()->getName() << std::endl;
            }
    };
    EventQueue::getInstance().subscribe(&listener);
    KeyboardShortCut::addShortcut(shortcut1);
    KeyboardShortCut::addShortcut(shortcut);

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
