#include "new_segmentation.h"

#include "rendering/ui/modales/error_message.h"

void Rendering::NewSegmentationModal::showModal(const std::shared_ptr<::core::project::Project>& project) {
    name_ = "";
    description_ = "";
    confirm_ = false;

    draw_fct = [project = project_, this](bool& show, bool& enter, bool& escape) {

        KeyboardShortCut::ignoreNormalShortcuts();

        ImGui::Text("Segmentation name:");
        ImGui::SameLine();
        ImGui::InputText("##new_seg_name", &name_);

        //ImGui::Text("Segmentation description:");
        //ImGui::InputTextMultiline("##new_seg_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

        //ImGui::ColorEdit4("color of the masks", color_);
        ImGui::Text("Overlay color (mask color):");
        ImGui::SameLine();
        ImGui::ColorButton("overlay color (with alpha)", color, ImGuiColorEditFlags_AlphaPreviewHalf);
        if (ImGui::BeginPopupContextItem("Change color", 0)) {
            ImGui::ColorPicker4("color", color_, ImGuiColorEditFlags_AlphaPreviewHalf);
            color = { color_[0], color_[1], color_[2], color_[3] };
            ImGui::EndPopup();
        }
        

        if (escape) {
            show = false;
        }
        if (ImGui::Button("Create segmentation") || enter) {
            enter = false;
            if (name_.empty()) {
                show_error_modal("New segmentation error", "Please enter a name for the segmentation", "");
            }
            else {
                std::string err = project_manager_.getCurrentProject()->addSegmentation(std::make_shared<::core::segmentation::Segmentation>(
                    name_,
                    description_, 
                    ImVec4(color_[0], color_[1], color_[2], color_[3])
                ));
                if (err.empty()) {
                    project_manager_.getCurrentProject()->saveSegmentations();
                    name_ = "";
                    description_ = "";
                    show = false;
                }
                else {
                    show_error_modal("Error occured when saving", err);
                }
            }
        }
    };

    Modals::getInstance().setModal("New segmentation", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
}
