#pragma once

#include <vector>
#include <set>
#include <string>
#include "project.h"

namespace core {
    namespace project {
        class ProjectManager {
        private:

            std::vector<std::shared_ptr<Project>> projects_;
            std::shared_ptr<Project> current_project_ = nullptr;

            ProjectManager() = default;

        public:
            /**
             * Copy constructors stay empty, because of the Singleton
             */
            ProjectManager(ProjectManager const &) = delete;

            void operator=(ProjectManager const &) = delete;


            /**
             * @return instance of the Singleton of the Project Manager
             */
            static ProjectManager &getInstance() {
                static ProjectManager instance;
                return instance;
            }

            /**
             * Sets the active project
             * @param project pointer
             */
            void setCurrentProject(std::shared_ptr<Project> project);

            /**
             * Returns the current active projects in the project manager
             * @return current active project
             */
            std::shared_ptr<Project> getCurrentProject() { return current_project_; }

            /**
             * Returns the number of current projects
             * @return
             */
            size_t getNumProjects() { return projects_.size(); }

            /**
             * Creates a new project a adds it to the ProjectManager
             * @param name
             * @return pointer to current created project
             */
            std::shared_ptr<Project> newProject(const std::string& name, const std::string& description);

            /**
             * Duplicates the current project (useful for making a copy of a project)
             */
            std::shared_ptr<Project> duplicateCurrentProject();

            void removeProject(const std::shared_ptr<Project>& project);

            /**
             * Creates a project object by opening a file (in toml format)
             * @param file
             * @return
             */
            std::shared_ptr<Project> openProjectFromFile(const std::string &filename);

            /**
             * Saves a project to disk
             * @param project
             * @param filename
             * @return true if successful, false if not
             */
            static bool saveProjectToFile(const std::shared_ptr<Project>& project, const std::string &filename);

            /**
             * Returns only the name of a project without opening it completely
             * @param filename path of the project
             * @return name of the project
             */
            std::string &getNameFromFile(const std::string &filename);

            /**
             * Iterator function for C++14 usage of for(auto p: ProjectManager)
             * @return iterator on Project*
             */
            auto begin() { return projects_.begin(); }

            /**
             * Iterator function for C++14 usage of for(auto p: ProjectManager)
             * @return iterator on Project*
             */
            auto end() { return projects_.end(); }

        };
    }
}
