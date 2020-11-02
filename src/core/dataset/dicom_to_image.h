#pragma once

#include <vector>

#include "python/py_api.h"
#include "pybind11/numpy.h"

#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "jobscheduler.h"
#include "log.h"
#include "util.h"

namespace core {
    namespace dataset {
        namespace py = pybind11;

        /**
         * Custom class for sending the result of the dicom image
         */
        class DicomReadyEvent : public Event {
        private:
            cv::Mat mat_;
        public:
            explicit DicomReadyEvent(const std::string& name, cv::Mat& mat) : mat_(std::move(mat)), Event(std::string("dataset/dicom/") + name) {}

            cv::Mat&& getMat() { return std::move(mat_); }
        };

        struct DicomResult : public JobResult {
            cv::Mat data;
            std::string error_msg;
        };

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
        void dicom_to_matrix(const std::string &path, jobResultFct result_fct);
    }
}