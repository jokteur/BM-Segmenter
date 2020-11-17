#pragma once

#include <vector>
#include <string>
#include <map>
#include "first_include.h"
#include "imgui.h"

#include "rendering/drawables.h"
#include "core/project/project_manager.h"
#include "jobscheduler.h"

namespace Rendering {
    /**
     * Defines the UI for opening and exploring new folders
     */
    class ProjectInfo : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_ = ::core::project::ProjectManager::getInstance();
        bool is_set_ = false;
    public:
        ProjectInfo();
        ~ProjectInfo() override = default;

        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };

}