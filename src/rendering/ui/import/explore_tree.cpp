#include "explore_tree.h"
#include "rendering/ui/widgets/util.h"
#include "rendering/drag_and_drop.h"
#include "rendering/views/project_view.h"
#include "settings.h"

#include "log.h"

namespace dataset = ::core::dataset;

Rendering::ExploreFolder::ExploreFolder(ImGuiID docking_id) : AbstractLayout(docking_id) {
    log_listener_.callback = [=](Event_ptr &event) {
        auto log = LOGEVENT_PTRCAST(event.get());
        std::string message = log->getMessage() + "\n";
        log_buffer_.append(message.c_str());
        BM_DEBUG("Dicom search: " + message);
    };
    log_listener_.filter = "log/dicom_search";

    error_listener_.callback = [=](Event_ptr& event) {
        auto log = LOGEVENT_PTRCAST(event.get());
        std::string message = log->getMessage() + "\n";
        error_buffer_.append(message.c_str());
        BM_DEBUG("Dicom error: " + message);
    };
    error_listener_.filter = "log/dicom_error";

    EventQueue::getInstance().subscribe(&log_listener_);
    EventQueue::getInstance().subscribe(&error_listener_);
}

void Rendering::ExploreFolder::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    // Open, and cancel a new folder

    // TODO: correct docking
//    if (docking_id_) {
//        std::cout << docking_id_ << std::endl;
//        ImGui::SetNextWindowDockID(docking_id_);
//    }
    ImGui::Begin("Find dicoms in folder");
    if (explorer_->getStatus() == ::core::dataset::Explore::EXPLORE_WORKING) {
        if (ImGui::Button("Stop search")) {
            BM_DEBUG("Stop search");
            JobScheduler::getInstance().stopJob(explorer_->getJobReference());
        }
        ImGui::SameLine();
        Widgets::HelpMarker("Cancel the current on-going search.\n"
                            "Does not erase the found results.");
    } else {
        if (ImGui::Button("Explore folder")) {
            NFD_Init();
            nfdchar_t *outPath;
            nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
            if (result == NFD_OKAY) {
                log_buffer_.clear();
                error_buffer_.clear();
                case_filter_.Clear();
                study_filter_.Clear();
                series_filter_.Clear();
                EventQueue::getInstance().post(Event_ptr(new Event("dataset/dicom/reset")));
                explorer_->findDicoms(outPath);
                path_ = outPath;

                BM_DEBUG("Begin folder exploration");
            } else if (result == NFD_ERROR) {
                show_error_modal("Error: find dicoms",
                                 "An error has occurred when opening the folder.",
                                 NFD_GetError());
            }
            NFD_Quit();
        }
        ImGui::SameLine();
        Widgets::HelpMarker("Open a folder on the computer or an external hard-drive.\n"
                            "This will explore the folder and all sub-folders and try to find"
                            " dicom images.\n"
                            "This step does not import the images as a dataset. You can"
                            " do this later once the process is finished.");
    }
    // Status indicator
    ImGui::Text("Status:");
    ImGui::Separator();
    switch (explorer_->getStatus()) {
        case dataset::Explore::EXPLORE_WORKING:
            build_tree_ = true;
            display_import_button_ = false;
            is_new_tree_ = true;
            set_tree_closed_ = true;
            ImGui::Text("Searching the folder %s...", path_.c_str());
            break;
        case dataset::Explore::EXPLORE_SUCCESS:
            ImGui::Text("Finished searching. Found the following results:");
            break;
        case dataset::Explore::EXPLORE_CANCELLED:
            ImGui::Text("Cancelled last search");
            break;
        case dataset::Explore::EXPLORE_ERROR:
            ImGui::TextColored(ImVec4(200.f/255.f, 0.f,0.f,1.f), "Error when searching the folder.");
            break;
        case dataset::Explore::EXPLORE_PARTIAL_SUCCESS:
            ImGui::Text("Finished searching. Found some errors.");
            break;
        case dataset::Explore::EXPLORE_SLEEPING:
            ImGui::Text("Open a folder to start searching for dicoms.");
            break;
    }
    ImGui::BeginChild("patient_ids", ImVec2(0, -60), true, ImGuiWindowFlags_HorizontalScrollbar);

    if (close_view_) {
        if (JobScheduler::getInstance().stopJob(explorer_->getJobReference())) {
            EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
        }
    }
    else {
        // Draw the log of the exploration
        if (explorer_->getStatus() == dataset::Explore::EXPLORE_WORKING) {
            ImGui::Separator();
            ImGui::BeginChild("log_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::TextUnformatted(log_buffer_.begin(), log_buffer_.end());
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
        }
        // Draw the result and / or the errors
        else if (explorer_->getStatus() != dataset::Explore::EXPLORE_SLEEPING) {
            ImGui::Separator();
            display_import_button_ = true;

            // Error log
            if (!error_buffer_.empty()) {
                ImGui::BeginChild("error_scroll", ImVec2(0, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextUnformatted(error_buffer_.begin(), error_buffer_.end());
                ImGui::EndChild();
            }

            // Show all the cases found
            ImGui::Separator();
            ImGui::Text("Found %llu images.", explorer_->getCases()->size());

            if (ImGui::CollapsingHeader("Filters", ImGuiTreeNodeFlags_None)) {
                ImGui::SameLine();
                Widgets::HelpMarker("Filter usage:\n"
                    "  \"\"         display all lines\n"
                    "  \"xxx\"      display lines containing \"xxx\"\n"
                    "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
                    "  \"-xxx\"     hide lines containing \"xxx\"");

                case_filter_.Draw("Case ID filter");
                study_filter_.Draw("Study filter");
                series_filter_.Draw("Modality filter");
            }
            if (case_filter_str_ != std::string(case_filter_.InputBuf)
                || study_filter_str_ != std::string(study_filter_.InputBuf)
                || series_filter_str_ != std::string(series_filter_.InputBuf)) {
                case_filter_str_ = std::string(case_filter_.InputBuf);
                study_filter_str_ = std::string(study_filter_.InputBuf);
                series_filter_str_ = std::string(series_filter_.InputBuf);
                build_tree_ = true;
            }

            build_tree();

            ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
            auto disabled_text_color = Settings::getInstance().getColors().disabled_text;
            for (auto& patient : *explorer_->getCases()) {
                // Check if node must be display (because of filters)
                if (patient.tree_count < 4) {
                    continue;
                }

                // Check if node is disabled
                bool patient_active = patient.is_active;
                if (!patient_active)
                    ImGui::PushStyleColor(ImGuiCol_Text, disabled_text_color);

                // Patient ID node
                bool node1 = ImGui::TreeNodeEx((void*)&patient, nodeFlags, "Case ID: %s", patient.ID.c_str());
                exclude_menu(patient.is_active, "case");

                if (node1) {
                    for (auto& study : patient.study) {
                        // Check if node must be display (because of filters)
                        if (study.tree_count < 3) {
                            continue;
                        }

                        // Check if node is disabled but not from parent
                        bool study_active = study.is_active && patient_active;
                        if (!study_active)
                            ImGui::PushStyleColor(ImGuiCol_Text, disabled_text_color);
                        // Study node
                        bool node2 = ImGui::TreeNodeEx((void*)&study, nodeFlags, "%s", study.description.c_str());
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("%s at %s", study.date.c_str(), study.time.c_str());
                        }
                        exclude_menu(study.is_active, "study");

                        if (node2) {
                            for (auto& series : study.series) {
                                if (series->tree_count < 2) {
                                    continue;
                                }


                                bool series_active = series->is_active && study_active && patient_active;
                                if (!series_active)
                                    ImGui::PushStyleColor(ImGuiCol_Text, disabled_text_color);
                                // Series node
                                if (set_tree_closed_) {
                                    ImGui::SetNextTreeNodeOpen(false);
                                }
                                bool node3 = ImGui::TreeNodeEx((void*)&series, nodeFlags | ImGuiTreeNodeFlags_Framed, "Series %s, Modality %s",
                                    series->number.c_str(), series->modality.c_str());
                                exclude_menu(series->is_active, "series");

                                if (node3) {
                                    for (auto& image : series->images) {
                                        if (!image.tree_count) {
                                            continue;
                                        }
                                        ImGuiTreeNodeFlags leaf_flag = nodeFlags | ImGuiTreeNodeFlags_Bullet;

                                        bool image_active =
                                            image.is_active && series_active && study_active && patient_active;
                                        if (!image_active)
                                            ImGui::PushStyleColor(ImGuiCol_Text, disabled_text_color);

                                        // Image node
                                        bool node4 = ImGui::TreeNodeEx((void*)&image, leaf_flag, "%s",
                                            image.number.c_str());
                                        exclude_menu(image.is_active, "image");
                                        if (node4)
                                            ImGui::TreePop();
                                        if (!image_active)
                                            ImGui::PopStyleColor();
                                    }
                                    ImGui::TreePop();
                                }
                                if (!series_active)
                                    ImGui::PopStyleColor();
                            }
                            ImGui::TreePop();
                        }
                        if (!study_active)
                            ImGui::PopStyleColor();
                    }
                    ImGui::TreePop();
                }
                if (!patient_active)
                    ImGui::PopStyleColor();
            }
            set_tree_closed_ = false;
        }
    }
    ImGui::EndChild();


    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, .1f, 0.f, 1.f));
    if (ImGui::Button("Cancel import")) {
        BM_DEBUG("Cancel import, go back to project");
        JobScheduler::getInstance().stopJob(explorer_->getJobReference());
        close_view_ = true;
    }
    // Cancel and import buttons
    if (display_import_button_) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, .8f, 0.f, 1.f));
        if (ImGui::Button("Import data to project")) {
            BM_DEBUG("Open import modal");
            import_modal_.showModal(explorer_->getCases());
        }
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor();
    ImGui::End();
}

Rendering::ExploreFolder::~ExploreFolder() {
    JobScheduler::getInstance().stopJob(explorer_->getJobReference());
    close_view_ = true;
    EventQueue::getInstance().unsubscribe(&log_listener_);
    EventQueue::getInstance().unsubscribe(&error_listener_);
}

void Rendering::ExploreFolder::build_tree() {
    if (build_tree_) {
        // Build tree, first look which nodes should be drawn
        BM_DEBUG("Build tree");
        ::core::dataset::build_tree(explorer_->getCases(), case_filter_, study_filter_, series_filter_);
        if (is_new_tree_)
            EventQueue::getInstance().post(Event_ptr(new ::core::dataset::ExplorerBuildEvent(explorer_->getCases())));
        is_new_tree_ = false;
        build_tree_ = false;
    }
}

void Rendering::ExploreFolder::exclude_menu(bool &is_active, const std::string &desc) {
    if (ImGui::BeginPopupContextItem()) {
        if (is_active) {
            if (ImGui::Selectable((std::string("Exclude ") + desc).c_str()))
                is_active = !is_active;
        }
        else {
            if (ImGui::Selectable((std::string("Include ") + desc).c_str()))
                is_active = !is_active;
        }
        ImGui::EndPopup();
    }
}
