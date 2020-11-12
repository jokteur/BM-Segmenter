#include "new_project.h"

#include "rendering/views/project_view.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/keyboard_shortcuts.h"

namespace Rendering {
	NewProjectModal::NewProjectModal() : project_manager_(::core::project::ProjectManager::getInstance()) {
        draw_fct = [this](bool& show, bool& enter, bool& escape) {
            //            bool& confirm = confirm_;
            Shortcut shortcut;
            shortcut.keys = { KEY_ENTER, CMD_KEY };
            shortcut.name = "confirm";
            shortcut.callback = [this] {
                confirm_ = true;
            };

            KeyboardShortCut::addTempShortcut(shortcut);
            KeyboardShortCut::ignoreNormalShortcuts();

            ImGui::Text("Project name:");
            ImGui::SameLine();
            ImGui::InputText("##new_project_name", &name_);

            ImGui::Text("Project description:");
            ImGui::InputTextMultiline("##new_project_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

            if (ImGui::Button("Create project") || confirm_) {
                if (name_.empty()) {
                    show_error_modal("New project error", "Cannot create project with an empty name", "");
                }
                else {
                    auto project = project_manager_.newProject(name_, description_);
                    project_manager_.setCurrentProject(project);

                    // Set project view
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                    name_ = "";
                    description_ = "";
                    show = false;
                }
                confirm_ = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || escape) {
                name_ = "";
                description_ = "";
                show = false;
            }
        };
	}
    void NewProjectModal::showModal() {
        Modals::getInstance().setModal("New project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
    }
}