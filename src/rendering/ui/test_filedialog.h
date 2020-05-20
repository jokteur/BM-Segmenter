#ifndef BM_SEGMENTER_TEST_FILEDIALOG_H
#define BM_SEGMENTER_TEST_FILEDIALOG_H

#include "nfd.h"

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"


namespace Rendering {
    class MyFile : public AbstractLayout {
    private:
        int counter_ = 0;
        nfdchar_t *outPath = NULL;

    public:
        MyFile() {
        }

        void draw(GLFWwindow* window) override {
            ImGui::Begin("File window");
            if(ImGui::Button("Open file")) {
                nfdresult_t result = NFD_OpenDialog( "png,jpg;pdf", NULL, &outPath );
            }
            ImGui::End();

        }
    };
}

#endif //BM_SEGMENTER_TEST_FILEDIALOG_H
