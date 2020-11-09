#pragma once

#include <functional>
#include <utility>

#include "nfd.h"

#include "first_include.h"
#include "imgui.h"

#include "rendering/drawables.h"
#include "main_menu_bar.h"

#include "events.h"

namespace Rendering {
    typedef std::function<void (ImGuiID* dock_id)> set_dock_fct;

    class SetDockSpaceFctEvent : public Event {
    private:
        set_dock_fct fct_;
    public:
        explicit SetDockSpaceFctEvent(set_dock_fct fct) : fct_(std::move(fct)), Event("set_dock_fct") {}

        set_dock_fct getFct() { return fct_; }
    };

    class Dockspace : public AbstractLayout {
    private:
        bool open_ = true;
        MainMenuBar menu_bar_;
        ImGuiDockNodeFlags dockspace_flags_;
        ImGuiWindowFlags window_flags_;
        ImGuiID dockspace_id_;

        bool rebuild_ = false;

        Listener listener_;
        std::function<void (ImGuiID* dock_id)> build_dock_fct_ = [] (ImGuiID*) {};

    public:

        /**
         * @return instance of the Singleton of the Job Scheduler
         */
        Dockspace();

        ~Dockspace() override;

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;

        ImGuiID& getDockSpaceID() { return dockspace_id_; }
    };
}
