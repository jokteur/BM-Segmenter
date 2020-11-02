#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "core/dataset/explore.h"
#include "rendering/drawables.h"
#include "rendering/ui/dataset/dicom_preview.h"
#include "jobscheduler.h"
#include "util.h"

namespace Rendering {

    /**
     * Defines the UI for opening and exploring new folders
     */
    class ExplorerPreview : public AbstractLayout {
    private:

        /*
         * For capturing the log sent by the Dicom Search
         */
        Listener build_tree_listener_;
        Listener filter_tree_listener_;

        std::vector<core::dataset::PatientNode> cases_;
        std::map<::core::dataset::SeriesNode*, DicomPreview> dicom_previews_;

        int num_cols_ = 3;

        bool open_ = true;
        bool just_opened_ =  true;
        ImVec2 init_size_;

    public:
        /**
         * Initializes the listener and subscribes to the queue
         */
        explicit ExplorerPreview(ImVec2 init_size = ImVec2(800, 400));

        /**
         * Destructor
         */
        ~ExplorerPreview() override;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}
