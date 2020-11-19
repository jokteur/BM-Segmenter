#include "close_project_modal.h"
#include "rendering/ui/modales/error_message.h"
#include "nfd.h"
#include "settings.h"

Rendering::CloseProjectModal::CloseProjectModal()
    : project_manager_(::core::project::ProjectManager::getInstance()) {
}

void Rendering::CloseProjectModal::setProject(const std::shared_ptr<::core::project::Project>& project) {
    project_ = project;

    draw_fct = [project = project_, this](bool& show, bool& enter, bool& escape) {

        if (project->isSaved()) {
            show = false;
            project_manager_.removeProject(project);
            return;
        }

        ImGui::Text("You are closing ' %s '", project->getName().c_str());
        ImGui::Text("The project has not been saved.");
        ImGui::Dummy(ImVec2(30, 30));
        if (ImGui::Button("Close and save")) {
            std::string out_path, str_err;
            NFD_Init();
            nfdchar_t* outPath;
            bool proceed = false;
            if (project->getSaveFile().empty()) {
                nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
                if (result == NFD_OKAY) {
                    out_path = outPath;
                    proceed = project->setUpWorkspace(out_path, project->getName(), STRING(PROJECT_EXTENSION), out_path);
                }
            }
            else {
                proceed = true;
                out_path = project->getSaveFile();
            }

            if (proceed) {
                bool result = project_manager_.saveProjectToFile(project, out_path);
                if (!result) {
                    show_error_modal("Error", "Error when saving '" + project->getName() + "', please try again");
                }
                else {
                    Settings::getInstance().addRecentFile(project->getSaveFile());
                    Settings::getInstance().saveSettings();
                    show = false;
                    project_manager_.removeProject(project);
                }
            }
            else {
                show_error_modal("Could not save project",
                    "The program failed to set up the workspace at the given path",
                    str_err);
            }
            NFD_Quit();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close without saving")) {
            show = false;
            project_manager_.removeProject(project);
        }
    };
}

void Rendering::CloseProjectModal::showModal() {
    Modals::getInstance().setModal("Close project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
}
