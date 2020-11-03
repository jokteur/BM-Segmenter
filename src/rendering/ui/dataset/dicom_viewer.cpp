#include "dicom_viewer.h"
#include "log.h"
#include "core/dataset/dicom_to_image.h"
#include "core/dataset/extract_view_from_dicom.h"
#include "rendering/drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

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

    // Listen to selection in the Dicom explorer
    listener_.callback = [=](Event_ptr &event) {
        auto log = reinterpret_cast<dataset::SelectSeriesEvent*>(event.get());
        loadSeries(log->getSeries());
    };
    listener_.filter = "dataset/dicom_open";
    EventQueue::getInstance().subscribe(&listener_);

}

Rendering::DicomViewer::~DicomViewer() {
    EventQueue::getInstance().unsubscribe(&listener_);
}

void Rendering::DicomViewer::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::Begin((std::string("DICOM viewer###") + identifier_).c_str(), &is_open_);

    if (sagittal_ready_ && coronal_ready_) {
        reset_image_ = true;
    }
    // Reload the image when it has been loaded
    // We do this because setting the image_ can lead to segfault when not done
    // in the main thread
    if (reset_image_) {
        dicom_to_image();
        reset_image_ = false;
    }

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

    if (dicom_matrix_.size() > 1) {
        if (case_select_ != previous_select_) {
            previous_select_ = case_select_;
            if (case_select_ < 0 || case_select_ >= series_.size())
                case_select_ = 1;
            reset_image_ = true;
        }
        ImGui::SliderInt((std::string("Select image###") + identifier_).c_str(), &case_select_, 1, dicom_matrix_.size(), "image n° %d");
        ImGui::SameLine();
        Widgets::HelpMarker("Ctrl+click to input manually the number");
    }
    if (dicom_matrix_.size() == image_size_ && !load_finish_) {
        load_finish_ = true;
        build_views();
    }

    windowing_button_.ImGuiDraw(window, dimensions_);
    ImGui::SameLine();
    ImGui::Text("Window Center: %d, Window Width: %d", window_center_, window_width_);
    point_select_button_.ImGuiDraw(window, dimensions_);
    ImGui::Separator();

    if (!error_message_.empty()) {
        image_.reset();
        ImGui::Text("When trying to open the DICOM file, the following error appeared:\n\n%s", error_message_.c_str());
    }

    // Windowing tool
    if (windowing_button_.isPressed()) {
        if (ImGui::IsMouseDown(0) && Widgets::check_hitbox(ImGui::GetMousePos(), dimensions_)) {
            active_dragging_ = true;
        }
        if (ImGui::IsMouseReleased(0)) {
            active_dragging_ = false;
            drag_delta_.x = 0;
            drag_delta_.y = 0;
        }
        if (active_dragging_) {
            image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
            auto drag = ImGui::GetMouseDragDelta(0, 0);
            float attenuation = 1.f;
            if(io.KeyShift)
                attenuation = 0.5f;
            if (drag.x != drag_delta_.x || drag.y != drag_delta_.y) {
                window_width_ += (int)((drag.x - drag_delta_.x)*attenuation);
                if (window_width_ < 1)
                    window_width_ = 1;
                window_center_ += (int)((drag.y - drag_delta_.y)*attenuation);
                reset_image_ = true;
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
    if (point_select_button_.isPressed()) {
        image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
        Rect dimensions = image_widget_.getDimensions();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        if (ImGui::IsMouseClicked(0) && Widgets::check_hitbox(mouse_pos, dimensions)) {
            sagittal_x_ = (mouse_pos.x - dimensions.xpos) / dimensions.width;
            coronal_x_ = (mouse_pos.y - dimensions.ypos) / dimensions.height;
            std::cout << sagittal_x_ << " " << coronal_x_ << std::endl;
            build_views(true);
        }
    }
    else {
        image_widget_.setImageDrag(SimpleImage::IMAGE_NORMAL_INTERACT);
    }

    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    auto style = ImGui::GetStyle();

    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x + 2 * style.WindowPadding.x;
    dimensions_.height = content.y + 2 * style.WindowPadding.y;

    if (dicom_matrix_.size() > 4) {
        ImGui::Columns(2);
    }
    auto col_content = ImGui::GetContentRegionAvail();

    if (image_.isImageSet()) {
        float x = col_content.x;
        float y = content.y;
        if (x < y) {
            image_widget_.setSize(ImVec2(0, x));
        }
        else {
            image_widget_.setSize(ImVec2(y, 0));
        }
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, parent_dimension);
    }
    else
        ImGui::Dummy(ImVec2(content.x, content.y - 3 * ImGui::GetItemRectSize().y));
    // Accept drag and drop from dicom explorer
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_DICOM_VIEW")) {
            auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
            auto data = drag_and_drop.returnData();
            loadSeries(data);
        }
        ImGui::EndDragDropTarget();
    }

    if (sagittal_ready_ && coronal_ready_) {
        ImGui::NextColumn();
        Rect dimension = dimensions_;
        dimension.width = col_content.x;
        dimension.height = col_content.y/2.f;

        ImGui::BeginChild((identifier_ + std::string("sagittal")).c_str(), ImVec2(dimension.width, dimension.height));
        sagittal_widget_.setAutoScale(true);
        sagittal_widget_.ImGuiDraw(window, dimension);
        ImGui::EndChild();

        ImGui::BeginChild((identifier_ + std::string("coronal")).c_str(), ImVec2(dimension.width, dimension.height));
        coronal_widget_.setAutoScale(true);
        coronal_widget_.ImGuiDraw(window, dimension);
        ImGui::EndChild();
    }
    if (dicom_matrix_.size() > 4) {
        ImGui::Columns(1);
    }
    ImGui::End();
}

void Rendering::DicomViewer::loadCase(const std::string &path) {
    error_message_.clear();
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto dicom_result = std::dynamic_pointer_cast<::core::dataset::DicomResult>(result);
        if (dicom_result->success) {
            auto& dicom = dicom_result->data;
            cv::Rect ROI(0, 0, dicom.rows, dicom.cols);
            if (crop_x_.x != crop_x_.y && crop_y_.x != crop_y_.y) {
                ROI = {
                        (int)((float)dicom.rows*crop_x_.x/100.f),
                        (int)((float)dicom.cols*crop_y_.x/100.f),
                        (int)((float)dicom.rows*(crop_x_.y - crop_x_.x)/100.f),
                        (int)((float)dicom.rows*(crop_y_.y - crop_y_.x)/100.f)
                };
            }
            cv::Mat mat;
            dicom(ROI).copyTo(mat);
            dicom_matrix_.emplace_back(mat);
            reset_image_ = true;
        }
        else {
            error_message_ = dicom_result->error_msg;
        }
        pending_jobs_.erase(result->id);
    };
    auto job = dataset::dicom_to_matrix(path, fct);
    pending_jobs_.insert(job->id);
}

void Rendering::DicomViewer::dicom_to_image() {
    if (case_select_ < dicom_matrix_.size() && case_select_ > 0) {
        image_.setImageFromHU(dicom_matrix_[case_select_ - 1], (float)window_width_, (float)window_center_);
        image_widget_.setImage(image_);
    }
    if (sagittal_ready_ && coronal_ready_) {
        sagittal_image_.setImageFromHU(sagittal_matrix_, (float)window_width_, (float)window_center_);
        coronal_image_.setImageFromHU(coronal_matrix_, (float)window_width_, (float)window_center_);
        sagittal_widget_.setImage(sagittal_image_);
        coronal_widget_.setImage(coronal_image_);
    }
}

void Rendering::DicomViewer::loadSeries(const dataset::SeriesPayload &data) {
    // Reset things
    if (!pending_jobs_.empty()) {
        for (auto& id : pending_jobs_)
            JobScheduler::getInstance().stopJob(id);
        pending_jobs_.clear();
    }
    dicom_matrix_.clear();
    windowing_button_.setState(false);
    case_select_ = 1;
    load_finish_ = false;
    sagittal_ready_ = coronal_ready_ = false;

    // Set the newest data
    image_size_ = data.series.images.size();
    case_ = data.case_;
    window_center_ = data.window_center;
    window_width_ = data.window_width;
    crop_x_ = data.crop_x;
    crop_y_ = data.crop_y;

    for (const auto &image : data.series.images) {
        loadCase(image.path);
    }
}

void Rendering::DicomViewer::build_views(bool no_reset) {
    if (dicom_matrix_.size() < 5) {
        return;
    }

    if (!no_reset) {
        sagittal_ready_ = false;
        coronal_ready_ = false;
    }
    // Sagittal
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto view_result = std::dynamic_pointer_cast<::core::dataset::DicomViewResult>(result);
        sagittal_matrix_ = view_result->data;
        sagittal_ready_ = true;
    };
    dataset::extract_view(dicom_matrix_, fct, sagittal_x_, false);
    // Coronal
    jobResultFct fct2 = [=] (const std::shared_ptr<JobResult> &result) {
        auto view_result = std::dynamic_pointer_cast<::core::dataset::DicomViewResult>(result);
        coronal_matrix_ = view_result->data;
        coronal_ready_ = true;
    };
    dataset::extract_view(dicom_matrix_, fct2, coronal_x_, true);
}
