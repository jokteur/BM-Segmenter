#pragma once

#include <vector>

#include "python/py_api.h"
#include "pybind11/numpy.h"

#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "jobscheduler.h"
#include "log.h"

namespace core {
    namespace dataset {
        namespace py = pybind11;

#define JOB_DICOM_TO_IMAGE dicom_to_image

        /**
         * Opens the dicom image, converts it into an array of in16
         * and puts the data into the opencv matrix
         *
         * This function launches a job in the JobScheduler
         * Listen to the JOB_DICOM_TO_IMAGE in the event queue
         *
         * @param image image to which the data will be copied
         * @param path path to the dicom image
         */
        void dicom_to_matrix(cv::Mat* image, const std::string& path) {
            static bool is_working = false;
            static int num_instances = 0;

            jobFct job = [=](float &progress, bool &abort) -> bool {
                is_working = true;
                num_instances++;
                bool success = false;
                auto state = PyGILState_Ensure();
                try {
                    py::module scripts = py::module::import("python.scripts.load_dicom");
                    py::tuple return_tuple = scripts.attr("load_scan_from_dicom")(path).cast<py::tuple>();
                    if (py::isinstance<py::bool_>(return_tuple[0])) {
                        std::string error = return_tuple[1].cast<std::string>();
                        success = false;
                        EventQueue::getInstance().post(Event_ptr(new LogEvent("dicom_read_error", error)));
                    }
                    else {
                        // We await a 2D numpy array
                        auto buffer = return_tuple[0].cast<py::buffer>();
                        py::buffer_info info = buffer.request();
                        int rows = info.shape[0];
                        int cols = info.shape[1];

                        auto array = static_cast<short int*>(info.ptr);
                        image->create(rows, cols, CV_16S);
                        memcpy(image->data, array, sizeof(short int)*rows*cols);
                        success = true;
                    }
                }
                catch (const std::exception &e) {
                    std::string error(e.what());
                    EventQueue::getInstance().post(Event_ptr(new LogEvent("dicom_read_error", error)));
                    py::gil_scoped_release release;
                }

                PyGILState_Release(state);
                is_working = false;
                return success;
            };
            if (!is_working)
                JobScheduler::getInstance().addJob(STRING(JOB_DICOM_TO_IMAGE), job);
        }
    }
}