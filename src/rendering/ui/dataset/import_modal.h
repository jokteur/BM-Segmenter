#pragma once

#include "first_include.h"

#include "imgui.h"
#include "imgui_stdlib.h"

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include "core/project/project_manager.h"
#include "core/dataset/explore.h"
#include "core/dicom.h"

#include "events.h"


namespace Rendering {
    class ImportDataModal {
    private:
        ::core::project::ProjectManager& project_manager_;

        std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases_;

        modal_fct draw_fct;

    public:
        ImportDataModal();

        /**
         * Returns if the modal is finished (canceled or ok)
         * @return
         */
        void showModal(std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases);
    };
}

