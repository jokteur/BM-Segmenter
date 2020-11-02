#include "dicom_viewer.h"
#include "log.h"
#include "core/dataset/dicom_to_image.h"
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
            ) {
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

    // Reload the image when it has been loaded
    // We do this because setting the image_ can lead to segfault when not done
    // in the main thread
    if (reset_image_) {
        dicom_to_image();
        reset_image_ = false;
    }

    auto aCase = case_;
    if (!series_.empty()) {
        aCase = series_[0];
    }

    if (!aCase.patientID.empty()) {
        ImGui::Text("Patient ID %s /", aCase.patientID.c_str());
        ImGui::SameLine();
        ImGui::Text("Study %s /", aCase.studyDescription.c_str());
        ImGui::SameLine();
        ImGui::Text("Series %s ", aCase.seriesNumber.c_str());
        ImGui::SameLine();
        ImGui::Text("with modality %s ", aCase.modality.c_str());
    }
    else {
        ImGui::Text("Drag Series or double click on Series to show DICOM");
    }

    if (series_.size() > 1) {
        if (case_select_ != previous_select_) {
            previous_select_ = case_select_;
            if (case_select_ > 0 && case_select_ < series_.size())
                selectCase(series_[case_select_ - 1]);
        }
        ImGui::SliderInt((std::string("Select image###") + identifier_).c_str(), &case_select_, 1, series_.size(), "image n° %d");
        ImGui::SameLine();
        Widgets::HelpMarker("Ctrl+click to input manually the number");
    }
    else if(!aCase.path.empty()) {
        ImGui::SameLine();
        ImGui::Text("/ Image number %s", aCase.instanceNumber.c_str());
    }
    windowing_button_.ImGuiDraw(window, parent_dimension);
    ImGui::SameLine();
    ImGui::Text("Window Center: %d, Window Width: %d", window_center_, window_width_);
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
                dicom_to_image();
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

    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    auto style = ImGui::GetStyle();

    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x + 2 * style.WindowPadding.x;
    dimensions_.height = content.y + 2 * style.WindowPadding.y;

    if (image_.isImageSet()) {
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
    ImGui::End();
}

void Rendering::DicomViewer::selectCase(dataset::Case& aCase) {
    error_message_.clear();
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto dicom_result = std::dynamic_pointer_cast<::core::dataset::DicomResult>(result);
        if (dicom_result->success) {
            dicom_matrix_ = dicom_result->data;
            reset_image_ = true;
        }
        else {
            error_message_ = dicom_result->error_msg;
        }
    };
    dataset::dicom_to_matrix(aCase.path, fct);
}

void Rendering::DicomViewer::dicom_to_image() {
    image_.setImageFromHU(dicom_matrix_, (float)window_width_, (float)window_center_);
    image_widget_.setImage(image_);
}

void Rendering::DicomViewer::loadSeries(const dataset::SeriesPayload &data) {
    series_.clear();
    windowing_button_.setState(false);
    case_select_ = 1;
    for (auto &image : data.series.images) {
        auto Case = dataset::Case {
                std::string(data.case_.patientID),
                std::string(data.case_.studyDate),
                std::string(data.case_.studyTime),
                std::string(data.case_.studyDescription),
                std::string(data.series.number),
                std::string(data.series.modality),
                std::string(image.path),
                std::string(image.number)
        };
        series_.push_back(Case);
    }
    if (!series_.empty())
        selectCase(series_[0]);
}