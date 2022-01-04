#include "explore.h"
#include "python/py_api.h"
#include "log.h"
#include "jobscheduler.h"
#include "util.h"
#include "rendering/animation_util.h"

namespace core {
    namespace dataset {
        namespace py = pybind11;

        bool Explore::destroy_ = false;

        Explore::Explore(const Explore& other) : event_queue_(EventQueue::getInstance()) {
            status_ = other.status_;
            cases_ = other.cases_;
            destroy_ = false;
        }

        Explore::~Explore() {
            destroy_ = true;
        }

        void Explore::findDicoms(const std::string &path) {
            path_ = path;
            //cases_->clear();
            jobFct job = [=](float &progress, bool &abort) -> std::shared_ptr<JobResult> {
                status_ = EXPLORE_WORKING;
                auto cases = std::make_shared<std::vector<PatientNode>>();
                auto state = PyGILState_Ensure();

                Explore::status status = EXPLORE_SUCCESS;
                JobResult job_result;
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
                        Rendering::push_animation();
                        event_queue_.post(Event_ptr(new LogEvent("dicom_search", message)));
                        
                        if (!errors.empty()) {
                            status = EXPLORE_PARTIAL_SUCCESS;
                            event_queue_.post(Event_ptr(new LogEvent("dicom_error", errors)));
                        }
                    }
                    // Once it has finished building the cases, copy the dictionary that represents
                    // all the cases
                    // See load_dicom.py
                    py::list data_dict = dicoms.attr("data").attr("get_cases_list")();
                    for (auto &py_patient : data_dict) {
                        auto patient_tuple = py_patient.cast<py::tuple>();
                        auto patientID = patient_tuple[0].cast<std::string>();

                        PatientNode patient{patientID};

                        for (auto &py_study : patient_tuple[1].cast<py::dict>()) {
                            auto study_tuple = py_study.first.cast<py::tuple>();

                            StudyNode study {study_tuple[0].cast<std::string>(),
                                             study_tuple[1].cast<std::string>(),
                                             study_tuple[2].cast<std::string>()};

                            for (auto &py_series : py_study.second.cast<py::dict>()) {
                                auto series_tuple = py_series.first.cast<py::tuple>();

                                SeriesNode series {series_tuple[1].cast<std::string>(),
                                                   series_tuple[0].cast<std::string>()};

                                std::vector<std::string> paths;
                                for (auto &py_image : py_series.second.cast<py::list>()) {
                                    auto image_dict = py_image.cast<py::dict>();

                                    ImageNode image {image_dict["path"].cast<std::string>(),
                                                     image_dict["instanceNumber"].cast<std::string>()};
                                    series.images.push_back(image);
                                    paths.push_back(image.path);
                                }
                                series.data = DicomSeries(paths);
                                study.series.push_back(std::make_shared<SeriesNode>(series));
                            }
                            patient.study.push_back(study);
                        }
                        cases->push_back(patient);
                    }
                }
                catch (const std::exception &e) {
                    std::string error(e.what());
                    event_queue_.post(Event_ptr(new LogEvent("dicom_error", error)));
                    status = EXPLORE_ERROR;
                }

                PyGILState_Release(state);

                if (destroy_) {
                    return std::make_shared<JobResult>(job_result);
                }
                cases_ = cases;

                status_ = status;
                if (status == EXPLORE_ERROR)
                    job_result.success = false;
                else {
                    job_result.success = true;
                }
                return std::make_shared<JobResult>(job_result);
            };

            if (status_ != EXPLORE_WORKING)
                jobRef_ = JobScheduler::getInstance().addJob(STRING(JOB_EXPLORE_NAME), job)->id;
        }

        template <typename T>
        std::vector<size_t> sort_indexes(const std::vector<T>& v) {

            // initialize original index locations
            std::vector<size_t> idx(v.size());
            std::iota(idx.begin(), idx.end(), 0);

            // sort indexes based on comparing values in v
            // using std::stable_sort instead of std::sort
            // to avoid unnecessary index re-orderings
            // when v contains elements of equal values 
            std::stable_sort(idx.begin(), idx.end(),
                [&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

            return idx;
        }

        void build_tree(std::shared_ptr<std::vector<PatientNode>> tree, const ImGuiTextFilter& case_filter, const ImGuiTextFilter& study_filter, const ImGuiTextFilter& series_filter) {
            for (auto &patient : *tree) {
                patient.tree_count = 0;
                if (case_filter.PassFilter(patient.ID.c_str())) {
                    patient.tree_count++;
                } else {
                    continue;
                }
                for (auto &study : patient.study) {
                    study.tree_count = 0;
                    if (study_filter.PassFilter(study.description.c_str())) {
                        patient.tree_count++;
                        study.tree_count++;
                    } else {
                        patient.tree_count--;
                        continue;
                    }
                    for (auto &series : study.series) {
                        if (series_filter.PassFilter(series->modality.c_str())) {
                            patient.tree_count++;
                            study.tree_count++;
                            series->tree_count++;
                        } else {
                            patient.tree_count--;
                            study.tree_count--;
                            continue;
                        }
                        for (auto &image : series->images) {
                            patient.tree_count++;
                            study.tree_count++;
                            series->tree_count++;
                            image.tree_count = 1;
                        }
                    }
                }
                // Second pass without filters to order the series
                std::vector<std::vector<uint64_t>> orders;
                for (auto& study : patient.study) {
                    std::vector<uint64_t> series_order;
                    for (auto& study : patient.study) {
                        std::string time_str = study.date + study.time;
                        long long time = 0;
                        if (!time_str.empty())
                            time = std::stoull(time_str);
                        for (auto& series : study.series) {
                            series_order.push_back(time); // Insert the time (which can be ordered) of the series
                        }
                    }
                    orders.push_back(series_order);
                }
                int i = 0;
                int patient_count = 0;
                for (auto& study : patient.study) {
                    // Order by time, assign the numbers
                    std::vector<int> idx_order;
                    for (auto j : sort_indexes(orders[patient_count])) {
                        idx_order.push_back(j + 1);
                    }
                    for (auto& series : study.series) {
                        series->order = idx_order[i];
                        i++;
                    }
                }
            }
        }

    }
}