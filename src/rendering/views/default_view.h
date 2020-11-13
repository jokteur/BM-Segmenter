#pragma once

#include "views.h"

#include "ui/project/welcome.h"

#include "events.h"

namespace Rendering {
    class DefaultView : public View {
    private:
        std::shared_ptr<WelcomeView> welcome = std::make_shared<WelcomeView>();
    public:
        DefaultView() {
            EventQueue::getInstance().post(Event_ptr(new Event("menu/change_title/Home page")));
            drawables_.push_back(welcome);
        }
        ~DefaultView() override = default;

    };
}