#ifndef BM_SEGMENTER_NEW_PROJECT_H
#define BM_SEGMENTER_NEW_PROJECT_H

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"
#include "rendering/ui/modales/error_message.h"

#include "core/project/project_manager.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/keyboard_shortcuts.h"

namespace Rendering {
    class NewProjectModal {
    private:
        ProjectManager &project_manager_;
        std::string name_;
        std::string description_;

        bool confirm = false;

        modal_fct draw_fct = [this] (bool &show, bool &enter, bool &escape) {
            Shortcut shortcut;
            shortcut.keys = {KEY_ENTER, CMD_KEY};
            shortcut.name = "confirm";
            shortcut.callback = [this] {
                confirm = true;
            };

            KeyboardShortCut::addTempShortcut(shortcut);
            KeyboardShortCut::ignoreNormalShortcuts();

            ImGui::Text("Project name:");
            ImGui::SameLine();
            ImGui::InputText("##new_project_name", &name_);

            ImGui::Text("Project description:");
            ImGui::InputTextMultiline("##new_project_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

            if (ImGui::Button("Create project") || confirm) {
                if (name_.empty()) {
                    show_error_modal("New project error", "Cannot create project with an empty name", "");
                }
                else {
                    auto project = project_manager_.newProject(name_, description_);
                    project_manager_.setCurrentProject(project);
                    name_ = "";
                    description_ = "";
                    show = false;
                }
                confirm = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || escape) {
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
