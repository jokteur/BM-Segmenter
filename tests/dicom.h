#include <doctest/doctest.h>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <vector>

#include "core/data/dicom.h"

cv::Mat getTestData() {
    cv::Mat mat = cv::Mat(512, 512, CV_32F, 0.f);
    for (int i = 210;i < 300;i++) {
        for (int j = 210;j < 210;j++) {
            mat.at<float>(i, j) = (float)(i + j);
        }
    }
    return mat;
}

bool compareMatrices(const cv::Mat& mat1, const cv::Mat& mat2) {
    if (mat1.cols != mat2.cols || mat1.rows != mat2.rows)
        return false;

    for (int i = 0;i < mat1.rows;i++) {
        for (int j = 0;j < mat1.cols;j++) {
            if (mat1.at<float>(i, j) != mat2.at<float>(i, j))
                return false;
        }
    }
    return true;
}

class Matrix2DFixture {
protected:
    cv::Mat data = getTestData();
    Data::Matrix2D test_slice;
    Data::Matrix2D data_slice;
    Data::Matrix2DLoader& loader = Data::Matrix2DLoader::getInstance();
    Tempo::JobScheduler& scheduler = Tempo::JobScheduler::getInstance();

    std::chrono::steady_clock::time_point start;
    bool is_active = true;
public:
    Matrix2DFixture() {
        data_slice.data = data;
    }

    void Start() {
        start = std::chrono::high_resolution_clock::now();
    }

    void Wait(int milliseconds = 10) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    bool IsTimedOut(int milliseconds = 1000) {
        auto tp = std::chrono::high_resolution_clock::now();
        auto time_passed = std::chrono::duration_cast<std::chrono::milliseconds>(tp - start).count();
        return time_passed > milliseconds;
    }

    void Stop() { is_active = false; }
};

TEST_SUITE("Dicom") {
    TEST_CASE_FIXTURE(Matrix2DFixture, "Simple save and load"
        * doctest::description("Simple save and load")) {
        Start();

        Tempo::jobId job_save, job_load;
        bool save_finished = false;
        bool load_finished = false;

        job_save = this->loader.save("test.mz", this->data_slice, [this, &save_finished](bool success) {
            save_finished = true;
            REQUIRE_MESSAGE(success, "Save has not succeeded");
            });

        while (this->is_active && !IsTimedOut(2000)) {
            Wait(); // Like in a real application, the loop does not execute infinitely fast
            if (save_finished) {
                job_load = this->loader.load("test.mz", this->test_slice, [this, &load_finished](bool success) {
                    load_finished = true;
                    REQUIRE_MESSAGE(success, "Load has not succeeded");
                    });
            }
        }
        REQUIRE_MESSAGE(save_finished, "Save did not finish");
        REQUIRE_MESSAGE(!IsTimedOut(), "Timed out");

        REQUIRE_MESSAGE(compareMatrices(this->test_slice.data, this->data_slice.data), "Matrices do not correspond");
    }

}