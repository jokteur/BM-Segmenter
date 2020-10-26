#pragma once

#include <GL/gl3w.h>

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

        bool success_ = false;

        void load_texture(unsigned char* data, int width, int height, Filtering filtering);
        void load_texture_from_file(const char *filename, Filtering filtering);
        void load_texture_from_memory(unsigned char* data, int width, int height, Filtering filtering);
        void reset();
    public:
        Image() = default;

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