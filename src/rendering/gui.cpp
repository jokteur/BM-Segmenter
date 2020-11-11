#include "gui.h"

#include <utility>

#include "rendering/ui/shortcuts_list.h"
#include "rendering/ui/modales/error_message.h"
#include "settings.h"
#include "events.h"

void Rendering::GUI::init(Rendering::Application &app) {
    if (initialized)
        return;

    Shortcuts::init_global_shortcuts();

    try {
        Settings::getInstance().loadSettings("settings.toml");
    }
    catch (std::exception &e) {
        show_error_modal("Load setting error",
                         "An error occured when loading settings file (settings.toml)\n"
                         "Default settings have been applied",
                         e.what());
    }

    app_ = &app;
    app_->getMainWindow().addDrawable(dockspace_);
    app_->getMainWindow().addDrawable(modals_);

    initialized = true;

    listener_.filter = "views/set_view";
    listener_.callback = [=](Event_ptr &event) {
        auto view_event = reinterpret_cast<SetViewEvent*>(event.get());
        setView(view_event->getView());
    };
    EventQueue::getInstance().subscribe(&listener_);
}

Rendering::GUI::~GUI() {
    if (initialized) {
        EventQueue::getInstance().unsubscribe(&listener_);
    }
}

void Rendering::GUI::setView(std::shared_ptr<View> view) {
    if (!initialized)
        return;

//    view_draw_ = std::dynamic_pointer_cast<AbstractDrawable>(view);

    app_->getMainWindow().removeDrawable(view_draw_);
    if (view_ != nullptr) {
        for (auto &drawable : view_->getDrawables()) {
            app_->getMainWindow().removeDrawable(drawable);
        }
    }

//    app_->getMainWindow().addDrawable(view_draw_);
    view_ = std::move(view);

    for (auto &drawable : view_->getDrawables()) {
        app_->getMainWindow().addDrawable(drawable);
    }
}

