#pragma once

#include "application.h"
#include "ui/dataset/ui_explore.h"
#include "ui/test_jobScheduler.h"
#include "ui/test_filedialog.h"
#include "ui/modales/modals.h"
#include "ui/dockspace.h"

namespace Rendering {
    class GUI {
    private:
//        MyWindow window_;
        ExploreFolder exploreFolder_;
        Dockspace dockspace_;
        ModalsDrawable modals_;
    public:
        GUI(Application &app);
    };
}
