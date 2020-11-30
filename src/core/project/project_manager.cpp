#include "project_manager.h"

#include "settings.h"
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

        void ProjectManager::removeProject(const std::shared_ptr<Project>& project) {
            auto it = projects_.find(project);
            if (it != projects_.end()) {
                projects_.erase(it);
            }
            current_project_ = nullptr;
            for (auto prj : projects_) {
                current_project_ = prj;
                break;
            }
        }

        std::shared_ptr<Project> ProjectManager::newProject(const std::string& name, const std::string& description) {
            std::shared_ptr<Project> project = std::make_shared<Project>(name, description);
            projects_.insert(project);

            if (projects_.size() == 1)
                current_project_ = project;

            return project;
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
                        {"description", project->getDescription()},
                        {"users", project->getUsers()}
                };

                file << general << std::endl;

                project->setSaveFile(filename);
                project->setSavedState();
                Settings::getInstance().addRecentFile(filename);
                return true;
            } 
            else {
                return false;
            }
        }

        std::shared_ptr<Project> ProjectManager::openProjectFromFile(const std::string &filename) {
            // First look if the project is not already opened
            for (auto &project: projects_) {
                if (project->getSaveFile() == filename) {
                    Settings::getInstance().addRecentFile(filename);
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
            const auto users_find = toml::find<std::vector<std::string>>(general, "users");

            std::vector<std::string> users;
            for (auto& user : users_find) {
                users.push_back(user);
            }

            std::shared_ptr<Project> new_project = std::make_shared<Project>(name, description);
            new_project->setSaveFile(filename);
            new_project->setUsers(users);
            new_project->setSavedState();

            //std::string err = new_project->getDataset().load(filename);

            //if (!err.empty()) {
            //    throw ProjectManagerError(err);
            //}


            projects_.insert(new_project);
            Settings::getInstance().addRecentFile(filename);
            return new_project;
        }

        std::shared_ptr<Project> ProjectManager::duplicateCurrentProject() {
            if (current_project_ != nullptr) {
                std::shared_ptr<Project> project = std::make_shared<Project>(current_project_->getName(), current_project_->getDescription());
                *project = *current_project_;
                current_project_ = project;
                projects_.insert(project);

                return project;
            }
            return nullptr;
        }
    }
}