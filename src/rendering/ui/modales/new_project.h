#ifndef BM_SEGMENTER_NEW_PROJECT_H
#define BM_SEGMENTER_NEW_PROJECT_H

#include "rendering/drawables.h"
#include "core/project/project_manager.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    class NewProjectModale : public AbstractLayout {
    private:
        ProjectManager &project_manager_;

    public:
        NewProjectModale() : project_manager_(ProjectManager::getInstance()) {}

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {
        }
    };
}

#endif //BM_SEGMENTER_NEW_PROJECT_H
