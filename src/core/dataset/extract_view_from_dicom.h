#pragma once

#include <vector>

#include "core/dicom.h"

#include "opencv2/opencv.hpp"
#include "jobscheduler.h"

namespace core {
    namespace dataset {
        /**
         *
         */
        struct DicomViewResult : public JobResult {
            Dicom image;
        };

        /**
         *
         * @param matrices
         * @param result_fct
         * @param position
         * @param horizontal
         * @return
         */
        std::shared_ptr<Job> extract_view(const std::vector<Dicom> &matrices,
                                          jobResultFct result_fct,
                                          float position = 0.5,
                                          bool horizontal = true);
    }
}