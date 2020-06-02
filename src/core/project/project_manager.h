#ifndef BM_SEGMENTER_PROJECT_MANAGER_H
#define BM_SEGMENTER_PROJECT_MANAGER_H

#include <vector>
#include <set>
#include <string>
#include "project.h"

class ProjectManager {
private:
    using iterator_ = Project**;

    std::vector<Project*> projects_;
    Project* current_project = NULL;

    ProjectManager() {}
public:
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    ProjectManager(ProjectManager const &) = delete;
    void operator=(ProjectManager const &) = delete;

    ~ProjectManager();

    /**
     * @return instance of the Singleton of the Project Manager
     */
    static ProjectManager& getInstance () {
        static ProjectManager instance;
        return instance;
    }

    /**
     * Sets the active project
     * @param project pointer
     */
    void setCurrentProject(Project* project);

    /**
     * Returns the current active projects in the project manager
     * @return current active project
     */
    Project* getCurrentProject() { return current_project; }

    /**
     * Returns the number of current projects
     * @return
     */
    int getNumProjects() { return projects_.size(); }

    /**
     * Creates a new project a adds it to the ProjectManager
     * @param name
     * @return pointer to current created project
     */
    Project* newProject(std::string name, std::string description);

    /**
     * Duplicates the current project (useful for making a copy of a project)
     */
    Project* duplicateCurrentProject();

    void removeProject(Project* project);

    /**
     * Creates a project object by opening a file
     * (should be in .ml_proj format)
     * @param file
     * @return
     */
    Project* openProjectFromFile(const std::string& filename);

    /**
     * Saves a project to disk
     * @param project
     * @param filename
     * @return true if successful, false if not
     */
    bool saveProjectToFile(Project* project, const std::string& filename);

    /**
     * Iterator function for C++14 usage of for(auto p: ProjectManager)
     * @return iterator on Project*
     */
    iterator_ begin() { return &projects_[0]; }

    /**
     * Iterator function for C++14 usage of for(auto p: ProjectManager)
     * @return iterator on Project*
     */
    iterator_ end() { return &projects_[projects_.size()]; }

};


#endif //BM_SEGMENTER_PROJECT_MANAGER_H
