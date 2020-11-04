#pragma once

#include "opencv2/opencv.hpp"
#include "imgui.h"

namespace core {
    struct Dicom {
        cv::Mat data;
        ImVec2 pixel_spacing = ImVec2(1,1);
        float slice_thickness = 1.f;
        float slice_position = 1.f;
    };
}