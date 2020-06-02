#ifndef BM_SEGMENTER_MAIN_MENU_BAR_H
#define BM_SEGMENTER_MAIN_MENU_BAR_H

#include "nfd.h"
#include "rendering/drawables.h"
#include <GLFW/glfw3.h>
#include "events.h"
#include "imgui.h"

#include "core/project/project_manager.h"
#include "settings.h"
#include "rendering/ui/modales/new_project.h"

namespace Rendering {
    class MainMenuBar : public AbstractLayout {
    private:
        int counter_ = 0;
        nfdchar_t *outPath = NULL;
        ProjectManager& project_manager_;
        EventQueue& event_queue_;
        Settings& settings_;
        std::vector<Listener*> listeners_;

        NewProjectModal new_project_modal_;

        void init_listeners();
        void destroy_listeners();

        void save_project();

        void undo();
        void redo();

        void file_menu();
        //void edit_menu();
        void projects_menu();
        void settings_menu();

        void open_file();
    public:
        MainMenuBar()
        : project_manager_(ProjectManager::getInstance()),
            event_queue_(EventQueue::getInstance()),
            settings_(Settings::getInstance()) {
            init_listeners();
        }


        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        ~MainMenuBar() { destroy_listeners(); }
    };
}

#endif //BM_SEGMENTER_MAIN_MENU_BAR_H
