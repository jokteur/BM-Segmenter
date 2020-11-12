#include "welcome.h"

#include "rendering/ui/modales/error_message.h"
#include "rendering/views/project_view.h"

namespace Rendering {
    void WelcomeView::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
        ImGui::Begin("Welcome page");
        ImVec2 content = ImGui::GetContentRegionAvail();

        ImGui::Text("What do you want to do ?");
        if (ImGui::Button("Create a new project")) {
            new_project_modal_.showModal();
        }
        ImGui::SameLine();
        if (ImGui::Button("Open an existing project")) {
            nfdchar_t* outPath;
            nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);

            bool success = true;
            if (result == NFD_ERROR) {
                show_error_modal("Load project error",
                    "Could not open project");
                success = false;
            }
            else if (result == NFD_CANCEL) {
                success = false;
            }
            if (success) {
                std::string filename = outPath;
                try {
                    auto project = project_manager_.openProjectFromFile(filename);
                    project_manager_.setCurrentProject(project);
                    Settings::getInstance().addRecentFile(filename);
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                }
                catch (std::exception& e) {
                    show_error_modal("Load project error",
                        "An error occured when loading the project ''\n",
                        e.what());
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Recent projects");
        for (auto& filename : Settings::getInstance().getRecentFiles()) {
            if (ImGui::Selectable(filename.c_str())) {
                try {
                    auto project = project_manager_.openProjectFromFile(filename);
                    project_manager_.setCurrentProject(project);
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                }
                catch (std::exception& e) {
                    Settings::getInstance().removeRecentFile(filename);
                    Settings::getInstance().saveSettings();
                    show_error_modal("Load project error",
                        "An error occured when loading the project ''\n",
                        e.what());
                }
            }
        }

        ImGui::End();
    }
}