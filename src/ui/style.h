#pragma once

void defineStyle() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.94f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.31f, 0.36f, 0.38f, 0.41f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.35f, 0.35f, 0.35f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.36f, 0.70f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.12f, 0.53f, 0.90f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.36f, 0.70f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.31f, 0.36f, 0.38f, 1.00f);


    auto& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(4, 5);
    style.ScrollbarSize = 18;

    // Borders
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;

    style.WindowRounding = 3;
    style.PopupRounding = 3;
    style.FrameRounding = 5;
    style.GrabRounding = 2;
    style.WindowMenuButtonPosition = ImGuiDir_None;

}