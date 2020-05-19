#ifndef BM_SEGMENTER_TEST_WINDOW_H
#define BM_SEGMENTER_TEST_WINDOW_H

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    class MyWindow : public AbstractLayout {
    private:

    public:
        MyWindow() {
        }

        void draw(GLFWwindow* window) override {
            ImGui::Begin("My Window");
            ImGui::Button("Hello, this is a button");
            ImGui::End();
        }
    };
}

#endif //BM_SEGMENTER_TEST_WINDOW_H
