#pragma once

#include <string>

#include "core/image.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"

namespace Rendering {

    class ImageViewer : public AbstractLayout {
    private:
        constexpr static const float zoom_speed = 1.3f;
        static int instance_number;

        const char* identifier_;

        SimpleImage image_widget_;

    public:
        ImageViewer() {
            instance_number++;
            identifier_ = (std::to_string(instance_number) + std::string("ImageViewer")).c_str();
            image_widget_.setInteractiveZoom(SimpleImage::IMAGE_MODIFIER_INTERACT);
            image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
        }

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}