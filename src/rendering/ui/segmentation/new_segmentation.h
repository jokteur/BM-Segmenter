#pragma once

#include "first_include.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include "core/project/project_manager.h"
#include "core/segmentation/segmentation.h"

#include "events.h"


namespace Rendering {
    class NewSegmentationModal {
    private:
        ::core::project::ProjectManager& project_manager_;
        std::string name_;
        std::string description_;
        float color_[4] = {1.f, 0.f, 0.f, 0.5f};
        ImVec4 color = { 1.f, 0.f, 0.f, 0.5f };
        std::shared_ptr < ::core::project::Project> project_;

        bool confirm_ = false;

        modal_fct draw_fct;

    public:
        NewSegmentationModal() : project_manager_(core::project::ProjectManager::getInstance()) {}

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal(const std::shared_ptr<::core::project::Project>& project);
    };
}

