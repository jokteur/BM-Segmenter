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
    /**
     * Little widget class for visualizing DICOM images
     */
    class DicomViewer : public AbstractLayout {
    private:
        static int instance_number;
        int my_instance_;

        std::string identifier_;
        bool is_open_ = true;

        SimpleImage image_widget_;
        ImageButton windowing_button_;
        ::core::dataset::Case case_;
        ::core::Image image_;
        Listener listener_;
        Listener job_listener_;
        Listener log_listener_;

        cv::Mat dicom_matrix_;
        bool reset_image_ = false;
        bool active_dragging_ = false;
        ImVec2 drag_delta_;
        int window_width_ = 400;
        int window_center_ = 40;

        std::string error_message_;

        std::vector<::core::dataset::Case> series_;
        int case_select_ = 1;
        int previous_select_ = 0;

        void loadSeries(const ::core::dataset::SeriesPayload& data);
        void selectCase(::core::dataset::Case& aCase);
        void dicom_to_image();

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