#include "dataset_view.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

void Rendering::DatasetView::drag_and_drop(std::shared_ptr<::core::DicomSeries> case_) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
        drag_and_drop.giveData(case_);

        int a = 0; // Dummy int
        ImGui::SetDragDropPayload("_DICOM_PAYLOAD", &a, sizeof(a));
        ImGui::Text("Drag %s", ::core::parse_dicom_id(case_->getId()).first.c_str());
        ImGui::EndDragDropSource();
    }
}


Rendering::DatasetView::DatasetView() {
	auto& project = project_manager_.getCurrentProject();
}

Rendering::DatasetView::~DatasetView() {
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
                dicom_previews_.emplace(std::move(std::make_pair(dicom, Preview())));
            }
            for (auto &dic : dicom_previews_) {
                dic.second.setSeries(dic.first);
            }
        }

        // Interaction for the column viewing
        if (dicom_previews_.size() < num_cols_ && !dicom_previews_.empty()) {
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

        // Show group selection
        if (std::equal(groups_.begin(), groups_.end(), project->getDataset().getGroups().begin())) {
            groups_ = project->getDataset().getGroups();
            group_names_.clear();
            group_names_.push_back("Show all");
            for (auto& group : groups_) {
                group_names_.push_back(group.getName().c_str());
            }
        }
        if (!groups_.empty()) {
            const char* combo_label = group_names_[item_select_];
            if (ImGui::BeginCombo("Select group", combo_label)) {
                int n = 0;
                for (int n = 0; n < groups_.size() + 1; n++) {
                    const bool is_selected = (item_select_ == n);
                    if (ImGui::Selectable(group_names_[n], is_selected))
                        item_select_ = n;

                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
        ImGui::Separator();

        ImGui::BeginChild("Dataset_child_view");
        ImVec2 content = ImGui::GetContentRegionAvail();
        ImVec2 window_pos = ImGui::GetWindowPos();
        Rect sub_window_dim = { window_pos.x, window_pos.y, content.x, content.y };
        ImVec2 mouse_pos = ImGui::GetMousePos();

        ImGui::Columns(num_cols_);
        float width = ImGui::GetContentRegionAvailWidth();
        // Show all cases
        if (item_select_ == 0) {
            for (auto& dicom : dicoms_) {
                preview_widget(dicom_previews_[dicom], width, mouse_pos, sub_window_dim, dicom, window, parent_dimension);
            }
        }
        else {
            for (auto& dicom : groups_[item_select_ - 1].getOrderedDicoms()) {
                preview_widget(dicom_previews_[dicom], width, mouse_pos, sub_window_dim, dicom, window, parent_dimension);
            }
        }
        ImGui::Columns(1);
        ImGui::EndChild();
	}
	ImGui::End();
}

void Rendering::DatasetView::preview_widget(Preview& preview, float width, ImVec2 mouse_pos, Rect sub_window_dim, std::shared_ptr<::core::DicomSeries> dicom, GLFWwindow* window, Rect& parent_dimension) {
    // Title
    auto pair = ::core::parse_dicom_id(dicom->getId().c_str());
    ImGui::Text("%s", pair.first.c_str());

    if (Widgets::check_hitbox(mouse_pos, sub_window_dim)) {
        //preview.setAllowScroll(true);
    }
    else {
        //preview.setAllowScroll(false);
    }

    auto& dim = preview.getDimensions();
    float margin = 3 * width;
    if (dim.ypos - sub_window_dim.ypos < sub_window_dim.height + margin && dim.ypos - sub_window_dim.ypos > -margin)
        preview.reload();
    else
        preview.unload();

    // Image widget
    preview.setSize(ImVec2(width * 0.98, width * 0.98));
    preview.ImGuiDraw(window, parent_dimension);
    drag_and_drop(dicom);
    ImGui::NextColumn();
}