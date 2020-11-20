#include "edit_mask.h"

#include "drag_and_drop.h"

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
    }
    else if (num_active == 1) {
        active_button_ = active;
    }
    else {
        active_button_->setState(false);
        active_button_ = next_active;
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

    ImGui::Separator();


    // Draw the image
    ImGui::BeginChild("Mask_edit");
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, dimensions_);
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