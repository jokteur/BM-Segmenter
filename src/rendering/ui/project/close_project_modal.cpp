#include "close_project_modal.h"
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
            if (project->getSaveFile().empty()) {
                std::string out_path;

                nfdchar_t* outPath = nullptr;
                nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };

                nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
                if (result == NFD_OKAY) {
                    out_path = outPath;
                    project_manager_.saveProjectToFile(project, out_path);
                    Settings::getInstance().addRecentFile(out_path);
                    Settings::getInstance().saveSettings();
                    show = false;
                    project_manager_.removeProject(project);
                }
            }
            else {
                show = false;
                project_manager_.saveProjectToFile(project, project->getSaveFile());
                project_manager_.removeProject(project);
            }
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
