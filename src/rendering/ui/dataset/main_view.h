#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "core/dataset/explore.h"
#include "rendering/drawables.h"
#include "rendering/ui/import/dicom_preview.h"
#include "jobscheduler.h"
#include "util.h"

namespace Rendering {

    /**
     * Defines the UI for opening and exploring new folders
     */
    class DatasetView : public AbstractLayout {
    private:
        std::vector<std::shared_ptr<::core::DicomSeries>> cases_;

        int num_cols_ = 3;

        // Image editing options
        //ImVec2 crop_x_ = ImVec2(0, 100);
        //ImVec2 crop_y_ = ImVec2(0, 100);
        //int window_width_ = 400;
        //int window_center_ = 40;

        bool open_ = true;
        bool is_cases_set_ = false;
        bool just_opened_ = true;
        ImVec2 init_size_;

    public:
        /**
         * Initializes the listener and subscribes to the queue
         */
        explicit DatasetView(ImVec2 init_size = ImVec2(800, 400));

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
