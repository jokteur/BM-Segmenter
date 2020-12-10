#include "import_modal.h"

#include "rendering/ui/widgets/progress_bar.h"
#include "rendering/views/project_view.h"
#include "rendering/ui/modales/error_message.h"
#include "rendering/keyboard_shortcuts.h"
#include "rendering/animation_util.h"

namespace Rendering {
    ImportDataModal::ImportDataModal() : project_manager_(::core::project::ProjectManager::getInstance()) {
        draw_fct = [this](bool& show, bool& enter, bool& escape) {
            show = false;
        };
        enter_shortcut_.keys = { KEY_ENTER };
        enter_shortcut_.name = "enter";
        enter_shortcut_.callback = [this] {
            confirm_ = true;
        };
    }
    void ImportDataModal::showModal(std::shared_ptr<std::vector<::core::dataset::PatientNode>> cases) {
        int num_series = 0;
        int num_excluded_series = 0;
        int num_images = 0;
        int num_excluded_images = 0;
        for (auto& patient : *cases) {
            for (auto& study : patient.study) {
                for (auto& series : study.series) {
                    bool is_active = series->is_active && study.is_active && patient.is_active;
                    if (is_active) {
                        int prev_num_images = num_images;
                        for (auto& image : series->images) {
                            if (image.is_active) {
                                num_images++;
                            }
                            else {
                                num_excluded_images++;
                            }
                        }
                        if (prev_num_images != num_images) {
                            num_series++;
                        }
                        else {
                            num_excluded_series++;
                        }
                    }
                    else {
                        num_excluded_series++;
                    }
                }
            }
        }

        auto& dataset = project_manager_.getCurrentProject()->getDataset();

        build_names_ = false;
        confirm_ = false;
        start_work_ = false;
        job_finished_ = false;
        item_select_ = 0;
        progress = 0.f;

        draw_fct = [=, &dataset](bool& show, bool& enter, bool& escape) {
            if (num_images != num_series) {
                ImGui::Text("You are about to import %d series, for a total of %d images", num_series, num_images);
                if (num_excluded_images == 1) {
                    ImGui::Text("One image has been excluded.", num_excluded_images);
                }
                else if (num_excluded_images > 1) {
                    ImGui::Text("%d images have been excluded.", num_excluded_images);
                }
                if (num_excluded_series == 1) {
                    ImGui::Text("One series has been excluded.");
                }
                else if (num_excluded_series > 1) {
                    ImGui::Text("%d series have been excluded.", num_excluded_series);
                }
            }
            else {
                ImGui::Text("You are about to import %d images", num_series, num_images);
                if (num_excluded_series == 1) {
                    ImGui::Text("One series has been excluded.");
                }
                else if (num_excluded_series > 1) {
                    ImGui::Text("%d series have been excluded.", num_excluded_series);
                }
            }
            ImGui::Separator();

            ImGui::Checkbox("Replace data in project if name conflict.", &replace_);

            auto& groups = dataset.getGroups();
            if (groups.empty()) {
                ImGui::Text("No group. Please create a group.  ");
            }
            else {
                if (!build_names_) {
                    build_names_ = false;
                    group_names_.clear();
                    for (auto& group : groups) {
                        group_names_.push_back(group.getName().c_str());
                    }
                }
                const char* combo_label = group_names_[item_select_];
                if (ImGui::BeginCombo("Select group", combo_label)) {
                    int n = 0;
                    for (auto& group : groups) {
                        const bool is_selected = (item_select_ == n);
                        if (ImGui::Selectable(group_names_[n], is_selected))
                            item_select_ = n;

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();

                        n++;
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::SameLine();
            ImGui::Button("Create group");
            if (ImGui::BeginPopupContextItem("Create group", 0)) {
                ImGui::InputText("Name of group", &group_name_);
                KeyboardShortCut::addTempShortcut(enter_shortcut_);
                KeyboardShortCut::ignoreNormalShortcuts();

                if ((ImGui::Button("OK") || confirm_) && !group_name_.empty()) {
                    dataset.createGroup(group_name_);
                    group_name_ = "";
                    item_select_ = dataset.getGroups().size() - 1;
                    ImGui::CloseCurrentPopup();
                    confirm_ = false;
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button("Cancel") || escape)
                show = false;

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, .8f, 0.f, 1.f));
            if (ImGui::Button("Import")) {
                if (groups.empty()) {
                    show_error_modal("No group selected.", "Before importing the data, you have to create a group.");
                }
                else {
                    start_work_ = true;
                    job_id_ = dataset.importData(
                        dataset.getGroups()[item_select_],
                        cases, 
                        project_manager_.getCurrentProject()->getRoot(), 
                        [=](const std::shared_ptr<JobResult>& result) {
                            import_result_ = std::dynamic_pointer_cast<::core::dataset::ImportResult>(result);
                            job_finished_ = true;
                        },
                        replace_
                    );

                    Modals::getInstance().stackModal(
                        "Importing...", 
                        [=, &dataset, &show](bool& show_, bool& enter_, bool& escape_) {
                            auto& job_info = JobScheduler::getInstance().getJobInfo(job_id_);
                            push_animation();

                            const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
                            const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
                            ImGui::Text("Import image %d / %d", (int)(job_info.progress*num_images), num_images);
                            ImGui::Spinner("##spinner", 15, 6, col);
                            ImGui::BufferingBar("##buffer_bar", job_info.progress, ImVec2(400, 6), bg, col);

                            if (job_info.name.empty()) {
                                ImGui::CloseCurrentPopup();
                            }
                        }, 
                        ImGuiWindowFlags_None, 
                        true
                    );
                }
            }
            ImGui::PopStyleColor();

            if (job_finished_) {
                Modals::getInstance().stackModal(
                    "Import result",
                    [=, &dataset, &show](bool& show_, bool& enter_, bool& escape_) {
                        if (import_result_->success) {
                            ImGui::Text("%d images successfully imported into the project.", import_result_->save_paths.size());
                            if (!import_result_->existing.empty()) {
                                ImGui::Text("The following images have not been imported because they were already in the project.");
                                ImGui::BeginChild("existing_scroll", ImVec2(0, 400));
                                for (auto& dicom : import_result_->existing) {
                                    ImGui::BulletText(dicom.getId().c_str());
                                }
                                ImGui::EndChild();
                            }
                        }
                        else {
                            ImGui::Text("When importing the images, the following error happened:");
                            ImGui::Separator();
                            ImGui::Text(import_result_->error_msg.c_str());
                        }

                        if (ImGui::Button("OK")) {
                            if (import_result_->success) {
                                std::string err = dataset.registerFiles(import_result_->save_paths, dataset.getGroups()[item_select_], project_manager_.getCurrentProject()->getRoot());
                                if (!err.empty()) { // TODO: correct error handling
                                    std::cout << err << std::endl;
                                }
                                EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                            }
                            show = false;
                            ImGui::CloseCurrentPopup();
                        }
                    },
                    ImGuiWindowFlags_None,
                    true
                );
            }
        };
        Modals::getInstance().setModal("Import data into project", draw_fct, ImGuiWindowFlags_AlwaysAutoResize);
    }
}