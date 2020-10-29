#pragma once

#include "first_include.h"

#include "ui/dataset/dicom_viewer.h"
#include "ui/dataset/ui_explore.h"
#include "ui/modales/modals.h"
#include "ui/dockspace.h"
#include "application.h"

namespace Rendering {
    class GUI {
    private:
//        MyWindow window_;
        ExploreFolder exploreFolder_;
        Dockspace dockspace_;
        DicomViewer dicomViewer_;
        ModalsDrawable modals_;
    public:
        explicit GUI(Application &app);
    };
}
