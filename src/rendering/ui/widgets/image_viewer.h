#pragma once

#include "core/image.h"
#include "rendering/drawables.h"

namespace Rendering {

    /**
     * Defines the UI for opening and exploring new folders
     */
    class ImageViewer : public AbstractLayout {
    private:
        core::Image image_;

    public:
        ImageViewer() = default;

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        void setImage(core::Image& image) { image_ = image; }

    };
}