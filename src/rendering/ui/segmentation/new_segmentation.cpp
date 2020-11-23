#include "new_segmentation.h"

#include "rendering/ui/modales/error_message.h"

void Rendering::NewSegmentationModal::showModal(const std::shared_ptr<::core::project::Project>& project) {
    name_ = "";
    description_ = "";
    confirm_ = false;

    draw_fct = [project = project_, this](bool& show, bool& enter, bool& escape) {

        Shortcut shortcut;
        shortcut.keys = { KEY_ENTER, CMD_KEY };
        shortcut.name = "confirm";
        shortcut.callback = [this] {
            confirm_ = true;
        };

        KeyboardShortCut::ignoreNormalShortcuts();

        ImGui::Text("Segmentation name:");
        ImGui::SameLine();
        ImGui::InputText("##new_seg_name", &name_);

        ImGui::Text("Segmentation description:");
        ImGui::InputTextMultiline("##new_seg_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

        if (escape) {
            show = false;
        }
        if (ImGui::Button("Create segmentation") || confirm_) {
            if (name_.empty()) {
                show_error_modal("New segmentation error", "Please enter a name for the segmentation", "");
            }
            else {
                std::string err = project_manager_.getCurrentProject()->addSegmentation(std::make_shared<::core::segmentation::Segmentation>(name_, description_));
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
