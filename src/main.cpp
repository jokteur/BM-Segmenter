#include <iostream>

#include "gui/application.h"

#include "gui/layout/test_layout.h"

int main(int, char**)
{
    // Test a simple initialization with an empty window
    GUI::Application app("TestApp", 1280, 720);

    GUI::MyLayout layout;

    app.getMainWindow().addDrawable(&layout);

    app.loop();
    return 0;
}
