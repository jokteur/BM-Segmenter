
#define IMGUI_USE_WCHAR32

#include <iostream>
#include <tempo.h>
#include <chrono>

#include "ui/main_window.h"


// using namespace core;
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main() {

    Tempo::Config config{
        .app_name = "BM-Segmenter",
        .app_title = "Bio-medical image segmenting",
    };
    config.maximized = true;
    config.wait_timeout = 10.;
    // config.imgui_config_flags = 0;

    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}