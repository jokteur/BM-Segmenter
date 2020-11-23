#include <algorithm>

#include "edit_mask.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

int Rendering::EditMask::instance_number = 0;

void Rendering::EditMask::loadDicom(const std::shared_ptr<::core::DicomSeries> dicom) {
    if (dicom_ != nullptr) {
        dicom_->cancelPendingJobs();
        dicom_->unloadData();
    }
    dicom_ = dicom;
    loadCase(0);
}

void Rendering::EditMask::loadCase(int idx) {
    if (dicom_ != nullptr) {
        dicom_->loadCase(0, false, [this](const core::Dicom& dicom) {
            image_.setImageFromHU(dicom.data, (float)dicom_->getWW(), (float)dicom_->getWC());
            image_widget_.setImage(image_);
            reset_image_ = true;
            dicom_->cleanData();
            });
    }
}

Rendering::EditMask::EditMask()
    : lasso_select_b_(
        "assets/lasso.png",
        "assets/lasso_dark.png",
        true,
        "Lasso select"
    ), box_select_b_(
        "assets/box_select.png",
        "assets/box_select_dark.png",
        true,
        "Box select"
    ), brush_select_b_(
        "assets/brush.png",
        "assets/brush_dark.png",
        true,
        "Circular brush"
    ) {
    my_instance_ = instance_number++;
    identifier_ = std::to_string(my_instance_) + std::string("DicomViewer");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    //image_widget_.setCenterY(true);
    image_widget_.setCenterX(true);

    // Buttons
    buttons_list_.push_back(&lasso_select_b_);
    buttons_list_.push_back(&box_select_b_);
    buttons_list_.push_back(&brush_select_b_);


    // Listen to selection in the dataset viewer
    listener_.callback = [=](Event_ptr& event) {
        auto log = reinterpret_cast<core::DicomSelectEvent*>(event.get());
        loadDicom(log->getDicom());
    };
    listener_.filter = "dataset/dicom_edit";

    reset_viewer_listener_.callback = [=](Event_ptr& event) {
        if (dicom_ != nullptr) {
            dicom_->cancelPendingJobs();
            dicom_->unloadData();
            dicom_ = nullptr;
            image_.reset();
            image_widget_.setImage(image_);
        }
    };
    reset_viewer_listener_.filter = "dataset/dicom/reset";

    EventQueue::getInstance().subscribe(&listener_);
    EventQueue::getInstance().subscribe(&reset_viewer_listener_);
}

Rendering::EditMask::~EditMask() {
    EventQueue::getInstance().unsubscribe(&listener_);
    EventQueue::getInstance().unsubscribe(&reset_viewer_listener_);
}

void Rendering::EditMask::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
    auto& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    button_logic();

    ImGui::Begin("Edit mask", &is_open_); // TODO: unique ID

    if (dicom_ != nullptr) {
        ImGui::Text("ID: %s", ::core::parse_dicom_id(dicom_->getId()).first.c_str());
    }

    // Draw the buttons
    lasso_select_b_.ImGuiDraw(window, dimensions_);
    ImGui::SameLine();
    box_select_b_.ImGuiDraw(window, dimensions_);
    ImGui::SameLine();
    brush_select_b_.ImGuiDraw(window, dimensions_);

    if (active_button_ != nullptr) {
        ImGui::Text("Tool options");
        ImGui::RadioButton("Add", &add_sub_option_, 0); ImGui::SameLine();
        ImGui::RadioButton("Substract", &add_sub_option_, 1); ImGui::SameLine();
        ImGui::Checkbox("Threshold HU", &threshold_hu_);
        if (threshold_hu_) {
            ImGui::DragFloatRange2("HU threshold", &hu_min_, &hu_max_, 1.f, 0.0f, 100.0f, "Min: %.1f HU", "Max: %.1f HU");
        }
        if (active_button_ == &brush_select_b_) {
            ImGui::SliderInt("Brush size", &brush_size_, 1, 200);
        }
    }

    ImGui::Separator();

    // Draw the image
    ImGui::BeginChild("Mask_edit");
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();

    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, dimensions_);

        Rect& dimensions = image_widget_.getDimensions();

        if (active_button_ == &lasso_select_b_) {
            lasso_widget(dimensions);
        }
        else if (active_button_ == &box_select_b_) {
            box_widget(dimensions);
        }
        else if (active_button_ == &brush_select_b_) {
            brush_widget(dimensions);
        }
    }
    else {
        ImGui::Text("Drag an image here to open it");
        ImGui::Dummy(ImVec2(content.x, content.y - 3 * ImGui::GetItemRectSize().y));
    }
    accept_drag_and_drop();
    ImGui::EndChild();

    ImGui::End();
}

void Rendering::EditMask::accept_drag_and_drop() {
    if (ImGui::BeginDragDropTarget()) {
        if (ImGui::AcceptDragDropPayload("_DICOM_PAYLOAD")) {
            auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
            auto data = drag_and_drop.returnData();
            loadDicom(data);
        }
        ImGui::EndDragDropTarget();
    }
}

void Rendering::EditMask::button_logic() {
    int num_active = 0;
    ImageButton* next_active = nullptr;
    ImageButton* active;
    for (auto button : buttons_list_) {
        if (button->isActive()) {
            num_active++;
            if (button != active_button_) {
                next_active = button;
            }
            active = button;
        }
    }
    if (num_active == 0) {
        active_button_ = nullptr;
        image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    }
    else if (num_active == 1) {
        active_button_ = active;
        image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
    }
    else {
        active_button_->setState(false);
        image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
        active_button_ = next_active;
    }
}

void Rendering::EditMask::lasso_widget(Rect& dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(0)) {
        ImGuiIO& io = ImGui::GetIO();
        if (Widgets::check_hitbox(mouse_pos, dimensions) && !begin_action_ && !io.KeyCtrl) {
            begin_action_ = true;
        }
        if (begin_action_) {
            if (mouse_pos.x != last_mouse_pos_.x || mouse_pos.y != last_mouse_pos_.y) {
                last_mouse_pos_ = mouse_pos;
                path_size++;
                ImVec2* tmp_paths = new ImVec2[path_size];
                if (raw_path_ != nullptr) {
                    memcpy(tmp_paths, raw_path_, sizeof(ImVec2)*(path_size - 1));
                    delete[] raw_path_;
                }
                raw_path_ = tmp_paths;

                raw_path_[path_size - 1] = mouse_pos;
            }
            if (raw_path_ != nullptr)
                ImGui::GetForegroundDrawList()->AddPolyline(raw_path_, path_size, ImColor(0.7f, 0.7f, 0.7f, 0.5f), true, 3);
        }
    }
    if (ImGui::IsMouseReleased(0)) {
        if (raw_path_ != nullptr) {
            std::vector<ImVec2> positions;
            for (int i = 0; i < path_size; i++) {
                positions.push_back(ImVec2(
                    (raw_path_[i].x - dimensions.xpos)/dimensions.width * image_.width(), 
                    (raw_path_[i].y - dimensions.ypos)/dimensions.height * image_.height()
                ));
            }
            delete[] raw_path_;
            path_size = 0;
            raw_path_ = nullptr;
        }
        begin_action_ = false;
    }
}

void Rendering::EditMask::box_widget(Rect& dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(0)) {
        ImGuiIO& io = ImGui::GetIO();
        if (Widgets::check_hitbox(mouse_pos, dimensions) && !begin_action_ && !io.KeyCtrl) {
            begin_action_ = true;
            last_mouse_pos_ = mouse_pos;
        }
        if (begin_action_) {
            ImGui::GetForegroundDrawList()->AddRect(last_mouse_pos_, mouse_pos, ImColor(0.7f, 0.7f, 0.7f, 0.5f), 0, 0, 3);
        }
    }
    if (ImGui::IsMouseReleased(0)) {
        begin_action_ = false;
    }
}

void Rendering::EditMask::brush_widget(Rect& dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (Widgets::check_hitbox(mouse_pos, dimensions) && !begin_action_) {
        int num_seg = brush_size_/4 + 12;
        ImGui::GetForegroundDrawList()->AddCircle(mouse_pos, brush_size_, ImColor(0.7f, 0.7f, 0.7f, 0.5f), num_seg, 4);
    }
    if (ImGui::IsMouseDown(0)) {
        if (mouse_pos.x != last_mouse_pos_.x || mouse_pos.y != last_mouse_pos_.y) {
            last_mouse_pos_ = mouse_pos;
        }
    }
    if (ImGui::IsMouseReleased(0)) {
    }
}