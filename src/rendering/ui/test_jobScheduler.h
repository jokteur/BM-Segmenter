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
        EventQueue & event_queue_;
        int counter_ = 0;
        jobId job_id_;
        std::vector<JobReference> jobs_;
        bool open_ = true;

        std::string to_state(Job::jobState state) {
            switch (state) {
                case Job::JOB_STATE_PENDING:
                    return "PENDING";
                case Job::JOB_STATE_RUNNING:
                    return "RUNNING";
                case Job::JOB_STATE_FINISHED:
                    return "FINISHED";
                case Job::JOB_STATE_ERROR:
                    return "ERROR";
                case Job::JOB_STATE_CANCELED:
                    return "CANCELED";
                case Job::JOB_STATE_ABORTED:
                    return "ABORTED";
                default:
                    return "NOTEXISTING";
            }
        }

        Listener listener{.filter="jobs/names/*",
                          .callback = [this] (Event_ptr &event) {
                auto job = JOBEVENT_PTRCAST(event.get());
                std::cout << "Job finished: " << job->getJob().id <<
                "State : " << to_state(job->getJob().state) << std::endl;
            }
        };
    public:
        MyWindow() : scheduler_(JobScheduler::getInstance()), event_queue_(EventQueue::getInstance()) {
            scheduler_.setWorkerPoolSize(2);
            event_queue_.subscribe(&listener);
        }

        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override {
            ImGui::Begin("My Window");
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

            if(ImGui::Button("LOWEST")) {
                std::string name = "foo";
                jobs_.push_back(scheduler_.addJob(name, job, Job::JOB_PRIORITY_LOWEST));
                counter_++;
            }
            ImGui::SameLine();
            if(ImGui::Button("LOW")) {
                std::string name = "foo";
                jobs_.push_back(scheduler_.addJob(name, job, Job::JOB_PRIORITY_LOW));
                counter_++;
            }
            ImGui::SameLine();
            if(ImGui::Button("NORMAL")) {
                std::string name = "foo";
                jobs_.push_back(scheduler_.addJob(name, job, Job::JOB_PRIORITY_NORMAL));
                counter_++;
            }
            ImGui::SameLine();
            if(ImGui::Button("HIGH")) {
                std::string name = "foo";
                jobs_.push_back(scheduler_.addJob(name, job, Job::JOB_PRIORITY_HIGH));
                counter_++;
            }
            ImGui::SameLine();
            if(ImGui::Button("HIGHEST")) {
                std::string name = "foo";
                jobs_.push_back(scheduler_.addJob(name, job, Job::JOB_PRIORITY_HIGHEST));
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
            for (auto &jobRef : jobs_) {
                Job jobInfo = *jobRef.it;
                if (jobInfo.state == Job::JOB_STATE_PENDING) {
                    std::string priority = "(N)";
                    if (jobInfo.priority == Job::JOB_PRIORITY_HIGHEST)
                        priority = "(H+)";
                    if (jobInfo.priority == Job::JOB_PRIORITY_HIGH)
                        priority = "(H)";
                    if (jobInfo.priority == Job::JOB_PRIORITY_LOW)
                        priority = "(L)";
                    if (jobInfo.priority == Job::JOB_PRIORITY_LOWEST)
                        priority = "(L-)";
                    name = std::string("Job ") + std::to_string(jobInfo.id) + priority;
                    ImGui::Text(name.c_str());
                    ImGui::SameLine();
                    ImGui::PushID(jobInfo.id);
                    if (ImGui::Button("Cancel")) {
                        scheduler_.stopJob(jobRef);
                    }
                    ImGui::PopID();
                } else if (jobInfo.state == Job::JOB_STATE_RUNNING) {
                    float progress = jobInfo.progress;
                    name = std::string("Job ") + std::to_string(jobInfo.id) + std::string(" ( ") +
                           std::to_string(progress) + std::string("%)");
                    ImGui::Text(name.c_str());
                    ImGui::SameLine();
                    ImGui::PushID(jobInfo.id);
                    if (ImGui::Button("Cancel")) {
                        scheduler_.stopJob(jobRef);
                    }
                    ImGui::PopID();
                }
            }
            ImGui::Separator();
            for (int i = jobs_.size() - 1;i >= 0; i--) {
                Job jobInfo = *jobs_[i].it;
                if (jobInfo.state == Job::JOB_STATE_FINISHED) {
                    name = std::string("Job ") + std::to_string(jobInfo.id) + std::string(" (F)");
                    ImGui::Text(name.c_str());
                }
                if (jobInfo.state == Job::JOB_STATE_CANCELED || jobInfo.state == Job::JOB_STATE_ABORTED) {
                    name = std::string("Job ") + std::to_string(jobInfo.id) + std::string(" (C)");
                    ImGui::Text(name.c_str());
                }
            }
            ImGui::End();
            ImGui::ShowDemoWindow(&open_);
        }
    };
}

#endif //BM_SEGMENTER_TEST_WINDOW_H
