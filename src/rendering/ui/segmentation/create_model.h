#pragma once

#include "first_include.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"
#include "rendering/ui/widgets/util.h"

#include "core/project/project_manager.h"
#include "core/segmentation/segmentation.h"

#include "events.h"


namespace Rendering {
    class CreateModelModal {
    private:
        ::core::project::ProjectManager& project_manager_;
        std::string name_;
        int ww_ = 400;
        int wc_ = 40;

        int num_layers_ = 3;
        int input_size_ = 512;

        std::shared_ptr < ::core::project::Project> project_;

        bool confirm_ = false;

        Widgets::Selectable sizes_select_;

        modal_fct draw_fct;

    public:
        CreateModelModal();

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal(std::shared_ptr<::core::segmentation::Segmentation> segmentation);
    };
}

