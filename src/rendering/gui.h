#pragma once

#include "first_include.h"

#include "ui/widgets/image_viewer.h"
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
        ImageViewer imageViewer_;
        ModalsDrawable modals_;
    public:
        explicit GUI(Application &app);
    };
}
