#pragma once

#include <string>
#include <vector>

#include "python/py_api.h"
#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
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
        Listener job_listener_;

        std::vector<cv::Mat> dicom_matrix_;
        cv::Mat sagittal_matrix_;
        cv::Mat coronal_matrix_;

        bool load_finish_ = false;
        bool sagittal_ready_ = false;
        bool coronal_ready_ = false;

        bool reset_image_ = false;
        bool views_set_ = false;

        bool display_reference_lines_ = true;

        bool active_dragging_ = false;
        ImVec2 drag_delta_;

        int window_width_ = 400;
        int window_center_ = 40;
        ImVec2 crop_x_ = ImVec2(0, 100);
        ImVec2 crop_y_ = ImVec2(0, 100);

        float sagittal_x_ = 0.5f;
        float coronal_x_ = 0.5f;

        std::string error_message_;
        std::set<jobId> pending_jobs_;

        std::vector<std::string> series_;
        ::core::dataset::Case case_;
        int case_select_ = 1;
        int previous_select_ = 0;
        int image_size_ = 0;

        void loadSeries(const ::core::dataset::SeriesPayload& data);
        void loadCase(const std::string &path);

        void set_views();
        void set_image();

        void build_views();

        static Line calculate_line_coord(const Rect &dimensions, const Crop &crop, float position, bool horizontal);

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