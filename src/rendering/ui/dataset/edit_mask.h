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
#include "rendering/ui/widgets/util.h"
#include "rendering/keyboard_shortcuts.h"

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

        // Shortcuts
        Shortcut ctrl_z_;
        Shortcut ctrl_y_;
        Shortcut hide_mask_shortcut;

        // Buttons
        ImageButton lasso_select_b_;
        ImageButton box_select_b_;
        ImageButton brush_select_b_;
        ImageButton validate_b_;
        ImageButton undo_b_;
        ImageButton redo_b_;
        ImageButton info_b_;
        ImageButton *active_button_ = nullptr;
        std::vector<ImageButton *> buttons_list_;

        std::shared_ptr<::core::segmentation::Segmentation> active_seg_ = nullptr;

        ImVec2 dicom_dimensions_;

        // State variables
        float brush_size_ = 10;
        float auto_brush_size_ = 3;
        int add_sub_option_ = 0;

        bool threshold_hu_ = false;
        float hu_min_ = -29;
        float hu_max_ = 150;
        float prev_hu_min_ = 0;
        float prev_hu_max_ = 0;

        ImVec2 last_mouse_pos_;
        bool begin_action_ = false;

        ImVec2 *raw_path_ = nullptr;
        int path_size = 0;

        bool show_mask = true;
        bool previous_show_mask = true;

        // Listeners
        Listener load_dicom_;
        Listener group_listener_;
        Listener reload_seg_;
        Listener load_segmentation_;
        Listener deactivate_buttons_;
        Listener reset_viewer_listener_;

        bool reset_image_ = false;

        bool build_hu_mask_ = false;
        bool active_dragging_ = false;
        ImVec2 drag_delta_;

        std::shared_ptr<::core::DicomSeries> dicom_series_ = nullptr;

        Widgets::Selectable name_select_;
        int num_names_ = 0;

        ::core::segmentation::Mask tmp_mask_;
        ::core::segmentation::Mask hu_threshold_mask;
        ::core::segmentation::Mask vertebra_distance_mask;
        ::core::segmentation::Mask visceral_fat_mask;
        ::core::segmentation::Mask other_segmentation_mask;
        ::core::segmentation::Mask edition_limit_mask;
        bool use_edition_limit_mask;

        std::shared_ptr<::core::segmentation::MaskCollection> mask_collection_ = nullptr;

        int case_select_ = 1;
        int previous_select_ = 0;
        int image_size_ = 0;

        // Variables for telling if there is a next or previous dicom in list
        std::shared_ptr<::core::DicomSeries> next_dicom_ = nullptr;
        std::shared_ptr<::core::DicomSeries> prev_dicom_ = nullptr;
        int group_idx_ = -1;

        void unload_mask();

        void unload_dicom(bool no_reset = false);

        void loadDicom(const std::shared_ptr<::core::DicomSeries> dicom, bool no_reset = false);

        void loadCase(int idx);

        void load_segmentation(std::shared_ptr<::core::segmentation::Segmentation> seg);

        bool load_mask();

        void disable_buttons();

        void button_logic();

        // For group select
        void set_NextPrev_buttons();

        void next();

        void previous();

        void set_mask();

        void undo();

        void redo();

        void mask_changed();


        void lasso_widget(Rect &dimensions);

        void box_widget(Rect &dimensions);

        void brush_widget(Rect &dimensions);

        void flood_fill_widget(Rect &dimensions);

        void accept_drag_and_drop();

    public:
        EditMask();

        ~EditMask();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        bool use_hu_range = false;
        bool prev_use_hu_range = use_hu_range;
        Shortcut highlight_hu_range_shortcut;
        Shortcut vertebra_min_distance_shortcut;
        Shortcut lasso_shortcut;
        Shortcut brush_shortcut;
        Shortcut next_shortcut;
        Shortcut previous_shortcut;
        Shortcut visceral_fat_help_shortcut;
        Shortcut other_mask_shortcut;
        Shortcut flood_fill_shortcut;
        int lasso_or_brush = 0;

        void editTmpMaskAreaFromClick(const core::segmentation::Mask &area_to_edit, bool left_click,
                                      bool right_click);

        bool ignore_small_holes_and_objects = false;
        bool prev_ignore_small_holes_and_objects = ignore_small_holes_and_objects;

        void automaticBrushBorders();

        void editTmpMaskArea(const core::segmentation::Mask &area_to_edit, bool add, bool remove,
                             bool only_if_limitation_used);

        float vertebra_min_distance = 1;
        float prev_vertebra_min_distance = vertebra_min_distance;


        bool use_vertebra_min_distance = false;
        bool prev_use_vertebra_min_distance = use_vertebra_min_distance;
        float vertebra_min_HU = 150;
        float prev_vertebra_min_HU = vertebra_min_HU;

        void updateHuThresholdMask();
        void updateVertebraDistanceMask();
        void updateEditionLimitMask();
        void updateVisceralFatSegmentationsBox();
        void updateVisceralFatMask();
        void updateOtherMaskBox();
        void updateOtherSegmentationMask();

        bool disable_edit = false;
        int opening_size = 0;
        int prev_opening_size = opening_size;
        bool use_visceral_fat_help = false;
        bool prev_use_visceral_fat_help = use_visceral_fat_help;

        Widgets::Selectable muscle_segmentation_selection_box;
        std::shared_ptr<core::segmentation::Segmentation> muscle_segmentation_for_visceral_fat_help;
        std::shared_ptr<core::segmentation::Segmentation> prev_muscle_segmentation_for_visceral_fat_help;

        std::set<int> visceral_fat_help_debug_rows;
        std::set<int> visceral_fat_help_debug_cols;
        bool draw_visceral_fat_debug_lines = false;
        bool prev_draw_visceral_fat_debug_lines = draw_visceral_fat_debug_lines;
        bool use_other_mask_filter = false;
        bool prev_use_other_mask_filter = use_other_mask_filter;

        Widgets::Selectable other_mask_selection_box;
        std::shared_ptr<core::segmentation::Segmentation> segmentation_for_other_mask;
        std::shared_ptr<core::segmentation::Segmentation> prev_segmentation_for_other_mask;
    };
}