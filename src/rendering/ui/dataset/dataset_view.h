#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "rendering/drawables.h"
#include "rendering/ui/dataset/preview.h"
#include "core/project/project_manager.h"
#include "jobscheduler.h"

namespace Rendering {

    /**
     * Defines the UI for opening and exploring new folders
     */
    class DatasetView : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_ = ::core::project::ProjectManager::getInstance();
        core::dataset::dicom_set dicoms_;

        std::map<std::shared_ptr<::core::DicomSeries>, Preview> dicom_previews_;

        std::vector<const char*> group_names_ = { "Show all" };
        std::vector<::core::dataset::Group> groups_;
        int group_idx_ = 0;
        int group_prev_idx_ = 0;


        std::shared_ptr<::core::segmentation::Segmentation> active_seg_ = nullptr;
        int seg_idx_ = 0;
        int seg_prev_idx_ = 0;
        std::vector<std::string> seg_names_;
        std::map<int, std::shared_ptr<::core::segmentation::Segmentation>> seg_map_;
        int num_segs_ = 0;

        int num_cols_ = 3;
        static void drag_and_drop(std::shared_ptr<::core::DicomSeries> case_);

        void preview_widget(Preview& preview, float width, ImVec2 mouse_pos, Rect sub_window_dim, std::shared_ptr<::core::DicomSeries> dicom, GLFWwindow* window, Rect& parent_dimension);
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