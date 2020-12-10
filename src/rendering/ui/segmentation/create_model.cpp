#include "create_model.h"

#include "core/segmentation/ml.h"

#include "rendering/animation_util.h"
#include "rendering/ui/modales/error_message.h"

Rendering::CreateModelModal::CreateModelModal() : project_manager_(core::project::ProjectManager::getInstance()) {
    std::vector<std::string> options = { "128x128", "256x256", "312x312", "512x512", "758x758", "1024x1024" };
    sizes_select_.setOptions(options, [=] (int idx) {
        switch (idx) {
        case 0:
            input_size_ = 128;
            break;
        case 1:
            input_size_ = 256;
            break;
        case 2:
            input_size_ = 312;
            break;
        case 3:
            input_size_ = 512;
            break;
        case 4:
            input_size_ = 758;
            break;
        case 5:
            input_size_ = 1024;
            break;
        };
    });
    sizes_select_.setIdx(3);
}

void Rendering::CreateModelModal::showModal(std::shared_ptr<::core::segmentation::Segmentation> segmentation) {
    num_layers_ = 3;
    ww_ = 400;
    wc_ = 40;
    input_size_ = 512;
    sizes_select_.setIdx(3);

    draw_fct = [project = project_, this](bool& show, bool& enter, bool& escape) {

        KeyboardShortCut::ignoreNormalShortcuts();

        ImGui::Text("Modal name:");
        ImGui::SameLine();
        ImGui::InputText("##new_seg_name", &name_);

        ImGui::Text("Model size");
        sizes_select_.ImGuiDraw("Select input/output size");
        ImGui::SliderInt("Number of upscaling layers", &num_layers_, 1, 6);

        if (escape) {
            show = false;
        }
        if (ImGui::Button("Create model") || enter) {
            enter = false;
            if (name_.empty()) {
                show_error_modal("New model error", "Please enter a name for the model", "");
            }
            else {
                std::make_shared<::core::segmentation::ML_Model>(name_, ww_, wc_, input_size_);
                push_animation();
                show = false;
            }
        }
    };

    Modals::getInstance().setModal("New model", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
}
