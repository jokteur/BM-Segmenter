#pragma once

#include "opencv2/opencv.hpp"
#include "python/py_api.h"

namespace core {

    void npy_buffer_to_cv(pybind11::object object, cv::Mat& mat);
}