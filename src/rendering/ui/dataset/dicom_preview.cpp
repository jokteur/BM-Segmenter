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

Rendering::DicomPreview::DicomPreview(const DicomPreview& other) {
    instance_number++;
    identifier_ = std::to_string(instance_number) + std::string("DicomPreview");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setCenterX(true);
    image_widget_.setCenterY(true);
    series_node_ = other.series_node_;
}

Rendering::DicomPreview::DicomPreview(const DicomPreview&& other) {
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setCenterX(true);
    image_widget_.setCenterY(true);
    series_node_ = other.series_node_;
    identifier_ = other.identifier_;
}

void Rendering::DicomPreview::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    // Remove all padding for the child window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    const int num_pop = 3;

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
        reset_image_ = false;
        image_widget_.setDragSourceFunction([this] {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
                auto series_payload = ::core::dataset::SeriesPayload{series_node_, case_};
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
                auto series_payload = ::core::dataset::SeriesPayload{series_node_, case_};
                EventQueue::getInstance().post(Event_ptr(new ::core::dataset::SelectSeriesEvent(series_payload)));
            }
            ImGui::Separator();
            ImGui::Text("Cropping:");
            ImGui::DragFloatRange2("Crop in x", &crop_x_.x, &crop_x_.y, 1.f, 0.0f, 100.0f, "Left: %.1f %%", "Right: %.1f %%");
            ImGui::DragFloatRange2("Crop in y", &crop_y_.x, &crop_y_.y, 1.f, 0.0f, 100.0f, "Top: %.1f %%", "Bottom: %.1f %%");
            set_crop(crop_x_, crop_y_, true);
            ImGui::Separator();
            ImGui::Text("Windowing: ");
            ImGui::DragInt("Window center###dicom_preview_wc", &series_node_->data.getWC(), 0.5, -1000, 3000, "%d HU");
            ImGui::DragInt("Window width###dicom_preview_ww", &series_node_->data.getWW(), 0.5, 1, 3000, "%d HU");
            set_window(series_node_->data.getWW(), series_node_->data.getWC(), true);
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void Rendering::DicomPreview::selectCase(int idx) {
    series_node_->data.loadCase(idx, false, [=](const core::Dicom& dicom) {
        image_.setImageFromHU(dicom.data, (float)series_node_->data.getWW(), (float)series_node_->data.getWC());
        image_widget_.setImage(image_);
        reset_image_ = true;
        series_node_->data.cleanData();
    });
}

void Rendering::DicomPreview::loadSeries(std::shared_ptr<::core::dataset::SeriesNode> series_node, const ::core::dataset::Case& aCase) {
    case_ = aCase;
    if (series_node_ != nullptr) {
        series_node_->data.unloadData();
    }
    series_node_ = series_node;
    selectCase(0);
}

void Rendering::DicomPreview::setCase(float percentage) {
    int idx = (int)(percentage * (float)(series_node_->data.size() - 1));
    if (idx != case_idx) {
        selectCase(idx);
        case_idx = idx;
        reset_image_ = true;
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

        series_node_->data.setCrops(crop_x_, crop_y_);
        selectCase(case_idx);
        reset_image_ = true;
    }
}

void Rendering::DicomPreview::set_window(int width, int center, bool lock) {
    if (width != prev_ww_ || center != prev_wc_) {
        series_node_->data.getWW() = width;
        series_node_->data.getWC() = center;
        prev_ww_ = width;
        prev_wc_ = center;
        reset_image_ = true;
        if (lock)
            is_window_locked = true;
        selectCase(case_idx);
    }
}
