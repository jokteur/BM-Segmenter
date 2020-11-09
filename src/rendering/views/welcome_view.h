#pragma once

#include "views.h"

#include "ui/dataset/dicom_viewer.h"

namespace Rendering {
    class WelcomeView : public View {
    private:
    public:
        WelcomeView() {
            std::cout << "Create exploreview" << std::endl;
        }
        WelcomeView(const WelcomeView& other)  : View(other) {
            std::cout << "Copy exploreview" << std::endl;
        }
        WelcomeView(const WelcomeView&& other)  noexcept {
            std::cout << "Move exploreview" << std::endl;
        }
        ~WelcomeView() override {
            std::cout << "Destroy exploreview" << std::endl;
        }

    };
}