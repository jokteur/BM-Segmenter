#include "GLFWwindow_handler.h"
#include "rendering/keyboard_shortcuts.h"

#include <iostream>

std::multimap<int, GLFWwindow*> GLFWwindowHandler::windows_;
bool GLFWwindowHandler::focus_all = false;
bool GLFWwindowHandler::all_windows_unfocused = false;

void GLFWwindowHandler::focus_callback(GLFWwindow *window, int focused) {
    // If previously all windows were unfocused and
    // the user clicked on a window, we can put all windows from
    // the app to the front (if focus_all == true)
    if (all_windows_unfocused && focus_all && focused == GLFW_TRUE) {
        focusAll();
        all_windows_unfocused = false;
        return;
    }
    all_windows_unfocused = true;
    for(auto &element : windows_) {
        if (glfwGetWindowAttrib(element.second, GLFW_FOCUSED) == GLFW_TRUE) {
            all_windows_unfocused = false;
        }
    }
}

void GLFWwindowHandler::focusAll() {
    for(auto &element : windows_) {
        int focused = glfwGetWindowAttrib(element.second, GLFW_FOCUSED);
        if (focused == GLFW_FALSE)
            glfwShowWindow(element.second);
    }
}

void GLFWwindowHandler::addWindow(GLFWwindow *window, int z_index) {
    windows_.insert(std::pair<int, GLFWwindow*>(z_index, window));
    glfwSetWindowFocusCallback(window, &GLFWwindowHandler::focus_callback);
    glfwSetKeyCallback(window, &KeyboardShortCut::key_callback);
    //glfwSetCharCallback(window, &KeyboardShortCut::character_callback);
}

void GLFWwindowHandler::removeWindow(GLFWwindow *window) {
    for(auto it = windows_.begin(); it != windows_.end();it++) {
        if (it->second == window) {
            windows_.erase(it);
            break;
        }
    }
}

void GLFWwindowHandler::setZIndex(GLFWwindow *window, int z_index) {
    removeWindow(window);
    addWindow(window, z_index);

}
