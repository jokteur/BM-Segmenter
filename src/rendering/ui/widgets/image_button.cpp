#include <iostream>
#include "image_button.h"
#include "settings.h"
#include "rendering/ui/widgets/util.h"

int Rendering::ImageButton::instance_number = 0;

bool Rendering::ImageButton::isMouseReleased() {
    if (mouse_released_) {
        mouse_released_ = false;
        return true;
    }
    return false;
}

void Rendering::ImageButton::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    is_just_pressed_ = false;
    // Set the styling for the button
    auto &settings = Settings::getInstance();
    auto theme = settings.getCurrentTheme();
    auto size = settings.getAdjustedScale();

    int style_pop_count = 0;
    int style_var_pop_count = 0;
    if (size != ui_size_) {
        ui_size_ = size;
        image_widget_.setSize(ImVec2(default_size_.x * size / 2.f, default_size_.y * size / 2.f));
        image_widget_.setAutoScale(false);
    }
    if (theme == Settings::SETTINGS_LIGHT_THEME) {
        if (is_hovering_) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.hovered_color_light);
            style_pop_count++;
        }
        else if (is_pressed_) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.pressed_color_light);
            style_pop_count++;
        }
        else if (style_.background_light) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.background_color_light);
            style_pop_count++;
        }
        if (style_.border_light) {
            ImGui::PushStyleColor(ImGuiCol_Border, style_.border_color_light);
            style_pop_count++;
        }
        if (last_theme_ != theme) {
            last_theme_ = theme;
            image_widget_.setImage(image_light_);
        }
    }
    else {
        if (is_hovering_) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.hovered_color_dark);
            style_pop_count++;
        }
        else if (is_pressed_) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.pressed_color_dark);
            style_pop_count++;
        }
        else if (style_.background_light) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style_.background_color_dark);
            style_pop_count++;
        }
        if (style_.border_light) {
            ImGui::PushStyleColor(ImGuiCol_Border, style_.border_color_dark);
            style_pop_count++;
        }
        if (last_theme_ != theme) {
            last_theme_ = theme;
            image_widget_.setImage(image_dark_);
        }
    }
    if (style_.border_light) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5,5));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6);
        image_widget_.setBorder(true);
        style_var_pop_count = 2;
    }
    image_widget_.ImGuiDraw(window, parent_dimension);
    if (style_pop_count)
        ImGui::PopStyleColor(style_pop_count);
    if (style_var_pop_count)
        ImGui::PopStyleVar(style_var_pop_count);

    // Interaction logic
    if (!is_toggle_)
        is_pressed_ = false;

    dimensions_ = image_widget_.getDimensions();
    ImVec2 mouse_pos = ImGui::GetMousePos();

    if (Widgets::check_hitbox(mouse_pos, dimensions_)) {
        is_hovering_ = true;
        if (ImGui::IsMouseClicked(0)) {
            mouse_released_ = false;
            if (is_toggle_)
                is_pressed_ = !is_pressed_;
            else
                is_pressed_ = true;
            click_duration_ = 0.f;
            is_just_pressed_ = true;
        }
        if (ImGui::IsMouseDown(0)) {
            mouse_released_ = false;
            ImGuiIO& io = ImGui::GetIO();
            click_duration_ = io.MouseDownDuration[0];
        }
    }
    else {
        is_hovering_ = false;
    }

    if (ImGui::IsMouseReleased(0)) {
        if (Widgets::check_hitbox(mouse_pos, dimensions_)) {
            mouse_released_ = true;
        }

        if (!is_toggle_)
            is_pressed_ = false;
//        click_duration_ = 0.f;
    }

}
