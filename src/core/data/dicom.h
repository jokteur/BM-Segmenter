#pragma once

#include <vector>
#include <atomic>
#include <thread>
#include <optional>
#include <unordered_set>
#include <functional>
#include <unordered_map>
#include <map>
#include <string>

#include <tempo.h>
#include "opencv2/opencv.hpp"

#include "basic.h"
#include "compress.h"

namespace Data {
    struct Slice;
    class Dicom;

    Dicom loadDicomFromFile(std::string path, bool shallow_load = true);
    void saveDicomToFile(const Dicom& dicom, bool erase_existing_data = false);

    // The basic data structures should be agnostic from the load sources
    struct Matrix2D {
        cv::Mat data;
        Vec2 pixel_spacing = { 1.f, 1.f };
        float slice_thickness = 1.f;
        float slice_position = 1.f;
        std::string uid;

        // For thread-safe loading
        std::atomic<bool> is_set;
        std::atomic<int> counter;

        Matrix2D copy();
    };

    class Dicom {
        std::map<int, std::vector<Matrix2D>> slices;
        std::unordered_set<Vec2> resolutions;
        std::string uid;
        std::string name;

    public:
        Dicom() {}

        Dicom copy();

        std::string setName();
        std::string const getUID();

        std::vector<int> getSliceNumbers();

        std::optional<Matrix2D&> getSlice(int number);
        void removeSlice(int number);
        void addSlice(const Matrix2D& slice, int number);
    };

    typedef std::function<void(bool)> NotifyFct;
    static NotifyFct no_op_fct = [](bool) {};

    // Static class
    class Matrix2DLoader {
    private:
    public:
        Matrix2DLoader() {}

        /**
         * Copy constructors stay empty, because of the Singleton
         */
        Matrix2DLoader(Matrix2DLoader const&) = delete;
        void operator=(Matrix2DLoader const&) = delete;

        /**
         * @return instance of the Singleton of the Job Scheduler
         */
        static Matrix2DLoader& getInstance() {
            static Matrix2DLoader instance;
            return instance;
        }

        void unload(Matrix2D& matrix2D, NotifyFct callback = no_op_fct);
        Tempo::jobId load(const std::string& path, Matrix2D& matrix2D, NotifyFct callback = no_op_fct);
        Tempo::jobId save(const std::string& path, Matrix2D& matrix2D, NotifyFct callback = no_op_fct);
    };
}