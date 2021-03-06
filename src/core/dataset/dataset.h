#pragma once

#include <string>
#include <utility>
#include <vector>
#include <set>

#include "imgui.h"

#include "core/dicom.h"
#include "explore.h"

#include "events.h"
#include "jobscheduler.h"

namespace core {
    namespace dataset {
        typedef std::set<std::shared_ptr<DicomSeries>, OrderDicom> dicom_set;

        class Group {
        private:
            std::string name_;
            dicom_set dicoms_;
        public:
            Group(const std::string& name);

            bool operator==(const Group& other);
            bool operator!=(const Group& other);

            const std::string& getName() const { return name_; }

            std::vector<std::shared_ptr<DicomSeries>> getOrderedDicoms();
            dicom_set& getDicoms() { return dicoms_; }

            void addDicom(std::shared_ptr<DicomSeries> dicom);
            void removeDicom(std::shared_ptr<DicomSeries> dicom);
        };


        struct ImportResult : public JobResult {
            std::vector<DicomSeries> existing;
            std::vector<std::string> save_paths;

            std::string error_msg;
        };


        class Dataset {
        private:
            std::vector<Group> groups_;
            dicom_set dicoms_;

            bool is_loaded_ = false;

        public:
            Dataset() = default;

            std::string load(const std::string& path);

            Group& createGroup(const std::string& name);

            /**
             * Whenever importData is called, a new job is launched.
             * @return the job reference created in the findDicoms function
             */
            jobId& importData(const Group& group, std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases, const std::string& root_path, jobResultFct result_fct, bool replace=false);

            std::string registerFiles(std::vector<std::string> paths, const Group& group, const std::string& root_path);

            std::string save(const std::string& root_path);

            std::vector<Group>& getGroups() { return groups_; }
            dicom_set& getDicoms() { return dicoms_; }
            std::vector<std::shared_ptr<::core::DicomSeries>> getOrderedDicoms();
        };
    }
}