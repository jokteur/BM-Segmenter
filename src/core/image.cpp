#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void core::Image::reset() {
    if (success_) {
        success_ = false;
        width_ = 0;
        height_ = 0;
        glDeleteTextures(1, &texture_);
    }
}
void core::Image::load_texture(unsigned char *data, int width, int height, Filtering filtering) {
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    int gl_filter = 0;
    if (filtering == FILTER_NEAREST) {
        gl_filter = GL_NEAREST;
    }
    else if (filtering == FILTER_BILINEAR) {
        gl_filter = GL_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    texture_ = image_texture;
}

void core::Image::load_texture_from_file(const char *filename, Filtering filtering) {
    reset();

    // Load from file
    unsigned char* image_data = stbi_load(filename, &width_, &height_, NULL, 4);
    if (image_data == nullptr)
        return;

    load_texture(image_data, width_, height_, filtering);

    stbi_image_free(image_data);
    success_ = true;
}

void core::Image::load_texture_from_memory(unsigned char *data, int width, int height, Filtering filtering) {
    reset();

    width_ = width;
    height_ = height;

    load_texture(data, width_, height_, filtering);
    success_ = true;

}

bool core::Image::setImage(const char *filename, Filtering filtering) {
    load_texture_from_file(filename, filtering);
    return success_;
}

bool core::Image::setImage(unsigned char *data, int width, int height, Filtering filtering) {
    load_texture_from_memory(data, width, height, filtering);
    return success_;
}

bool core::Image::setImageFromHU(const cv::Mat& image, float window_width, float window_center, Filtering filtering, const cv::Mat& mask, ImVec4 mask_color,
                                 bool show_mask, bool compare_with_other_mask, const cv::Mat& other_mask,
                                 const std::set<int> &debug_lines_x, const std::set<int> &debug_lines_y) {
    auto* new_image = new unsigned char[(int)image.rows * (int)image.cols * 4];
    int new_image_pixel_index = 0;

    bool draw_mask = mask.rows == image.rows && mask.cols == image.cols
                      && mask.rows > 0 && mask.cols > 0
                      && image.rows > 0 && image.cols > 0
                      && show_mask;

    bool draw_debug_lines = debug_lines_x.size() > 0 || debug_lines_y.size() > 0;

    for(int row_index = 0; row_index < image.rows; ++row_index) {
        auto image_pixel_pointer = image.ptr<short int>(row_index);

        for(int col_index = 0; col_index < image.cols; ++col_index) {
            auto image_pixel_value = (float)*image_pixel_pointer;

            bool pixel_is_on_debug_line =
                    draw_debug_lines &&
                    (debug_lines_x.find(col_index) != debug_lines_x.end()
                     || debug_lines_y.find(row_index) != debug_lines_y.end());

            if (pixel_is_on_debug_line) {
                new_image[4 * new_image_pixel_index] = 255;
                new_image[4 * new_image_pixel_index + 1] = 255;
                new_image[4 * new_image_pixel_index + 2] = 255;
                new_image[4 * new_image_pixel_index + 3] = 255;
            }
            else {

                // below values are between 0 and 1
                float overlay_color_r = 0;
                float overlay_color_g = 0;
                float overlay_color_b = 0;
                float overlay_transparency = 0;

                bool pixel_is_in_mask = false;
                bool pixel_is_in_range = false;

                if (draw_mask)
                    pixel_is_in_mask = mask.at<uchar>(row_index, col_index) != 0;
                if (compare_with_other_mask)
                    pixel_is_in_range = other_mask.at<uchar>(row_index, col_index) != 0;

                if (draw_mask && !compare_with_other_mask) {
                    if (pixel_is_in_mask) {
                        overlay_color_r = mask_color.x;
                        overlay_color_g = mask_color.y;
                        overlay_color_b = mask_color.z;
                        overlay_transparency = mask_color.w;
                    }
                } else if (!draw_mask && compare_with_other_mask) {
                    if (pixel_is_in_range) {
                        overlay_color_r = 1.f;
                        overlay_color_g = 0.f;
                        overlay_color_b = 0.f;
                        overlay_transparency = mask_color.w;
                    }
                } else if (draw_mask && compare_with_other_mask) {
                    if (pixel_is_in_range && !pixel_is_in_mask) {
                        overlay_color_r = 1.f;
                        overlay_color_g = 0.f;
                        overlay_color_b = 0.f;
                        overlay_transparency = mask_color.w;
                    }
                    if (pixel_is_in_range && pixel_is_in_mask) {
                        overlay_color_r = 0.f;
                        overlay_color_g = 1.f;
                        overlay_color_b = 0.f;
                        overlay_transparency = mask_color.w;
                    }
                    if (!pixel_is_in_range && pixel_is_in_mask) {
                        overlay_color_r = 0.f;
                        overlay_color_g = 0.f;
                        overlay_color_b = 1.f;
                        overlay_transparency = mask_color.w;
                    }
                }

                float image_pixel_display_value;
                if (image_pixel_value <= window_center - 0.5f - (window_width - 1.f) * 0.5f)
                    image_pixel_display_value = 0;
                else if (image_pixel_value > window_center - 0.5f + (window_width - 1.f) * 0.5f)
                    image_pixel_display_value = 1;
                else
                    image_pixel_display_value = ((image_pixel_value - (window_center - 0.5f)) / (window_width - 1.f) +
                                                 0.5f);

                new_image[4 * new_image_pixel_index] = (char) ((image_pixel_display_value * (1 - overlay_transparency) +
                                                                overlay_color_r * overlay_transparency) * 255);
                new_image[4 * new_image_pixel_index + 1] = (char) (
                        (image_pixel_display_value * (1 - overlay_transparency) +
                         overlay_color_g * overlay_transparency) * 255);
                new_image[4 * new_image_pixel_index + 2] = (char) (
                        (image_pixel_display_value * (1 - overlay_transparency) +
                         overlay_color_b * overlay_transparency) * 255);
                new_image[4 * new_image_pixel_index + 3] = 255;
            }
            new_image_pixel_index++;
            image_pixel_pointer++;
        }
    }

    load_texture_from_memory(new_image, image.cols, image.rows, filtering);
    delete [] new_image;
    return false;
}

core::Image::~Image() {
    reset();
}

