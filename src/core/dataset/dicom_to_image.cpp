#include "core/dataset/dicom_to_image.h"

namespace py = pybind11;
using namespace py::literals;

std::shared_ptr<Job> core::dataset::npy_to_matrix(const std::string& path, jobResultFct result_fct) {

    jobFct job = [=](float& progress, bool& abort) -> std::shared_ptr<JobResult> {
        auto dicom_result = std::make_shared<DicomResult>();
        auto state = PyGILState_Ensure();
        try {
            py::module numpy = py::module::import("numpy");

            auto kwargs = py::dict("allow_pickle"_a = true);
            auto data = numpy.attr("load")(path, **kwargs);

            // We await a 2D numpy array
            auto buffer = data["matrix"].cast<py::buffer>();
            py::buffer_info info = buffer.request();
            int rows = info.shape[0];
            int cols = info.shape[1];

            auto array = static_cast<short int*>(info.ptr);
            dicom_result->image.data.create(rows, cols, CV_16S);
            memcpy(dicom_result->image.data.data, array, sizeof(short int) * rows * cols);

            auto pixel_spacing = data["spacing"].cast<py::list>();
            dicom_result->image.pixel_spacing = ImVec2(pixel_spacing[0].cast<float>(), pixel_spacing[1].cast<float>());
            auto slice_info = data["slice_info"].cast<py::list>();
            dicom_result->image.slice_thickness = slice_info[0].cast<float>();
            dicom_result->image.slice_position = slice_info[1].cast<float>();
            dicom_result->success = true;
        }
        catch (const std::exception& e) {
            dicom_result->error_msg = e.what();
        }

        PyGILState_Release(state);
        return dicom_result;
    };
    return JobScheduler::getInstance().addJob("dicom_to_image", job, result_fct);
}

std::shared_ptr<Job> core::dataset::dicom_to_matrix(const std::string &path, jobResultFct result_fct) {
    static int num_instances = 0;

    jobFct job = [=](float &progress, bool &abort) -> std::shared_ptr<JobResult> {
        num_instances++;
        auto dicom_result = std::make_shared<DicomResult>();
        auto state = PyGILState_Ensure();
        try {
            py::module scripts = py::module::import("python.scripts.load_dicom");
            py::tuple return_tuple = scripts.attr("load_scan_from_dicom")(path).cast<py::tuple>();
            if (py::isinstance<py::bool_>(return_tuple[0])) {
                std::string error = return_tuple[1].cast<std::string>();
                dicom_result->error_msg = error;
            }
            else {
                // We await a 2D numpy array
                auto buffer = return_tuple[0].cast<py::buffer>();
                py::buffer_info info = buffer.request();
                int rows = info.shape[0];
                int cols = info.shape[1];

                auto array = static_cast<short int*>(info.ptr);
                dicom_result->image.data.create(rows, cols, CV_16S);
                memcpy(dicom_result->image.data.data, array, sizeof(short int)*rows*cols);

//                py::print(return_tuple[1]);
                auto pixel_spacing = return_tuple[1].cast<py::tuple>();
                dicom_result->image.pixel_spacing = ImVec2(pixel_spacing[0].cast<float>(), pixel_spacing[1].cast<float>());
                dicom_result->image.slice_thickness = return_tuple[2].cast<float>();
                dicom_result->image.slice_position = return_tuple[3].cast<float>();
                dicom_result->success = true;
            }
        }
        catch (const std::exception &e) {
            dicom_result->error_msg = e.what();
            //py::gil_scoped_release release;
        }

        PyGILState_Release(state);
        return dicom_result;
    };
    return JobScheduler::getInstance().addJob("dicom_to_image", job, result_fct);
}