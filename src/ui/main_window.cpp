#include <fstream>
#include <chrono>

#include "main_window.h"
#include "style.h"
#include "fonts.h"

#include "implot.h"
#include "imgui_internal.h"
#include "translations/translate.h"
#include "imgui_stdlib.h"

// namespace py = pybind11;

MainApp::MainApp() {
    m_menu_bar = std::make_shared<MenuBar>(m_ui_state);
}
void MainApp::InitializationBeforeLoop() {
    buildFonts(m_ui_state);
    defineStyle();
    BuildSymbols();
}

void MainApp::AfterLoop() {
    ImPlot::DestroyContext();
}

void MainApp::BuildSymbols() {
    m_menu_bar->BuildSymbols();
}

void MainApp::FrameUpdate() {
    m_ui_state->search.FreeFrame();
    // m_ui_state->scaling = Tempo::GetScaling();
    // float height = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    // ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, height));
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Main window", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking);
    ImGui::PopStyleVar(2);

    m_menu_bar->FrameUpdate();

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    ImGui::End();

    if (ImGui::GetIO().MouseClicked[0]) {
        m_ui_state->search.StopAllHighlights();
    }

    // static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // const ImGuiViewport* viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(viewport->WorkPos);
    // ImGui::SetNextWindowSize(viewport->WorkSize);
    // ImGui::SetNextWindowViewport(viewport->ID);

    // // because it would be confusing to have two docking targets within each others.
    // ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

    // window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    // window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // ImGui::Begin("DockSpace Demo", nullptr, window_flags);

    // ImGui::PopStyleVar(2);

    // ImGuiID dockspace_id = ImGui::GetID("Dockspace");

    // ImGui::End();

    // if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
    //     // Clear out existing layout
    //     ImGui::DockBuilderRemoveNode(dockspace_id);
    // }
    // dockspace_id;

    static bool w1, w2, w3;
    ImGui::Begin("Window 1", &w1);
    if (ImGui::Button("Search")) {
        m_ui_state->search.Highlight("item3");
    }

    ImGui::End();

    Widgets::Begin(S_PRE, "window2", "Window 2", &w2, ImGuiWindowFlags_MenuBar);

    ImGui::BeginMenuBar();
    if (Widgets::BeginMenu(S_PRE, "menu1", "My menu")) {
        Widgets::MenuItem(S_PRE, "item1", "Item 1");
        Widgets::MenuItem(S_PRE, "item2", "Item 2");
        if (Widgets::BeginMenu(S_PRE, "submenu", "Submenu")) {
            Widgets::MenuItem(S_PRE, "item3", "Item 3");
            Widgets::EndMenu();
        }
        Widgets::EndMenu();
    }
    ImGui::EndMenuBar();

    Widgets::End();

    ImGui::Begin("Window 3", &w3);
    ImGui::Button("Hello 3");
    ImGui::End();
    // if (m_open)
    ImGui::ShowDemoWindow(&m_open);
}
void MainApp::BeforeFrameUpdate() {
    m_menu_bar->BeforeFrameUpdate();
}