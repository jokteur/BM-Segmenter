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

