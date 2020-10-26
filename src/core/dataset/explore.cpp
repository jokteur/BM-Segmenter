#include "explore.h"
#include "python/py_api.h"
#include "log.h"
#include "jobscheduler.h"
#include "util.h"

namespace core {
    namespace dataset {
        namespace py = pybind11;

        void Explore::findDicoms(const std::string &path) {
            path_ = path;
            jobFct job = [=](float &progress, bool &abort) -> bool {
                status_ = EXPLORE_WORKING;
                auto state = PyGILState_Ensure();

                Explore::status status = EXPLORE_SUCCESS;
                try {
                    py::module scripts = py::module::import("python.scripts.load_dicom");
                    py::object dicoms = scripts.attr("DiscoverDicoms")(path_.c_str());
                    for (auto &element : dicoms) {
                        if (abort) {
                            status = EXPLORE_CANCELLED;
                            break;
                        }
                        auto tuple = element.cast<py::tuple>();
                        bool found = tuple[1].cast<bool>();
                        std::string errors = tuple[2].cast<std::string>();

                        std::string message = std::string("Searching in '")
                                              + tuple[0].cast<std::string>()
                                              + std::string((found ? "', found DICOM(s)" : "'"));

                        event_queue_.post(Event_ptr(new LogEvent("dicom_search", message)));
                        if (!errors.empty()) {
                            status = EXPLORE_PARTIAL_SUCCESS;
                            event_queue_.post(Event_ptr(new LogEvent("dicom_error", errors)));
                        }
                    }
                    // Once it has finished building the cases, copy the dictionary that represents
                    // all the cases
                    // See load_dicom.py
                    cases_.clear();
                    py::list data_dict = dicoms.attr("data").attr("get_cases_list")();
                    for (auto &py_patient : data_dict) {
                        auto patient_tuple = py_patient.cast<py::tuple>();
                        auto patientID = patient_tuple[0].cast<std::string>();

                        PatientNode patient{patientID};

                        for (auto &py_study : patient_tuple[1].cast<py::dict>()) {
                            auto study_tuple = py_study.first.cast<py::tuple>();

                            StudyNode study {&patient,
                                             study_tuple[0].cast<std::string>(),
                                             study_tuple[1].cast<std::string>(),
                                             study_tuple[2].cast<std::string>()};

                            for (auto &py_series : py_study.second.cast<py::dict>()) {
                                auto series_tuple = py_series.first.cast<py::tuple>();

                                SeriesNode series {&study,
                                                   series_tuple[1].cast<std::string>(),
                                                   series_tuple[0].cast<std::string>()};

                                for (auto &py_image : py_series.second.cast<py::list>()) {
                                    auto image_dict = py_image.cast<py::dict>();

                                    ImageNode image {&series,
                                                     image_dict["path"].cast<std::string>(),
                                                     image_dict["instanceNumber"].cast<std::string>()};
                                    series.images.push_back(image);
                                }
                                study.series.push_back(series);
                            }
                            patient.study.push_back(study);
                        }
                        cases_.push_back(patient);
                    }
                }
                catch (const std::exception &e) {
                    std::string error(e.what());
                    event_queue_.post(Event_ptr(new LogEvent("dicom_error", error)));
                    status = EXPLORE_ERROR;
                }

                PyGILState_Release(state);

                status_ = status;
                if (status == EXPLORE_ERROR)
                    return false;
                else
                    return true;
            };

            if (status_ != EXPLORE_WORKING)
                jobRef_ = JobScheduler::getInstance().addJob(STRING(JOBEXPLORENAME), job);
        }

    }
}