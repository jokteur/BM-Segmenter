#pragma once

#include "nfd.h"
#include "first_include.h"

#include <utility>
#include "events.h"
#include "imgui.h"

#include "rendering/drawables.h"
#include "core/project/project_manager.h"
#include "settings.h"
#include "rendering/ui/modales/new_project.h"

#include "jobscheduler.h"

namespace Rendering {
    class MainMenuBar : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_;
        EventQueue& event_queue_;
        Settings& settings_;
        Listener shortcuts_listener_;

        std::string error_msg;

        modal_fct error_fct = [this] (bool &show, bool &escape, bool &enter) {
            ImGui::Text("%s", error_msg.c_str());
            if(ImGui::Button("Ok") || escape || enter)
                show = false;
        };

        NewProjectModal new_project_modal_;

        void init_listeners();
        void destroy_listeners();

        void save_project();
        void save_project_under();
        // Called by either save_project or save_project_under()
        void save(const std::shared_ptr<::core::project::Project>& project, const std::string& filename);

        void file_menu();
        //void edit_menu();
        void projects_menu();
        void settings_menu();


        void open_file(std::string filename = "");
    public:
        MainMenuBar()
        : project_manager_(::core::project::ProjectManager::getInstance()),
            event_queue_(EventQueue::getInstance()),
            settings_(Settings::getInstance()) {
            init_listeners();
        }


        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        ~MainMenuBar() override { destroy_listeners(); }
    };
}
