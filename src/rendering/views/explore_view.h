#pragma once

#include "views.h"

#include "drawables.h"

#include "ui/import/explore_tree.h"
#include "ui/import/explore_preview.h"
#include "ui/import/dicom_viewer.h"

namespace Rendering {
    class ExploreView : public View {
    private:
        std::shared_ptr<ExploreFolder> explore_ = std::make_shared<ExploreFolder>();
        //std::shared_ptr<DicomViewer> dicom_ = std::make_shared<DicomViewer>();
        std::shared_ptr<ExplorerPreview> preview_ = std::make_shared<ExplorerPreview>();

    public:
        ExploreView();
    };
}