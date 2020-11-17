#pragma once

#include <vector>
#include <functional>
#include <string>
#include <set>

#include "opencv2/opencv.hpp"
#include "imgui.h"
#include "jobscheduler.h"

namespace core {
    /**
     * Dicom marker that can be used in the project
     *
     * Can be insert into sets
     */
    struct DicomMarker {
        std::string name;
        ImColor color;

        bool operator<(const DicomMarker& rhs) const {
            return std::hash<std::string>{}(name) < std::hash<std::string>{}(rhs.name);
        }
        bool operator==(const DicomMarker& rhs) const {
            return name == rhs.name;
        }
    };

    /**
     * Stores a coordinate in the axial, coronal and sagittal 3D coordinate system
     * Values should be between 0 and 1
     */
    struct DicomCoordinate {
        float sagittal = 0.5f;
        float coronal = 0.5f;
        float axial = 0.f;
        std::string name;
    };

    /**
     * Stores necessary information to store and process Dicom files
     */
    struct Dicom {
        cv::Mat data;
        ImVec2 pixel_spacing = ImVec2(1,1);
        std::string error_message;
        float slice_thickness = 1.f;
        float slice_position = 1.f;
        bool is_set = false;
    };

    class DicomSeries {
    public:
        enum file_format { F_DICOM, F_NP };
    private:
        std::vector<Dicom> data_;
        std::vector<std::string> images_path_;
        std::string id_;
        std::vector<DicomCoordinate> coordinates_;
        ImVec2 crop_x_ = ImVec2(0, 100);
        ImVec2 crop_y_ = ImVec2(0, 100);
        int window_width_ = 400; // Window width
        int window_center_ = 40; // Window center
        file_format format_ = F_DICOM;

        DicomCoordinate current_coordinate_;

        std::set<jobId> pending_jobs_;
        int selected_index_ = 0;
        int num_jobs_ = 0;
        bool load_all_ = false;

        void free_memory(int index);
        void reload();

        void init();
    public:
        DicomSeries(file_format format = F_DICOM);
        explicit DicomSeries(std::vector<std::string> paths, const std::string& id = "", file_format format = F_DICOM);
        ~DicomSeries();

        void setPaths(const std::vector<std::string> &paths);
        void setId(const std::string& id) { id_ = id; }

        void loadAll(bool force_load = false);
        void loadCase(float percentage, bool force_replace = false, const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});
        void loadCase(int index, bool force_replace = false, const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});
        void unloadData(bool keep_current = false);
        void cleanData();
        void cancelPendingJobs();

        ImVec2 getCropX() { return crop_x_; }
        ImVec2 getCropY() { return crop_y_; }
        int& getWW() { return window_width_; }
        int& getWC() { return window_center_; }
        int size() { return images_path_.size(); }

        std::vector<Dicom>& getData() { return data_; }
        int getCurrentIndex() const { return selected_index_; }
        std::string getId() { return id_; }
        Dicom& getCurrentDicom();
        void eraseCurrent();

        std::vector<std::string>& getPaths() { return images_path_; }

        DicomCoordinate& getCurrentCoordinate() { return current_coordinate_; }
        std::vector<DicomCoordinate>& getAllCoordinates() { return coordinates_; }

        void setCrops(ImVec2 crop_x, ImVec2 crop_y);
        void setCropX(ImVec2 crop_x);
        void setCropY(ImVec2 crop_y);

        bool isReady();
    };
}