#include "project.h"
#include "python/py_api.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace core {
    namespace project {
        namespace py = pybind11;

        Project::Project(const std::string &name, const std::string &description)
                : name_(name), description_(description) {

        }

        bool Project::operator==(Project &project) {
            if (project.name_ != name_)
                return false;
            if (project.description_ != description_)
                return false;
            if (project.is_saved_ != is_saved_)
                return false;

            if (!(project.markers == markers))
                return false;

            return true;
        }
        bool Project::setUpWorkspace(const std::string& path, const std::string& name, const std::string& extension, std::string& out_path) {
            auto state = PyGILState_Ensure();
            try {
                py::module scripts = py::module::import("python.scripts.workspace");
                py::tuple return_tuple = scripts.attr("setup_workspace")(path, name, extension).cast<py::tuple>();
                if (return_tuple[0].cast<bool>()) {
                    out_path = return_tuple[1].cast<std::string>();
                }
                else {
                    out_path = return_tuple[1].cast<std::string>();
                    PyGILState_Release(state);
                    return false;
                }
            }
            catch (const std::exception& e) {
                out_path = e.what();
                PyGILState_Release(state);
                return false;
            }

            PyGILState_Release(state);
            return true;
        }
        void Project::setSaveFile(const std::string& save_file) {
            save_file_ = save_file;
            auto state = PyGILState_Ensure();
            try {
                py::module scripts = py::module::import("python.scripts.workspace");
                root_path_ = scripts.attr("get_root")(save_file).cast<std::string>();
            }
            catch (const std::exception& e) {
                PyGILState_Release(state);
                return;
            }

            PyGILState_Release(state);
        }
        void Project::setUsers(std::vector<std::string> users) {
            users_.clear();
            for (auto& user : users) {
                users_.insert(user);
            }
        }

        void Project::setCurrentUser(std::string user) {
            current_user_ = user;
            if (!user.empty())
                users_.insert(user);
        }

        std::string Project::saveSegmentations() {
            auto state = PyGILState_Ensure();
            for (auto& seg : segmentations_) {
                try {
                    py::module scripts = py::module::import("python.scripts.segmentation");
                    auto color = seg->getMaskColor();
                    std::vector<float> color_arr = { color.x, color.y, color.z, color.w };
                    auto save_file = scripts.attr("save_segmentation")(save_file_, seg->getName(), seg->getDescription(), seg->getFilename(), color_arr).cast<std::string>();
                    seg->setFilename(save_file);
                }   
                catch (const std::exception& e) {
                    PyGILState_Release(state);
                    return e.what();
                }
            }
            PyGILState_Release(state);
            return "";
        }
        std::string Project::loadSegmentations() {
            segmentations_.clear();
            auto state = PyGILState_Ensure();
            try {
                auto dicoms = dataset_.getDicoms();
                std::map <std::string, std::shared_ptr<DicomSeries>> dicom_id_map;

                for (auto& dicom : dicoms) {
                    dicom_id_map[dicom->getId()] = dicom;
                }

                py::module scripts = py::module::import("python.scripts.segmentation");
                auto result = scripts.attr("load_segmentations")(root_path_);
                for (auto& seg : result) {
                    segmentation::Segmentation segmentation(seg["name"].cast<std::string>(), seg["description"].cast<std::string>());
                    segmentation.setFilename(seg["path"].cast<std::string>());
                    segmentation.setStrippedName(seg["stripped_name"].cast<std::string>());
                    segmentation.setMaskColor(seg["color"].cast <std::vector<float>>());
                    for (auto& id : seg["ids"]) {
                        std::string _id = id.cast<std::string>();
                        if (dicom_id_map.find(_id) == dicom_id_map.end()) {
                            segmentation.addDicom(dicom_id_map[_id]);
                        }
                    }
                    segmentations_.insert(std::make_shared<segmentation::Segmentation>(segmentation));
                }
            }
            catch (const std::exception& e) {
                PyGILState_Release(state);
                std::cout << e.what() << std::endl;
                return e.what();
            }
            PyGILState_Release(state);
            return "";
        }
        std::string Project::addSegmentation(std::shared_ptr<segmentation::Segmentation> segmentation) {
            auto state = PyGILState_Ensure();
            try {
                auto util = py::module::import("python.scripts.util");
                for (auto& seg : segmentations_) {
                    if (seg->getName() == segmentation->getName()) {
                        return std::string("A segmentation with identical name already exists");
                    }
                    if (util.attr("make_safe_filename")(segmentation->getName()).cast<std::string>() == seg->getStrippedName()) {
                        PyGILState_Release(state);
                        return std::string("Cannot have an (almost) identical name to an existing segmentation");
                    }
                }
                segmentation->setStrippedName(util.attr("make_safe_filename")(segmentation->getName()).cast<std::string>());
            }
            catch (const std::exception& e) {
                PyGILState_Release(state);
                return e.what();
            }
            PyGILState_Release(state);
            segmentations_.insert(segmentation);
            return "";
        }
    }
}