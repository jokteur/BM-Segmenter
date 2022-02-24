#include <fstream>

#include "main_window.h"

#include "implot.h"
#include "imgui_internal.h"

// namespace py = pybind11;
void MainApp::InitializationBeforeLoop() {
    ui_state->font_regular = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 18).value();
    ui_state->font_italic = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Italic.ttf", 18).value();
    ui_state->font_bold = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Bold.ttf", 18).value();
    ui_state->font_title = Tempo::AddFontFromFileTTF("fonts/Roboto/Roboto-Regular.ttf", 30).value();
}

void MainApp::AfterLoop() {
    ImPlot::DestroyContext();
}

void MainApp::FrameUpdate() {
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
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::BeginTabBar("Nav bar");

    if (ImGui::BeginTabItem("Import")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Dataset")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Segmentation")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Measurements")) {
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Machine learning")) {
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::End();
    if (m_open)
        ImGui::ShowDemoWindow(&m_open);
}
void MainApp::BeforeFrameUpdate() {
}