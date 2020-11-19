#include "explore_preview.h"
#include "ui/widgets/util.h"
#include "settings.h"

Rendering::ExplorerPreview::ExplorerPreview(ImVec2 init_size) : init_size_(init_size) {
    build_tree_listener_.callback = [=](Event_ptr &event) {
        auto explorer = reinterpret_cast<::core::dataset::ExplorerBuildEvent*>(event.get());
        cases_ = explorer->getCases();
        is_cases_set_ = true;
        dicom_previews_.clear();
        for (auto &patient : *cases_) {
            for (auto &study : patient.study) {
                for (auto &series : study.series) {
                    auto ret = dicom_previews_.emplace(std::move(std::make_pair(series, DicomPreview())));
                    ::core::dataset::Case case_ = {
                            patient.ID,         // Patient ID, can be a number, of some sort
                            study.date,         // Date of study
                            study.time,         // Time of study
                            study.description,  // Description of the study
                            series->number,      // Number that identifies the series in the study
                            series->modality,    // Modality of the series (CT, MR, etc.)
                    };
                    (*ret.first).second.loadSeries(series, case_);
                }
            }
        }
    };
    build_tree_listener_.filter = "dataset/explorer/build";

    filter_tree_listener_.callback = [=](Event_ptr &event) {
        auto filters = reinterpret_cast<::core::dataset::ExplorerFilterEvent*>(event.get());
        ::core::dataset::build_tree(cases_, filters->caseFilter(), filters->studyFilter(), filters->seriesFilter());
    };
    filter_tree_listener_.filter = "dataset/explorer/filter";

    reset_tree_listener_.callback = [=](Event_ptr& event) {
        cases_ = std::make_shared<std::vector<core::dataset::PatientNode>>();
    };
    reset_tree_listener_.filter = "dataset/dicom/reset";

    EventQueue::getInstance().subscribe(&build_tree_listener_);
    EventQueue::getInstance().subscribe(&reset_tree_listener_);
    EventQueue::getInstance().subscribe(&filter_tree_listener_);
}

Rendering::ExplorerPreview::~ExplorerPreview() {
    EventQueue::getInstance().unsubscribe(&build_tree_listener_);
    EventQueue::getInstance().unsubscribe(&reset_tree_listener_);
    EventQueue::getInstance().unsubscribe(&filter_tree_listener_);
}

void Rendering::ExplorerPreview::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    ImGui::Begin("Exploration preview", &open_); // TODO: unique ID
    if (just_opened_) {
        just_opened_ = false;
        ImGui::SetWindowSize(init_size_);
    }
    if (is_cases_set_) {
        if (dicom_previews_.size() < num_cols_) {
            num_cols_ = dicom_previews_.size();
        }
        ImGui::Text("Num. columns: %d", num_cols_);
        ImGui::SameLine();
        if (ImGui::Button(" -###explorer_preview_button_minus")) {
            if (num_cols_ > 1)
                num_cols_--;
        }
        ImGui::SameLine();
        if (ImGui::Button("+###explorer_preview_button_plus")) {
            if (num_cols_ < 5)
                num_cols_++;
        }
        if (ImGui::CollapsingHeader("Edit all images###explore_preview_edit")) {
            ImGui::SameLine();
            Widgets::HelpMarker("Edit all images from the dataset at the same time.\n"
                                "These are non-destructive actions, you can change the parameters"
                                " even after the data has been imported.");
            ImGui::Text("Cropping");
            ImGui::DragFloatRange2("Crop in x", &crop_x_.x, &crop_x_.y, 1.f, 0.0f, 100.0f, "Left: %.1f %%", "Right: %.1f %%");
            ImGui::DragFloatRange2("Crop in y", &crop_y_.x, &crop_y_.y, 1.f, 0.0f, 100.0f, "Top: %.1f %%", "Bottom: %.1f %%");
            if (ImGui::Button("Apply crop###explore_apply_crop")) {
                for (auto& preview : dicom_previews_) {
                    preview.second.setCrop(crop_x_, crop_y_, false);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Force crop on all images")) {
                for (auto& preview : dicom_previews_) {
                    preview.second.setCrop(crop_x_, crop_y_, true);
                }
            }
            ImGui::Separator();
            ImGui::Text("Windowing: ");
            ImGui::DragInt("Window center###explore_preview_wc", &window_center_, 0.5, -1000, 3000, "%d HU");
            ImGui::DragInt("Window width###explore_preview_ww", &window_width_, 0.5, 1, 3000, "%d HU");
            if (ImGui::Button("Apply window###explore_apply_window")) {
                for (auto& preview : dicom_previews_) {
                    preview.second.setWindowing(window_width_, window_center_, false);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Force window on all images")) {
                for (auto& preview : dicom_previews_) {
                    preview.second.setWindowing(window_width_, window_center_, true);
                }
            }
        }
        ImGui::Separator();

        ImGui::BeginChild("explore_preview_scroll");

        ImVec2 content = ImGui::GetContentRegionAvail();
        ImVec2 window_pos = ImGui::GetWindowPos();
        Rect sub_window_dim = {window_pos.x, window_pos.y, content.x, content.y};

        ImVec2 mouse_pos = ImGui::GetMousePos();

        ImGui::Columns(num_cols_);
        float width = ImGui::GetContentRegionAvailWidth();
        auto disabled_color = Settings::getInstance().getColors().disabled_text;
        float y_cursor;
        for (auto &patient : *cases_) {
            if (patient.tree_count < 4)
                continue;
            for (auto &study : patient.study) {
                if (study.tree_count < 3)
                    continue;
                for (auto &series : study.series) {
                    if (series->tree_count < 2)
                        continue;
                    dicom_previews_[series].setSize(ImVec2(width*0.98, width*0.98));

                    bool is_active = patient.is_active && study.is_active && series->is_active;
                    if (!is_active) {
                        ImGui::PushStyleColor(ImGuiCol_Text, disabled_color);
                        dicom_previews_[series].setIsDisabled(true);
                    }
                    else {
                        dicom_previews_[series].setIsDisabled(false);
                    }

                    if (dicom_previews_[series].isLocked()) {
                        ImGui::Text("%s *", patient.ID.c_str());
                    }
                    else {
                        ImGui::Text("%s", patient.ID.c_str());
                    }

                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Study: %s\nSeries: %s\nModality: %s", study.description.c_str(), series->number.c_str(), series->modality.c_str());
                    }

                    if (Widgets::check_hitbox(mouse_pos, sub_window_dim)) {
                        dicom_previews_[series].setAllowScroll(true);
                    }
                    else {
                        dicom_previews_[series].setAllowScroll(false);
                    }
                    dicom_previews_[series].ImGuiDraw(window, parent_dimension);

                    if (!is_active) {
                        ImGui::PopStyleColor();
                    }
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
