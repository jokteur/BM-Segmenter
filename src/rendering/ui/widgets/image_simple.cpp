#include "image_simple.h"
#include "imgui.h"

#include <iostream>
#include <cmath>

int Rendering::SimpleImage::instance_number = 0;

void Rendering::SimpleImage::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    ImGui::BeginChild((std::string("Image") + identifier_).c_str(), size_, border_, flags_);

    // Calculate the available pixels in viewport for drawing the image
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();

    auto style = ImGui::GetStyle();
    // Set the dimensions of the widget
    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x + 2*style.WindowPadding.x;
    dimensions_.height = content.y + 2*style.WindowPadding.y;

    if ((content.x != content_size_.x || content.y != content_size_.x) && !fixed_size_) {
        redraw_image_ = true;
        content_size_ = content;
    }
    if (redraw_image_) {
        float rescale_factor_x = 1.f;
        float rescale_factor_y = 1.f;

        auto width = (float)image_.width();
        auto height = (float)image_.height();
        rescale_factor_x = content.x / (float) image_.width();
        rescale_factor_y = content.y / (float) image_.height();

        rescale_factor_ = (rescale_factor_x > rescale_factor_y) ? rescale_factor_y : rescale_factor_x;
        scaled_sizes_.x = width*rescale_factor_;
        scaled_sizes_.y = height*rescale_factor_;

        redraw_image_ = false;
    }


    if (image_.isImageSet()) {
        ImGui::BeginGroup();
        // Center the image
        float x_difference = 0;
        if (scaled_sizes_.x < content.x) {
            x_difference = 0.5f*(content.x - scaled_sizes_.x);
            ImGui::Dummy(ImVec2(x_difference, scaled_sizes_.y));
            ImGui::SameLine();
        }
        ImVec2 mouse_pos =  ImGui::GetMousePos();
        ImGuiIO& io = ImGui::GetIO();

        ImVec2 rel_mouse_pos = ImVec2((mouse_pos.x - window_pos.x - x_difference) / scaled_sizes_.x,
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


        ImGui::PushID(identifier_.c_str());
        ImGui::Image(
                image_.texture(),
                ImVec2(scaled_sizes_.x*0.98f, scaled_sizes_.y*0.98f),
                ImVec2(crop_.x0,crop_.y0),
                ImVec2(crop_.x1,crop_.y1)
        );
        if (ImGui::IsItemHovered() && tooltip_[0] != '\0') {
            ImGui::SetTooltip("%s", tooltip_);
        }
        ImGui::PopID();
        ImGui::EndGroup();
    }
    ImGui::EndChild();
}