#pragma once

#include "views.h"

#include "ui/project/project_info.h"
#include "ui/dataset/dataset_view.h"

namespace Rendering {
    class ProjectView : public View {
    private:
        std::shared_ptr<ProjectInfo> info = std::make_shared<ProjectInfo>();
        std::shared_ptr<DatasetView> dataset = std::make_shared<DatasetView>();
    public:
        ProjectView() {
            EventQueue::getInstance().post(Event_ptr(new Event("menu/change_title/Project page")));
            drawables_.push_back(info);
            drawables_.push_back(dataset);
        }
        ~ProjectView() override = default;

    };
}