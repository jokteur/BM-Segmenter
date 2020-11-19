#pragma once

#include <vector>

#include "python/py_api.h"
#include "pybind11/numpy.h"

#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "core/dicom.h"
#include "jobscheduler.h"
#include "log.h"
#include "util.h"

namespace core {
    namespace dataset {
        namespace py = pybind11;

        struct DicomResult : public JobResult {
            Dicom image;
            std::string error_msg;
        };

        std::shared_ptr<Job> npy_to_matrix(const std::string& path, jobResultFct result_fct);

        /**
         * Opens the dicom image, converts it into an array of in16
         * and puts the data into the opencv matrix
         *
         * This function launches a job in the JobScheduler
         * Listen to the JOB_DICOM_TO_IMAGE in the event queue
         *
         * @param event_name name of event that will tell when the image is ready
         * will be named "dataset/dicom/<event_name>"
         * @param path path to the dicom image
         */
        std::shared_ptr<Job> dicom_to_matrix(const std::string &path, jobResultFct result_fct);
    }
}