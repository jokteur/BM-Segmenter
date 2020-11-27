#pragma once

#include <string>
#include <vector>

#include "core/image.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"
#include "settings.h"

namespace Rendering {
    struct ImageButtonStyle {
        ImVec4 hovered_color_light = ImVec4(0.7, 0.7, 0.7, 0.5);
        ImVec4 pressed_color_light = ImVec4(0.5, 0.5, 0.5, 0.5);
        ImVec4 background_color_light = ImVec4(0.9, 0.9, 0.9, 0.5);
        bool background_light = false;
        ImVec4 border_color_light = ImVec4(0.2, 0.2, 0.2, 1);
        bool border_light = true;

        ImVec4 hovered_color_dark = ImVec4(0.5, 0.5, 0.5, 0.5);
        ImVec4 pressed_color_dark = ImVec4(0.7, 0.7, 0.7, 0.5);
        ImVec4 background_color_dark = ImVec4(0.2, 0.2, 0.2, 0.5);
        bool background_dark = false;
        ImVec4 border_color_dark = ImVec4(0.6, 0.6, 0.6, 1);
        bool border_dark = true;
    };

    class ImageButton : public AbstractLayout {
    private:
        static int instance_number;

        std::string identifier_;
        ::core::Image image_light_;
        ::core::Image image_dark_;

        Settings::Theme last_theme_;
        ImVec2 default_size_;
        float ui_size_ = 0;
        ImageButtonStyle style_;

        bool is_toggle_;
        bool is_pressed_ = false;
        bool is_just_pressed_ = false;
        bool is_hovering_ = false;
        bool mouse_released_ = false;
        const char* tooltip_;

        float click_duration_ = 0.f;
        float default_duration = 1.f;

        SimpleImage image_widget_;

    public:
        /**
         * Specify the image of the button
         * @param filename
         */
        explicit ImageButton(const char* light_filename,
                             const char* dark_filename,
                             bool is_toggle = false,
                             const char* tooltip = "",
                             ImVec2 size = ImVec2(64, 64))
                             : is_toggle_(is_toggle), tooltip_(tooltip), default_size_(size) {
            instance_number++;
            identifier_ = std::to_string(instance_number) + std::string("ImageButton");
            image_light_.setImage(light_filename, core::Image::FILTER_BILINEAR);
            image_dark_.setImage(dark_filename, core::Image::FILTER_BILINEAR);
            image_widget_.setImage(image_light_);
            image_widget_.setSize(size);
        }

        /**
         * Sets the state of the button (pressed or not)
         * @param state
         */
        void setState(bool state) { is_pressed_ = state; }

        /**
         * Sets the tooltip of the button
         * @param tooltip tooltip description
         */
        void setTooltip(const char* tooltip) { tooltip_ = tooltip; }

        /**
         * Sets the style of the button
         * @param style style struct that defines backgrounds
         */
        void setStyle(ImageButtonStyle& style) { style_ = style; }

        /**
         * Returns if the button is down or up
         * @return
         */
        bool isActive() const { return is_pressed_; }

        /**
         * Returns if the button has just been pressed
         * @return
         */
        bool isPressed() const { return is_just_pressed_; }

        /**
         * Returns if the button has been pressed for a certain amount of time
         * @param duration duratino in second
         * @return
         */
        bool isLongPressed(float duration = 1.f) const { return click_duration_ > duration; }

        bool isMouseReleased();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}