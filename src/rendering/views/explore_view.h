#pragma once

#include "views.h"

#include "drawables.h"

#include "ui/dataset/explore_tree.h"
#include "ui/dataset/explore_preview.h"
#include "ui/dataset/dicom_viewer.h"

namespace Rendering {
    class ExploreView : public View, public AbstractDrawable {
    private:
        std::shared_ptr<ExploreFolder> explore_ = std::make_shared<ExploreFolder>();
        std::shared_ptr<DicomViewer> dicom_ = std::make_shared<DicomViewer>();
        std::shared_ptr<ExplorerPreview> preview_ = std::make_shared<ExplorerPreview>();

    public:
        ExploreView();

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension);
    };
}