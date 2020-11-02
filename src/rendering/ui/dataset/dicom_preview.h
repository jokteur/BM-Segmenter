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

namespace Rendering {
    /**
     * Little widget class for previewing dicom images
     */
    class DicomPreview : public AbstractLayout {
    private:
        static int instance_number;

        std::string identifier_;

        SimpleImage image_widget_;
        ::core::Image image_;
        Listener job_listener_;

        cv::Mat dicom_matrix_;
        bool reset_image_ = false;
        int window_width_ = 400;
        int window_center_ = 40;

        ImVec2 size_ = ImVec2(100, 100);

        std::string error_message_;

        std::vector<std::string> series_;
        ::core::dataset::SeriesPayload series_payload_;
        bool is_subscribed_ = false;

        int max_im_size_ = 512;
        float crop_ = 0.f;

        bool allow_scroll_ = false;
        int case_idx = 0;

        void selectCase(const std::string& path);
        void dicom_to_image();

    public:
        DicomPreview();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        void setSize(const ImVec2& size) { size_ = size; }

        void setWindowing(int width, int center) { window_width_ = width; window_center_ = center; reset_image_ = true; }

        /**
         * Sets the case number (in percentage)
         * @param percentage
         */
        void setCase(float percentage);

        void setCrop(float crop);

        std::string& getIdentifier() { return identifier_; }

        void setAllowScroll(bool allow_scroll) { allow_scroll_ = allow_scroll; }

        void loadSeries(const ::core::dataset::SeriesPayload& series);

    };
}