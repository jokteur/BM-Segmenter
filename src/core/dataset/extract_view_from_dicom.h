#pragma once

#include <vector>

#include "opencv2/opencv.hpp"
#include "jobscheduler.h"

namespace core {
    namespace dataset {
        /**
         *
         */
        struct DicomViewResult : public JobResult {
            cv::Mat data;
        };

        /**
         *
         * @param matrices
         * @param result_fct
         * @param position
         * @param horizontal
         * @return
         */
        std::shared_ptr<Job> extract_view(const std::vector<cv::Mat>& matrices,
                                          jobResultFct result_fct,
                                          float position = 0.5,
                                          bool horizontal = true);
    }
}