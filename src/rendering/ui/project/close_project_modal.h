#pragma once

#include "first_include.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include "core/project/project_manager.h"

#include "events.h"


namespace Rendering {
    class CloseProjectModal {
    private:
        ::core::project::ProjectManager& project_manager_;
        std::string name_;
        std::string description_;
        std::shared_ptr < ::core::project::Project> project_;

        modal_fct draw_fct;

    public:
        CloseProjectModal();

        void setProject(const std::shared_ptr<::core::project::Project>& project);

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal();
    };
}

