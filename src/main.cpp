#include <iostream>

#include "application.h"

#include "rendering/ui/test_layout.h"
#include "rendering/ui/test_filedialog.h"
#include "rendering/ui/test_jobScheduler.h"

int main(int, char**)
{
    // Test a simple initialization with an empty window
    Rendering::Application app("TestApp", 1280, 720);

    app.addImGuiFlags(ImGuiConfigFlags_ViewportsEnable
    | ImGuiConfigFlags_DockingEnable
    | ImGuiConfigFlags_NavEnableKeyboard);

    app.init();

    Rendering::MyLayout layout;
    Rendering::MyWindow window;
    Rendering::MyFile file;

    app.getMainWindow().addDrawable(&layout);
    app.getMainWindow().addDrawable(&window);
    app.getMainWindow().addDrawable(&file);

    app.loop();
    return 0;
}
