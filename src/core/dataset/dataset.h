#pragma once

#include <string>
#include <utility>
#include <vector>

#include "imgui.h"

#include "core/dicom.h"
#include "events.h"
#include "jobscheduler.h"

namespace core {
    namespace dataset {
        class Group {
        private:
            std::string name_;
            Dicom 
        public:
            Group();
            ~Group();
        };

        class Dataset {
        private:
            std::vector<Group> groups_;
            std::vector<DicomSeries> dicoms_;
        public:
            Dataset();
            ~Dataset();

            load(const std::string& path);

            void createGroup();

            std::vector<Group>& getGroups() { return groups_; }
        };
    }
}