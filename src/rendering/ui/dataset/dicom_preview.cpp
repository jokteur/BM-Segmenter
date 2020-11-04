#include "dicom_preview.h"
#include "core/dataset/dicom_to_image.h"
#include "ui/widgets/util.h"
#include "drag_and_drop.h"
#include "log.h"

int Rendering::DicomPreview::instance_number = 0;
namespace dataset = ::core::dataset;


Rendering::DicomPreview::DicomPreview() {
    instance_number++;
    identifier_ = std::to_string(instance_number) + std::string("DicomPreview");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setCenterX(true);
    image_widget_.setCenterY(true);
}

void Rendering::DicomPreview::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    // Remove all padding for the child window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    int num_pop = 3;

    ImGui::BeginChild(identifier_.c_str(), size_, true);
    ImGui::PopStyleVar(num_pop);

    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos = ImGui::GetMousePos();

    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x;
    dimensions_.height = content.y;

    Rect image_dimensions = image_widget_.getDimensions();

    // Reload the image when it has been loaded
    // We do this because setting the image_ can lead to segfault when not done
    // in the main thread
    if (reset_image_) {
        dicom_to_image();
        reset_image_ = false;
        image_widget_.setDragSourceFunction([this] {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
                auto series_payload = ::core::dataset::SeriesPayload{*series_node_, case_, crop_x_, crop_y_, window_width_, window_center_};
                drag_and_drop.giveData(series_payload);

                int a = 0; // Dummy int
                ImGui::SetDragDropPayload("_DICOM_VIEW", &a, sizeof(a));
                ImGui::Text("Drag Series %s to viewer to visualize", series_node_->number.c_str());
                ImGui::PushID("Image_Drag_Drop");
                ImGui::Image(
                        image_.texture(),
                        ImVec2(128, 128)
                );
                ImGui::PopID();
                ImGui::EndDragDropSource();
            }
            if (is_disabled_) {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(
                        ImVec2(dimensions_.xpos, dimensions_.ypos),
                        ImVec2(dimensions_.xpos + dimensions_.width, dimensions_.ypos + dimensions_.height),
                        ImColor(255, 0, 0, 100));
            }
        }
        );
    }

    if (allow_scroll_ && Widgets::check_hitbox(mouse_pos, image_dimensions)) {
        float percentage = (mouse_pos.x - image_dimensions.xpos) / image_dimensions.width;
        if (percentage >= 0.f && percentage <= 1.f)
            setCase(percentage);
    }
    else {
        setCase(0.f);
    }
    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, dimensions_);
    }

    if (ImGui::BeginPopupContextItem(identifier_.c_str())) {
        if (!series_node_->is_active) {
            if (ImGui::Selectable("Include series")) {
                series_node_->is_active = true;
            }
        }
        else {
            if (ImGui::Selectable("Exclude series")) {
                series_node_->is_active = false;
            }
        }
        ImGui::SameLine();
        Widgets::HelpMarker("If the parent study or case ID is excluded,\n"
                            " the series will still be excluded.");

        if (series_node_->is_active) {
            if (ImGui::Selectable((std::string("Open file in DICOM viewer###") + identifier_).c_str())) {
                auto series_payload = ::core::dataset::SeriesPayload{*series_node_, case_, crop_x_, crop_y_, window_width_, window_center_};
                EventQueue::getInstance().post(Event_ptr(new ::core::dataset::SelectSeriesEvent(series_payload)));
            }
            ImGui::Separator();
            ImGui::Text("Cropping:");
            ImGui::DragFloatRange2("Crop in x", &crop_x_.x, &crop_x_.y, 1.f, 0.0f, 100.0f, "Left: %.1f %%", "Right: %.1f %%");
            ImGui::DragFloatRange2("Crop in y", &crop_y_.x, &crop_y_.y, 1.f, 0.0f, 100.0f, "Top: %.1f %%", "Bottom: %.1f %%");
            set_crop(crop_x_, crop_y_, true);
            ImGui::Separator();
            ImGui::Text("Windowing: ");
            ImGui::DragInt("Window center###dicom_preview_wc", &window_center_, 0.5, -1000, 3000, "%d HU");
            ImGui::DragInt("Window width###dicom_preview_ww", &window_width_, 0.5, 1, 3000, "%d HU");
            set_window(window_width_, window_center_, true);
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void Rendering::DicomPreview::selectCase(const std::string& path) {
    error_message_.clear();
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto dicom_result = std::dynamic_pointer_cast<::core::dataset::DicomResult>(result);
        if (dicom_result->success) {

            auto &dicom = dicom_result->data;
            // Crop image if needed

            cv::Rect ROI(0, 0, dicom.rows, dicom.cols);
            if (crop_x_.x < crop_x_.y && crop_y_.x < crop_y_.y
                && crop_x_.x >= 0.f && crop_x_.x <= 100.f
                && crop_x_.y >= 0.f && crop_x_.y <= 100.f) {
                ROI = {
                    (int)((float)dicom.rows*crop_x_.x/100.f),
                    (int)((float)dicom.cols*crop_y_.x/100.f),
                    (int)((float)dicom.rows*(crop_x_.y - crop_x_.x)/100.f),
                    (int)((float)dicom.rows*(crop_y_.y - crop_y_.x)/100.f)
                };
            }

            dicom = dicom(ROI);
            int rows = ROI.width;
            int cols = ROI.height;
            // Resize image to not fill the ram
            if (rows > max_im_size_ || cols > max_im_size_) {
                int width, height;
                if (rows > cols) {
                    width = max_im_size_;
                    height = (int)(max_im_size_ / (float)rows * (float)cols);
                }
                else {
                    width = (int)(max_im_size_ / (float)cols * (float)rows );
                    height = max_im_size_;
                }
                cv::resize(dicom, dicom_matrix_, cv::Size(width, height), 0, 0, cv::INTER_AREA);
            }
            else {
                dicom_matrix_ = dicom_result->data;
            }
            reset_image_ = true;
        }
        else {
            error_message_ = dicom_result->error_msg;
        }
    };
    dataset::dicom_to_matrix(path, fct);
}

void Rendering::DicomPreview::dicom_to_image() {
    image_.setImageFromHU(dicom_matrix_, (float)window_width_, (float)window_center_);
    image_widget_.setImage(image_);
}

void Rendering::DicomPreview::loadSeries(::core::dataset::SeriesNode* series_node, const ::core::dataset::Case& aCase) {
    series_.clear();
    case_ = aCase;
    series_node_ = series_node;
    for (auto &image : series_node->images) {
        series_.push_back(image.path);
    }
    if (!series_.empty()) {
        selectCase(series_[0]);
    }
}

void Rendering::DicomPreview::setCase(float percentage) {
    int idx = (int)(percentage * (float)(series_.size() - 1));
    if (idx != case_idx) {
        selectCase(series_[idx]);
        case_idx = idx;
    }
}

void Rendering::DicomPreview::setCrop(ImVec2 crop_x, ImVec2 crop_y, bool force) {
    if (!force && is_crop_locked)
        return;
    if (force)
        is_crop_locked = false;
    set_crop(crop_x, crop_y);
}

void Rendering::DicomPreview::setIsDisabled(bool disabled) {
    is_disabled_ = disabled;
}

void Rendering::DicomPreview::setWindowing(int width, int center, bool force) {
    if (!force && is_window_locked)
        return;
    if (is_window_locked)
        is_window_locked = false;
    set_window(width, center);
}

void Rendering::DicomPreview::set_crop(ImVec2 crop_x, ImVec2 crop_y, bool lock) {
    if (crop_x.x != prev_crop_x_.x || crop_x.y != prev_crop_x_.y || crop_y.x != prev_crop_y_.x || crop_y.y != prev_crop_y_.y) {
        crop_x_ = crop_x;
        crop_y_ = crop_y;
        prev_crop_x_ = crop_x;
        prev_crop_y_ = crop_y;
        if (lock)
            is_crop_locked = true;
        selectCase(series_[0]);
    }
}

void Rendering::DicomPreview::set_window(int width, int center, bool lock) {
    if (width != prev_ww_ || center != prev_wc_) {
        window_width_ = width;
        window_center_ = center;
        prev_ww_ = width;
        prev_wc_ = center;
        reset_image_ = true;
        if (lock)
            is_window_locked = true;
    }
}
