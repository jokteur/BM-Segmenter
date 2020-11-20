#include "preview.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

int Rendering::Preview::instance_number = 0;

void Rendering::Preview::init() {
    instance_number++;
    identifier_ = std::to_string(instance_number) + std::string("Preview");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setCenterX(true);
}

Rendering::Preview::Preview() {
    init();
}

Rendering::Preview::Preview(const Preview& other) {
    init();
    dicom_ = other.dicom_;
    is_valid_ = other.is_valid_;
}

Rendering::Preview::Preview(const Preview&& other) {
    init();
    dicom_ = other.dicom_;
    is_valid_ = other.is_valid_;
}

void Rendering::Preview::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
    // Window ImGui intro
    auto& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    // Remove all padding for the child window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    const int num_pop = 3;
    ImGui::BeginChild(identifier_.c_str(), ImVec2(int(size_.x), int(size_.y)), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(num_pop);

    // Calculate the available dimensions
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos = ImGui::GetMousePos();
    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x;
    dimensions_.height = content.y;
    Rect image_dimensions = image_widget_.getDimensions();

    if (reset_image_) {
        reset_image_ = false;
        image_widget_.setDragSourceFunction([this] {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
                drag_and_drop.giveData(dicom_);

                int a = 0; // Dummy int
                ImGui::SetDragDropPayload("_DICOM_PAYLOAD", &a, sizeof(a));
                ImGui::Text("Drag %s", ::core::parse_dicom_id(dicom_->getId()).first.c_str());
                ImGui::PushID("Image_Drag_Drop");
                ImGui::Image(
                    image_.texture(),
                    ImVec2(128, 128)
                );
                ImGui::PopID();
                ImGui::EndDragDropSource();
            }
        });
    }

    // Interaction with the mouse cursor
    if (allow_scroll_ && Widgets::check_hitbox(mouse_pos, image_dimensions)) {
        float percentage = (mouse_pos.x - image_dimensions.xpos) / image_dimensions.width;
        if (percentage >= 0.f && percentage <= 1.f)
            setCase(percentage);
    }
    else {
        setCase(0.f);
    }

    // Draw the image
    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, dimensions_);
    }
    if (ImGui::BeginPopupContextItem(identifier_.c_str())) {
        popup_context_menu();
        ImGui::EndPopup();
    }
    ImGui::EndChild();
}

void Rendering::Preview::unload() {
}

void Rendering::Preview::popup_context_menu() {
    if (ImGui::Selectable("Add to group")) {
    }
    if (ImGui::Selectable("Add to segmentation")) {

    }
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Cropping and windowing", ImGuiTreeNodeFlags_None)) {
        ImGui::Text("Cropping:");
        ImGui::DragFloatRange2("Crop in x", &crop_x_.x, &crop_x_.y, 1.f, 0.0f, 100.0f, "Left: %.1f %%", "Right: %.1f %%");
        ImGui::DragFloatRange2("Crop in y", &crop_y_.x, &crop_y_.y, 1.f, 0.0f, 100.0f, "Top: %.1f %%", "Bottom: %.1f %%");
        set_crop(crop_x_, crop_y_, true);
        ImGui::Separator();
        ImGui::Text("Windowing: ");
        ImGui::DragInt("Window center###dicom_preview_wc", &dicom_->getWC(), 0.5, -1000, 3000, "%d HU");
        ImGui::DragInt("Window width###dicom_preview_ww", &dicom_->getWW(), 0.5, 1, 3000, "%d HU");
        set_window(dicom_->getWW(), dicom_->getWC(), true);
    }
}

void Rendering::Preview::setCase(float percentage) {
    if (!is_valid_)
        return;

    int idx = (int)(percentage * (float)(dicom_->size() - 1));
    if (idx != case_idx) {
        set_case(idx);
        case_idx = idx;
        reset_image_ = true;
    }
}

void Rendering::Preview::set_case(int idx) {
    if (!is_valid_)
        return;

    dicom_->loadCase(idx, false, [this](const core::Dicom& dicom) {
        image_.setImageFromHU(dicom.data, (float)dicom_->getWW(), (float)dicom_->getWC());
        image_widget_.setImage(image_);
        reset_image_ = true;
        dicom_->cleanData();
    });
}

void Rendering::Preview::set_crop(ImVec2 crop_x, ImVec2 crop_y, bool lock) {
    if (!is_valid_)
        return;
}

void Rendering::Preview::set_window(int width, int center, bool lock) {
    if (!is_valid_)
        return;
}

void Rendering::Preview::setSeries(std::shared_ptr<::core::DicomSeries> dicom) {
    is_valid_ = true;
    dicom_ = dicom; 
    set_case(0);
}
