#pragma once

#include <string>
#include <vector>

#include "python/py_api.h"
#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "core/dicom.h"
#include "events.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"
#include "rendering/ui/widgets/image_button.h"
#include "rendering/ui/widgets/util.h"

namespace Rendering {
    struct Line {
        ImVec2 start;
        ImVec2 end;
    };
    /**
     * Little widget class for visualizing DICOM images
     */
    class DicomViewer : public AbstractLayout {
    private:
        static int instance_number;
        int my_instance_;

        std::string identifier_;
        bool is_open_ = true;

        ::core::Image image_;
        SimpleImage image_widget_;

        ::core::Image sagittal_image_;
        SimpleImage sagittal_widget_;

        ::core::Image coronal_image_;
        SimpleImage coronal_widget_;

        ImageButton windowing_button_;
        ImageButton point_select_button_;
        ImageButton* active_button_ = nullptr;

        Listener listener_;
        Listener reset_tree_listener_;

        Widgets::Selectable markers_select_;
        Widgets::Selectable markers_type = Widgets::Selectable({ "Axial", "Coronal", "Sagittal" }, [](int) {});
        bool rebuild_markers_select_ = false;
        std::string marker_name_input_;
        float marker_color_input_[3] = {0.9f, 0.9f, 0.9f};


        ::core::Dicom sagittal_matrix_;
        ::core::Dicom coronal_matrix_;

        int num_images_loaded_ = 0;
        bool is_load_finished_ = false;
        bool is_sagittal_ready_ = false;
        bool is_coronal_ready_ = false;

        bool is_context_menu_open = false;
        bool tmp_context_menu_open = false;

        bool reset_axial_image_ = false;
        bool views_set_ = false;

        bool display_reference_lines_ = true;

        bool active_dragging_ = false;
        bool tmp_deactivate_tool_ = false;
        ImVec2 drag_delta_;

        ::core::DicomCoordinate coordinate;
        float sagittal_x_ = 0.5f;
        float coronal_x_ = 0.5f;

//        std::string error_message_;

        std::vector<std::string> series_;
        std::shared_ptr<::core::dataset::SeriesNode> series_node_ = nullptr;
        ::core::dataset::Case case_;
        int case_select_ = 1;
        int previous_select_ = 0;
        int image_size_ = 0;

        int tmp_WW_ = 400;
        int tmp_WC_ = 40;

        void loadSeries(const ::core::dataset::SeriesPayload& data);
        void loadCase(int idx);

        int calculate_case_idx(float position);

        void set_side_views();
        void set_image();

        void build_views();

        // ImGui windows
        void header_window(GLFWwindow* window, Rect& parent_dimension);
        void axial_view_window(GLFWwindow* window, Rect& parent_dimension);
        void side_view_windows(GLFWwindow* window, Rect& parent_dimension);

        void button_logic();
        void windowing_widget_logic();
        void windowing_options();
        void point_select_widget_logic();
        void point_select_options();
        void display_reference_lines();

        static Line calculate_line_coord(const Rect &dimensions, const Crop &crop, float position, bool horizontal);

        bool begin_popup(const Rect& window_dimensions);
        void context_menu(const Rect& window_dimensions);

        void accept_drag_and_drop();

    public:
        DicomViewer();
        ~DicomViewer();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}