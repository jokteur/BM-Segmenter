#pragma once

#include "nfd.h"
#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "core/dataset/explore.h"
#include "rendering/drawables.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/ui/dataset/dicom_preview.h"
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
        Listener job_listener_;

        ::core::dataset::Explore explorer_;

        ImGuiTextBuffer log_buffer_;
        ImGuiTextBuffer error_buffer_;

        ImGuiTextFilter case_filter_;
        ImGuiTextFilter study_filter_;
        ImGuiTextFilter series_filter_;
        ImGuiTextFilter image_filter_;

        std::map<::core::dataset::SeriesNode*, DicomPreview> previews_;

        std::string path_;

        bool build_preview_ = false;
        void build_preview();

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
