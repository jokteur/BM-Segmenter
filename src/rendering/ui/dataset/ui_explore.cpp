#include "ui_explore.h"
#include "rendering/ui/widgets/util.h"
#include "rendering/drag_and_drop.h"

namespace dataset = ::core::dataset;

Rendering::ExploreFolder::ExploreFolder() {
    log_listener_.callback = [=](Event_ptr &event) {
        auto log = LOGEVENT_PTRCAST(event.get());
        std::string message = log->getMessage() + "\n";
        log_buffer_.append(message.c_str());
    };
    log_listener_.filter = "log/dicom_search";

    error_listener_.callback = [=](Event_ptr &event) {
        auto log = LOGEVENT_PTRCAST(event.get());
        std::string message = log->getMessage() + "\n";
        error_buffer_.append(message.c_str());
    };
    error_listener_.filter = "log/dicom_error";


    EventQueue::getInstance().subscribe(&log_listener_);
    EventQueue::getInstance().subscribe(&error_listener_);
}

void Rendering::ExploreFolder::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    // Open, and cancel a new folder
    ImGui::Begin("Find dicoms in folder");
    if (explorer_.getStatus() == ::core::dataset::Explore::EXPLORE_WORKING) {
        if (ImGui::Button("Cancel")) {
            JobScheduler::getInstance().stopJob(explorer_.getJobReference());
            std::cout << explorer_.getJobReference().getJob()->exception.what() << std::endl;
        }
        ImGui::SameLine();
        Widgets::HelpMarker("Cancel the current on-going search.\n"
                            "Does not erase the found results.");
    } else {
        if (ImGui::Button("Open folder")) {
            NFD_Init();
            nfdchar_t *outPath;
            nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
            if (result == NFD_OKAY) {
                log_buffer_.clear();
                error_buffer_.clear();
                case_filter_.Clear();
                study_filter_.Clear();
                series_filter_.Clear();
                explorer_.findDicoms(outPath);
                path_ = outPath;
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
                            "do this later once the process is finished.");
    }
    // Status indicator
    ImGui::Text("Status:");
    ImGui::Separator();
    switch (explorer_.getStatus()) {
        case dataset::Explore::EXPLORE_WORKING:
            build_tree_ = true;
            is_new_tree_ = true;
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

    // Draw the log of the exploration
    if (explorer_.getStatus() == dataset::Explore::EXPLORE_WORKING) {
        ImGui::Separator();
        ImGui::BeginChild("log_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(log_buffer_.begin(), log_buffer_.end());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }
    // Draw the result and / or the errors
    else if (explorer_.getStatus() != dataset::Explore::EXPLORE_SLEEPING) {
        ImGui::Separator();

        // Error log
        if (!error_buffer_.empty()) {
            ImGui::BeginChild("error_scroll", ImVec2(0, 200), false, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::TextUnformatted(error_buffer_.begin(), error_buffer_.end());
            ImGui::EndChild();
        }

        // Show all the cases found
        ImGui::Separator();
        ImGui::Text("Found %llu images.", explorer_.getCases().size());

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
            is_new_tree_ = false;
        }

        build_tree();

        ImGui::BeginChild("patient_ids", ImVec2(0, -10), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;
        ImGui::Columns(2, "dicom_explore_col");
        for (auto &patient : explorer_.getCases()) {
            if (patient.tree_count < 4) {
                continue;
            }
            bool node1 = ImGui::TreeNodeEx((void*)&patient, nodeFlags, "Case ID: %s", patient.ID.c_str());
            ImGui::NextColumn(); ImGui::NextColumn();
            if (!node1)
                continue;

            int i = 0;
            for (auto &study : patient.study) {
                if (study.tree_count < 3) {
                    continue;
                }
                bool node2 = ImGui::TreeNodeEx((void*)&study, nodeFlags, "%s", study.description.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s at %s", study.date.c_str(), study.time.c_str());
                }
                ImGui::NextColumn(); ImGui::NextColumn();
                if (!node2)
                    continue;

                for (auto &series : study.series) {
                    if (series.tree_count < 2) {
                        continue;
                    }
                    bool node3 = ImGui::TreeNodeEx((void*)&series, nodeFlags, "Series %s, Modality %s", series.number.c_str(), series.modality.c_str());
                    if (!node3)
                        continue;

                    if (ImGui::BeginDragDropSource()) {
                        dataset::SeriesPayload payload;
                        payload.series = series;
                        payload.case_.patientID = patient.ID;
                        payload.case_.studyDescription = study.description;
                        payload.case_.studyDate = study.date;
                        payload.case_.studyTime = study.time;
                        auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
                        drag_and_drop.giveData(payload);

                        int a = 0; // Dummy int
                        ImGui::SetDragDropPayload("_DICOM_VIEW", &a, sizeof(a));
                        ImGui::Text("Drag Series %s to viewer to visualize", series.number.c_str());
                        ImGui::EndDragDropSource();
                    }
                    if(ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("You can drag and drop the entire series to the DICOM viewer to visualize");
                    }
                    ImGui::NextColumn();
                    ImGui::NextColumn();

                    for (auto &image : series.images) {
                        if (!image.tree_count) {
                            continue;
                        }
                        ImGuiTreeNodeFlags leaf_flag = nodeFlags | ImGuiTreeNodeFlags_Bullet;
                        bool node4 = ImGui::TreeNodeEx((void*)&image, leaf_flag, "%s", image.number.c_str());
                        if (node4)
                            ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        ImGui::Columns(1);
        ImGui::EndChild();
    }
    ImGui::End();
}

Rendering::ExploreFolder::~ExploreFolder() {
    EventQueue::getInstance().unsubscribe(&log_listener_);
    EventQueue::getInstance().unsubscribe(&error_listener_);
}

void Rendering::ExploreFolder::build_tree() {
    if (build_tree_) {
        // Build tree, first look which nodes should be drawn
        ::core::dataset::build_tree(explorer_.getCases(), case_filter_, study_filter_, series_filter_);
        if (is_new_tree_)
            EventQueue::getInstance().post(Event_ptr(new ::core::dataset::ExplorerBuildEvent(explorer_.getCases())));
        else
            EventQueue::getInstance().post(Event_ptr(new ::core::dataset::ExplorerFilterEvent(case_filter_, study_filter_, series_filter_)));
        is_new_tree_ = false;
        build_tree_ = false;
    }
}
