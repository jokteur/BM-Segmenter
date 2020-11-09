#pragma once

#include "views.h"

#include "ui/dataset/explore_tree.h"
#include "ui/dataset/explore_preview.h"
#include "ui/dataset/dicom_viewer.h"

namespace Rendering {
    class DefaultView : public View {
    private:
    public:
        DefaultView() {}
        ~DefaultView() override = default;

    };
}