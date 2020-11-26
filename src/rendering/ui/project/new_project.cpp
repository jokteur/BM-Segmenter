#include "new_project.h"

#include "rendering/views/project_view.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/keyboard_shortcuts.h"
#include "rendering/ui/widgets/util.h"

#include "nfd.h"

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

            if (ImGui::Button("Set project folder")) {
                NFD_Init();
                nfdchar_t* outPath;
                nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
                if (result == NFD_OKAY) {
                    out_path_ = outPath;
                }
                NFD_Quit();
            }
            ImGui::SameLine();
            Widgets::HelpMarker("A new folder with the name of project will be created.\n"
                "All data and models will be saved inside this folder");
            if (!out_path_.empty()) {
                ImGui::Text("Project folder: \"%s\"", out_path_.c_str());
            }

            ImGui::Text("Project name:");
            ImGui::SameLine();
            ImGui::InputText("##new_project_name", &name_);

            ImGui::Text("Project description:");
            ImGui::InputTextMultiline("##new_project_description", &description_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);



            if (ImGui::Button("Create project") || confirm_) {
                if (name_.empty()) {
                    show_error_modal("New project error", "Cannot create project with an empty name", "");
                }
                else if (out_path_.empty()) {
                    show_error_modal("New project error", "A project folder has to be set.");
                }
                else {
                    auto project = project_manager_.newProject(name_, description_);
                    bool proceed = project->setUpWorkspace(out_path_, project->getName(), STRING(PROJECT_EXTENSION), out_path_);

                    if (proceed) {
                        project_manager_.setCurrentProject(project);

                        proceed = project_manager_.saveProjectToFile(project, out_path_);
                        if (!proceed) {
                            show_error_modal("New project error", "Error when saving '" + project->getName() + "', please try again");
                        }
                        else {
                            Settings::getInstance().addRecentFile(out_path_);
                            // Set project view
                            EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                            name_ = "";
                            description_ = "";
                            show = false;
                        }

                    }
                    else {
                        show_error_modal("New project error", "An error happened when setting up the project workspace", out_path_);
                    }
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