#include "explore_preview.h"
#include "ui/widgets/util.h"

Rendering::ExplorerPreview::ExplorerPreview(ImVec2 init_size) : init_size_(init_size) {
    build_tree_listener_.callback = [=](Event_ptr &event) {
        auto explorer = reinterpret_cast<::core::dataset::ExplorerBuildEvent*>(event.get());
        cases_.clear();
        dicom_previews_.clear();
        cases_ = explorer->getCases();
        for (auto &patient : cases_) {
            for (auto &study : patient.study) {
                for (auto &series : study.series) {
                    auto ret = dicom_previews_.emplace(std::make_pair(&series, DicomPreview()));
                    ::core::dataset::Case case_ = {
                            patient.ID,         // Patient ID, can be a number, of some sort
                            study.date,         // Date of study
                            study.time,         // Time of study
                            study.description,  // Description of the study
                            series.number,      // Number that identifies the series in the study
                            series.modality,    // Modality of the series (CT, MR, etc.)
                    };
                    (*ret.first).second.loadSeries(::core::dataset::SeriesPayload{series, case_});
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

    EventQueue::getInstance().subscribe(&build_tree_listener_);
    EventQueue::getInstance().subscribe(&filter_tree_listener_);
}

Rendering::ExplorerPreview::~ExplorerPreview() {
    EventQueue::getInstance().unsubscribe(&build_tree_listener_);
    EventQueue::getInstance().unsubscribe(&filter_tree_listener_);
}

void Rendering::ExplorerPreview::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    ImGui::Begin("Exploration preview", &open_);
    if (just_opened_) {
        just_opened_ = false;
        ImGui::SetWindowSize(init_size_);
    }
    if (!cases_.empty()) {
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
        ImGui::Separator();

        ImGui::BeginChild("explore_preview_scroll");

        ImVec2 content = ImGui::GetContentRegionAvail();
        ImVec2 window_pos = ImGui::GetWindowPos();
        Rect sub_window_dim = {window_pos.x, window_pos.y, content.x, content.y};

        ImVec2 mouse_pos = ImGui::GetMousePos();

        ImGui::Columns(num_cols_);
        float width = ImGui::GetContentRegionAvailWidth();
        for (auto &patient : cases_) {
            if (patient.tree_count < 4)
                continue;
            for (auto &study : patient.study) {
                if (study.tree_count < 3)
                    continue;
                for (auto &series : study.series) {
                    if (series.tree_count < 2)
                        continue;
                    ImGui::Text("%s", patient.ID.c_str());
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Study: %s\nSeries: %s\nModality: %s", study.description.c_str(), series.number.c_str(), series.modality.c_str());
                    }
                    dicom_previews_[&series].setSize(ImVec2(width, width));
                    if (Widgets::check_hitbox(mouse_pos, sub_window_dim)) {
                        dicom_previews_[&series].setAllowScroll(true);
                    }
                    else {
                        dicom_previews_[&series].setAllowScroll(false);
                    }
                    dicom_previews_[&series].ImGuiDraw(window, parent_dimension);
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
