#pragma once

#include <vector>
#include <functional>
#include <string>
#include <set>
#include <utility>

#include "opencv2/opencv.hpp"
#include "imgui.h"
#include "jobscheduler.h"

namespace core {
    /**
     * Dicom marker that can be used in the project
     *
     * Can be insert into sets
     */
    struct DicomMarkerName {
        enum Type {AXIAL, CORONAL, SAGITTAL};
        std::string name;
        ImColor color;
        Type type;

        bool operator<(const DicomMarkerName& rhs) const {
            return std::hash<std::string>{}(name) < std::hash<std::string>{}(rhs.name);
        }
        bool operator==(const DicomMarkerName& rhs) const {
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
        DicomMarkerName name;
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

    std::pair<std::string, std::string> parse_dicom_id(const std::string& id);

    class DicomSeries {
    public:
        enum file_format { F_DICOM, F_NP };
    private:
        std::vector<Dicom> data_;
        std::vector<std::string> images_path_;
        std::string id_;
        std::pair<std::string, std::string> id_pair_;
        std::vector<DicomCoordinate> coordinates_;
        ImVec2 crop_x_ = ImVec2(0, 100);
        ImVec2 crop_y_ = ImVec2(0, 100);
        int window_width_ = 400; // Window width
        int window_center_ = 40; // Window center
        file_format format_ = F_DICOM;
        
        std::map<int, int> ref_counter_;


        static int num_loaded_;

        DicomCoordinate current_coordinate_;

        std::set<jobId> pending_jobs_;
        int selected_index_ = 0;
        int num_jobs_ = 0;
        bool load_all_ = false;

        void free_memory(int index);
        void reload();

        void set_ref(int idx);
        void add_one_to_ref(int idx);
        void remove_one_to_ref(int idx);

        jobId load_case(int index, bool force_replace, bool keep_previous, const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});

        void init();
    public:
        DicomSeries(file_format format = F_DICOM);
        explicit DicomSeries(std::vector<std::string> paths, const std::string& id = "", file_format format = F_DICOM);

        ~DicomSeries();

        void setPaths(const std::vector<std::string> &paths);
        void setId(const std::string& id);

        void loadAll(const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});
        jobId loadCase(float percentage, bool force_replace = false, const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});
        jobId loadCase(int index, bool force_replace = false, const std::function<void(const Dicom&)>& when_finished_fct = [](const Dicom&) {});

        void unloadCase(int index = -1);
        void unloadAll(bool keep_current = false);
        void forceClean();
        void cancelPendingJobs();

        ImVec2 getCropX() { return crop_x_; }
        ImVec2 getCropY() { return crop_y_; }
        int& getWW() { return window_width_; }
        int& getWC() { return window_center_; }
        int size() { return images_path_.size(); }

        int rows();
        int cols();

        std::vector<Dicom>& getData() { return data_; }
        int getCurrentIndex() const { return selected_index_; }

        std::string getId() { return id_; }
        std::pair<std::string, std::string> getIdPair() { return id_pair_; }
        Dicom& getCurrentDicom();

        std::vector<std::string>& getPaths() { return images_path_; }

        void removeCoordinate();
        void addCoordinate(const DicomCoordinate& coordinate);
        DicomCoordinate& getCurrentCoordinate() { return current_coordinate_; }
        std::vector<DicomCoordinate>& getAllCoordinates() { return coordinates_; }

        void setCrops(ImVec2 crop_x, ImVec2 crop_y, bool no_reload = false);
        void setCropX(ImVec2 crop_x, bool no_reload = false);
        void setCropY(ImVec2 crop_y, bool no_reload = false);

        bool isReady();
    };


    struct OrderDicom {
        bool operator()(const std::shared_ptr<DicomSeries>& dicom1, const std::shared_ptr <DicomSeries>& dicom2) const;
    };


    class DicomSelectEvent : public Event {
    private:
        std::shared_ptr<DicomSeries> dicom_;
    public:
        explicit DicomSelectEvent(std::shared_ptr<DicomSeries> dicom) : dicom_(dicom), Event("dataset/dicom_edit") {}

        std::shared_ptr<DicomSeries>& getDicom() {
            return dicom_;
        }
    };
}