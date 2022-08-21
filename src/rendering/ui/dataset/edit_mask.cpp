#include <algorithm>

#include "edit_mask.h"
#include "util.h"

#include "animation_util.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

#include "log.h"

int Rendering::EditMask::instance_number = 0;


void Rendering::EditMask::unload_mask() {
    if (mask_collection_ != nullptr) { BM_DEBUG("Unload mask");
        mask_collection_->unloadData(true, "edit_mask");
    }
}

void Rendering::EditMask::unload_dicom(bool no_reset) {
    if (dicom_series_ != nullptr) {
        dicom_series_->cancelPendingJobs();
        dicom_series_->unloadCase();BM_DEBUG("Unload dicom " + dicom_series_->getIdPair().first);
        if (!no_reset) {
            dicom_series_ = nullptr;
            image_.reset();
        }
    }
}

void Rendering::EditMask::loadDicom(const std::shared_ptr<core::DicomSeries> dicom, bool no_reset) {
    unload_dicom(no_reset);
    unload_mask();

    dicom_series_ = dicom;BM_DEBUG("Set dicom " + dicom->getIdPair().first);
    loadCase(0);
    push_animation();
}

void Rendering::EditMask::loadCase(int idx) {
    if (dicom_series_ != nullptr) {
        dicom_series_->loadCase(0, false, [this](const core::Dicom &dicom) {
            image_.setImageFromHU(dicom.data, (float) dicom_series_->getWW(), (float) dicom_series_->getWC());
            image_widget_.setImage(image_);
            dicom_dimensions_.x = dicom.data.rows;
            dicom_dimensions_.y = dicom.data.cols;BM_DEBUG("Dicom loaded " + dicom_series_->getIdPair().first);
            load_mask();
        });
        set_NextPrev_buttons();
    }
}

void Rendering::EditMask::load_segmentation(std::shared_ptr<::core::segmentation::Segmentation> seg) {
#ifdef LOG_DEBUG
    if (seg != nullptr)
        BM_DEBUG("Load segmentation " + seg->getName());
    else
        BM_DEBUG("Load no segmentation");
#endif
    unload_mask();
    active_seg_ = seg;
    load_mask();
}

bool Rendering::EditMask::load_mask() {
    if (dicom_series_ != nullptr) {
        if (active_seg_ != nullptr) {
            mask_collection_ = active_seg_->getMask(dicom_series_);
            mask_collection_->loadData(true, true, "edit_mask");

            if (mask_collection_->getIsValidated()) {
                tmp_mask_ = mask_collection_->getValidated().copy();
            } else {
                // No edited mask but a prediction is available
                if (mask_collection_->size() == 0 && !mask_collection_->getPrediction().empty()) {
                    tmp_mask_ = mask_collection_->getPrediction().copy();
                }
                    // No edited mask and no prediction
                else if (mask_collection_->size() == 0) {
                    tmp_mask_ = ::core::segmentation::Mask(dicom_dimensions_.x, dicom_dimensions_.y);
                    mask_collection_->setDimensions(dicom_dimensions_.x, dicom_dimensions_.y);
                    //set_mask();
                }
                    // Edited mask
                else {
                    tmp_mask_ = mask_collection_->getCurrent().copy();
                }
            }
            if (tmp_mask_.rows() == 0 || tmp_mask_.cols() == 0) {
                tmp_mask_ = ::core::segmentation::Mask(dicom_dimensions_.x, dicom_dimensions_.y);
                mask_collection_->setDimensions(dicom_dimensions_.x, dicom_dimensions_.y);
            }
            reset_image_ = true;
            build_hu_mask_ = true;
        } else {
            image_.setImageFromHU(
                    dicom_series_->getCurrentDicom().data,
                    (float) dicom_series_->getWW(),
                    (float) dicom_series_->getWC());
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
), undo_b_(
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
    buttons_list_.push_back(&validate_b_);


    // Listen to selection in the dataset viewer
    load_dicom_.callback = [=](Event_ptr &event) {
        auto log = reinterpret_cast<core::DicomSelectEvent *>(event.get());
        loadDicom(log->getDicom());
    };
    load_dicom_.filter = "dataset/dicom_edit";

    // Deactivate buttons if user is dragging
    deactivate_buttons_.callback = [=](Event_ptr &event) {
        disable_buttons();
    };
    deactivate_buttons_.filter = "global/no_action";

    // Reset the view if there is a request
    reset_viewer_listener_.callback = [=](Event_ptr &event) {
        unload_dicom();
        unload_mask();
    };
    reset_viewer_listener_.filter = "dataset/dicom/reset";

    // Load a new segmentation
    load_segmentation_.callback = [=](Event_ptr &event) {
        load_segmentation(
                reinterpret_cast<::core::segmentation::SelectSegmentationEvent *>(event.get())->getSegmentation());
    };
    load_segmentation_.filter = "segmentation/select";

    // Mainly used for changing colors
    reload_seg_.callback = [=](Event_ptr &event) {
        reset_image_ = true;
    };
    reload_seg_.filter = "segmentation/reload";

    group_listener_.callback = [=](Event_ptr &event) {
        std::string id = event->getName().substr(21);
        if (id == "all") {
            group_idx_ = -1;
        } else {
            group_idx_ = std::stoi(id);
        }
        set_NextPrev_buttons();
    };
    group_listener_.filter = "dataset/group/select/*";

    // Undo and redo
    ctrl_z_.keys = {KEY_CTRL, GLFW_KEY_Z};
    ctrl_z_.name = "undo";
    ctrl_z_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!mask_collection_->isCursorBegin()) {
                undo();
            }
        }
    };

    ctrl_y_.keys = {KEY_CTRL, GLFW_KEY_Y};
    ctrl_y_.name = "undo";
    ctrl_y_.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            if (!mask_collection_->isCursorEnd()) {
                redo();
            }
        }
    };


    hide_mask_shortcut.keys = {GLFW_KEY_SPACE};
    hide_mask_shortcut.name = "hide mask";
    hide_mask_shortcut.callback = [this] {
        if (active_seg_ != nullptr && dicom_series_ != nullptr) {
            show_mask = !show_mask;
            reset_image_ = true;
        }
    };

    highlight_hu_range_shortcut.keys = {GLFW_KEY_H};
    highlight_hu_range_shortcut.name = "highlight hu range";
    highlight_hu_range_shortcut.callback = [this] {
        highlight_hu_range = !highlight_hu_range;
        reset_image_ = true;
    };

    EventQueue::getInstance().subscribe(&load_dicom_);
    EventQueue::getInstance().subscribe(&reload_seg_);
    EventQueue::getInstance().subscribe(&load_segmentation_);
    EventQueue::getInstance().subscribe(&group_listener_);
    EventQueue::getInstance().subscribe(&deactivate_buttons_);
    EventQueue::getInstance().subscribe(&reset_viewer_listener_);
}

Rendering::EditMask::~EditMask() {
    unload_mask();
    EventQueue::getInstance().unsubscribe(&load_dicom_);
    EventQueue::getInstance().unsubscribe(&reload_seg_);
    EventQueue::getInstance().unsubscribe(&load_segmentation_);
    EventQueue::getInstance().unsubscribe(&group_listener_);
    EventQueue::getInstance().unsubscribe(&deactivate_buttons_);
    EventQueue::getInstance().unsubscribe(&reset_viewer_listener_);
}

void Rendering::EditMask::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    auto project = ::core::project::ProjectManager::getInstance().getCurrentProject();

    button_logic();
    KeyboardShortCut::addTempShortcut(ctrl_z_);
    KeyboardShortCut::addTempShortcut(ctrl_y_);
    KeyboardShortCut::addTempShortcut(hide_mask_shortcut);
    KeyboardShortCut::addTempShortcut(highlight_hu_range_shortcut);

    ImGui::Begin("Edit mask", &is_open_); // TODO: unique ID

    if (project == nullptr) {
        ImGui::End();
        return;
    }

    if (dicom_series_ != nullptr) {
        if (ImGui::Button("Close image")) {
            unload_mask();
            unload_dicom();
            ImGui::End();
            return;
        }
        if (prev_dicom_ != nullptr) {
            ImGui::SameLine();
            if (ImGui::Button("Previous")) {
                previous();
            }
        }
        if (next_dicom_ != nullptr) {
            ImGui::SameLine();
            if (ImGui::Button("Next")) {
                next();
            }
        }
        auto &groups = project->getDataset().getGroups();
        if (prev_dicom_ != nullptr || next_dicom_ != nullptr) {
            ImGui::SameLine();
            if (group_idx_ == -1) {
                ImGui::Text("(all images)");
            } else {
                ImGui::Text("(in group `%s`)", groups[group_idx_].getName().c_str());
            }
        } else {
            ImGui::Text("(this image does not belong to group `%s`)", groups[group_idx_].getName().c_str());
        }
        if (active_seg_ != nullptr) {
            ImGui::SameLine();
            ImGui::Text("(%s)", active_seg_->getName().c_str());
        }
        if (prev_dicom_ != nullptr || next_dicom_ != nullptr) {
            ImGui::Separator();
        }

        ImGui::Text("ID: %s", ::core::parse_dicom_id(dicom_series_->getId()).first.c_str());
    }

    // Redraw image if necessary
    if (reset_image_) {
        reset_image_ = false;
        ImVec4 color = {1.f, 0.f, 0.f, 0.3f};
        if (active_seg_ != nullptr) {
            color = active_seg_->getMaskColor();
        }
        if (dicom_series_ != nullptr)
            image_.setImageFromHU(
                    dicom_series_->getCurrentDicom().data,
                    (float) dicom_series_->getWW(),
                    (float) dicom_series_->getWC(),
                    core::Image::FILTER_NEAREST,
                    tmp_mask_.getData(),
                    color,
                    show_mask,
                    highlight_hu_range || vert_min_distance != 0,
                    thresholded_hu_.getData()
            );
    }


    // Buttons and validation
    {
        if (active_seg_ == nullptr) {
            ImGui::Text("Please select a segmentation before editing the image.");
        } else if (dicom_series_ != nullptr && mask_collection_ != nullptr) {
            bool is_validated = mask_collection_->getIsValidated();

            // Validation tool
            auto &username = project->getCurrentUser();
            auto &names = mask_collection_->getValidatedBy();
            auto &users = project->getUsers();


            bool username_is_validated = names.find(username) != names.end();

            ImGui::Text("Validation:");
            ImGui::SameLine();

            if (name_select_.size() != users.size() + 1) {
                std::vector<std::string> opts;
                opts.push_back("");
                for (auto &opt: users) {
                    opts.push_back(opt);
                }
                name_select_.setOptions(opts, [=](std::string name) {
                    project->setCurrentUser(name);
                });
            }
            name_select_.ImGuiDraw(" ", 200);

            ImGui::SameLine();

            if (is_validated) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, .1f, 0.f, 0.7f));
                if (ImGui::Button("Unvalidate (all)")) { BM_DEBUG("Unvalidate (all)");
                    mask_collection_->removeAllValidatedBy();
                    mask_collection_->saveCollection();
                    mask_changed();
                }
                ImGui::PopStyleColor();
            }
            if (!username.empty()) {
                if (is_validated) {
                    if (names.find(username) != names.end()) {
                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, .5f, 0.3f, 0.7f));
                        if (ImGui::Button(
                                (("Unvalidate (only " + project->getCurrentUser() + ")").c_str()))) { BM_DEBUG(
                                    "Unvalidate (only)");
                            mask_collection_->removeValidatedBy(username);
                            mask_collection_->saveCollection();
                            mask_changed();
                        }
                        ImGui::PopStyleColor();
                    }
                }
                if (!username_is_validated) {
                    if (is_validated)
                        ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, .8f, 0.f, 0.7f));
                    if (ImGui::Button(("Validate (by " + project->getCurrentUser() + ")").c_str())) { BM_DEBUG(
                                "Validate");
                        if (!is_validated) {
                            mask_collection_->setValidated(tmp_mask_);
                        }
                        mask_collection_->setValidatedBy(username);
                        mask_collection_->saveCollection();
                        mask_changed();
                    }
                    ImGui::PopStyleColor();
                }
            }

            if (!names.empty()) {
                std::string text;
                for (auto &name: names) {
                    text += name + ", ";
                }
                text = text.substr(0, text.size() - 2);
                ImGui::SameLine();
                ImGui::Text("Validated by: %s", text.c_str());
            }

            if (username.empty()) {
                if (users.empty()) {
                    ImGui::Text("For validation of the image, please create a user in the Project Info tab");
                } else {
                    ImGui::Text("For validation of the image, please select a user");
                }
            }

            // Pixel info button
            info_b_.ImGuiDraw(window, dimensions_);
            // Show mask checkbox
            ImGui::SameLine();
            ImGui::Checkbox("Show mask (space)", &show_mask);
            if (previous_show_mask != show_mask) {
                previous_show_mask = show_mask;
                reset_image_ = true;
            }
            // Highlight HU range checkbox
            ImGui::SameLine();
            ImGui::Checkbox("HU threshold (H)", &highlight_hu_range);

            // HU threshold fields
            ImGui::SameLine();
            ImGui::DragFloatRange2(" ", &hu_min_, &hu_max_, 1.f, -1000.f, 3000.f, "Min: %.1f HU",
                                   "Max: %.1f HU");
            ImGui::SameLine();
            ImGui::Checkbox("ignore small holes", &ignore_small_holes_and_objects);

            ImGui::SameLine();
            ImGui::DragFloat("Vertebra min distance", &vert_min_distance, 1, 0, 10);
            if (prev_hu_max_ != hu_max_ || prev_hu_min_ != hu_min_ ||
                prev_ignore_small_holes_and_objects != ignore_small_holes_and_objects ||
                prev_vert_min_distance != vert_min_distance ||
                previous_highlight_hu_range != highlight_hu_range ||
                build_hu_mask_) {
                build_hu_mask_ = false;
                previous_highlight_hu_range = highlight_hu_range;
                prev_hu_max_ = hu_max_;
                prev_hu_min_ = hu_min_;
                prev_vert_min_distance = vert_min_distance;
                prev_ignore_small_holes_and_objects = ignore_small_holes_and_objects;
                if (dicom_series_ != nullptr)
                    ::core::segmentation::buildHuMask(dicom_series_->getCurrentDicom().data, thresholded_hu_,
                                                      highlight_hu_range, hu_min_,
                                                      hu_max_, ignore_small_holes_and_objects, vert_min_distance);
                reset_image_ = true;
            }


            if (!is_validated) {
                ImGui::RadioButton("Lasso", &lasso_or_brush, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Brush", &lasso_or_brush, 1);
                if (lasso_or_brush == 1) {
                    ImGui::SameLine();
                    ImGui::SliderFloat("Brush size", &brush_size_, 1, 200, "%.1f px", ImGuiSliderFlags_Logarithmic);
                }


                if (!mask_collection_->getPrediction().empty()) {
                    // Make the option to revert the mask to prediction
                    if (ImGui::Button("Revert to ML pred.")) {
                        mask_collection_->clearHistory(true);
                        tmp_mask_ = mask_collection_->getPrediction().copy();
                        mask_collection_->saveCollection();
                        mask_changed();
                        reset_image_ = true;
                    }
                    ImGui::SameLine();
                }

                if (ImGui::Button("Auto brush borders")) {
                    automaticBrushBorders();
                }
                ImGui::SameLine();

                if (!mask_collection_->isCursorBegin()) {
                    undo_b_.ImGuiDraw(window, dimensions_);
                    ImGui::SameLine();
                }
                if (!mask_collection_->isCursorEnd()) {
                    redo_b_.ImGuiDraw(window, dimensions_);
                }


            }
        }
    }

    ImGui::Separator();

// Draw the image
    image_widget_.
            setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
    image_widget_.
            setInteractiveZoom(SimpleImage::IMAGE_MODIFIER_INTERACT);

    ImGui::BeginChild("Mask_edit");
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();

    if (image_.

            isImageSet()

            ) {
        image_widget_.setAutoScale(true);
        image_widget_.
                ImGuiDraw(window, dimensions_
        );

        Rect &dimensions = image_widget_.getDimensions();

        if (lasso_or_brush == 0) {
            lasso_widget(dimensions);
        } else if (lasso_or_brush == 1) {
            brush_widget(dimensions);
        }

        if (info_b_.

                isActive()

                ) {
            auto mouse_pos = ImGui::GetMousePos();
            if (
                    Widgets::check_hitbox(mouse_pos, dimensions
                    )) {
                Crop crop = image_widget_.getCrop();
                ImVec2 pos = {
                        (crop.x0 + (crop.x1 - crop.x0) * (mouse_pos.x - dimensions.xpos) / dimensions.width) *
                        image_.width(),
                        (crop.y0 + (crop.y1 - crop.y0) * (mouse_pos.y - dimensions.ypos) / dimensions.height) *
                        image_.height()
                };
                if (pos.x >= 0 && pos.y >= 0 && pos.x < image_.

                        width() &&

                    pos.y < image_.

                            height()

                        ) {
                    int value = dicom_series_->getCurrentDicom().data.at<short int>((int) pos.y, (int) pos.x);

                    ImGui::BeginTooltip();

                    ImGui::PushTextWrapPos(ImGui::GetFontSize()

                                           * 35.0f);
                    ImGui::Text("At %d;%d, %d HU", (int) pos.x, (int) pos.y, value);

                    ImGui::PopTextWrapPos();

                    ImGui::EndTooltip();

                }
            }
        }
    } else {
        ImGui::Text("Drag an image here to open it");
        ImGui::Dummy(ImVec2(content.x, content.y - 3 * ImGui::GetItemRectSize().y)
        );
    }

    accept_drag_and_drop();

    ImGui::EndChild();

    ImGui::End();

}

void Rendering::EditMask::accept_drag_and_drop() {
    if (ImGui::BeginDragDropTarget()) { BM_DEBUG("Accept drag and drop");
        if (ImGui::AcceptDragDropPayload("_DICOM_PAYLOAD")) {
            auto &drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries >>
            ::getInstance();
            auto data = drag_and_drop.returnData();
            loadDicom(data);
            active_button_ = nullptr;
            drag_and_drop.giveData(nullptr);
        }
        ImGui::EndDragDropTarget();
    }
}

void Rendering::EditMask::disable_buttons() {
    for (auto button: buttons_list_) {
        if (button != &validate_b_)
            button->setState(false);
    }
    info_b_.setState(false);
    active_button_ = nullptr;
}

void Rendering::EditMask::button_logic() {
    int num_active = 0;
    ImageButton *next_active = nullptr;
    ImageButton *active;
    for (auto button: buttons_list_) {
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
    } else if (num_active == 1) {
        active_button_ = active;
        image_widget_.setImageDrag(SimpleImage::IMAGE_MODIFIER_INTERACT);
        image_widget_.setInteractiveZoom(SimpleImage::IMAGE_MODIFIER_INTERACT);
    } else {
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
    auto &project = ::core::project::ProjectManager::getInstance().getCurrentProject();
    std::vector<std::shared_ptr<::core::DicomSeries>> dicoms;
    if (group_idx_ >= 0) {
        auto &groups = project->getDataset().getGroups();
        dicoms = groups[group_idx_].getOrderedDicoms();
    } else {
        dicoms = project->getDataset().getOrderedDicoms();
    }
    std::shared_ptr<::core::DicomSeries> prev = nullptr;
    std::shared_ptr<::core::DicomSeries> next = nullptr;
    bool found = false;
    int n = 0;
    if (dicom_series_ != nullptr) {
        for (auto &dicom: dicoms) {
            if (found) {
                next = dicom;
                break;
            }
            if (dicom == dicom_series_) {
                found = true;
            } else {
                prev = dicom;
            }
            n++;
        }
    }

    if (found) {
        prev_dicom_ = prev;
        next_dicom_ = next;
    } else {
        prev_dicom_ = nullptr;
        next_dicom_ = nullptr;
    }
}

void Rendering::EditMask::next() {
    if (next_dicom_ != nullptr) { BM_DEBUG("Select next");
        loadDicom(next_dicom_, true);
    }
}

void Rendering::EditMask::previous() {
    if (prev_dicom_ != nullptr) { BM_DEBUG("Select previous");
        loadDicom(prev_dicom_, true);
    }
}

void Rendering::EditMask::set_mask() {
    if (active_seg_ != nullptr && mask_collection_ != nullptr) {
#ifdef LOG_DEBUG
        std::string msg = "Set mask "
            + std::to_string(info_b_.isActive())
            + std::to_string(lasso_select_b_.isActive())
            + std::to_string(brush_select_b_.isActive())
            + std::to_string(validate_b_.isActive())
            + " "
            + std::to_string(add_sub_option_) + " "
            + std::to_string(brush_size_) + " "
            + std::to_string(threshold_hu_) + " "
            + std::to_string(hu_min_) + " "
            + std::to_string(hu_max_);
        BM_DEBUG(msg);
#endif
        auto &collections = active_seg_->getMasks();
        mask_collection_->push(tmp_mask_.copy());
        mask_collection_->saveCollection();
        mask_changed();
        reset_image_ = true;
    }
}

void Rendering::EditMask::undo() {
    if (active_seg_ != nullptr && mask_collection_ != nullptr) { BM_DEBUG("Undo mask");
        auto &collections = active_seg_->getMasks();
        tmp_mask_ = mask_collection_->undo().copy();
        mask_collection_->saveCollection();
        mask_changed();
        reset_image_ = true;
    }
}

void Rendering::EditMask::redo() {
    BM_DEBUG("Redo mask");
    if (active_seg_ != nullptr && mask_collection_ != nullptr) {
        auto &collections = active_seg_->getMasks();
        tmp_mask_ = mask_collection_->redo().copy();
        mask_collection_->saveCollection();
        mask_changed();
        reset_image_ = true;
    }
}


void Rendering::EditMask::mask_changed() {
    EventQueue::getInstance().post(Event_ptr(new Event("mask/changed/" + dicom_series_->getId())));
}

void Rendering::EditMask::lasso_widget(Rect &dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(0)) {
        ImGuiIO &io = ImGui::GetIO();
        if (Widgets::check_hitbox(mouse_pos, dimensions) && !begin_action_ && !io.KeyCtrl) {
            begin_action_ = true;
        }
        if (begin_action_) {
            if (mouse_pos.x != last_mouse_pos_.x || mouse_pos.y != last_mouse_pos_.y) {
                last_mouse_pos_ = mouse_pos;
                path_size++;
                ImVec2 *tmp_paths = new ImVec2[path_size];
                if (raw_path_ != nullptr) {
                    memcpy(tmp_paths, raw_path_, sizeof(ImVec2) * (path_size - 1));
                    delete[] raw_path_;
                }
                raw_path_ = tmp_paths;

                raw_path_[path_size - 1] = mouse_pos;
            }
            if (raw_path_ != nullptr)
                ImGui::GetForegroundDrawList()->AddPolyline(raw_path_, path_size, ImColor(0.7f, 0.7f, 0.7f, 0.5f),
                                                            true,
                                                            2);
        }
    }
    if (ImGui::IsMouseReleased(0) && begin_action_) {
        if (raw_path_ != nullptr) {
            std::vector<cv::Point> positions;
            Crop crop = image_widget_.getCrop();
            for (int i = 0; i < path_size; i++) {
                positions.push_back(cv::Point(
                        (crop.x0 + (crop.x1 - crop.x0) * (raw_path_[i].x - dimensions.xpos) / dimensions.width) *
                        image_.width(),
                        (crop.y0 + (crop.y1 - crop.y0) * (raw_path_[i].y - dimensions.ypos) / dimensions.height) *
                        image_.height()
                ));
            }

            // Draw the polygon
            auto *mask = &tmp_mask_;
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
                } else {
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

void Rendering::EditMask::box_widget(Rect &dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(0)) {
        ImGuiIO &io = ImGui::GetIO();
        if (Widgets::check_hitbox(mouse_pos, dimensions) && !begin_action_ && !io.KeyCtrl) {
            begin_action_ = true;
            last_mouse_pos_ = mouse_pos;
        }
        if (begin_action_) {
            ImGui::GetForegroundDrawList()->AddRect(last_mouse_pos_, mouse_pos, ImColor(1.f, 0.7f, 0.7f, 0.5f), 0,
                                                    0,
                                                    2);
        }
    }
    if (ImGui::IsMouseReleased(0)) {
        begin_action_ = false;
    }
}

void Rendering::EditMask::brush_widget(Rect &dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    if (Widgets::check_hitbox(mouse_pos, dimensions)) {
        ImGuiIO &io = ImGui::GetIO();

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
        float brush_size = brush_size_ * dimensions.width / (float) image_.width() / 2.f / (crop.x1 - crop.x0);

        // Draw the brush
        ImGui::GetForegroundDrawList()->AddCircle(mouse_pos, brush_size, ImColor(1.f, 0.7f, 0.7f, 0.5f),
                                                  brush_size / 4 + 12, 2);

        // Apply brush to mask
        if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1)) && !io.KeyCtrl) {
            bool left_click = ImGui::IsMouseDown(0);
            bool right_click = ImGui::IsMouseDown(1);
            if (mouse_pos.x != last_mouse_pos_.x || mouse_pos.y != last_mouse_pos_.y) {
                float distance =
                        pow((mouse_pos.x - last_mouse_pos_.x), 2) + pow((mouse_pos.y - last_mouse_pos_.y), 2);

                std::vector<ImVec2> positions;

                if (pow(brush_size / 2.f, 2) < distance && begin_action_) {
                    int num = (int) sqrt(distance) / brush_size * 2.f;
                    for (int i = 0; i < num; i++) {
                        positions.push_back(ImVec2(
                                last_mouse_pos_.x + (mouse_pos.x - last_mouse_pos_.x) * i / (float) num,
                                last_mouse_pos_.y + (mouse_pos.y - last_mouse_pos_.y) * i / (float) num
                        ));
                    }
                } else {
                    positions.push_back(mouse_pos);
                }
                last_mouse_pos_ = mouse_pos;

                ::core::segmentation::Mask mask(tmp_mask_.rows(), tmp_mask_.cols());

                for (auto &pos: positions) {
                    Crop crop = image_widget_.getCrop();
                    ImVec2 corrected_mouse_pos = {
                            (crop.x0 + (crop.x1 - crop.x0) * (pos.x - dimensions.xpos) / dimensions.width) *
                            image_.width(),
                            (crop.y0 + (crop.y1 - crop.y0) * (pos.y - dimensions.ypos) / dimensions.height) *
                            image_.height()
                    };
                    ::core::segmentation::brushToMask(brush_size_ / 2.f, corrected_mouse_pos, mask, 1);
                }
                editTmpMaskAreaFromClick(mask, left_click, right_click);
                reset_image_ = true;
                begin_action_ = true;
            }
        }
    }
    if ((ImGui::IsMouseReleased(0) || ImGui::IsMouseReleased(1)) && begin_action_) {
        begin_action_ = false;
        set_mask();
    }
}

void Rendering::EditMask::automaticBrushBorders() {
    std::vector<std::pair<int, int>> mask_border_coordinates;
    auto tmp_mask_matrix = tmp_mask_.getData();
    for (int row_index = 0; row_index < tmp_mask_.rows(); row_index++) {
        for (int col_index = 0; col_index < tmp_mask_.cols(); col_index++) {
            if (tmp_mask_matrix.at<uchar>(row_index, col_index) == 1) {
                bool is_border = false;
                //left
                if (col_index - 1 >= 0 && tmp_mask_matrix.at<uchar>(row_index, col_index - 1) == 0) {
                    is_border = true;
                }
                //right
                if (col_index + 1 < tmp_mask_.cols() && tmp_mask_matrix.at<uchar>(row_index, col_index + 1) == 0) {
                    is_border = true;
                }
                //up
                if (row_index - 1 >= 0 && tmp_mask_matrix.at<uchar>(row_index - 1, col_index) == 0) {
                    is_border = true;
                }
                //down
                if (row_index + 1 < tmp_mask_.rows() && tmp_mask_matrix.at<uchar>(row_index + 1, col_index) == 0) {
                    is_border = true;
                }
                if (is_border) {
                    mask_border_coordinates.push_back(std::pair<int, int>(row_index, col_index));
                }
            }

        }
    }
    for (auto border_coordinate: mask_border_coordinates) {
        core::segmentation::Mask edition_mask(tmp_mask_.rows(), tmp_mask_.cols());
        auto position = ImVec2{static_cast<float>(border_coordinate.second),
                               static_cast<float>(border_coordinate.first)};
        ::core::segmentation::brushToMask(3, position, edition_mask, 1);
        editTmpMaskArea(edition_mask, true, true, true);
    }
    reset_image_ = true;
    set_mask();
}

void Rendering::EditMask::editTmpMaskAreaFromClick(const core::segmentation::Mask &area_to_edit, bool left_click,
                                                   bool right_click) {
    bool add = false;
    bool remove = false;
    if (limit_hu == 1 && both_add_and_remove) {
        add = true;
        remove = true;
    } else {
        add = left_click;
        remove = right_click;
    }

    editTmpMaskArea(area_to_edit, add, remove, false);

}

void Rendering::EditMask::editTmpMaskArea(const core::segmentation::Mask &area_to_edit, bool add, bool remove,
                                          bool only_if_threshold_used) {
    if (add) {
        core::segmentation::Mask area_to_add = area_to_edit.copy();
        area_to_add.intersect_with(thresholded_hu_);
        bool threshold_used = !area_to_add.isEqualTo(area_to_edit);
        if (!only_if_threshold_used || threshold_used) {
            tmp_mask_.union_with(area_to_add);
        }
    }

    if (remove) {
        core::segmentation::Mask area_to_remove = area_to_edit.copy();
        area_to_remove.difference_with(thresholded_hu_);
        bool threshold_used = !area_to_remove.isEqualTo(area_to_edit);
        if (!only_if_threshold_used || threshold_used)
            tmp_mask_.difference_with(area_to_remove);
    }
}