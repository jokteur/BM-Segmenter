#include <fstream>

#include "main_window.h"
#include "style.h"

#include "implot.h"
#include "imgui_internal.h"

// namespace py = pybind11;

MainApp::MainApp() {
    m_menu_bar = std::make_shared<MenuBar>(ui_state);
}
void MainApp::InitializationBeforeLoop() {
    ui_state->font_regular = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 18).value();
    ui_state->font_italic = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Italic.ttf", 18).value();
    ui_state->font_bold = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Bold.ttf", 18).value();
    ui_state->font_title = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 30).value();

    defineStyle();
}

void MainApp::AfterLoop() {
    ImPlot::DestroyContext();
}

void MainApp::FrameUpdate() {
    ui_state->scaling = Tempo::GetScaling();
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
#else 
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif

    ImGui::Begin("Main window", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoBringToFrontOnFocus);

    m_menu_bar->FrameUpdate();

    ImGui::End();
    // if (m_open)
    ImGui::ShowDemoWindow(&m_open);
}
void MainApp::BeforeFrameUpdate() {
    m_menu_bar->BeforeFrameUpdate();
}