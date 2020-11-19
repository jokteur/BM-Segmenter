#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "rendering/drawables.h"
#include "core/project/project_manager.h"
#include "jobscheduler.h"

namespace Rendering {

    /**
     * Defines the UI for opening and exploring new folders
     */
    class DatasetView : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_ = ::core::project::ProjectManager::getInstance();
        std::vector<std::shared_ptr<::core::DicomSeries>> cases_;
    public:
        /**
         * Initializes the listener and subscribes to the queue
         */
        explicit DatasetView();

        /**
         * Destructor
         */
        ~DatasetView() override;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };
}