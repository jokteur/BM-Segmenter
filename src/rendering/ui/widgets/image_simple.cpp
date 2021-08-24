#include "image_simple.h"
#include "imgui.h"

#include <iostream>
#include <cmath>

int Rendering::SimpleImage::instance_number = 0;

void Rendering::SimpleImage::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    // Remove all padding for the child window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    int num_pop = 3;

    // Calculate the available pixels in viewport for drawing the image
    // The image will take a certain amount of this available content.
    // The precise x and y amount that will be taken by the image depends
    // on the aspect ratio of the image

    // If a certain size of the image is specified, then the image
    // will be fitted as best as it can in the specified amount
    ImVec2 content = ImGui::GetContentRegionAvail();

    float rescale_factor_x;
    float rescale_factor_y;

    auto width = (float)image_.width();
    auto height = (float)image_.height();
    if (size_.x == 0.f && size_.y == 0.f) {
        rescale_factor_x = content.x / (float) image_.width();
        rescale_factor_y = content.y / (float) image_.height();
    }
    else {
        rescale_factor_x = size_.x / (float) image_.width();
        rescale_factor_y = size_.y / (float) image_.height();
    }

    rescale_factor_ = (rescale_factor_x > rescale_factor_y) ? rescale_factor_y : rescale_factor_x;
    scaled_sizes_.x = width*rescale_factor_;
    scaled_sizes_.y = height*rescale_factor_;

    // Set the dimensions of the widget
    // If no size is specified, then the size of the widget
    // will be the size that the image will take
    ImVec2 child_size;
    float available_width;
    float available_height;
    if (size_.x == 0.f && size_.y == 0.f) {
        dimensions_.width = scaled_sizes_.x;
        dimensions_.height = scaled_sizes_.y;
        child_size = scaled_sizes_;
        available_width = content.x;
        available_height = content.y;
    }
    else {
        child_size = size_;
        dimensions_.width = size_.x;
        dimensions_.height = size_.y;
        available_width = size_.x;
        available_height = size_.y;
    }

    // Center the image if there is content left
    if (center_x_ && scaled_sizes_.x < available_width) {
        float x_difference = 0.5f*(available_width - scaled_sizes_.x);

        ImGui::Dummy(ImVec2(x_difference, scaled_sizes_.y));
        ImGui::SameLine();
    }
    if (center_y_ && scaled_sizes_.y < available_height) {
        float y_difference = int(0.5f*(available_height - scaled_sizes_.y));
        if (y_difference > 0 && y_difference + scaled_sizes_.x < available_height) {
            ImGui::Dummy(ImVec2(scaled_sizes_.x, y_difference));
        }
    }

    ImGui::BeginChild(window_id_.c_str(), child_size, border_, flags_);

    ImVec2 window_pos = ImGui::GetWindowPos();
    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;

    if (image_.isImageSet()) {
        ImVec2 mouse_pos =  ImGui::GetMousePos();
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 rel_mouse_pos = ImVec2((mouse_pos.x - dimensions_.xpos) / scaled_sizes_.x,
                                    (mouse_pos.y - dimensions_.ypos) / scaled_sizes_.y);


        ImVec2 corrected_rel = ImVec2(crop_.x0 + (crop_.x1 - crop_.x0) * rel_mouse_pos.x,
                                      crop_.y0 + (crop_.y1 - crop_.y0) * rel_mouse_pos.y);

        bool do_zoom = false;
        float zoom = 0.9;
        if (rel_mouse_pos.x > 0.f && rel_mouse_pos.x < 1.f
            && rel_mouse_pos.y > 0.f && rel_mouse_pos.y < 1.f
            && interactive_zoom_ != IMAGE_NO_INTERACT) {

            float mouse_wheel = io.MouseWheel;
            if (mouse_wheel != 0.f) {
                if (io.KeyShift) {
                    zoom = 0.95;
                }
                if (mouse_wheel < 0.f) {
                    zoom = 1.f / zoom;
                }
                do_zoom = true;
            }
            if (ImGui::IsMouseClicked(0) && (image_drag_ == IMAGE_NORMAL_INTERACT || (image_drag_ == IMAGE_MODIFIER_INTERACT && io.KeyCtrl))) {
                is_moving_ = true;
                initial_drag_ = corrected_rel;
            }
        }
        if (ImGui::IsMouseReleased(0)) {
            is_moving_ = false;
        }
        if (do_zoom && (interactive_zoom_ == IMAGE_NORMAL_INTERACT || (interactive_zoom_ == IMAGE_MODIFIER_INTERACT && io.KeyCtrl))) {
            float x0 = crop_.x0 - (corrected_rel.x - crop_.x0) * (zoom - 1);
            float x1 = x0 + (crop_.x1 - crop_.x0) * zoom;
            float y0 = crop_.y0 - (corrected_rel.y - crop_.y0) * (zoom - 1);
            float y1 = y0 + (crop_.y1 - crop_.y0) * zoom;

            crop_ = { x0, x1, y0, y1 };

            if (crop_.x0 < 0.f && crop_.x1 > 1.f) {
                crop_.x0 = 0.f;
                crop_.x1 = 1.f;
            }
            else {
                if (crop_.x0 < 0.f) {
                    crop_.x1 -= crop_.x0;
                    crop_.x0 = 0.f;
                }
                if (crop_.x1 > 1.f) {
                    crop_.x0 -= (crop_.x1 - 1.f);
                    crop_.x1 = 1.f;
                }
            }
            if (crop_.y0 < 0.f && crop_.y1 > 1.f) {
                crop_.y0 = 0.f;
                crop_.y1 = 1.f;
            }
            else {
                if (crop_.y0 < 0.f) {
                    crop_.y1 -= crop_.y0;
                    crop_.y0 = 0.f;
                }
                if (crop_.y1 > 1.f) {
                    crop_.y0 -= (crop_.y1 - 1.f);
                    crop_.y1 = 1.f;
                }
            }
        }

        Crop tmp_crop;

        if (is_moving_ && (current_drag_.x != corrected_rel.x || current_drag_.y != corrected_rel.y)) {
            current_drag_ = corrected_rel;
            Crop tmp_crop = {
                    crop_.x0 - (current_drag_.x - initial_drag_.x),
                    crop_.x1 - (current_drag_.x - initial_drag_.x),
                    crop_.y0 - (current_drag_.y - initial_drag_.y),
                    crop_.y1 - (current_drag_.y - initial_drag_.y),
            };
            if (tmp_crop.x0 > 0 && tmp_crop.x1 < 1) {
                crop_.x0 = tmp_crop.x0;
                crop_.x1 = tmp_crop.x1;
            }
            if (tmp_crop.y0 > 0 && tmp_crop.y1 < 1) {
                crop_.y0 = tmp_crop.y0;
                crop_.y1 = tmp_crop.y1;
            }
        }


        ImGui::PushID(identifier_.c_str());
        ImGui::Image(
                image_.texture(),
                ImVec2(scaled_sizes_.x, scaled_sizes_.y),
                ImVec2(crop_.x0,crop_.y0),
                ImVec2(crop_.x1,crop_.y1)
        );
        if (ImGui::IsItemHovered() && tooltip_[0] != '\0') {
            ImGui::SetTooltip("%s", tooltip_);
        }
        drag_source_fct_();
        draw_fct_(scaled_sizes_, crop_);
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(num_pop);
}