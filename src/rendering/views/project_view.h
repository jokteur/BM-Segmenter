#pragma once

#include "views.h"

#include "ui/project/project_info.h"

namespace Rendering {
    class ProjectView : public View {
    private:
        std::shared_ptr<ProjectInfo> info = std::make_shared<ProjectInfo>();
    public:
        ProjectView() {
            drawables_.push_back(info);
        }
        ~ProjectView() override = default;

    };
}