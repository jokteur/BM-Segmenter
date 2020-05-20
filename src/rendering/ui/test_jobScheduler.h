#ifndef BM_SEGMENTER_TEST_WINDOW_H
#define BM_SEGMENTER_TEST_WINDOW_H

#include <iostream>
#include <unistd.h>
#include <vector>

#include "../drawables.h"
#include <GLFW/glfw3.h>
#include "imgui.h"

#include "../../jobscheduler.h"

namespace Rendering {
    class MyWindow : public AbstractLayout {
    private:
        JobScheduler &scheduler_;
        int counter_ = 0;
        jobId job_id_;
        std::vector<jobId> jobs_;
    public:
        MyWindow() : scheduler_(JobScheduler::getInstance()) {
            scheduler_.setWorkerPoolSize(3);
        }

        void draw(GLFWwindow* window) override {
            ImGui::Begin("My Window");
            jobFct job;
            if(ImGui::Button("Launch job")) {
                std::string name = "myJob";
                int counter = counter_;
                jobFct job = [counter] (float &progress, bool &abort) -> bool {
                    // Simulate a progression of some kind, update every 0.5 second
                    for(int i = 0;i < 20;i++) {
                        usleep(0.5*1e6);
                        glfwPostEmptyEvent();
                        if (abort)
                            return false;
                        progress = float(i+1)/20.;
                    }
                    return true ;
                };
                jobs_.push_back(scheduler_.addJob(name, job));
                counter_++;
            }
            ImGui::SameLine();
            if(ImGui::Button("+")) {
                scheduler_.setWorkerPoolSize(scheduler_.getNumberOfWorkers() + 1);
            }
            ImGui::SameLine();
            if(ImGui::Button("-")) {
                if (scheduler_.getNumberOfWorkers() > 1)
                    scheduler_.setWorkerPoolSize(scheduler_.getNumberOfWorkers() - 1);
            }
            ImGui::SameLine();
            std::string text = std::to_string(scheduler_.getNumberOfWorkers());
            ImGui::Text(text.c_str());

            ImGui::Separator();
            std::string name;
            for (auto job_id : jobs_) {
                Job jobInfo = scheduler_.getJobInfo(job_id);
                if (jobInfo.state == Job::JOB_STATE_PENDING) {
                    name = std::string("Job ") + std::to_string(job_id) + std::string(" (P)");
                    ImGui::Text(name.c_str());
                    ImGui::SameLine();
                    ImGui::PushID(jobInfo.id);
                    if(ImGui::Button("Cancel")) {
                        scheduler_.stopJob(jobInfo.id);
                    }
                    ImGui::PopID();
                }
                else if (jobInfo.state == Job::JOB_STATE_RUNNING) {
                    float progress = jobInfo.progress;
                    name = std::string("Job ") + std::to_string(job_id) + std::string(" ( ") +
                            std::to_string(progress) + std::string("%)");
                    ImGui::Text(name.c_str());
                    ImGui::SameLine();
                    ImGui::PushID(jobInfo.id);
                    if(ImGui::Button("Cancel")) {
                        scheduler_.stopJob(jobInfo.id);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::Separator();
            for (int i = jobs_.size() - 1;i >= 0; i--) {
                Job jobInfo = scheduler_.getJobInfo(jobs_[i]);
                if (jobInfo.state == Job::JOB_STATE_FINISHED) {
                    name = std::string("Job ") + std::to_string(jobs_[i]) + std::string(" (F)");
                    ImGui::Text(name.c_str());
                }
                if (jobInfo.state == Job::JOB_STATE_CANCELED || jobInfo.state == Job::JOB_STATE_ABORTED) {
                    name = std::string("Job ") + std::to_string(jobs_[i]) + std::string(" (C)");
                    ImGui::Text(name.c_str());
                }
            }
            ImGui::End();
        }
    };
}

#endif //BM_SEGMENTER_TEST_WINDOW_H
