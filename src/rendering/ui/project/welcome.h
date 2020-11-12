#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"
#include "nfd.h"

#include "rendering/drawables.h"
#include "rendering/ui/project/new_project.h"

#include "core/project/project_manager.h"

#include "jobscheduler.h"
#include "settings.h"
#include "util.h"

namespace Rendering {
    /**
     * Defines the UI for opening and exploring new folders
     */
    class WelcomeView : public AbstractLayout {
    private:
        NewProjectModal new_project_modal_;
        ::core::project::ProjectManager& project_manager_ = ::core::project::ProjectManager::getInstance();
    public:
        WelcomeView() = default;
        ~WelcomeView() override = default;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };
}