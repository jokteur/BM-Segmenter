#include <algorithm>

#include "edit_mask.h"
#include "util.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

int Rendering::EditMask::instance_number = 0;


void Rendering::EditMask::loadDicom(const std::shared_ptr<core::DicomSeries> dicom) {
    if (dicom_series_ != nullptr) {
        dicom_series_->cancelPendingJobs();
        dicom_series_->unloadData();
        if (active_seg_ != nullptr) {
            active_seg_->getMasks()[dicom_series_].unloadData(true);
        }
    }
    dicom_series_ = dicom;

    set_and_load();
    loadCase(0);
}

void Rendering::EditMask::loadCase(int idx) {
    if (dicom_series_ != nullptr) {
        dicom_series_->loadCase(0, false, [this](const core::Dicom& dicom) {
            image_.setImageFromHU(dicom.data, (float)dicom_series_->getWW(), (float)dicom_series_->getWC());
            image_widget_.setImage(image_);

            if (active_seg_ != nullptr) {
                active_seg_->getMasks()[dicom_series_].setDimensions(dicom.data.rows, dicom.data.cols);
                if (active_seg_->getMasks()[dicom_series_].size() == 0) {
                    active_seg_->getMasks()[dicom_series_].push_new();
                }
            }
            dicom_dimensions_.x = dicom.data.rows;
            dicom_dimensions_.y = dicom.data.cols;

            tmp_mask_.setDimensions(dicom.data.rows, dicom.data.cols);
            tmp_dicom_ = dicom;

            dicom_series_->cleanData();
            });
    }
}

void Rendering::EditMask::load_segmentation(std::shared_ptr<::core::segmentation::Segmentation> seg) {
    if (active_seg_ != nullptr) {
        if (dicom_series_ != nullptr) {
            active_seg_->getMasks()[dicom_series_].unloadData(true);
        }
    }
    active_seg_ = seg;
    if (dicom_series_ != nullptr) {
        if (!set_and_load()) {
            tmp_mask_ = ::core::segmentation::Mask(thresholded_hu_.getData().rows, thresholded_hu_.getData().cols);
        }
        reset_image_ = true;
    }
}

bool Rendering::EditMask::set_and_load() {
    if (active_seg_ != nullptr && dicom_series_ != nullptr) {
        active_seg_->addDicom(dicom_series_);
        auto& mask_collection = active_seg_->getMasks()[dicom_series_];
        mask_collection.loadData(true);
        if (!mask_collection.isValid()) {
            set_mask();
        }
    }
    return active_seg_ != nullptr;
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
    ), undo_b_ (
        "assets/undo.png",
        "assets/undo_dark.png",
        false,
        "Undo last action"
    ), redo_b_(
        "assets/redo.png",
        "assets/redo_dark.png",
        false,
        "Redo_last_action"
    ), info_b_(
        "assets/info.png",
        "assets/info_dark.png",
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
    load_dicom_.callback = [=](Event_ptr& event) {
        auto log = reinterpret_cast<core::DicomSelectEvent*>(event.get());
        loadDicom(log->getDicom());
    };
    load_dicom_.filter = "dataset/dicom_edit";

    // Deactivate buttons if user is dragging
    deactivate_buttons_.callback = [=](Event_ptr& event) {
        std::cout << "Hello" << std::endl;
        active_button_ = nullptr;
    };
    deactivate_buttons_.filter = "global/no_action";

    // Reset the view if there is a request
    reset_viewer_listener_.callback = [=](Event_ptr& event) {
        if (dicom_series_ != nullptr) {
            dicom_series_->cancelPendingJobs();
            dicom_series_->unloadData();
            dicom_series_ = nullptr;
            image_.reset();
            image_widget_.setImage(image_);
        }
        if (active_seg_ != nullptr) {
            active_seg_->clear();
        }
    };
    reset_viewer_listener_.filter = "dataset/dicom/reset";

    // Load a new segmentation
    load_segmentation_.callback = [=](Event_ptr& event) {
        load_segmentation(reinterpret_cast<::core::segmentation::SelectSegmentationEvent*>(event.get())->getSegmentation());
    };
    load_segmentation_.filter = "segmentation/select";

    // Mainly used for changing colors
    reload_seg_.callback = [=](Event_ptr& event) {
        reset_image_ = true;
    };
    reload_seg_.filter = "segmentation/reload";

    // Undo and redo
    ctrl_z_.keys = { KEY_CTRL, GLFW_KEY_Z};
    ctrl_z_.name = "undo";
    ctrl_z_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!active_seg_->getMasks()[dicom_series_].isCursorBegin()) {
                undo();
            }
        }
    };

    ctrl_y_.keys = { KEY_CTRL, GLFW_KEY_Y };
    ctrl_y_.name = "undo";
    ctrl_y_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!active_seg_->getMasks()[dicom_series_].isCursorEnd()) {
                redo();
            }
        }
    };

    EventQueue::getInstance().subscribe(&load_dicom_);
    EventQueue::getInstance().subscribe(&reload_seg_);
    EventQueue::getInstance().subscribe(&load_segmentation_);
    EventQueue::getInstance().subscribe(&deactivate_buttons_);
    EventQueue::getInstance().subscribe(&reset_viewer_listener_);
}

Rendering::EditMask::~EditMask() {
    EventQueue::getInstance().unsubscribe(&load_dicom_);
    EventQueue::getInstance().unsubscribe(&reload_seg_);
    EventQueue::getInstance().unsubscribe(&load_segmentation_);
    EventQueue::getInstance().unsubscribe(&deactivate_buttons_);
    EventQueue::getInstance().unsubscribe(&reset_viewer_listener_);
}

void Rendering::EditMask::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
    auto& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    auto project = ::core::project::ProjectManager::getInstance().getCurrentProject();
    
    button_logic();
    KeyboardShortCut::addTempShortcut(ctrl_z_);
    KeyboardShortCut::addTempShortcut(ctrl_y_);

    ImGui::Begin("Edit mask", &is_open_); // TODO: unique ID

    if (project == nullptr) {
        ImGui::End();
        return;
    }

    if (dicom_series_ != nullptr) {
        ImGui::Text("ID: %s", ::core::parse_dicom_id(dicom_series_->getId()).first.c_str());
    }

    // Redraw image if necessary
    if (reset_image_) {
        reset_image_ = false;
        ImVec4 color = { 1.f, 0.f, 0.f, 0.3f };
        if (active_seg_ != nullptr) {
            color = active_seg_->getMaskColor();
        }
        image_.setImageFromHU(tmp_dicom_.data, (float)dicom_series_->getWW(), (float)dicom_series_->getWC(), core::Image::FILTER_NEAREST, tmp_mask_.getData(), color);
    }

    // Mask edition buttons
    if (active_seg_ == nullptr) {
        ImGui::Text("Please select a segmentation before editing the image.");
    }
    else {
        // Draw the buttons
        lasso_select_b_.ImGuiDraw(window, dimensions_);
        ImGui::SameLine();
        //box_select_b_.ImGuiDraw(window, dimensions_);
        //ImGui::SameLine();
        brush_select_b_.ImGuiDraw(window, dimensions_);
        ImGui::SameLine();
        info_b_.ImGuiDraw(window, dimensions_);
        if (active_seg_ != nullptr) {
            if (!active_seg_->getMasks()[dicom_series_].isCursorBegin()) {
                ImGui::SameLine();
                undo_b_.ImGuiDraw(window, dimensions_);
            }
            if (!active_seg_->getMasks()[dicom_series_].isCursorEnd()) {
                ImGui::SameLine();
                redo_b_.ImGuiDraw(window, dimensions_);
            }
        }
        if (active_button_ != nullptr) {
            ImGui::SameLine();
            ImGui::Text("Use Ctrl + Mouse to zoom and pan");
        }

        if (active_button_ != nullptr) {
            ImGui::Text("Tool options");
            ImGui::RadioButton("Add", &add_sub_option_, 0); ImGui::SameLine();
            ImGui::RadioButton("Substract", &add_sub_option_, 1); ImGui::SameLine();
            ImGui::Checkbox("Threshold HU", &threshold_hu_);
            if (threshold_hu_) {
                ImGui::DragFloatRange2("HU threshold", &hu_min_, &hu_max_, 1.f, -1000.f, 3000.f, "Min: %.1f HU", "Max: %.1f HU");

                if (prev_hu_max_ != hu_max_ || prev_hu_min_ != hu_min_) {
                    prev_hu_max_ = hu_max_;
                    prev_hu_min_ = hu_min_;
                    ::core::segmentation::buildHuMask(tmp_dicom_.data, thresholded_hu_, hu_min_, hu_max_);
                }
            }
            if (active_button_ == &brush_select_b_) {
                ImGui::SliderFloat("Brush size", &brush_size_, 1, 200, "%.1f px", 2.f);
            }
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

        if (info_b_.isActive()) {
            auto mouse_pos = ImGui::GetMousePos();
            if (Widgets::check_hitbox(mouse_pos, dimensions)) {
                Crop crop = image_widget_.getCrop();
                ImVec2 pos = {
                    (crop.x0 + (crop.x1 - crop.x0) * (mouse_pos.x - dimensions.xpos) / dimensions.width) * image_.width() - 1,
                    (crop.y0 + (crop.y1 - crop.y0) * (mouse_pos.y - dimensions.ypos) / dimensions.height) * image_.height() - 1
                };
                if (pos.x >= 0 && pos.y >= 0 && pos.x < image_.width() && pos.y < image_.height()) {
                    int value = tmp_dicom_.data.at<short int>((int)pos.y, (int)pos.x);
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::Text("At %d;%d, %d HU", (int)pos.x, (int)pos.y, value);
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
            }
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
        image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);
    }
    else if (num_active == 1) {
        active_button_ = active;
        image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
        image_widget_.setInteractiveZoom(SimpleImage::IMAGE_MODIFIER_INTERACT);
    }
    else {
        active_button_->setState(false);
        image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
        image_widget_.setInteractiveZoom(SimpleImage::IMAGE_MODIFIER_INTERACT);
        active_button_ = next_active;
    }

    if (undo_b_.isMouseReleased()) {
        undo();
    }
    if (redo_b_.isMouseReleased()) {
        redo();
    }
}

void Rendering::EditMask::set_mask() {
    reset_image_ = true;
    if (active_seg_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        collections[dicom_series_].push(tmp_mask_.copy());
        collections[dicom_series_].saveCollection();
    }
}

void Rendering::EditMask::undo() {
    if (active_seg_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        tmp_mask_ = collections[dicom_series_].undo().copy();
        collections[dicom_series_].saveCollection();
        reset_image_ = true;
    }
}

void Rendering::EditMask::redo() {
    if (active_seg_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        tmp_mask_ = collections[dicom_series_].redo().copy();
        collections[dicom_series_].saveCollection();
        reset_image_ = true;
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
                ImGui::GetForegroundDrawList()->AddPolyline(raw_path_, path_size, ImColor(0.7f, 0.7f, 0.7f, 0.5f), true, 2);
        }
    }
    if (ImGui::IsMouseReleased(0) && begin_action_) {
        if (raw_path_ != nullptr) {
            std::vector<cv::Point> positions;
            Crop crop = image_widget_.getCrop();
            for (int i = 0; i < path_size; i++) {
                positions.push_back(cv::Point(
                    (crop.x0 + (crop.x1 - crop.x0) * (raw_path_[i].x - dimensions.xpos) / dimensions.width) * image_.width(),
                    (crop.y0 + (crop.y1 - crop.y0) * (raw_path_[i].y - dimensions.ypos)/dimensions.height) * image_.height()
                ));
            }

            // Draw the polygon
            auto* mask = &tmp_mask_;
            if (threshold_hu_) {
                mask = new ::core::segmentation::Mask(tmp_mask_.width(), tmp_mask_.height());
            }
            if (threshold_hu_)
                ::core::segmentation::lassoSelectToMask(positions, *mask, 1);
            else
                ::core::segmentation::lassoSelectToMask(positions, *mask, (!add_sub_option_) ? 1 : 0);

            if (threshold_hu_) {
                mask->intersect_with(thresholded_hu_);
                if (add_sub_option_) {
                    tmp_mask_.difference_with(*mask);
                }
                else {
                    tmp_mask_.union_with(*mask);
                }
                delete mask;
            }

            delete[] raw_path_;
            path_size = 0;
            raw_path_ = nullptr;
            set_mask();
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
            ImGui::GetForegroundDrawList()->AddRect(last_mouse_pos_, mouse_pos, ImColor(1.f, 0.7f, 0.7f, 0.5f), 0, 0, 2);
        }
    }
    if (ImGui::IsMouseReleased(0)) {
        begin_action_ = false;
    }
}

void Rendering::EditMask::brush_widget(Rect& dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (Widgets::check_hitbox(mouse_pos, dimensions)) {
        ImGuiIO& io = ImGui::GetIO();

        // Change size of brush if needed
        if (io.MouseWheel != 0 && !io.KeyCtrl) {
            float factor = pow(1.f + brush_size_ / 200.f, 4) / 2;
            brush_size_ += io.MouseWheel * factor;

            if (brush_size_ < 1)
                brush_size_ = 1;
            if (brush_size_ > 200)
                brush_size_ = 200;
        }

        // Get the corrected brushsize
        Crop crop = image_widget_.getCrop();
        float brush_size = brush_size_ * dimensions.width / (float)image_.width() / 2.f / (crop.x1 - crop.x0);

        // Draw the brush
        ImGui::GetForegroundDrawList()->AddCircle(mouse_pos, brush_size, ImColor(1.f, 0.7f, 0.7f, 0.5f), brush_size / 4 + 12, 2);
        
        // Apply brush to mask
        if (ImGui::IsMouseDown(0) && !io.KeyCtrl) {
            if (mouse_pos.x != last_mouse_pos_.x || mouse_pos.y != last_mouse_pos_.y) {
                float distance = pow((mouse_pos.x - last_mouse_pos_.x), 2) + pow((mouse_pos.y - last_mouse_pos_.y), 2);

                std::vector<ImVec2> positions;

                if (pow(brush_size / 2.f, 2) < distance && begin_action_) {
                    int num = (int)sqrt(distance) / brush_size * 2.f;
                    for (int i = 0; i < num; i++) {
                        positions.push_back(ImVec2(
                            last_mouse_pos_.x + (mouse_pos.x - last_mouse_pos_.x) * i / (float)num,
                            last_mouse_pos_.y + (mouse_pos.y - last_mouse_pos_.y) * i / (float)num
                        ));
                    }
                }
                else {
                    positions.push_back(mouse_pos);
                }
                last_mouse_pos_ = mouse_pos;

                // If the user is thresholding, then we create a new mask
                // Otherwise, we can operate directly on tmp_mask_
                auto* mask = &tmp_mask_;
                if (threshold_hu_) {
                    mask = new ::core::segmentation::Mask(tmp_mask_.width(), tmp_mask_.height());
                }

                for (auto& pos : positions) {
                    Crop crop = image_widget_.getCrop();
                    ImVec2 corrected_mouse_pos = {
                        (crop.x0 + (crop.x1 - crop.x0) * (pos.x - dimensions.xpos) / dimensions.width) * image_.width(),
                        (crop.y0 + (crop.y1 - crop.y0) * (pos.y - dimensions.ypos) / dimensions.height) * image_.height()
                    };
                    if (threshold_hu_)
                        ::core::segmentation::brushToMask(brush_size_ / 2.f, corrected_mouse_pos, *mask, 1);
                    else
                        ::core::segmentation::brushToMask(brush_size_ / 2.f, corrected_mouse_pos, *mask, (!add_sub_option_) ? 1 : -1);
                }
                if (threshold_hu_) {
                    mask->intersect_with(thresholded_hu_);
                    if (add_sub_option_) {
                        tmp_mask_.difference_with(*mask);
                    }
                    else {
                        tmp_mask_.union_with(*mask);
                    }
                    delete mask;
                }
                reset_image_ = true;
                begin_action_ = true;
            }
        }
    }
    if (ImGui::IsMouseReleased(0) && begin_action_) {
        begin_action_ = false;
        set_mask();
    }
}