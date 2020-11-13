#pragma once

#include <string>
#include <utility>
#include <vector>
#include <set>

#include "imgui.h"

#include "core/dicom.h"
#include "events.h"
#include "jobscheduler.h"

namespace core {
    namespace dataset {
        class Group {
        private:
            std::string name_;
            std::set<std::shared_ptr<DicomSeries>> dicoms_;
        public:
            Group(const std::string& name);


            const std::string& getName() { return name_; }

            void addDicom(std::shared_ptr<DicomSeries> dicom);
            void removeDicom(std::shared_ptr<DicomSeries> dicom);
        };

        class Dataset {
        private:
            std::vector<Group> groups_;
            std::vector<std::shared_ptr<DicomSeries>> dicoms_;

        public:
            Dataset() = default;

            void load(const std::string& path);

            Group& createGroup(const std::string& name);

            std::vector<Group>& getGroups() { return groups_; }
        };
    }
}