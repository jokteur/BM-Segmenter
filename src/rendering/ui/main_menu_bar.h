#pragma once

#include "first_include.h"

#include <utility>
#include "events.h"
#include "imgui.h"
#include "nfd.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/ui/project/new_project.h"
#include "rendering/ui/project/close_project_modal.h"

#include "core/project/project_manager.h"

#include "settings.h"
#include "jobscheduler.h"

namespace Rendering {
    class MainMenuBar : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_;
        EventQueue& event_queue_;
        Settings& settings_;
        Listener shortcuts_listener_;

        std::string error_msg;

        modal_fct error_fct;

        NewProjectModal new_project_modal_;
        int num_projects = 0;
        bool close_projects_ = false;
        bool show_modal_ = false;
        CloseProjectModal close_project_modal_;

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
        MainMenuBar();


        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        ~MainMenuBar() override { destroy_listeners(); }
    };
}
