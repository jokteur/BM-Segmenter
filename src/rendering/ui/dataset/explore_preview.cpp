#include "explore_preview.h"

Rendering::ExplorerPreview::ExplorerPreview(ImVec2 init_size) : init_size_(init_size) {
    explore_tree_listener_.callback = [=](Event_ptr &event) {
        auto explorer = reinterpret_cast<::core::dataset::ExplorerBuildEvent*>(event.get());
        cases_.clear();
        dicom_previews_.clear();
        cases_ = explorer->getCases();
        for (auto &patient : cases_) {
            for (auto &study : patient.study) {
                for (auto &series : study.series) {
                    auto ret = dicom_previews_.emplace(std::make_pair(&series, DicomPreview()));
                    (*ret.first).second.loadSeries(series);
                }
            }
        }
    };
    explore_tree_listener_.filter = "dataset/explorer/build";

    EventQueue::getInstance().subscribe(&explore_tree_listener_);
}

Rendering::ExplorerPreview::~ExplorerPreview() {
    EventQueue::getInstance().unsubscribe(&explore_tree_listener_);
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
                    ImGui::Text("%s", study.description.c_str());
                    dicom_previews_[&series].setSize(ImVec2(width, width));
                    dicom_previews_[&series].ImGuiDraw(window, parent_dimension);
                    ImGui::Separator();
                    ImGui::NextColumn();
                }
            }
        }
    }
    ImGui::End();
}
