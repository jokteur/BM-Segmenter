#include "image_simple.h"
#include "imgui.h"

#include <iostream>
#include <cmath>

int Rendering::SimpleImage::instance_number = 0;

void Rendering::SimpleImage::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    ImGui::BeginChild(identifier_, size_);

    // Calculate the available pixels in viewport for drawing the image
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();

    // Set the dimensions of the widget
    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x;
    dimensions_.height = content.y;

    if ((content.x != content_size_.x || content.y != content_size_.x) && !fixed_size_) {
        redraw_image_ = true;
        content_size_ = content;
    }
    if (redraw_image_) {
        float rescale_factor_x = 1.f;
        float rescale_factor_y = 1.f;
        if ((float)image_.width() > content.x) {
            rescale_factor_x = content.x / (float) image_.width();
        }
        if ((float)image_.height() > content.y) {
            rescale_factor_y = content.y / (float) image_.height();
        }
        rescale_factor_ = (rescale_factor_x > rescale_factor_y) ? rescale_factor_y : rescale_factor_x;
        scaled_sizes_.x = (float)image_.width()*rescale_factor_ * 0.99f;
        scaled_sizes_.y = (float)image_.height()*rescale_factor_ * 0.99f;

        redraw_image_ = false;
    }


    if (image_.isImageSet()) {
        ImVec2 mouse_pos =  ImGui::GetMousePos();
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 rel_mouse_pos = ImVec2((mouse_pos.x - window_pos.x) / scaled_sizes_.x,
                                    (mouse_pos.y - window_pos.y) / scaled_sizes_.y);

        ImVec2 corrected_rel = ImVec2(crop_.x0 + (crop_.x1 - crop_.x0) * rel_mouse_pos.x,
                                      crop_.y0 + (crop_.y1 - crop_.y0) * rel_mouse_pos.y);

        bool do_zoom = false;
        if (rel_mouse_pos.x > 0.f && rel_mouse_pos.x < 1.f
            && rel_mouse_pos.y > 0.f && rel_mouse_pos.y < 1.f
            && interactive_zoom_ != IMAGE_NO_INTERACT) {

            float mouse_wheel = io.MouseWheel;
            if (mouse_wheel != 0.f) {
                float zoom_speed = 1.f + (zoom_speed_ - 1.f) * abs(mouse_wheel);
                if (io.KeyShift) {
                    zoom_speed = 1.f + (zoom_speed - 1.f)/2.5f;
                }
                if (mouse_wheel > 0.f) {
                    if (zoom_ < max_zoom_) {
                        zoom_ *= zoom_speed;
                        do_zoom = true;
                    }
                } else {
                    if (zoom_ > 1.f) {
                        zoom_ /= zoom_speed;
                        do_zoom = true;
                    }
                }
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
            crop_.x0 = -corrected_rel.x / zoom_ + corrected_rel.x;
            crop_.x1 = (1 - corrected_rel.x) / zoom_ + corrected_rel.x;
            crop_.y0 = -corrected_rel.y / zoom_ + corrected_rel.y;
            crop_.y1 = (1 - corrected_rel.y) / zoom_ + corrected_rel.y;
        }

        if (crop_.x0 < 0 || crop_.y0 < 0 || crop_.x1 > 1 || crop_.y1 > 1) {
            crop_.x0 = 0;
            crop_.y0 = 0;
            crop_.x1 = 1;
            crop_.y1 = 1;
        }

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

        ImGui::Image(
                image_.texture(),
                scaled_sizes_,
                ImVec2(crop_.x0,crop_.y0),
                ImVec2(crop_.x1,crop_.y1)
        );
    }
    ImGui::EndChild();
}