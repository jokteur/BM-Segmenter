#include "import_modal.h"

#include "rendering/views/project_view.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/keyboard_shortcuts.h"

namespace Rendering {
    ImportDataModal::ImportDataModal() : project_manager_(::core::project::ProjectManager::getInstance()) {
        draw_fct = [this](bool& show, bool& enter, bool& escape) {
            show = false;
        };
    }
    void ImportDataModal::showModal(std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases) {
        draw_fct = [this](bool& show, bool& enter, bool& escape) {
            if (ImGui::Button("SDFSDFSDF"))
                show = false;
        };
        Modals::getInstance().setModal("Import data into project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
    }
}