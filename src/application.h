#ifndef BM_SEGMENTER_APPLICATION_H
#define BM_SEGMENTER_APPLICATION_H

#include <string>
#include <vector>

#include "jobscheduler.h"
#include "rendering/keyboard_shortcuts.h"
#include "rendering/window.h"
#include "events.h"
#include "imgui.h"

//compatibility with older versions of Visual Studio
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace Rendering {
    /**
     * Class for building and launching the whole app
     *
     * It controls the main loop, worker threads (for other tasks than GUI logic)
     */
    class Application {
    private:
        std::vector<Window> windows_;
        Window* main_window_;

        // All singletons
        JobScheduler &scheduler_;
        EventQueue &event_queue_;

        // ImGui configs flags
        int configFlags_ = 0;

        struct state {
            bool error = false;
            bool imgui_init = false;
            const char* glsl_version;
        };
        state app_state_;

        /**
         * Inits GLFW and throw an exception if failed
         */
        void init_glfw();

        /**
         * Safely shuts down ImGUI
         */
        static void shutdown();
    public:
        /**
         * Constructor which initializes the main window of the application
         * The main window can be changed later
         * @param main_window_title title of the main window
         * @param main_window_width width of the main window
         * @param main_window_height height of the main window
         */
        Application(std::string main_window_title, uint16_t main_window_width, uint16_t main_window_height);

        /**
         * Inits Dear ImGui and GLFW
         */
        void init();

        /**
         * Set configuration flags for Dear ImGui
         * @param configFlags
         */
        void addImGuiFlags(int configFlags);

        ~Application();

        Window& getMainWindow();


        /**
         * Starts the main loop of the application
         * Terminates when the main window is closed
         * @return true when the application successfully terminated, false if not
         */
        bool loop();


    };
}


#endif //BM_SEGMENTER_APPLICATION_H
