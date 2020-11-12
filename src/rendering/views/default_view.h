#pragma once

#include "views.h"

#include "ui/project/welcome.h"

namespace Rendering {
    class DefaultView : public View {
    private:
        std::shared_ptr<WelcomeView> welcome = std::make_shared<WelcomeView>();
    public:
        DefaultView() {
            drawables_.push_back(welcome);
        }
        ~DefaultView() override = default;

    };
}