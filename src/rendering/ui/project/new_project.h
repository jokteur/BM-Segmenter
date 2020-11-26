#pragma once

#include "first_include.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include "core/project/project_manager.h"

#include "events.h"


namespace Rendering {
    class NewProjectModal {
    private:
        ::core::project::ProjectManager &project_manager_;
        std::string name_;
        std::string description_;
        std::string out_path_;

        bool confirm_ = false;

        modal_fct draw_fct;

    public:
        NewProjectModal();

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal();
    };
}

