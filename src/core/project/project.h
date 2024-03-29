#pragma once

#include <vector>
#include <set>
#include <string>
#include <map>

#include "core/dataset/dataset.h"
#include "core/dicom.h"
#include "core/segmentation/segmentation.h"

namespace core {
    namespace project {
        class Project {
        private:
            std::string name_;
            std::string description_;
            std::string save_file_;

            dataset::Dataset dataset_;

            std::string root_path_;

            bool is_saved_ = false;
            std::set<DicomMarkerName> markers;

            std::set<std::shared_ptr<segmentation::Segmentation>> segmentations_;

            std::set<std::string> users_;
            std::string current_user_;

        public:
            Project(const std::string &name, const std::string &description);

            /**
             * Compares two projects and looks if they are identical (name, description, datasets, ...)
             * @param project project to compare to
             * @return true if identical, false if not
             */
            bool operator==(Project& project);

            /**
             * Returns the current name of the project
             * @return name of project
             */
            std::string& getName() { return name_; }

            /**
             * Returns the current description of the project
             * @return description of project
             */
            std::string& getDescription() { return description_; }

            /**
             * Returns the save file if it has been precised
             * @return
             */
            std::string& getSaveFile() { return save_file_; }

            std::set<std::string> getUsers() { return users_; }

            std::string getCurrentUser() { return current_user_; }

            /**
             * Adds a marker group to the project
             * @return true if the name is already in the set, false if not
            */
            bool addMarkerName(const DicomMarkerName& marker_name);


            /**
             * Returns the set of markers (for 3D dicom file) in the project
             * @return set of DicomMarkerNames
             */
            std::set<DicomMarkerName>& getMarkers() { return markers; }

            /**
             * Returns the root path of the project
            */
            const std::string& getRoot() { return root_path_; }


            /**
             * Returns the dataset object contained in the project
             * @return
             */
            dataset::Dataset& getDataset() { return dataset_; }

            /**
             * Sets the workspace at the given path up
             * @param path 
             * @param out_path  
             * @return returns if successful. If not, the error message will be in out_path
            */
            bool setUpWorkspace(const std::string& path, const std::string& name, const std::string& extension, std::string& out_path);

            /**
             * Sets the current save file for this project
             * @param save_file path of the file
             */
            void setSaveFile(const std::string& save_file);

            /**
             * Sets the saved variable to true
             * If the project is modified, then the variable is again set to false
             */
            void setSavedState() {
                is_saved_ = true;
            }

            void setUsers(std::vector<std::string> users);

            void setCurrentUser(std::string user);

            /**
             * Saves all the segmentations to the disk
             * Returns an error in the form of a string if the save could not proceed
            */
            std::string saveSegmentations();

            /**
             * Loads all segmentations it can find in the project
            */
            std::string loadSegmentations();

            std::string addSegmentation(std::shared_ptr<segmentation::Segmentation> segmentation);

            std::set<std::shared_ptr<segmentation::Segmentation>>& getSegmentations() { return segmentations_; }

            /**
             * Returns if the project is in a saved state
             * @return true for saved state, false for non saved state
             */
            bool isSaved() const {
                return is_saved_;
            }

            /**
             * Sets the name of the projects
             * @param name name of project
             */
            void setName(const std::string& name) {
                name_ = name;
                is_saved_ = false;
            }
        };
    }
}