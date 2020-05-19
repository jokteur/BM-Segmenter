#include <iostream>

#include "rendering/application.h"

#include "rendering/ui/test_layout.h"
#include "rendering/ui/test_window.h"

int main(int, char**)
{
    // Test a simple initialization with an empty window
    Rendering::Application app("TestApp", 1280, 720);

    Rendering::MyLayout layout;
    Rendering::MyWindow window;

    app.getMainWindow().addDrawable(&layout);
    app.getMainWindow().addDrawable(&window);

    app.loop();
    return 0;
}
