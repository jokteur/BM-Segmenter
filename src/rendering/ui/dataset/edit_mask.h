#pragma once

#include <string>
#include <vector>

#include "python/py_api.h"
#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "core/dataset/dataset.h"
#include "core/segmentation/segmentation.h"
#include "core/project/project_manager.h"
#include "core/dicom.h"
#include "events.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"
#include "rendering/ui/widgets/image_button.h"

namespace Rendering {
    struct Line {
        ImVec2 start;
        ImVec2 end;
    };
    /**
     * Little widget class for visualizing DICOM images
     */
    class EditMask : public AbstractLayout {
    private:
        static int instance_number;
        int my_instance_;

        std::string identifier_;
        bool is_open_ = true;

        ::core::Image image_;
        SimpleImage image_widget_;

        // Buttons
        ImageButton lasso_select_b_;
        ImageButton box_select_b_;
        ImageButton brush_select_b_;
        ImageButton undo_b_;
        ImageButton redo_b_;
        ImageButton info_b_;
        ImageButton* active_button_ = nullptr;
        std::vector<ImageButton*> buttons_list_;

        std::shared_ptr<::core::segmentation::Segmentation> active_seg_ = nullptr;
        int seg_idx_ = 0;
        std::vector<std::string> seg_names_;
        std::map<int, std::shared_ptr<::core::segmentation::Segmentation>> seg_map_;
        int num_segs_ = 0;
        int previous_seg_ = 0;

        // State variables

        float brush_size_ = 10;
        int add_sub_option_ = 0;

        bool threshold_hu_ = false;
        float hu_min_ = -29;
        float hu_max_ = 150;
        float prev_hu_min_ = 0;
        float prev_hu_max_ = 0;

        ImVec2 last_mouse_pos_;
        bool begin_action_ = false;
        
        ImVec2* raw_path_ = nullptr;
        int path_size = 0;

        // Listeners
        Listener listener_;
        Listener deactivate_buttons_;
        Listener reset_viewer_listener_;

        bool reset_image_ = false;

        bool active_dragging_ = false;
        ImVec2 drag_delta_;

        std::shared_ptr<::core::DicomSeries> dicom_series_ = nullptr;

        ::core::segmentation::Mask tmp_mask_;
        ::core::segmentation::Mask thresholded_hu_;
        ::core::Dicom tmp_dicom_;

        int case_select_ = 1;
        int previous_select_ = 0;
        int image_size_ = 0;

        void loadDicom(const std::shared_ptr<::core::DicomSeries> dicom);
        void loadCase(int idx);

        void button_logic();

        void build_mask();

        void lasso_widget(Rect& dimensions);
        void box_widget(Rect& dimensions);
        void brush_widget(Rect& dimensions);

        void accept_drag_and_drop();

    public:
        EditMask();
        ~EditMask();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };
}