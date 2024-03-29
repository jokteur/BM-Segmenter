#include "dataset_view.h"
#include "log.h"

#include <algorithm>

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"
#include "views.h"
#include "rendering/views/project_view.h"


namespace py = pybind11;

Rendering::DatasetView::DatasetView() {
	auto& project = project_manager_.getCurrentProject();

    validated_.setImage("assets/validated_dark.png");
    edited_.setImage("assets/edited_dark.png");

    // Mainly used for changing colors
    reload_seg_.callback = [=](Event_ptr& event) {
        for (auto& preview : dicom_previews_) {
            preview.second.setSegmentation(active_seg_);
        }
    };
    reload_seg_.filter = "segmentation/reload";

    EventQueue::getInstance().subscribe(&reload_seg_);
}

Rendering::DatasetView::~DatasetView() {
    EventQueue::getInstance().unsubscribe(&reload_seg_);
}

void Rendering::DatasetView::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
	auto& project = project_manager_.getCurrentProject();
	ImGui::Begin("Dataset overview");

	if (project != nullptr) {
        // Set the dicoms and the preview
        if (project->getDataset().getDicoms().size() != dicoms_.size()) {
            dicoms_ = project->getDataset().getDicoms();
            dicom_previews_.clear();
            for (auto& dicom : dicoms_) {
                dicom_previews_.emplace(std::move(std::make_pair(dicom, Preview(validated_, edited_))));
                dicom_sizes_.emplace(std::make_pair(dicom, Rect(ImVec2(-10000, -10000), ImVec2(0, 0))));
            }
            for (auto &dic : dicom_previews_) {
                dic.second.setSeries(dic.first);
            }
        }

        // Interaction for the column viewing
        {
            if (dicom_previews_.size() < num_cols_ && !dicom_previews_.empty()) {
                num_cols_ = dicom_previews_.size();
            }
            ImGui::Text("Num. columns: %d", num_cols_);
            ImGui::SameLine();
            if (ImGui::Button(" -###explorer_preview_button_minus")) {
                if (num_cols_ > 1)
                    num_cols_--;
                BM_DEBUG("add one column (layout)");
            }
            ImGui::SameLine();
            if (ImGui::Button("+###explorer_preview_button_plus")) {
                if (num_cols_ < 5)
                    num_cols_++;
                BM_DEBUG("remove one column (layout)");
            }
        }

        // Show segmentation selection
        {
            auto& segs = project->getSegmentations();
            if (segs.size() != seg_map_.size()) {
                std::vector<std::string> names;
                names.push_back("Select segmentation");
                seg_map_.clear();
                int n = 0;
                for (auto& seg : segs) {
                    names.push_back(seg->getName());
                    seg_map_[n] = seg;
                    n++;
                }
                seg_select_.setOptions(names, [=](int idx) {
                    if (idx == 0) {
                        EventQueue::getInstance().post(Event_ptr(new ::core::segmentation::SelectSegmentationEvent(nullptr)));
                        active_seg_ = nullptr;
                        for (auto& preview : dicom_previews_) {
                            preview.second.setSegmentation(nullptr);
                        }
                    }
                    else {
                        EventQueue::getInstance().post(Event_ptr(new ::core::segmentation::SelectSegmentationEvent(seg_map_.at(idx - 1))));
                        active_seg_ = seg_map_.at(idx - 1);
                        for (auto& preview : dicom_previews_) {
                            preview.second.setSegmentation(active_seg_);
                        }
                    }
                });
                BM_DEBUG("generate segmentation list");
            }
            seg_select_.ImGuiDraw("Select segmentation");

            if (active_seg_ != nullptr) {
                if (ImGui::Button("Unvalidate all")) {
                    unvalidate_confirm_prompt = true;
                }
                if (unvalidate_confirm_prompt) {
                    ImGui::SameLine();
                    ImGui::Text("Are you sure ?");
                    ImGui::SameLine();
                    if (ImGui::Button("yes")) {
                        unvalidate_confirm_prompt = false;
                        for (auto &dicom: dicoms_) {
                            const std::shared_ptr<core::segmentation::MaskCollection> &mask_collection = active_seg_->getMask(
                                    dicom);
                            mask_collection->loadData(true);
                            mask_collection->removeAllValidatedBy();
                            mask_collection->saveCollection();
                            EventQueue::getInstance().post(Event_ptr(new Event("mask/changed/" + dicom->getId())));
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("no")) {
                        unvalidate_confirm_prompt = false;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Predict from ML (when finished the program will close, you will have to restart it)")) {
                    auto state = PyGILState_Ensure();
                    auto predict_module = py::module::import("python.scripts.predict");
                    predict_module.attr("predict")(project->getRoot(), active_seg_->getName());
                    std::exit(0);
                }
            }
        }

        // Show group selection
        {
            if (project->getDataset().getGroups().size() != groups_.size()) {
                groups_ = project->getDataset().getGroups();
                std::vector<std::string> names;
                names.push_back("Show all"); 
                for (auto& group : groups_) {
                    names.push_back(group.getName());
                }
                group_select_.setOptions(names, [=](int idx) { 
                    group_idx_ = idx; 
                    if (idx == 0) {
                        EventQueue::getInstance().post(Event_ptr(new Event("dataset/group/select/all")));
                        reset_draw_ = true;
                        BM_DEBUG("Select Show all dicoms");
                    }
                    else {
                        EventQueue::getInstance().post(Event_ptr(new Event("dataset/group/select/" + std::to_string(idx - 1))));
                        reset_draw_ = true;

                        // Unload all dicoms that are not in the group
                        auto group_dicoms = groups_[idx - 1].getOrderedDicoms();
                        auto all_dicoms = project->getDataset().getOrderedDicoms();

                        std::sort(group_dicoms.begin(), group_dicoms.end());
                        std::sort(all_dicoms.begin(), all_dicoms.end());

                        std::vector<std::shared_ptr<::core::DicomSeries>> to_unload;
                        std::set_difference(all_dicoms.begin(), all_dicoms.end(), group_dicoms.begin(), group_dicoms.end(), std::back_inserter(to_unload));
                        for (auto dicom : to_unload) {
                            dicom_previews_[dicom].unload();
                        }
                        BM_DEBUG("Select group " + groups_[idx - 1].getName());
                    }
                });
                BM_DEBUG("generate group selection options");
            }
            group_select_.ImGuiDraw("Select group");
        }
        ImGui::Separator();

        ImGui::BeginChild("Dataset_child_view");
        ImVec2 content = ImGui::GetContentRegionAvail();
        ImVec2 window_pos = ImGui::GetWindowPos();
        Rect sub_window_dim(window_pos, content);

        auto& io = ImGui::GetIO();

        if (prev_window_dim_.xpos != sub_window_dim.xpos
            || prev_window_dim_.ypos != sub_window_dim.ypos
            || prev_window_dim_.width != sub_window_dim.width
            || prev_window_dim_.height != prev_window_dim_.height) {
            prev_window_dim_ = sub_window_dim;
            reset_draw_ = true;
            calc_height = true;
        }
        if (io.MouseWheel && Widgets::check_hitbox(ImGui::GetMousePos(), sub_window_dim)) {
            reset_draw_ = true;
        }
        ImVec2 mouse_pos = ImGui::GetMousePos();

        ImGui::Columns(num_cols_);
        float width = ImGui::GetContentRegionAvail().x;
        // Show all cases
        col_count_ = 0;
        if (group_idx_ == 0) {
            for (auto& dicom : dicoms_) {
                 preview_widget(dicom_previews_[dicom], width, mouse_pos, sub_window_dim, dicom, window, parent_dimension);
            }
        }
        else {
            for (auto& dicom : groups_[group_idx_ - 1].getOrderedDicoms()) {
                preview_widget(dicom_previews_[dicom], width, mouse_pos, sub_window_dim, dicom, window, parent_dimension);
            }
        }
        reset_draw_ = false;
        ImGui::Columns(1);
        ImGui::EndChild();
	}
	ImGui::End();
}

inline void Rendering::DatasetView::preview_widget(Preview& preview, float width, ImVec2 mouse_pos, Rect sub_window_dim, std::shared_ptr<::core::DicomSeries> dicom, GLFWwindow* window, Rect& parent_dimension) {
    if (Widgets::check_hitbox(mouse_pos, sub_window_dim)) {
        preview.setAllowScroll(true);
    }
    else {
        preview.setAllowScroll(false);
    }

    auto& dim = preview.getDimensions();
    float margin = 3 * width;
    float margin_less = 2 * width;

    bool draw = dim.ypos - sub_window_dim.ypos < sub_window_dim.height + margin && dim.ypos - sub_window_dim.ypos > -margin_less;
    if (draw)
        preview.load();
    else if (dim.ypos - sub_window_dim.ypos >= sub_window_dim.height + margin || dim.ypos - sub_window_dim.ypos <= -margin)
        preview.unload();

    // Image widget
    float widget_size = width * 0.98;
    preview.setSize(ImVec2(widget_size, widget_size));

    if (col_count_ % num_cols_ == 0)
        ImGui::Separator();

    // Title
    if (draw) {
        ImGui::Text("%s", dicom->getIdPair().first.c_str());
        if (active_seg_ != nullptr) {
            auto state = preview.getMaskState();
            switch (state) {
            case Preview::VALIDATED:
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.f, 0.8f, 0.0f, 1.f), "(V)");
                break;
            case Preview::PREDICTED:
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.3f, 0.4f, 0.7f, 1.f), "(P)");
                break;
            case Preview::CURRENT:
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.5f, 0.0f, 1.f), "(E)");
                break;
            }
        }
        preview.setNoDraw(false);
        preview.ImGuiDraw(window, parent_dimension);
    }
    else {
        ImGui::Text("");
        preview.setNoDraw(true);
        preview.ImGuiDraw(window, parent_dimension);
    }

    col_count_++;
    ImGui::NextColumn();
}