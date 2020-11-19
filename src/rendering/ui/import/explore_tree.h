#pragma once

#include "nfd.h"
#include <vector>
#include <string>
#include <map>
#include "imgui.h"

#include "core/dataset/explore.h"
#include "rendering/drawables.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/ui/import/dicom_preview.h"
#include "rendering/ui/import/import_modal.h"
#include "jobscheduler.h"
#include "log.h"
#include "util.h"

namespace Rendering {
    /**
     * Defines the UI for opening and exploring new folders
     */
    class ExploreFolder : public AbstractLayout {
    private:

        /*
         * For capturing the log sent by the Dicom Search
         */
        Listener log_listener_;
        Listener error_listener_;
        //Listener job_listener_;

        std::shared_ptr<::core::dataset::Explore> explorer_ = std::make_shared<::core::dataset::Explore>();

        ImGuiTextBuffer log_buffer_;
        ImGuiTextBuffer error_buffer_;

        ImportDataModal import_modal_;

        bool build_tree_ = true;
        bool is_new_tree_ = true;
        bool set_tree_closed_ = true;
        bool display_import_button_ = false;
        bool close_view_ = false;

        ImGuiTextFilter case_filter_;
        std::string case_filter_str_;
        ImGuiTextFilter study_filter_;
        std::string study_filter_str_;
        ImGuiTextFilter series_filter_;
        std::string series_filter_str_;

        ImVec4 disabled_text_color_;

        std::string path_;

        void build_tree();

        static void exclude_menu(bool &is_active, const std::string &desc);

    public:
        /**
         * Initializes the listener and subscribes to the queue
         */
        explicit ExploreFolder(ImGuiID docking_id = 0);

        /**
         * Destructor
         */
        ~ExploreFolder() override;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override;
    };
}
