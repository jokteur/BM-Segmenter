#pragma once

#include <string>
#include <vector>


#include "python/py_api.h"
#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dicom.h"
#include "core/dataset/explore.h"
#include "core/segmentation/segmentation.h"

#include "events.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"

namespace Rendering {
    /**
     * Little widget class for previewing dicom images
     */
    class Preview : public AbstractLayout {
    private:
        static int instance_number;
        bool is_valid_ = false;
        bool is_loaded_ = false;

        std::string identifier_;

        SimpleImage image_widget_;
        ::core::Image image_;
        std::shared_ptr<::core::DicomSeries> dicom_ = nullptr;
        std::shared_ptr<::core::segmentation::MaskCollection> mask_collection_ = nullptr;

        bool reset_image_ = false;

        static int load_counter;

        int prev_ww_ = 400;
        int prev_wc_ = 40;

        ImVec2 size_ = ImVec2(100, 100);

        bool is_subscribed_ = false;

        int max_im_size_ = 512;
        ImVec2 crop_x_ = ImVec2(0, 100);
        ImVec2 crop_y_ = ImVec2(0, 100);
        ImVec2 prev_crop_x_ = ImVec2(0, 100);
        ImVec2 prev_crop_y_ = ImVec2(0, 100);

        std::shared_ptr<::core::segmentation::Segmentation> active_seg_ = nullptr;

        Listener job_listener_;
        jobId waiting_on_;

        bool is_crop_locked = false; // When true, setCrop won't affect excepted when forced
        bool is_window_locked = false;

        bool allow_scroll_ = false;
        int case_idx = 0;
        
        double __hack = 235.654885342;

        void set_case(int idx);
        void set_crop(ImVec2 crop_x, ImVec2 crop_y, bool lock = false);
        void set_window(int width, int center, bool lock = false);

        void init();

        void setAndLoadMask();

        void popup_context_menu();

    public:
        Preview();

        Preview(const Preview& other);

        Preview(const Preview&& other);

        ~Preview();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;

        void setSize(const ImVec2& size) { size_ = size; }

        /**
         * Unloads the image in the widget from the memory
        */
        void unload();

        void setSegmentation(std::shared_ptr<::core::segmentation::Segmentation> segmentation);

        /**
         * If the image has been unloaded before, call reload to show the widget again
        */
        void load();

        //void setWindowing(int width, int center, bool force = false);

        /**
         * Sets the case number (in percentage)
         * @param percentage
         */
        void setCase(float percentage);

        //void setLock(bool lock) { is_crop_locked = lock; }

        //void setCrop(ImVec2 crop_x, ImVec2 crop_y, bool force = false);

        std::string& getIdentifier() { return identifier_; }

        void setAllowScroll(bool allow_scroll) { allow_scroll_ = allow_scroll; }

        void setSeries(std::shared_ptr<::core::DicomSeries> dicom);

        //bool isLocked() const { return is_crop_locked || is_window_locked; }

        std::shared_ptr <::core::DicomSeries> getDicomSeries() { return dicom_; }

    };
}