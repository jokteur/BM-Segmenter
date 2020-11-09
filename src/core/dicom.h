#pragma once

#include <vector>
#include <functional>
#include <string>

#include "opencv2/opencv.hpp"
#include "imgui.h"

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
    };

    /**
     * Stores necessary information to store and process Dicom files
     */
    struct Dicom {
        cv::Mat data;
        ImVec2 pixel_spacing = ImVec2(1,1);
        float slice_thickness = 1.f;
        float slice_position = 1.f;
        int window_width = 400; // Window width
        int window_center = 40; // WIndow center
    };

    struct DicomSeries {
        std::vector<Dicom> series;
    };
}