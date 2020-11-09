#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <iostream>

#include "drawables.h"
#include "events.h"


namespace Rendering {
    /**
     *
     */
    class View {
    protected:
        std::vector<std::shared_ptr<AbstractDrawable>> drawables_;
    public:
        std::vector<std::shared_ptr<AbstractDrawable>>& getDrawables() { return drawables_; }
        /**
         * Default constructor, does nothing
         */
        View() = default;

        virtual ~View() = default;
    };

    class SetViewEvent : public Event {
    private:
        std::shared_ptr<View> view_;
    public:
        explicit SetViewEvent(std::shared_ptr<View> view) : view_(std::move(view)), Event("views/set_view") {}

        std::shared_ptr<View>&& getView() { return std::move(view_); }
    };
}