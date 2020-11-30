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

        bool reset_image_ = false;

        int prev_ww_= 400;
        int prev_wc_ = 40;

        ImVec2 size_ = ImVec2(100, 100);

        std::shared_ptr<::core::dataset::SeriesNode> series_node_ = nullptr;
        ::core::dataset::Case case_;
        bool is_subscribed_ = false;

        int max_im_size_ = 512;
        ImVec2 crop_x_ = ImVec2(0, 100);
        ImVec2 crop_y_ = ImVec2(0, 100);
        ImVec2 prev_crop_x_ = ImVec2(0, 100);
        ImVec2 prev_crop_y_ = ImVec2(0, 100);

        bool is_disabled_ = false;
        bool is_crop_locked = false; // When true, setCrop won't affect excepted when forced
        bool is_window_locked = false;

        bool allow_scroll_ = false;
        int case_idx = 0;

        void set_case(int idx);
        void set_crop(ImVec2 crop_x, ImVec2 crop_y, bool lock = false);
        void set_window(int width, int center, bool lock = false);

    public:
        DicomPreview();

        DicomPreview(const DicomPreview& other);

        DicomPreview(const DicomPreview&& other);

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        void setSize(const ImVec2& size) { size_ = size; }

        void setWindowing(int width, int center, bool force = false);

        /**
         * Sets the case number (in percentage)
         * @param percentage
         */
        void setCase(float percentage);

        void setLock(bool lock) { is_crop_locked = lock; }

        void setCrop(ImVec2 crop_x, ImVec2 crop_y, bool force = false);

        void setIsDisabled(bool disabled);

        std::string& getIdentifier() { return identifier_; }

        void setAllowScroll(bool allow_scroll) { allow_scroll_ = allow_scroll; }

        void loadSeries(std::shared_ptr<::core::dataset::SeriesNode> series_node, const ::core::dataset::Case& case_);

        bool isLocked() const { return is_crop_locked || is_window_locked; }

        ::core::DicomSeries& getDicomSeries() { return series_node_->data; }

    };
}