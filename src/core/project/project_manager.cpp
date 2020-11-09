#include "project_manager.h"

#include <toml.hpp>
#include <fstream>
#include <utility>

namespace core {
    namespace project {
        class ProjectManagerError : public std::exception {
            std::string error_msg_;
        public:
            ProjectManagerError(std::string error_msg) : error_msg_(std::move(error_msg)) {}

            virtual const char *what() const throw() {
                return error_msg_.c_str();
            }
        };

/*
 * Implementations
 */

        void ProjectManager::setCurrentProject(std::shared_ptr<Project> project) {
            current_project_ = project;
        }

        std::shared_ptr<Project> ProjectManager::newProject(const std::string& name, const std::string& description) {
            std::shared_ptr<Project> project = std::make_shared<Project>(name, description);
            projects_.push_back(project);

            if (projects_.size() == 1)
                current_project_ = project;

            return project;
        }

        void ProjectManager::removeProject(const std::shared_ptr<Project>& project) {
            for (auto it = projects_.begin(); it != projects_.end(); it++) {
                if (*it == project) {
                    projects_.erase(it);
                    if (current_project_ == project) {
                        current_project_ = nullptr;
                    }
                    break;
                }
            }
        }

        bool ProjectManager::saveProjectToFile(const std::shared_ptr<Project>& project, const std::string &filename) {
            if (project != nullptr) {
                if (filename.empty()) {
                    return false;
                }
                std::ofstream file(filename, std::ios::trunc);

                file << "[general]" << std::endl;

                const toml::value general{
                        {"name",        project->getName()},
                        {"description", project->getDescription()}};

                file << general << std::endl;

                project->setSaveFile(filename);
                project->setSavedState();
                return true;
            } else {
                return false;
            }
        }

        std::shared_ptr<Project> ProjectManager::openProjectFromFile(const std::string &filename) {
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


            const auto project = toml::parse(file);

            // General details about project
            const auto general = toml::find(project, "general");
            const auto name = toml::find<std::string>(general, "name");
            if (name.empty()) {
                auto error = toml::format_error("[error] name of project cannot be empty",
                                                general.at("name"), "non empty name needed here");
                throw ProjectManagerError(error);
            }


            const auto description = toml::find<std::string>(general, "description");

            std::shared_ptr<Project> new_project =  std::make_shared<Project>(name, description);
            new_project->setSaveFile(filename);
            new_project->setSavedState();
            projects_.push_back(new_project);
            return new_project;
        }

        std::shared_ptr<Project> ProjectManager::duplicateCurrentProject() {
            if (current_project_ != nullptr) {
                std::shared_ptr<Project> project = std::make_shared<Project>(current_project_->getName(), current_project_->getDescription());
                *project = *current_project_;
                current_project_ = project;
                projects_.push_back(project);

                return project;
            }
            return nullptr;
        }
    }
}