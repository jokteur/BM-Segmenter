#pragma once

#include <string>
#include <iostream>
#include <functional>
#include <utility>

#include "imgui.h"
#include "core/image.h"
#include "rendering/drawables.h"

namespace Rendering {

    struct Crop {
        float x0 = 0.f;
        float x1 = 1.f;
        float y0 = 0.f;
        float y1 = 1.f;
    };

    /**
     * Simple widget to show images in the user interface
     */
    class SimpleImage : public AbstractLayout {
    public:
        /**
         * IMAGE_NO_INTERACT: no interaction can happen
         * IMAGE_MODIFIER_INTERACT: interaction can only happen through with a modifier (Ctrl)
         * IMAGE_NORMAL_INTERACT: normal interaction (no modifier needed)
         */
        enum ImageInteraction {IMAGE_NO_INTERACT, IMAGE_MODIFIER_INTERACT, IMAGE_NORMAL_INTERACT};

    private:
        core::Image image_;
        float zoom_ = 1.f;
        float zoom_speed_;

        const float max_zoom_ = 20.f;

        static int instance_number;
        std::string window_id_;

        Crop crop_;
        ImVec2 size_;

        const char* tooltip_;

        bool border_ = false;
        bool center_x_ = false;
        bool center_y_ = false;
        ImGuiWindowFlags flags_ = ImGuiWindowFlags_NoScrollbar;


        // Image interaction variables
        ImageInteraction interactive_zoom_;
        ImageInteraction image_drag_;
        bool is_moving_ = false;
        ImVec2 initial_drag_;
        ImVec2 current_drag_;

        bool scale_to_viewport_ = false;
        float rescale_factor_ = 1.f;
        ImVec2 scaled_sizes_; // True size of the image (in px) taken in the viewport

        std::string identifier_;

        std::function<void ()> drag_source_fct_ = [] {};
        std::function<void (ImVec2, Crop)> draw_fct_ = [] (ImVec2, Crop) {};

    public:

        /**
         * Instantiate the Simple Image widget
         * @param interactive_zoom if set to true, the user can zoom in / and out in the image with
         * the scroll wheel
         * @param size set the size (in viewport) of the widget
         */
        explicit SimpleImage(ImVec2 size = ImVec2(0,0),
                             const char* tooltip = "",
                             ImageInteraction interactive_zoom = IMAGE_NO_INTERACT,
                             ImageInteraction image_drag = IMAGE_NO_INTERACT,
                             float max_zoom = 20.f,
                             float zoom_speed = 1.15f)
                             :
                             max_zoom_(max_zoom),
                             zoom_speed_(zoom_speed),
                             image_drag_(image_drag),
                             interactive_zoom_(interactive_zoom),
                             tooltip_(tooltip),
                             size_(size) {
            instance_number++;
            identifier_ = std::to_string(instance_number) + std::string("ImageSimple");
            window_id_ = std::string("Image") + identifier_;

        }

        SimpleImage(const SimpleImage& other) = default;

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        /**
         * @return returns the crop currently set in the image
         * The values in x and y are between 0, 1, indicating the
         * start and end of the crop
         */
        Crop& getCrop() { return crop_; }

        const std::string& getWindowID() { return window_id_; }
        const std::string& getImageID() { return identifier_; }

        /**
         * Set the size of the image viewport
         * @param size size of the widget
         */
        void setSize(ImVec2 size) {
            size_ = size;
        }

        /**
         * Sets the auto scale of the image.
         * The image will always be scaled to the viewport with the correct aspect ratio
         * @param autoscale autoscale yes or no
         */
        void setAutoScale(bool autoscale) {
            scale_to_viewport_ = autoscale;
        }

        /**
         * Sets the interactive zoom parameters
         * @param interactive_zoom if set to true, the user can zoom in / and out in the image with
         * the scroll wheel
         */
        void setInteractiveZoom(ImageInteraction interactive_zoom) { interactive_zoom_ = interactive_zoom;}

        /**
         * Sets the image drag parameter
         * @param image_drag if set to true, the user can
         */
        void setImageDrag(ImageInteraction image_drag) { image_drag_ = image_drag; }

        /**
         * Set manually the crop of the image
         * Will deactivate the interactive zoom feature in the image
         * @param crop uv coordinates (between 0 and 1)
         */
        void setCrop(Crop& crop) {
            crop_ = crop;
            interactive_zoom_ = IMAGE_NO_INTERACT;
            image_drag_ = IMAGE_NO_INTERACT;
        }

        /**
         * Set if the image will be centered in the x direction
         * @param center_x
         */
        void setCenterX(bool center_x) { center_x_ = center_x; }

        /**
         * Set if the image will be centered in the y direction
         * @param center_x
         */
        void setCenterY(bool center_y) { center_y_ = center_y; }

        /**
         * Sets the ImGui window flags for the child
         * @param flags
         */
        void setWindowFlags(ImGuiWindowFlags flags) { flags_ |= flags; }

        /**
         * Set the border of the viewport
         * @param border border (yes or no)
         */
        void setBorder(bool border) { border_ = border;}

        /**
         * Sets the image in the viewport
         * @param image image object that contains the OpenGL data for the image
         */
        void setImage(core::Image& image) {
            image_ = image;
        }

        void setDragSourceFunction(std::function<void ()> function) { drag_source_fct_ = std::move(function); }

        void setDrawFunction(std::function<void (ImVec2, Crop)> function) { draw_fct_ = std::move(function); }

    };
}