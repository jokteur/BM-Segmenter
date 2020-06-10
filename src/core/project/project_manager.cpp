#include "project_manager.h"

#include <toml.hpp>
#include <fstream>


class ProjectManagerError: public std::exception
{
    std::string error_msg_;
public:
    ProjectManagerError(std::string error_msg) : error_msg_(error_msg) {}
    virtual const char* what() const throw()
    {
        return error_msg_.c_str();
    }
};

/*
 * Implementations
 */

void ProjectManager::setCurrentProject(Project* project) {
    current_project_ = project;
}

Project *ProjectManager::newProject(std::string name, std::string description) {
    Project* project = new Project(name, description);
    projects_.push_back(project);

    Project* project_ptr = *(--projects_.end());
    if (projects_.size() == 1)
        current_project_ = project_ptr;

    return project_ptr;
}

void ProjectManager::removeProject(Project *project) {
    for(auto it = projects_.begin(); it != projects_.end();it++) {
        if (*it == project) {
            delete project;
            projects_.erase(it);
            if (current_project_ == project) {
                current_project_ = NULL;
            }
            break;
        }
    }
}

ProjectManager::~ProjectManager() {
    for(auto &project_ptr : projects_) {
        delete project_ptr;
    }
}

bool ProjectManager::saveProjectToFile(Project *project, const std::string &filename) {
    if (project != NULL) {
        if (filename.empty()) {
            return false;
        }
        std::ofstream file(filename, std::ios::trunc);

        file << "[general]" << std::endl;

        const toml::value general{
                {"name", project->getName()},
                {"description", project->getDescription()}};

        file << general << std::endl;

        project->setSaveFile(filename);
        project->setSavedState();
        return true;
    }
    else {
        return false;
    }
}

Project* ProjectManager::openProjectFromFile(const std::string &filename) {
    // First look if the project is not already opened
    for (auto &project: projects_) {
        if (project->getSaveFile() == filename) {
            return project;
        }
    }

    std::ifstream file(filename, std::ios_base::binary);
    if (!file) {
        throw ProjectManagerError("Could not open '" + filename + "'");
    }


    const auto project =  toml::parse(file);

    // General details about project
    const auto general = toml::find(project, "general");
    const auto name = toml::find<std::string> (general, "name");
    if (name.empty()) {
        auto error = toml::format_error("[error] name of project cannot be empty",
                                        general.at("name"), "non empty name needed here");
        throw ProjectManagerError(error);
    }


    const auto description = toml::find<std::string> (general, "description");

    Project* new_project = new Project(name, description);
    new_project->setSaveFile(filename);
    new_project->setSavedState();
    current_project_ = new_project;
    projects_.push_back(new_project);
    return new_project;
}

Project* ProjectManager::duplicateCurrentProject() {
    if (current_project_ != NULL) {
        Project *project = new Project(current_project_->getName(), current_project_->getDescription());
        current_project_ = project;
        projects_.push_back(project);

        Project *project_ptr = *(--projects_.end());
        if (projects_.size() == 1)
            current_project_ = project_ptr;

        return project_ptr;
    }
    return NULL;
}
