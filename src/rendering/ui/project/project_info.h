#pragma once

#include <vector>
#include <string>
#include <map>
#include "first_include.h"
#include "imgui.h"

#include "rendering/ui/segmentation/create_model.h"

#include "rendering/drawables.h"
#include "core/project/project_manager.h"
#include "core/segmentation/segmentation.h"
#include "rendering/ui/segmentation/new_segmentation.h"
#include "jobscheduler.h"

namespace Rendering {
    /**
     * Defines the UI for opening and exploring new folders
     */
    class ProjectInfo : public AbstractLayout {
    private:
        ::core::project::ProjectManager& project_manager_ = ::core::project::ProjectManager::getInstance();

        NewSegmentationModal new_segmentation_;
        CreateModelModal new_model_;
        Shortcut enter_shortcut_;

        std::vector<std::string> user_names_;
        std::set<std::string> users_;
        std::string current_user_;
        bool confirm_ = false;
        int user_idx_ = 0;
        int user_prev_idx_ = 0;

        bool is_set_ = false;
        bool set_tree_closed_ = true;
    public:
        ProjectInfo();
        ~ProjectInfo() override = default;

        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };

}