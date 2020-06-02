#ifndef BM_SEGMENTER_GUI_H
#define BM_SEGMENTER_GUI_H

#include "../application.h"
#include "ui/test_jobScheduler.h"
#include "ui/test_filedialog.h"
#include "ui/modales/modals.h"
#include "ui/dockspace.h"

namespace Rendering {
    class GUI {
    private:
        MyWindow window_;
        MyFile file_;
        Dockspace dockspace_;
        ModalsDrawable modals_;
    public:
        GUI(Application &app);
    };
}

#endif //BM_SEGMENTER_GUI_H
