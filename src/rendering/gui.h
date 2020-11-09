#pragma once

#include "first_include.h"

#include "ui/modales/modals.h"
#include "ui/dockspace.h"

#include "views.h"
#include "views/default_view.h"
#include "rendering/ui/main_menu_bar.h"

#include "application.h"
#include "events.h"

namespace Rendering {
    class GUI {
    private:
        bool initialized = false;
//        MyWindow window_;
        std::shared_ptr<ModalsDrawable> modals_ = std::make_shared<ModalsDrawable>();
        std::shared_ptr<Dockspace> dockspace_ = std::make_shared<Dockspace>();

        std::shared_ptr<View> view_ = nullptr;
        std::shared_ptr<AbstractDrawable> view_draw_ = nullptr;

        Application* app_ = nullptr;
        Listener listener_;

        GUI() = default;
    public:
        /**
         * Copy constructors stay empty, because of the Singleton
         */
        GUI(Settings const &) = delete;
        void operator=(GUI const &) = delete;

        ~GUI();

        void init(Application &app);

        /**
         * @return instance of the Singleton of the Job Scheduler
         */
        static GUI& getInstance () {
            static GUI instance;
            return instance;
        }

        void setView(std::shared_ptr<View> view);
    };
}
