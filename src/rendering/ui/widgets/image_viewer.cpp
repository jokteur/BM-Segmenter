#include <iostream>
#include "image_viewer.h"
#include "imgui.h"

int Rendering::ImageViewer::instance_number = 0;

void Rendering::ImageViewer::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    ImGui::Begin("Test Image");
    button_.ImGuiDraw(window, parent_dimension);
    if (ImGui::Button("open")) {
        auto image = ::core::Image();
        image.setImage("assets/giuli.jpg");
        image_widget_.setImage(image);
//        image_widget_.setSize(ImVec2(100, 100));
    }
    image_widget_.ImGuiDraw(window, parent_dimension);
    ImGui::End();
}
