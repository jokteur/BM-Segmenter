#pragma once

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/keyboard_shortcuts.h"
#include "rendering/views/explore_view.h"

#include "core/project/project_manager.h"
#include "events.h"


namespace Rendering {
    class NewProjectModal {
    private:
        ::core::project::ProjectManager &project_manager_;
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

                    // Set explore view
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ExploreView>())));
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
        NewProjectModal() : project_manager_(::core::project::ProjectManager::getInstance()) {}

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal() {
           Modals::getInstance().setModal("New project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
        }
    };
}

