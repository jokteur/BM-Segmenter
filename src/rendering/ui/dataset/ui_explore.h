#pragma once

#include "nfd.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "imgui.h"

#include "core/dataset/explore.h"
#include "rendering/drawables.h"
#include "rendering/ui/modales/error_message.h"
#include "jobscheduler.h"
#include "log.h"
#include "util.h"

namespace Rendering {
    /**
     * Defines the UI for opening and exploring new folders
     */
    class ExploreFolder : public AbstractLayout {
    private:

        /*
         * For capturing the log sent by the Dicom Search
         */
        Listener log_listener_;
        Listener error_listener_;

        ::core::dataset::Explore explorer_;

        ImGuiTextBuffer log_buffer_;
        ImGuiTextBuffer error_buffer_;

        ImGuiTextFilter case_filter_;
        ImGuiTextFilter study_filter_;
        ImGuiTextFilter series_filter_;
        ImGuiTextFilter image_filter_;

        ::core::dataset::ImageNode *selected_node_;

        std::string path_;

    public:
        /**
         * Initializes the listener and subscribes to the queue
         */
        ExploreFolder();

        /**
         * Destructor
         */
        ~ExploreFolder() override;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}
