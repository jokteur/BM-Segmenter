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

        std::string group_name_; 
        std::vector<const char*> group_names_;
        bool build_names_ = false;
        bool confirm_ = false;
        bool start_work_ = false;
        bool job_finished_ = false;
        bool replace_ = false;
        bool show_import_modal_;
        jobId job_id_;
        int item_select_ = 0;
        float progress = 0.f;

        Shortcut enter_shortcut_;

        std::shared_ptr<::core::dataset::ImportResult> import_result_;

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

