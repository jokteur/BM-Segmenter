#pragma once

#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "rendering/drawables.h"
#include "rendering/ui/dataset/preview.h"
#include "core/project/project_manager.h"
#include "rendering/ui/widgets/util.h"
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
        std::map<std::shared_ptr<::core::DicomSeries>, Rect> dicom_sizes_;

        std::vector<::core::dataset::Group> groups_;
        int group_idx_ = 0;
        Widgets::Selectable group_select_;

        int col_count_ = 0;

        bool reset_draw_ = true;
        Rect prev_window_dim_;

        float true_height_ = 0.f;
        bool calc_height = false;

        ::core::Image validated_;
        ::core::Image edited_;

        std::shared_ptr<::core::segmentation::Segmentation> active_seg_ = nullptr;
        std::map<int, std::shared_ptr<::core::segmentation::Segmentation>> seg_map_;
        Widgets::Selectable seg_select_;

        int num_cols_ = 3;

        inline void preview_widget(Preview& preview, float width, ImVec2 mouse_pos, Rect sub_window_dim, std::shared_ptr<::core::DicomSeries> dicom, GLFWwindow* window, Rect& parent_dimension);
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