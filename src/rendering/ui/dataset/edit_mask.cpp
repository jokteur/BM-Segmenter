#include <algorithm>

#include "edit_mask.h"
#include "util.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

int Rendering::EditMask::instance_number = 0;


void Rendering::EditMask::unload() {
    if (mask_collection_ != nullptr) {
        mask_collection_->unloadData(true);
    }
}

void Rendering::EditMask::loadDicom(const std::shared_ptr<core::DicomSeries> dicom) {
    if (dicom_series_ != nullptr) {
        dicom_series_->cancelPendingJobs();
        dicom_series_->unloadData();
        unload();
    }
    dicom_series_ = dicom;

    loadCase(0);
}

void Rendering::EditMask::loadCase(int idx) {
    if (dicom_series_ != nullptr) {
        dicom_series_->loadCase(0, false, [this](const core::Dicom& dicom) {
            image_.setImageFromHU(dicom.data, (float)dicom_series_->getWW(), (float)dicom_series_->getWC());
            image_widget_.setImage(image_);
            dicom_dimensions_.x = dicom.data.rows;
            dicom_dimensions_.y = dicom.data.cols;
            set_and_load();

            tmp_dicom_ = dicom;

            dicom_series_->cleanData();
        });
        set_NextPrev_buttons();
    }
}

void Rendering::EditMask::load_segmentation(std::shared_ptr<::core::segmentation::Segmentation> seg) {
    unload();
    active_seg_ = seg;
    set_and_load();
}

bool Rendering::EditMask::set_and_load() {
    if (active_seg_ != nullptr && dicom_series_ != nullptr) {
        active_seg_->addDicom(dicom_series_);

        mask_collection_ = active_seg_->getMasks()[dicom_series_];
        mask_collection_->loadData(true, true);

        if (mask_collection_->getIsValidated()) {
            tmp_mask_ = mask_collection_->getValidated().copy();
        }
        else {
            if (mask_collection_->size() == 0) {
                tmp_mask_ = ::core::segmentation::Mask(dicom_dimensions_.x, dicom_dimensions_.y);
                mask_collection_->setDimensions(dicom_dimensions_.x, dicom_dimensions_.y);
                set_mask();
            }
            else {
                tmp_mask_ = mask_collection_->getCurrent().copy();
            }
        }
        reset_image_ = true;
        build_hu_mask_ = true;
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
    ), validate_b_(
        "assets/validate.png",
        "assets/validate_dark.png",
        true,
        "Validate image"
    )
{
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
    buttons_list_.push_back(&validate_b_);


    // Listen to selection in the dataset viewer
    load_dicom_.callback = [=](Event_ptr& event) {
        auto log = reinterpret_cast<core::DicomSelectEvent*>(event.get());
        loadDicom(log->getDicom());
    };
    load_dicom_.filter = "dataset/dicom_edit";

    // Deactivate buttons if user is dragging
    deactivate_buttons_.callback = [=](Event_ptr& event) {
        disable_buttons();
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

    group_listener_.callback = [=](Event_ptr& event) {
        std::string id = event->getName().substr(21);
        if (id == "all") {
            group_idx_ = -1;
        }
        else {
            group_idx_ = std::stoi(id);
        }
        set_NextPrev_buttons();
    };
    group_listener_.filter = "dataset/group/select/*";

    // Undo and redo
    ctrl_z_.keys = { KEY_CTRL, GLFW_KEY_Z};
    ctrl_z_.name = "undo";
    ctrl_z_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!mask_collection_->isCursorBegin()) {
                undo();
            }
        }
    };

    ctrl_y_.keys = { KEY_CTRL, GLFW_KEY_Y };
    ctrl_y_.name = "undo";
    ctrl_y_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!mask_collection_->isCursorEnd()) {
                redo();
            }
        }
    };

    EventQueue::getInstance().subscribe(&load_dicom_);
    EventQueue::getInstance().subscribe(&reload_seg_);
    EventQueue::getInstance().subscribe(&load_segmentation_);
    EventQueue::getInstance().subscribe(&group_listener_);
    EventQueue::getInstance().subscribe(&deactivate_buttons_);
    EventQueue::getInstance().subscribe(&reset_viewer_listener_);
}

Rendering::EditMask::~EditMask() {
    unload();
    EventQueue::getInstance().unsubscribe(&load_dicom_);
    EventQueue::getInstance().unsubscribe(&reload_seg_);
    EventQueue::getInstance().unsubscribe(&load_segmentation_);
    EventQueue::getInstance().unsubscribe(&group_listener_);
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
        if (prev_dicom_ != nullptr) {
            if (ImGui::Button("Previous")) {
                previous();
            }
        }
        if (next_dicom_ != nullptr) {
            if (prev_dicom_ != nullptr)
                ImGui::SameLine();
            if (ImGui::Button("Next")) {
                next();
            }
        }
        if (prev_dicom_ != nullptr || next_dicom_ != nullptr) {
            ImGui::Separator();
        }
        
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


    // Buttons and validation
    {
        if (active_seg_ == nullptr) {
            ImGui::Text("Please select a segmentation before editing the image.");
        }
        else if (dicom_series_ != nullptr && mask_collection_ != nullptr) {
            bool is_validated = mask_collection_->getIsValidated();

            // Draw the buttons
            info_b_.ImGuiDraw(window, dimensions_);
            ImGui::SameLine();
            if (!is_validated) {
                lasso_select_b_.ImGuiDraw(window, dimensions_);
                ImGui::SameLine();
                //box_select_b_.ImGuiDraw(window, dimensions_);
                //ImGui::SameLine();
                brush_select_b_.ImGuiDraw(window, dimensions_);
                ImGui::SameLine();
            }
            validate_b_.ImGuiDraw(window, dimensions_);

            if (!is_validated) {
                if (!mask_collection_->isCursorBegin()) {
                    ImGui::SameLine();
                    undo_b_.ImGuiDraw(window, dimensions_);
                }
                if (!mask_collection_->isCursorEnd()) {
                    ImGui::SameLine();
                    redo_b_.ImGuiDraw(window, dimensions_);
                }
            }

            if (active_button_ != nullptr && active_button_ != &validate_b_) {
                ImGui::SameLine();
                ImGui::Text("Use Ctrl + Mouse to zoom and pan");
                Widgets::NewLine(3.f);
                ImGui::Text("Tool options");
                ImGui::RadioButton("Add", &add_sub_option_, 0); ImGui::SameLine();
                ImGui::RadioButton("Substract", &add_sub_option_, 1); ImGui::SameLine();
                ImGui::Checkbox("Threshold HU", &threshold_hu_);
                if (threshold_hu_) {
                    ImGui::DragFloatRange2("HU threshold", &hu_min_, &hu_max_, 1.f, -1000.f, 3000.f, "Min: %.1f HU", "Max: %.1f HU");

                    if (prev_hu_max_ != hu_max_ || prev_hu_min_ != hu_min_ || build_hu_mask_) {
                        build_hu_mask_ = false;
                        prev_hu_max_ = hu_max_;
                        prev_hu_min_ = hu_min_;
                        ::core::segmentation::buildHuMask(tmp_dicom_.data, thresholded_hu_, hu_min_, hu_max_);
                    }
                }
                if (active_button_ == &brush_select_b_) {
                    ImGui::SliderFloat("Brush size", &brush_size_, 1, 200, "%.1f px", 2.f);
                }
                Widgets::NewLine(3.f);
            }
            else if (active_button_ == &validate_b_ || is_validated) {
                image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
                image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);

                auto& username = project->getCurrentUser();
                auto& names = mask_collection_->getValidatedBy();
                auto& users = project->getUsers();


                bool username_is_validated = names.find(username) != names.end();

                ImGui::Text("Validation:");
                ImGui::SameLine();

                if (name_select_.size() != users.size() + 1) {
                    std::vector<std::string> opts;
                    opts.push_back("");
                    for (auto& opt : users) {
                        opts.push_back(opt);
                    }
                    name_select_.setOptions(opts, [=](std::string name) {
                        project->setCurrentUser(name);
                    });
                }
                name_select_.ImGuiDraw("Select user", 200);

                if (is_validated) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, .1f, 0.f, 0.7f));
                    if (ImGui::Button("Unvalidate (all)")) {
                        mask_collection_->removeAllValidatedBy();
                        mask_collection_->saveCollection();
                    }
                    ImGui::PopStyleColor();
                }
                if (!username.empty()) {
                    if (is_validated) {
                        if (names.find(username) != names.end()) {
                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, .5f, 0.3f, 0.7f));
                            if (ImGui::Button((("Unvalidate (only " + project->getCurrentUser() + ")").c_str()))) {
                                mask_collection_->removeValidatedBy(username);
                                mask_collection_->saveCollection();
                            }
                            ImGui::PopStyleColor();
                        }
                    }
                    if (!username_is_validated) {
                        if (is_validated)
                            ImGui::SameLine();

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, .8f, 0.f, 0.7f));
                        if (ImGui::Button(("Validate (by " + project->getCurrentUser() + ")").c_str())) {
                            if (!is_validated) {
                                mask_collection_->setValidated(tmp_mask_);
                            }
                            mask_collection_->setValidatedBy(username);
                            mask_collection_->saveCollection();
                        }
                        ImGui::PopStyleColor();
                    }
                }

                if (!names.empty()) {
                    std::string text;
                    for (auto& name : names) {
                        text += name + ", ";
                    }
                    text = text.substr(0, text.size() - 2);
                    ImGui::Text("Currently validated by: %s", text.c_str());
                }

                if (username.empty()) {
                    if (users.empty()) {
                        ImGui::Text("For validation of the image, please create a user in the Project Info tab");
                    }
                    else {
                        ImGui::Text("For validation of the image, please select a user");
                    }
                }
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
            active_button_ = nullptr;
        }
        ImGui::EndDragDropTarget();
    }
}

void Rendering::EditMask::disable_buttons() {
    for (auto button : buttons_list_) {
        if (button != &validate_b_)
            button->setState(false);
    }
    info_b_.setState(false);
    active_button_ = nullptr;
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

void Rendering::EditMask::set_NextPrev_buttons() {
    auto& project = ::core::project::ProjectManager::getInstance().getCurrentProject();
    std::vector<std::shared_ptr<::core::DicomSeries>> dicoms;
    if (group_idx_ >= 0) {
        auto& groups = project->getDataset().getGroups();
        dicoms = groups[group_idx_].getOrderedDicoms();
    }
    else {
        dicoms = project->getDataset().getOrderedDicoms();
    }
    std::shared_ptr<::core::DicomSeries> prev = nullptr;
    std::shared_ptr<::core::DicomSeries> next = nullptr;
    bool found = false;
    int n = 0;
    if (dicom_series_ != nullptr) {
        for (auto& dicom : dicoms) {
            if (found) {
                next = dicom;
                break;
            }
            if (dicom == dicom_series_) {
                found = true;
            }
            else {
                prev = dicom;
            }
            n++;
        }
    }
    
    prev_dicom_ = prev;
    next_dicom_ = next;
}

void Rendering::EditMask::next() {
    if (next_dicom_ != nullptr) {
        loadDicom(next_dicom_);
    }
}

void Rendering::EditMask::previous() {
    if (prev_dicom_ != nullptr) {
        loadDicom(prev_dicom_);
    }
}

void Rendering::EditMask::set_mask() {
    if (active_seg_ != nullptr && mask_collection_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        mask_collection_->push(tmp_mask_.copy());
        mask_collection_->saveCollection();
        reset_image_ = true;
    }
}

void Rendering::EditMask::undo() {
    if (active_seg_ != nullptr && mask_collection_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        tmp_mask_ = mask_collection_->undo().copy();
        mask_collection_->saveCollection();
        reset_image_ = true;
    }
}

void Rendering::EditMask::redo() {
    if (active_seg_ != nullptr && mask_collection_ != nullptr) {
        auto& collections = active_seg_->getMasks();
        tmp_mask_ = mask_collection_->redo().copy();
        mask_collection_->saveCollection();
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
                mask = new ::core::segmentation::Mask(tmp_mask_.rows(), tmp_mask_.cols());
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
                    mask = new ::core::segmentation::Mask(tmp_mask_.rows(), tmp_mask_.cols());
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