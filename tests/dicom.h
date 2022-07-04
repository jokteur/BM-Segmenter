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

    /*
     As the way BM-Segmenter is intended to be used, the user would only
     load / save Matrix2D from a local disk or a distant server (ftp, samba, ...).
     Not implementing timeouts in these tests could mean that the test could never
     finish if we wait on all the load / save to be finished.
    */
    bool IsTimedOut(int milliseconds = 30000) {
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

        // Could also use latches to know when the job is finished
        // but it could result in dead-lock
        Tempo::jobId job_save, job_load;
        bool save_finished = false;
        bool load_finished = false;

        job_save = this->loader.save("test.mz", this->data_slice, [this, &save_finished](bool success) {
            save_finished = true;
            REQUIRE_MESSAGE(success, "Save has not succeeded");
            });

        while (this->is_active && !IsTimedOut()) {
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

    TEST_CASE_FIXTURE(Matrix2DFixture, "Multiple calls to load / unload"
        * doctest::description("Test the robustness of multiple load / unload"
            "calls")) {
        const int NUM_LOADS = 3;
        bool save_finished = false;
        bool launch_simultaneous_loads = false;
        bool load_finished[NUM_LOADS] = { false };
        bool unload_finished[NUM_LOADS] = { false };

        // Prepare the data to be loaded in a file
        this->loader.save("test.mz", this->data_slice, [this, &save_finished, &launch_simultaneous_loads](bool success) {
            save_finished = true;
            REQUIRE_MESSAGE(success, "Save has not succeeded");
            if (success) launch_simultaneous_loads = true;
            });

        bool test_success = false;
        while (this->is_active && !IsTimedOut()) {
            Wait();
            if (save_finished) {
                if (launch_simultaneous_loads) {
                    for (int i = 0;i < NUM_LOADS;i++) {
                        this->loader.load("test.mz", this->test_slice, [this, i, &load_finished](bool success) {
                            load_finished[i] = true;
                            REQUIRE_MESSAGE(success, (std::string("Load call #") + std::to_string(i) + std::string(" has not finished")));
                            });
                    }
                    launch_simultaneous_loads = false;
                }
                int num_success = 0;
                for (int i = 0;i < NUM_LOADS;i++) {
                    num_success += (int)load_finished[i];
                }

                // Means that all load calls have succeeded
                if (num_success == NUM_LOADS) {
                    for (int i = 0;i < NUM_LOADS;i++) {
                        this->loader.unload(this->test_slice, [this, i, &unload_finished](bool success) {
                            unload_finished[i] = true;
                            REQUIRE_MESSAGE(success, (std::string("Unload call #") + std::to_string(i) + std::string(" has not finished")));
                            });
                    }
                }

                // Check if all unloads are done
                num_success = 0;
                for (int i = 0;i < NUM_LOADS;i++) {
                    num_success = (int)unload_finished[i];
                }

                if (num_success == NUM_LOADS) {
                    this->is_active = false;
                    test_success = true;
                }
            }
        }

        REQUIRE_MESSAGE(test_success, "Multiple calls to unload/call did not succeed properly");
    }

}