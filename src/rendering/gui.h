#pragma once

#include "first_include.h"

#include "ui/dataset/dicom_viewer.h"
#include "ui/dataset/ui_explore.h"
#include "ui/modales/modals.h"
#include "ui/dataset/explore_preview.h"
#include "ui/dockspace.h"
#include "application.h"

namespace Rendering {
    class GUI {
    private:
//        MyWindow window_;
        ExploreFolder exploreFolder_;
        Dockspace dockspace_;
        ExplorerPreview preview_;
        DicomViewer dicomViewer_;
        DicomViewer dicomViewer_2;
        ModalsDrawable modals_;
    public:
        explicit GUI(Application &app);
    };
}
