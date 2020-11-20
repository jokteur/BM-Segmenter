#include <unordered_map>
#include <toml.hpp>
#include <fstream>

#include "python/py_api.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"

#include "dataset.h"

namespace py = pybind11;

core::dataset::Group::Group(const std::string& name) : name_(name) {

}

bool core::dataset::Group::operator==(const Group& other) {
    return other.name_ == name_;
}

bool core::dataset::Group::operator!=(const Group& other) {
    return other.name_ != name_;
}

std::vector<std::shared_ptr<core::DicomSeries>> core::dataset::Group::getOrderedDicoms() {
    std::vector<std::shared_ptr<DicomSeries>> vector;
    for (auto& series : dicoms_) {
        vector.push_back(series);
    }
    return vector;
}

void core::dataset::Group::addDicom(std::shared_ptr<DicomSeries> dicom) {
	dicoms_.insert(dicom);
}

void core::dataset::Group::removeDicom(std::shared_ptr<DicomSeries> dicom) {
	dicoms_.erase(dicom);
}

std::string core::dataset::Dataset::load(const std::string& path) {
    dicoms_.clear();
    groups_.clear();

    auto state = PyGILState_Ensure();

    py::module scripts;
    bool skip = false;
    try {
        py::module import_data = py::module::import("python.scripts.dataset");
        py::tuple tuple = import_data.attr("load_dataset")(path);
        py::dict data = tuple[0].cast<py::dict>();
        auto& groups = tuple[1].cast<std::vector<std::string>>();

        // Create the groups
        for (auto& group_name : groups) {
            createGroup(group_name);
        }
        
        for (auto& pair : data) {
            std::string id = pair.first.cast<std::string>();
            py::dict file_info = pair.second.cast<py::dict>();
            
            auto file = file_info["files"].cast<std::vector<std::string>>();
            
            auto dicom = std::make_shared<DicomSeries>(file, id, DicomSeries::F_NP);
            dicoms_.insert(dicom);
            for (auto& group : file_info["groups"].cast<py::list>()) {
                std::string group_name = group.cast<std::string>();
                for (auto& current_group : groups_) {
                    if (current_group.getName() == group_name) {
                        current_group.addDicom(dicom);
                        break;
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::string err = e.what();
        PyGILState_Release(state);
        is_loaded_ = false;
        return err;
    }
    is_loaded_ = true;
    PyGILState_Release(state);
    return "";
}

core::dataset::Group& core::dataset::Dataset::createGroup(const std::string& name) {
    for (auto& group : groups_) {
        if (group.getName() == name)
            return group;
    }
	groups_.emplace_back(Group(name));
	return *(--groups_.end());
}

jobId& core::dataset::Dataset::importData(const Group& group, std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases, const std::string& root_path, jobResultFct result_fct, bool replace) {
    jobFct job_fct = [=](float& progress, bool& abort) -> std::shared_ptr<JobResult> {
        auto import_result = std::make_shared<ImportResult>();

        std::vector<DicomSeries> all_cases;

        // Flatten all the selected cases
        int num_images = 0;
        for (auto& patient : *cases) {
            for (auto& study : patient.study) {
                for (auto& series : study.series) {
                    bool is_active = series->is_active && study.is_active && patient.is_active;
                    if (is_active) {
                        std::vector<std::string> paths;
                        for (auto& image : series->images) {
                            if (image.is_active) {
                                num_images++;
                                paths.push_back(image.path);
                            }
                        }
                        if (!paths.empty()) {
                            auto &dicom = DicomSeries(paths, patient.ID + std::string("___") + std::to_string(
                                std::hash<std::string>{}(study.date + study.description + study.time + series->modality + series->number)));
                            dicom.setCrops(series->data.getCropX(), series->data.getCropY(), true);
                            all_cases.emplace_back(dicom);
                        }
                    }
                }
            }
        }

        int i = 0;
        std::string error_msg;
        bool failure = false;

        std::vector<DicomSeries> existing;
        std::vector<std::string> save_paths;
        import_result->success = true;
        auto state = PyGILState_Ensure();
        for (auto& dicom : all_cases) {
            if (failure)
                break;

            py::module scripts;
            bool skip = false;
            try {
                scripts = py::module::import("python.scripts.import_data");
                py::module create = py::module::import("python.scripts.workspace");
                py::tuple ret = create.attr("create_series_dir")(root_path, dicom.getId());
                if (!ret[1].cast<bool>() && !replace) {
                    import_result->existing.push_back(dicom);
                    skip = true;
                }
                else {
                    import_result->save_paths.push_back(ret[0].cast<std::string>());
                }
            }
            catch (const std::exception& e) {
                import_result->error_msg = e.what();
                failure = true;
            }

            if (!skip) {
                int img_num = 0;
                for (auto& path : dicom.getPaths()) {
                    if (failure)
                        break;
                    if (abort) {
                        failure = true;
                        import_result->error_msg = "Job canceled";
                        break;
                    }

                    try {
                        std::vector<float> crop_x = { dicom.getCropX().x, dicom.getCropX().y };
                        std::vector<float> crop_y = { dicom.getCropY().x, dicom.getCropY().y };
                        scripts.attr("import_dicom")(path, root_path, dicom.getId(), img_num, dicom.getWW(), dicom.getWC(), crop_x, crop_y, replace);
                    }
                    catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                        import_result->error_msg = e.what();
                        failure = true;
                    }

                    img_num++;

                    // Update progress
                    i++;
                    progress = float(i) / float(num_images);
                }
            }
            else {
                i += dicom.getPaths().size();
            }
        }
        PyGILState_Release(state);

        if (failure) {
            import_result->success = false;
        }
        return import_result;
    };

    auto job = JobScheduler::getInstance().addJob("import_data", job_fct, result_fct);
    return job->id;
}

std::string core::dataset::Dataset::registerFiles(std::vector<std::string> paths, const Group& group, const std::string& root_path) {
    auto state = PyGILState_Ensure();

    std::cout << "Save dataset" << std::endl;
    py::module scripts;
    bool skip = false;
    try {
        py::module import_data = py::module::import("python.scripts.import_data");
        import_data.attr("add_to_dataset")(paths, group.getName(), root_path);
    }
    catch (const std::exception& e) {
        std::string err = e.what();
        PyGILState_Release(state);
        return err;
    }

    PyGILState_Release(state);
    return "";
}

std::string core::dataset::Dataset::save(const std::string& root_path) {
    for (auto& group : groups_) {
        std::vector<std::string> ids;
        for (auto& dicom : group.getDicoms()) {
            ids.push_back(dicom->getId());
        }
        std::string err = registerFiles(ids, group, root_path);
        if (!err.empty()) {
            return std::string("Failed to save dataset:\n") + err;
        }
    }
    return "";
}
