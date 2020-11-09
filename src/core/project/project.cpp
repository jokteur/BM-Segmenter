#include "project.h"

namespace core {
    namespace project {
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
    }
}