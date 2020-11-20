#pragma once

#include "views.h"

#include "ui/project/project_info.h"
#include "ui/dataset/dataset_view.h"
#include "ui/dataset/edit_mask.h"

namespace Rendering {
    class ProjectView : public View {
    private:
        std::shared_ptr<ProjectInfo> info = std::make_shared<ProjectInfo>();
        std::shared_ptr<DatasetView> dataset = std::make_shared<DatasetView>();
        std::shared_ptr<EditMask> mask = std::make_shared<EditMask>();
    public:
        ProjectView() {
            EventQueue::getInstance().post(Event_ptr(new Event("menu/change_title/Project page")));
            drawables_.push_back(info);
            drawables_.push_back(dataset);
            drawables_.push_back(mask);
        }
        ~ProjectView() override = default;

    };
}