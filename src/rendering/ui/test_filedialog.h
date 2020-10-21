#ifndef BM_SEGMENTER_TEST_FILEDIALOG_H
#define BM_SEGMENTER_TEST_FILEDIALOG_H

#include "nfd.h"

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "python/py_api.h"

namespace Rendering {
    class MyFile : public AbstractLayout {
    private:
        int counter_ = 0;

    public:
        MyFile() {
        }

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {
            ImGui::Begin("File window");
            if(ImGui::Button("Open file")) {
                NFD_Init();
                nfdchar_t *outPath;
                nfdresult_t result = NFD_PickFolder(&outPath, NULL);
                if (result == NFD_OKAY) {
                    PyAPI::Handler::getInstance().testFct(outPath);
                }
                NFD_Quit();
            }
            ImGui::End();

        }
    };
}

#endif //BM_SEGMENTER_TEST_FILEDIALOG_H
