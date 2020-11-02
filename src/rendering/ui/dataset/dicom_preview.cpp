#include "dicom_preview.h"
#include "core/dataset/dicom_to_image.h"
#include "ui/widgets/util.h"
#include "drag_and_drop.h"
#include "log.h"

int Rendering::DicomPreview::instance_number = 0;
namespace dataset = ::core::dataset;


Rendering::DicomPreview::DicomPreview() {
    instance_number++;
    identifier_ = std::to_string(instance_number) + std::string("DicomPreview");
    image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
    image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
}

void Rendering::DicomPreview::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    auto &io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::BeginChild(identifier_.c_str(), size_);


    // Reload the image when it has been loaded
    // We do this because setting the image_ can lead to segfault when not done
    // in the main thread
    if (reset_image_) {
        dicom_to_image();
        reset_image_ = false;
        image_widget_.setDragSourceFunction([this] {
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                auto &drag_and_drop = DragAndDrop<dataset::SeriesPayload>::getInstance();
                drag_and_drop.giveData(series_payload_);

                int a = 0; // Dummy int
                ImGui::SetDragDropPayload("_DICOM_VIEW", &a, sizeof(a));
                ImGui::Text("Drag Series %s to viewer to visualize", series_payload_.series.number.c_str());
                ImGui::PushID("Image_Drag_Drop");
                ImGui::Image(
                        image_.texture(),
                        ImVec2(128, 128)
                );
                ImGui::PopID();
                ImGui::EndDragDropSource();
            }}
        );
    }

    ImVec2 content = ImGui::GetContentRegionAvail();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 mouse_pos = ImGui::GetMousePos();
    auto style = ImGui::GetStyle();

    dimensions_.xpos = window_pos.x;
    dimensions_.ypos = window_pos.y;
    dimensions_.width = content.x;
    dimensions_.height = dimensions_.width;

    if (allow_scroll_ && Widgets::check_hitbox(mouse_pos, dimensions_)) {
        setCase((mouse_pos.x - dimensions_.xpos)/dimensions_.width);
    }
    else {
        setCase(0.f);
    }
    if (image_.isImageSet()) {
        image_widget_.setAutoScale(true);
        image_widget_.ImGuiDraw(window, parent_dimension);
    }

    if (ImGui::BeginPopupContextItem(identifier_.c_str())) {
        if (ImGui::Selectable((std::string("Open file in DICOM viewer###") + identifier_).c_str())) {
            EventQueue::getInstance().post(Event_ptr(new ::core::dataset::SelectSeriesEvent(series_payload_)));
        }
        ImGui::EndPopup();
    }

    ImGui::EndChild();
}

void Rendering::DicomPreview::selectCase(const std::string& path) {
    error_message_.clear();
    jobResultFct fct = [=] (const std::shared_ptr<JobResult> &result) {
        auto dicom_result = std::dynamic_pointer_cast<::core::dataset::DicomResult>(result);
        if (dicom_result->success) {

            // Resize image to not fill the ram
            auto &dicom = dicom_result->data;
            if (dicom.rows > max_im_size_ || dicom.cols > max_im_size_) {
                int width, height;
                if (dicom.rows > dicom.cols) {
                    width = max_im_size_;
                    height = (int)((float)dicom.cols / (float)dicom.rows  * max_im_size_);
                }
                else {
                    width = (int)((float)dicom.rows / (float)dicom.cols  * max_im_size_);
                    height = max_im_size_;
                }
                cv::resize(dicom, dicom_matrix_, cv::Size(width, height), 0, 0, cv::INTER_AREA);
            }
            else {
                dicom_matrix_ = dicom_result->data;
            }
            reset_image_ = true;
        }
        else {
            error_message_ = dicom_result->error_msg;
        }
    };
    dataset::dicom_to_matrix(path, fct);
}

void Rendering::DicomPreview::dicom_to_image() {
    image_.setImageFromHU(dicom_matrix_, (float)window_width_, (float)window_center_);
    image_widget_.setImage(image_);
}

void Rendering::DicomPreview::loadSeries(const ::core::dataset::SeriesPayload& payload) {
    series_.clear();
    series_payload_ = payload;
    for (auto &image : payload.series.images) {
        series_.push_back(image.path);
    }
    if (!series_.empty()) {
        selectCase(series_[0]);
    }
}

void Rendering::DicomPreview::setCase(float percentage) {
    int idx = (int)(percentage * (float)(series_.size() - 1));
    if (idx != case_idx) {
        selectCase(series_[idx]);
        case_idx = idx;
    }
}

void Rendering::DicomPreview::setCrop(float crop) {
    crop_ = crop;
    selectCase(series_[0]);
}
