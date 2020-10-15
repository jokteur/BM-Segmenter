#define PY_SSIZE_T_CLEAN
#include <Python.h>

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
    wchar_t *home_dir = Py_DecodeLocale("python", NULL);
    Py_SetPythonHome(home_dir);
    Py_Initialize();
    PyRun_SimpleString("import numpy as np\n"
                       "import pydicom\n"
                       "print(np.arange(10))\n");
    if (Py_FinalizeEx() < 0) {
        exit(120);
    }

    GLFWwindowHandler::focus_all = true;

//    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

//    py::print("Hello, World!"); // use the Python API

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
