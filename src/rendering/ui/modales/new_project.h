#ifndef BM_SEGMENTER_NEW_PROJECT_H
#define BM_SEGMENTER_NEW_PROJECT_H

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include "core/project/project_manager.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_stdlib.h"

namespace Rendering {
    class NewProjectModal {
    private:
        ProjectManager &project_manager_;
        std::string name_;
        std::string description_;

        modal_fct error_fct = [] (bool &show) {
            ImGui::Text("Cannot create project with an empty name");
            if(ImGui::Button("Ok"))
                show = false;
        };

        modal_fct draw_fct = [this] (bool &show) {

            ImGui::Text("Project name:");
            ImGui::SameLine();
            ImGui::InputText("##new_project_name", &name_);

            ImGui::Text("Project description:");
            ImGui::InputTextMultiline("##new_project_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

            if (ImGui::Button("Create project")) {
                if (name_.empty()) {
                    Modals::getInstance().stackModal("Error", error_fct, ImGuiWindowFlags_AlwaysAutoResize);
                }
                else {
                    auto project = project_manager_.newProject(name_, description_);
                    project_manager_.setCurrentProject(project);
                    name_ = "";
                    description_ = "";
                    show = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                name_ = "";
                description_ = "";
                show = false;
            }
        };

    public:
        NewProjectModal() : project_manager_(ProjectManager::getInstance()) {}

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal() {
           Modals::getInstance().setModal("New project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
        }
    };
}

#endif //BM_SEGMENTER_NEW_PROJECT_H
