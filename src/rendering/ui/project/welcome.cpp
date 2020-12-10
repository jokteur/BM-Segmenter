#include "welcome.h"

#include "rendering/ui/modales/error_message.h"
#include "rendering/views/project_view.h"

#include "log.h"

namespace Rendering {
    void WelcomeView::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
        ImGui::Begin("Welcome page");
        ImVec2 content = ImGui::GetContentRegionAvail();

        ImGui::Text("What do you want to do ?");
        if (ImGui::Button("Create a new project")) {
            BM_DEBUG("Create new project modal");
            new_project_modal_.showModal();
        }
        ImGui::SameLine();
        if (ImGui::Button("Open an existing project")) {
            BM_DEBUG("Open project");
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
                    BM_DEBUG("Project loaded");
                }
                catch (std::exception& e) {
                    show_error_modal("Load project error",
                        "An error occured when loading the project ''\n",
                        e.what());
                    BM_DEBUG((std::string("Load project error: ") + e.what()));
                }
            }
        }
        ImGui::Separator();
        ImGui::Text("Recent projects");
        for (auto& filename : Settings::getInstance().getRecentFiles()) {
            if (ImGui::Selectable(filename.c_str())) {
                BM_DEBUG("Open project from recent files");
                try {
                    auto project = project_manager_.openProjectFromFile(filename);
                    project_manager_.setCurrentProject(project);
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                    BM_DEBUG("Create project view");
                }
                catch (std::exception& e) {
                    Settings::getInstance().removeRecentFile(filename);
                    Settings::getInstance().saveSettings();
                    show_error_modal("Load project error",
                        "An error occured when loading the project ''\n",
                        e.what());
                    BM_DEBUG(std::string("From recent file, load project error: ") + e.what());
                }
            }
            if (ImGui::BeginPopupContextItem(filename.c_str())) {
                if (ImGui::Selectable("Remove project from recent files")) {
                    Settings::getInstance().removeRecentFile(filename);
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }
}