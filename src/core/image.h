#pragma once

#include <GL/gl3w.h>
#include <opencv2/core/mat.hpp>

#include "core/segmentation/mask.h"

namespace core {

    /**
     * Image class for holding images in memory to be drawn to Dear ImGui
     */
    class Image {
    public:
        enum Filtering {FILTER_NEAREST, FILTER_BILINEAR};
    private:
        GLuint texture_ = -1;
        int width_ = 0;
        int height_ = 0;
        int samples_ = 4;

        bool success_ = false;

        void load_texture(unsigned char* data, int width, int height, Filtering filtering);
        void load_texture_from_file(const char *filename, Filtering filtering);
        void load_texture_from_memory(unsigned char* data, int width, int height, Filtering filtering);
    public:
        Image() = default;
        ~Image();

        /**
         * Set image from disk
         * @param filename path of the image (jpg, png)
         * @return if successful or not
         */
        bool setImage(const char* filename, Filtering filtering = FILTER_NEAREST);


        /**
         * Set image from memory
         * @param data RGB array
         * @return if successful or not
         */
        bool setImage(unsigned char* data, int width, int height, Filtering filtering = FILTER_NEAREST);

        /**
         * Sets image from a DICOM image in Houndsfield units
         * @param data data array
         * @param width width of image
         * @param height height of image
         * @param window_width VOI LUT window width (see https://dicom.innolitics.com/ciods/ct-image/voi-lut/00281050)
         * @param window_center VOI LUT window center (see https://dicom.innolitics.com/ciods/ct-image/voi-lut/00281050)
         * @param filtering FILTERING
         * @param mask draw a mask on top of the image
         * @param mask_color mask_color of the mask if it is defined
         * @return
         */
        bool setImageFromHU(const cv::Mat& image, float window_width, float window_center, Filtering filtering = FILTER_NEAREST, const cv::Mat& mask = cv::Mat(), ImVec4 mask_color = ImVec4(0, 0, 0, 0), bool show_mask = true,
                            bool compare_with_other_mask=false, const cv::Mat& other_mask = cv::Mat());

        /**
         * Erases any content in the image
         * After this function, isImageSet will return false
         */
        void reset();

        /**
         * @return if the image has been successfully loaded in mage
         */
        bool isImageSet() const { return success_; }

        /**
         * @return width of image as stored in memory
         */
        int width() const { return width_; }
        /**
         * @return height of image as stored in memory
         */
        int height() const { return height_; }
        /**
         * @return GL texture of image
         */
        void* texture() const { return (void*)(intptr_t)texture_; }

    };
}