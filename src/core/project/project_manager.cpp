#include "project_manager.h"

void ProjectManager::setCurrentProject(Project* project) {
    current_project = project;
}

Project *ProjectManager::newProject(std::string name, std::string description) {
    Project* project = new Project(name, description);
    projects_.push_back(project);

    Project* project_ptr = *(--projects_.end());
    if (projects_.size() == 1)
        current_project = project_ptr;

    return project_ptr;
}

void ProjectManager::removeProject(Project *project) {
    for(auto it = projects_.begin(); it != projects_.end();it++) {
        if (*it == project) {
            delete project;
            projects_.erase(it);
        }
    }
}

ProjectManager::~ProjectManager() {
    for(auto &project_ptr : projects_) {
        delete project_ptr;
    }
}

bool ProjectManager::saveProjectToFile(Project *project, const std::string &filename) {
    return false;
}

Project* ProjectManager::openProjectFromFile(const std::string &filename) {
    return current_project;
}

Project* ProjectManager::duplicateCurrentProject() {
    if (current_project != NULL) {
        Project *project = new Project(current_project->getName(), current_project->getDescription());
        current_project = project;
        projects_.push_back(project);

        Project *project_ptr = *(--projects_.end());
        if (projects_.size() == 1)
            current_project = project_ptr;

        return project_ptr;
    }
    return NULL;
}
