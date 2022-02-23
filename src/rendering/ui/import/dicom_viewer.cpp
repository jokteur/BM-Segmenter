#include "dicom_viewer.h"
#include "log.h"

#include "imgui_stdlib.h"

#include "core/dataset/dicom_to_image.h"
#include "core/dataset/extract_view_from_dicom.h"
#include "core/project/project_manager.h"

#include "rendering/ui/modales/error_message.h"
#include "rendering/drag_and_drop.h"
#include "rendering/ui/widgets/util.h"
#include "rendering/ui/widgets/progress_bar.h"
#include "rendering/animation_util.h"

int Rendering::DicomViewer::instance_number = 0;
namespace dataset = ::core::dataset;


Rendering::DicomViewer::DicomViewer()
    : windowing_button_(
            "assets/windowing.png",
            "assets/windowing_dark.png",
            true,
            "Windowing option \n"
            "When active, click and drag up and down to change the window center\n"
            "and drag left to right to change to change the window length"
            ),
      point_select_button_(
              "assets/point_select.png",
              "assets/point_select_dark.png",
              true,
              "Point and click in image to set sagittal and coronal view"
              )
{
    my_instance_ = instance_number++;
    identifier_ = std::to_string(my_instance_) + std::string("DicomViewer");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    image_widget_.setCenterY(true);
    image_widget_.setCenterX(true);

    sagittal_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);
    sagittal_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
//    sagittal_widget_.setCenterY(true);
    sagittal_widget_.setCenterX(true);

    coronal_widget_.setInteractiveZoom(SimpleImage::IMAGE_NORMAL_INTERACT);
    coronal_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
//    coronal_widget_.setCenterY(true);
    coronal_widget_.setCenterX(true);

    // Listen to selection in the Dicom explorer
    listener_.callback = [=](Event_ptr &event) {
        auto log = reinterpret_cast<dataset::SelectSeriesEvent*>(event.get());
        loadSeries(log->getSeries());
    };
    listener_.filter = "dataset/dicom_open";


    reset_tree_listener_.callback = [=](Event_ptr& event) {
        if (series_node_ != nullptr) {
            series_node_->data.cancelPendingJobs();
            series_node_->data.unloadAll();
            series_node_ = nullptr;
            image_.reset();
            image_widget_.setImage(image_);
        }
    };
    reset_tree_listener_.filter = "dataset/dicom/reset";

    EventQueue::getInstance().subscribe(&listener_);
    EventQueue::getInstance().subscribe(&reset_tree_listener_);

}

Rendering::DicomViewer::~DicomViewer() {
    EventQueue::getInstance().unsubscribe(&listener_);
    EventQueue::getInstance().unsubscribe(&reset_tree_listener_);
}

void Rendering::DicomViewer::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    button_logic();
    windowing_widget_logic();
    point_select_widget_logic();
    display_reference_lines();

    header_window(window, parent_dimension);
    axial_view_window(window, parent_dimension);
    side_view_windows(window, parent_dimension);
    tmp_context_menu_open = false;
}

void Rendering::DicomViewer::loadCase(int idx) {
    series_node_->data.loadCase(idx, false, [=] (const ::core::Dicom& dicom) {
        reset_axial_image_ = true;
    });
}

int Rendering::DicomViewer::calculate_case_idx(float position) {
    if (position >= 1.f)
        return (float)series_node_->data.size();
    return (int)(position * ((float)series_node_->data.size())) + 1;
}

void Rendering::DicomViewer::build_views() {
    if (series_node_ == nullptr || series_node_->data.size() < 5) {
        return;
    }
    views_set_ = false;

    is_sagittal_ready_ = false;
    is_coronal_ready_ = false;

    // Sagittal
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto view_result = std::dynamic_pointer_cast<::core::dataset::DicomViewResult>(result);
        if (view_result == nullptr) {
            BM_DEBUG("Failed to build sagittal view");
            return;
        }
        sagittal_matrix_ = view_result->image;

        if (sagittal_matrix_.data.rows <= 0 || sagittal_matrix_.data.cols <= 0) {
            return;
        }

        // Resize to correct aspect ratio
        float sag_aspect = sagittal_matrix_.pixel_spacing.y / sagittal_matrix_.slice_thickness;
        cv::Mat mat;
        cv::resize(
                sagittal_matrix_.data,
                mat,
                cv::Size((int)((float)sagittal_matrix_.data.rows*sag_aspect), sagittal_matrix_.data.cols),
                0,
                0,
                cv::INTER_CUBIC);
        sagittal_matrix_.data = mat;

        is_sagittal_ready_ = true;
    };
    dataset::extract_view(series_node_->data.getData(), fct, sagittal_x_, false);

    // Coronal
    jobResultFct fct2 = [=] (const std::shared_ptr<JobResult> &result) {
        auto view_result = std::dynamic_pointer_cast<::core::dataset::DicomViewResult>(result);
        if (view_result == nullptr) {
            BM_DEBUG("Failed to build coronal view");
            return;
        }
        coronal_matrix_ = view_result->image;

        if (coronal_matrix_.data.rows <= 0 || coronal_matrix_.data.cols <= 0) {
            return;
        }

        // Resize to correct aspect ratio
        float cor_aspect = coronal_matrix_.slice_thickness / coronal_matrix_.pixel_spacing.x;
        cv::Mat mat;
        cv::resize(
                coronal_matrix_.data,
                mat,
                cv::Size(coronal_matrix_.data.rows, (int)((float)coronal_matrix_.data.cols*cor_aspect)),
                0,
                0,
                cv::INTER_CUBIC);

        coronal_matrix_.data = mat;
        is_coronal_ready_ = true;
    };
    dataset::extract_view(series_node_->data.getData(), fct2, coronal_x_, true);
}

void Rendering::DicomViewer::header_window(GLFWwindow* window, Rect& parent_dimension) {
    ImGui::Begin("DICOM viewer", &is_open_); // TODO: unique ID

    // Reload the images when they are ready to be
    // copied into a GL texture
    if (!views_set_ && is_sagittal_ready_ && is_coronal_ready_) {
        set_side_views();
        views_set_ = true;
    }
    if (reset_axial_image_) {
        set_image();
    }

    // Display info about the DICOM
    if (!case_.patientID.empty()) {
        ImGui::Text("Patient ID %s /", case_.patientID.c_str());
        ImGui::SameLine();
        ImGui::Text("Study %s /", case_.studyDescription.c_str());
        ImGui::SameLine();
        ImGui::Text("Series %s ", case_.seriesNumber.c_str());
        ImGui::SameLine();
        ImGui::Text("with modality %s ", case_.modality.c_str());
    }
    else {
        ImGui::Text("Drag Series or double click on Series to show DICOM");
    }

    // If there are more than one image in the series
    // Display a select widget
    if (series_node_ != nullptr && series_node_->data.size() > 1) {
        if (case_select_ != previous_select_) {
            previous_select_ = case_select_;
            if (case_select_ < 1 || case_select_ > series_node_->data.size()) {
                case_select_ = 1;
            }
            reset_axial_image_ = true;
        }
        ImGui::SliderInt((std::string("Select image###") + identifier_).c_str(), &case_select_, 1, series_node_->data.size(), "image n° %d");
        ImGui::SameLine();
        Widgets::HelpMarker("Ctrl+click to input manually the number");
    }

    // Once all the images of the series are loaded into memory,
    // build the sagittal and coronal views
    if (series_node_ != nullptr && series_node_->data.isReady() && !is_load_finished_) {
        is_load_finished_ = true;
        build_views();
    }


    // Interaction buttons
    if (series_node_ != nullptr) {
        ImGui::Text("Window Center: %d, Window Width: %d", series_node_->data.getWC(), series_node_->data.getWW());
    }
    windowing_button_.ImGuiDraw(window, dimensions_);
    if (image_.isImageSet() && series_node_ != nullptr && series_node_->data.size() > 4) {
        ImGui::SameLine();
        point_select_button_.ImGuiDraw(window, dimensions_);
    }
    if (active_button_ != nullptr) {
        ImGui::SameLine();
        Widgets::HelpMarker("You can also right clic anywhere\n"
                            "on the image to open the tool options.");
    }

    if (series_node_ != nullptr && !series_node_->data.getCurrentDicom().error_message.empty()) {
        image_.reset();
        ImGui::Text("When trying to open the DICOM file, the following error appeared:\n\n%s", series_node_->data.getCurrentDicom().error_message.c_str());
    }

    if (active_button_ == &windowing_button_) {
        if (ImGui::CollapsingHeader("Windowing options")) {
            windowing_options();
        }
    }
    else if (active_button_ == &point_select_button_) {
        if (ImGui::CollapsingHeader("Place markers for importation")) {
            point_select_options();
        }
    }

    ImGui::End();
}

void Rendering::DicomViewer::axial_view_window(GLFWwindow* window, Rect& parent_dimension) {
    // Axial window
    ImGui::Begin("Axial view"); // TODO: unique ID
    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    auto style = ImGui::GetStyle();

    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x + 2 * style.WindowPadding.x;
    dimensions_.height = content.y + 2 * style.WindowPadding.y;

    if (!is_load_finished_ && series_node_ != nullptr) {
        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
        ImGui::Text("Loading dicom...");
        push_animation();
        ImGui::BufferingBar("##Loading dicom...", (float)(num_images_loaded_ + 1.f) / series_node_->data.size(), ImVec2(400, 6), bg, col);
        ImGui::Spinner("##spinner", 15, 6, col);
    }

    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, parent_dimension);
    }
    else
        ImGui::Dummy(ImVec2(content.x, content.y - 3 * ImGui::GetItemRectSize().y));

    // Accept drag and drop from dicom explorer
    accept_drag_and_drop();
    context_menu(dimensions_);
    ImGui::End();
}

void Rendering::DicomViewer::side_view_windows(GLFWwindow* window, Rect& parent_dimension) {
    if (image_.isImageSet() && series_node_ != nullptr && series_node_->data.size() > 4) {
        // Sagittal window
        ImGui::Begin("Sagittal"); // TODO: unique ID for dockspace
        ImVec2 content = ImGui::GetContentRegionAvail();
        ImVec2 window_pos = ImGui::GetWindowPos();

        sagittal_widget_.setAutoScale(true);
        sagittal_widget_.ImGuiDraw(window, dimensions_);
        context_menu(Rect(window_pos, content));
        ImGui::End();

        // Coronal window
        ImGui::Begin("Coronal"); // TODO: unique ID for dockspace
        content = ImGui::GetContentRegionAvail();
        window_pos = ImGui::GetWindowPos();

        coronal_widget_.setAutoScale(true);
        coronal_widget_.ImGuiDraw(window, dimensions_);
        context_menu(Rect(window_pos, content));
        ImGui::End();
    }
}

void Rendering::DicomViewer::button_logic() {
    // Button logic
    if (windowing_button_.isActive() && point_select_button_.isActive()) {
        if (active_button_ == &windowing_button_) {
            windowing_button_.setState(false);
            active_button_ = &point_select_button_;
        }
        else {
            point_select_button_.setState(false);
            active_button_ = &windowing_button_;
        }
    }
    else {
        if (windowing_button_.isActive())
            active_button_ = &windowing_button_;
        else if (point_select_button_.isActive())
            active_button_ = &point_select_button_;
        else
            active_button_ = nullptr;
    }
}

void Rendering::DicomViewer::windowing_widget_logic() {
    auto& io = ImGui::GetIO();

    // Windowing tool
    if (active_button_ == &windowing_button_) {
        if (ImGui::IsMouseDown(0) && Widgets::check_hitbox(ImGui::GetMousePos(), dimensions_)) {
            active_dragging_ = true;
        }
        if (ImGui::IsMouseReleased(0)) {
            active_dragging_ = false;
            drag_delta_.x = 0;
            drag_delta_.y = 0;
        }
        if (active_dragging_ && series_node_ != nullptr && !tmp_deactivate_tool_) {
            image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
            auto drag = ImGui::GetMouseDragDelta(0, 0);
            float attenuation = 1.f;
            if (io.KeyShift)
                attenuation = 0.5f;
            if (drag.x != drag_delta_.x || drag.y != drag_delta_.y) {
                series_node_->data.getWW() += (int)((drag.x - drag_delta_.x) * attenuation);
                if (series_node_->data.getWW() < 1)
                    series_node_->data.getWW() = 1;
                series_node_->data.getWC() += (int)((drag.y - drag_delta_.y) * attenuation);
                views_set_ = false;
                drag_delta_ = drag;
            }
        }
    }
    else {
        active_dragging_ = false;
    }
    if (!active_dragging_) {
        image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    }

    //Check if the Window Width or Window Height have changed
    if (series_node_ != nullptr && (tmp_WW_ != series_node_->data.getWW() || tmp_WC_ != series_node_->data.getWC())) {
        tmp_WW_ = series_node_->data.getWW();
        tmp_WC_ = series_node_->data.getWC();
        BM_DEBUG("Set windowing");
        loadCase(case_select_ - 1);
    }
}

void Rendering::DicomViewer::windowing_options() {
    ImGui::InputInt("Set window center", &series_node_->data.getWC());
    ImGui::InputInt("Set window width", &series_node_->data.getWW());
}

void Rendering::DicomViewer::point_select_widget_logic() {
    // Point select widget
    Rect image_dimensions = image_widget_.getDimensions();
    Rect sagittal_dimensions = sagittal_widget_.getDimensions();
    Rect coronal_dimensions = coronal_widget_.getDimensions();
    if (active_button_ == &point_select_button_ && !tmp_deactivate_tool_) {
        image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
        sagittal_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
        coronal_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);

        ImVec2 mouse_pos = ImGui::GetMousePos();

        if (Widgets::check_hitbox(mouse_pos, image_dimensions)) {
            if (ImGui::IsMouseClicked(0)) {
                Crop& crop = image_widget_.getCrop();
                sagittal_x_ = (mouse_pos.x - image_dimensions.xpos) / image_dimensions.width;
                coronal_x_ = (mouse_pos.y - image_dimensions.ypos) / image_dimensions.height;

                // Correct the click corresponding to the zoom level
                sagittal_x_ = crop.x0 + sagittal_x_ * (crop.x1 - crop.x0);
                coronal_x_ = crop.y0 + coronal_x_ * (crop.y1 - crop.y0);
                build_views();
            }
        }
        else if (Widgets::check_hitbox(mouse_pos, sagittal_dimensions)) {
            if (ImGui::IsMouseClicked(0)) {
                Crop& crop = sagittal_widget_.getCrop();
                float pos_y = (mouse_pos.y - sagittal_dimensions.ypos) / sagittal_dimensions.height;
                coronal_x_ = (mouse_pos.x - sagittal_dimensions.xpos) / sagittal_dimensions.width;

                // Correct the click corresponding to the zoom level
                pos_y = crop.y0 + pos_y * (crop.y1 - crop.y0);
                coronal_x_ = crop.x0 + coronal_x_ * (crop.x1 - crop.x0);

                if (pos_y >= 0.f && pos_y <= 1.f) {
                    case_select_ = calculate_case_idx(pos_y);
                }
                build_views();
            }
        }
        else if (Widgets::check_hitbox(mouse_pos, coronal_dimensions)) {
            if (ImGui::IsMouseClicked(0)) {
                Crop& crop = coronal_widget_.getCrop();
                float pos_y = (mouse_pos.y - coronal_dimensions.ypos) / coronal_dimensions.height;
                sagittal_x_ = (mouse_pos.x - coronal_dimensions.xpos) / coronal_dimensions.width;

                // Correct the click corresponding to the zoom level
                pos_y = crop.y0 + pos_y * (crop.y1 - crop.y0);
                sagittal_x_ = crop.x0 + sagittal_x_ * (crop.x1 - crop.x0);

                if (pos_y >= 0.f && pos_y <= 1.f) {
                    case_select_ = calculate_case_idx(pos_y);
                }
                build_views();
            }
        }
    }
    else {
        image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
        sagittal_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
        coronal_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    }
}

void Rendering::DicomViewer::point_select_options() {
    auto project = core::project::ProjectManager::getInstance().getCurrentProject();
    auto markers = project->getMarkers();
    if (markers.empty()) {
        ImGui::Text("Please create a marker group.");
    }
    else {
        markers_select_.ImGuiDraw("Select marker group", 0.3 * ImGui::GetContentRegionAvail().x);
    }
    ImGui::SameLine();
    if (ImGui::Button("Create marker group")) {
        Modals::getInstance().setModal("Create marker group", [=](bool& show_, bool& enter_, bool& escape_) {
            tmp_deactivate_tool_ = true;
            ImGui::InputText("Group name", &marker_name_input_);
            ImGui::ColorEdit3("Color", marker_color_input_);
            markers_type.ImGuiDraw("Type");
            if (ImGui::Button("Create") || enter_) {
                core::DicomMarkerName marker;
                marker.color = { marker_color_input_[0], marker_color_input_[1], marker_color_input_[2], 1.f };
                marker.name = marker_name_input_;
                if (markers_type.getCurrentOption() == "Axial") {
                    marker.type = core::DicomMarkerName::AXIAL;
                }
                else if (markers_type.getCurrentOption() == "Coronal") {
                    marker.type = core::DicomMarkerName::CORONAL;
                }
                else {
                    marker.type = core::DicomMarkerName::SAGITTAL;
                }

                if (marker_name_input_.empty()) {
                    BM_DEBUG("Create marker group error: name empty");
                    show_error_modal("Create marker group error", "The name of the marker group can not be empty");
                }
                // Test if marker with same name
                else if (project->addMarkerName(marker)) {
                    BM_DEBUG("Create marker group error: another name already existing");
                    show_error_modal("Create marker group error", "Another marker group has already the same name");
                }
                else {
                    show_ = false;
                    tmp_deactivate_tool_ = false;
                    rebuild_markers_select_ = true;
                    BM_DEBUG("Create marker group");
                    marker_name_input_ = "";
                    marker_color_input_[0] = 0.;
                    marker_color_input_[1] = 0;
                    marker_color_input_[2] = 0.2;
                }
            }
            if (escape_) {
                tmp_deactivate_tool_ = false;
                show_ = false;
            }
        });
    }
    if (rebuild_markers_select_) {
        rebuild_markers_select_ = false;
        std::vector<std::string> marker_names = { "" };
    
        // Build the background colors
        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors_list = style->Colors;
        std::vector<ImVec4> colors = { colors_list[ImGuiCol_FrameBg] };
        for (auto& marker : markers) {
            marker_names.push_back(marker.name);

            ImVec4 color(marker.color);
            colors.push_back(color);
        }
        markers_select_.setOptions(marker_names, [](int) {}, colors);
    }

    if (markers_select_.getCurrentIdx() != 0) {
        ImGui::SameLine();
        if (ImGui::Button("Add marker here")) {
            core::DicomCoordinate coordinate;
            coordinate.axial = (case_select_ - 1.f) / (float)image_size_;
            coordinate.sagittal = sagittal_x_;
            coordinate.coronal = coronal_x_;

            // There should be an easier version, but right now it stays that way
            core::DicomMarkerName dummy_marker;
            dummy_marker.name = markers_select_.getCurrentOption();
            auto& marker = markers.find(dummy_marker);
            coordinate.name = *marker;
            series_node_->data.addCoordinate(coordinate);
        }
    }
    int i = 0;
    int removal_coordinate = -1;
    auto& coordinates = series_node_->data.getAllCoordinates();
    for (auto& coordinate : coordinates) {
        ImGui::Text(("Marker #" + std::to_string(i + 1) + " (").c_str());
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(coordinate.name.color));
        ImGui::SameLine();
        ImGui::Text(coordinate.name.name.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text(")");
        ImGui::SameLine();
        if (ImGui::Button(("Go to###" + std::to_string(i) + "go_to").c_str())) {
            sagittal_x_ = coordinate.sagittal;
            coronal_x_ = coordinate.coronal;
            case_select_ = calculate_case_idx(coordinate.axial);
            //loadCase(case_select_);
            build_views();
        }
        ImGui::SameLine();
        if (ImGui::Button(("Remove###" + std::to_string(i) + "remove").c_str())) {
            removal_coordinate = i;
        }
        i++;
    }
    if (removal_coordinate != -1) {
        coordinates.erase(coordinates.begin() + removal_coordinate);
    }
}

void Rendering::DicomViewer::display_reference_lines() {
    // Display reference lines
    Rect image_dimensions = image_widget_.getDimensions();
    Rect sagittal_dimensions = sagittal_widget_.getDimensions();
    Rect coronal_dimensions = coronal_widget_.getDimensions();

    if (series_node_ != nullptr && series_node_->data.size() > 4 && display_reference_lines_) {
        float vertical_pos = ((float)case_select_ - 1) / ((float)series_node_->data.size() - 1);
        image_widget_.setDrawFunction([=](ImVec2 size, Crop crop) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            // Coronal line
            Line line1 = calculate_line_coord(image_dimensions, crop, coronal_x_, true);
            draw_list->AddLine(
                line1.start,
                line1.end,
                ImColor(255, 0, 0, 100),
                4);
            // Sagittal line
            Line line2 = calculate_line_coord(image_dimensions, crop, sagittal_x_, false);
            draw_list->AddLine(
                line2.start,
                line2.end,
                ImColor(0, 255, 0, 100),
                4);
            });
        sagittal_widget_.setDrawFunction([=](ImVec2 size, Crop crop) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            // Sagittal line
            Line line1 = calculate_line_coord(sagittal_dimensions, crop, coronal_x_, false);
            draw_list->AddLine(
                line1.start,
                line1.end,
                ImColor(255, 0, 0, 100),
                4);
            // Sagittal line
            Line line2 = calculate_line_coord(sagittal_dimensions, crop, vertical_pos, true);
            draw_list->AddLine(
                line2.start,
                line2.end,
                ImColor(0, 0, 255, 100),
                4);
            });
        coronal_widget_.setDrawFunction([=](ImVec2 size, Crop crop) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            // Sagittal line
            Line line1 = calculate_line_coord(coronal_dimensions, crop, sagittal_x_, false);
            draw_list->AddLine(
                line1.start,
                line1.end,
                ImColor(0, 255, 0, 100),
                4);
            // Horizontal line
            Line line2 = calculate_line_coord(coronal_dimensions, crop, vertical_pos, true);
            draw_list->AddLine(
                line2.start,
                line2.end,
                ImColor(0, 0, 255, 100),
                4);
            });
    }
    else {
        image_widget_.setDrawFunction([](ImVec2 size, Crop crop) {});
        coronal_widget_.setDrawFunction([](ImVec2 size, Crop crop) {});
        sagittal_widget_.setDrawFunction([](ImVec2 size, Crop crop) {});
    }
}

void Rendering::DicomViewer::set_image() {
    if (series_node_ == nullptr)
        return;

    auto &dicom = series_node_->data.getData();
    if (case_select_ <= series_node_->data.size() && case_select_ > 0 && dicom[case_select_ - 1].is_set) {
        image_.setImageFromHU(series_node_->data.getData()[case_select_ - 1].data, (float)series_node_->data.getWW(), (float)series_node_->data.getWC());
        image_widget_.setImage(image_);
        reset_axial_image_ = false;
    }
}

void Rendering::DicomViewer::set_side_views() {
    if (series_node_ == nullptr)
        return;
    if (is_sagittal_ready_ && is_coronal_ready_) {
        sagittal_image_.setImageFromHU(sagittal_matrix_.data, (float)series_node_->data.getWW(), (float)series_node_->data.getWC());
        coronal_image_.setImageFromHU(coronal_matrix_.data, (float)series_node_->data.getWW(), (float)series_node_->data.getWC());
        sagittal_widget_.setImage(sagittal_image_);
        coronal_widget_.setImage(coronal_image_);
    }
}

void Rendering::DicomViewer::loadSeries(const dataset::SeriesPayload &data) {
    if (series_node_ != nullptr) {
        series_node_->data.cancelPendingJobs();
        series_node_->data.unloadAll();
    }
    // Reset things
    windowing_button_.setState(false);
    case_select_ = 1;
    is_load_finished_ = false;
    is_sagittal_ready_ = is_coronal_ready_ = false;
    num_images_loaded_ = 0;

    sagittal_image_.reset();
    coronal_image_.reset();
    image_.reset();
    sagittal_widget_.setImage(sagittal_image_);
    coronal_widget_.setImage(coronal_image_);
    image_widget_.setImage(image_);


    // Set the newest data
    series_node_ = data.series;
    image_size_ = series_node_->data.size();
    case_ = data.case_;

    series_node_->data.loadAll([=](const ::core::Dicom& dicom) {
        num_images_loaded_++;
    });
    reset_axial_image_ = true;
}

Rendering::Line
Rendering::DicomViewer::calculate_line_coord(const Rect &dimensions, const Crop &crop, float position, bool horizontal) {
    Line line;
    if (horizontal) {
        float corrected_pos = (position - crop.y0)/(crop.y1 - crop.y0);

        if (corrected_pos < 0.f || corrected_pos > 1.f)
            return line;

        line.start = ImVec2(dimensions.xpos, dimensions.ypos + dimensions.height * corrected_pos);
        line.end = ImVec2(dimensions.xpos + dimensions.width, dimensions.ypos + dimensions.height * corrected_pos);
    }
    else {
        float corrected_pos = (position - crop.x0)/(crop.x1 - crop.x0);

        if (corrected_pos < 0.f || corrected_pos > 1.f)
            return line;

        line.start = ImVec2(dimensions.xpos + dimensions.width * corrected_pos, dimensions.ypos);
        line.end = ImVec2(dimensions.xpos + dimensions.width * corrected_pos, dimensions.ypos + dimensions.width);
    }
    return line;
}

void Rendering::DicomViewer::accept_drag_and_drop() {
    if (ImGui::BeginDragDropTarget()) {
        if (ImGui::AcceptDragDropPayload("_DICOM_VIEW")) {
            auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
            auto data = drag_and_drop.returnData();
            loadSeries(data);
        }
        ImGui::EndDragDropTarget();
    }
}

bool Rendering::DicomViewer::begin_popup(const Rect& window_dimensions) {
    auto mouse_pos = ImGui::GetMousePos();
    auto& io = ImGui::GetIO();

    bool collision = Widgets::check_hitbox(mouse_pos, window_dimensions);

    if (ImGui::IsMouseReleased(1) && collision) {
        is_context_menu_open = true;
        ImGui::OpenPopup("Context menu");
    }

    is_context_menu_open = ImGui::BeginPopup("Context menu", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);   

    if (!tmp_context_menu_open) {
        tmp_context_menu_open = is_context_menu_open;
    }
    return is_context_menu_open;
}

void Rendering::DicomViewer::context_menu(const Rect& window_dimensions) {
    if (series_node_ != nullptr) {
        auto mouse_pos = ImGui::GetMousePos();
        if (active_button_ == &windowing_button_) {
            if (begin_popup(window_dimensions)) {
                active_dragging_ = false; 
                tmp_deactivate_tool_ = true;
                windowing_options();
                ImGui::EndPopup();
            }
            else if (!tmp_context_menu_open) {
                tmp_deactivate_tool_ = false;
            }
            push_animation();
        }
        else if (active_button_ == &point_select_button_) {
            if (begin_popup(window_dimensions)) {
                tmp_deactivate_tool_ = true;
                point_select_options();
                ImGui::EndPopup();
            }
            else if (!tmp_context_menu_open) {
                tmp_deactivate_tool_ = false;
            }
        }
    }
}
