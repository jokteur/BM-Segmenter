#include "project.h"
#include "python/py_api.h"
#include "pybind11/pybind11.h"

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
    }
}