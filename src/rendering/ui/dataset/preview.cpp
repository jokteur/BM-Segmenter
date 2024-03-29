#include "preview.h"

#include "drag_and_drop.h"
#include "rendering/ui/widgets/util.h"

int Rendering::Preview::instance_number = 0;
int Rendering::Preview::load_counter = 0;

namespace Rendering {

    namespace seg = ::core::segmentation;

    void Preview::init() {
        instance_number++;
        identifier_ = std::to_string(instance_number) + std::string("Preview");
        image_widget_.setInteractiveZoom(SimpleImage::IMAGE_NO_INTERACT);
        image_widget_.setImageDrag(SimpleImage::IMAGE_NO_INTERACT);
        image_widget_.setCenterX(true);
        dimensions_ = Rect(ImVec2(-10000, -10000), ImVec2());

        mask_listener_.filter = "nothing";
        identifier_c_str_ = identifier_.c_str();
    }

    Preview::Preview(::core::Image& validated, ::core::Image& edited) {
        init();
    }

    Preview::Preview(const Preview& other) {
        init();
        if (other.dicom_ != nullptr)
            other.dicom_->cancelPendingJobs();
        dicom_ = other.dicom_;
        is_valid_ = other.is_valid_;
    }

    Preview::Preview() {
        init();
    }

    Preview::Preview(const Preview&& other) {
        init();
        if (other.dicom_ != nullptr)
            other.dicom_->cancelPendingJobs();
        dicom_ = other.dicom_;
        is_valid_ = other.is_valid_;
    }

    Preview::~Preview() {
        unload();
        EventQueue::getInstance().unsubscribe(&mask_listener_);
    }

    void Preview::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
        if (no_draw_) {
            ImGui::BeginChild(identifier_c_str_, ImVec2(int(size_.x), int(size_.y)));
            ImVec2 content = ImGui::GetContentRegionAvail();
            ImVec2 window_pos = ImGui::GetWindowPos();
            dimensions_.xpos = window_pos.x;
            dimensions_.ypos = window_pos.y;
            dimensions_.width = content.x;
            dimensions_.height = content.y;
        }
        else {
            // Window ImGui intro
            auto& io = ImGui::GetIO();
            io.ConfigWindowsMoveFromTitleBarOnly = true;
            // Remove all padding for the child window
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
            const int num_pop = 3;
            ImGui::BeginChild(identifier_.c_str(), ImVec2(int(size_.x), int(size_.y)), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::PopStyleVar(num_pop);

            // Calculate the available dimensions
            ImVec2 content = ImGui::GetContentRegionAvail();
            ImVec2 window_pos = ImGui::GetWindowPos();
            dimensions_.xpos = window_pos.x;
            dimensions_.ypos = window_pos.y;
            dimensions_.width = content.x;
            dimensions_.height = content.y;

            ImVec2 mouse_pos = ImGui::GetMousePos();
            Rect image_dimensions = image_widget_.getDimensions();

            if (reset_image_) {
                reset_image_ = false;
                if (dicom_ != nullptr) {
                    if (active_seg_ == nullptr) {
                        image_.setImageFromHU(
                            dicom_->getData()[case_idx_].data,
                            (float)dicom_->getWW(),
                            (float)dicom_->getWC(),
                            core::Image::FILTER_NEAREST
                        );
                    }
                    image_widget_.setImage(image_);
                }
                image_widget_.setDragSourceFunction([this] {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        auto& drag_and_drop = DragAndDrop<std::shared_ptr<::core::DicomSeries>>::getInstance();
                        drag_and_drop.giveData(dicom_);

                        int a = 0; // Dummy int
                        ImGui::SetDragDropPayload("_DICOM_PAYLOAD", &a, sizeof(a));
                        ImGui::Text("%s", ::core::parse_dicom_id(dicom_->getId()).first.c_str());
                        ImGui::PushID("Image_Drag_Drop");
                        ImGui::Image(
                            image_.texture(),
                            ImVec2(128, 128)
                        );
                        EventQueue::getInstance().post(Event_ptr(new Event("global/no_action")));
                        ImGui::PopID();
                        ImGui::EndDragDropSource();
                    }
                    });
            }

            // Interaction with the mouse cursor
            if (allow_scroll_ && Widgets::check_hitbox(mouse_pos, image_dimensions)) {
                float percentage = (mouse_pos.x - image_dimensions.xpos) / image_dimensions.width;
                if (percentage >= 0.f && percentage <= 1.f)
                    setCase(percentage);
            }
            else {
                setCase(0.f);
            }

            // Draw the image
            if (image_.isImageSet()) {
                image_widget_.setAutoScale(true);
                image_widget_.ImGuiDraw(window, dimensions_);
            }
            if (ImGui::BeginPopupContextItem(identifier_.c_str())) {
                popup_context_menu();
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }

    void Preview::load() {
        if (!is_loaded_) {
            set_case(0);
            load_counter++;
        }
    }

    void Preview::unload() {
        if (is_loaded_) {
            image_.reset();
            dicom_->unloadCase(case_idx_);
            reset_image_ = false;
            is_loaded_ = false;
            load_counter--;
        }
        unload_mask();
    }

    void Preview::unload_mask() {
        if (active_seg_ != nullptr) {
            auto& collection = active_seg_->getMask(dicom_);
            if (collection->isSet()) {
                reset_image_ = false;
                collection->unloadData();
            }
            else if (collection->numPendingJobs() > 0) {
                collection->cancelPendingJobs();
            }
        }
    }

    void Preview::setAndLoadMask(int idx, bool check_loaded) {
        if (dicom_ != nullptr && is_loaded_) {
            if (active_seg_ != nullptr) {
                auto& collection = active_seg_->getMask(dicom_);
                collection->loadData(
                    false, false, "",
                    [this]() {
                        if (__hack == 235.654885342) {
                            set_image();
                            reset_image_ = true;
                            is_mask_loaded = true;
                        }
                    },
                    Job::JOB_PRIORITY_LOW
                );
            }
            else {
                reset_image_ = true;
            }
        }
    }

    void Preview::set_image() {
        auto& collection = active_seg_->getMask(dicom_);
        seg::Mask* mask;
        // Sometimes, the mask has not loaded in time, and the collection
        // is not valid yet
        if (!collection->isValid()) {
            image_.setImageFromHU(
                dicom_->getData()[case_idx_].data,
                (float)dicom_->getWW(),
                (float)dicom_->getWC(),
                core::Image::FILTER_NEAREST
            );
        }
        else {
            auto& tmp = collection->getCurrent(true);
            if (collection->getIsValidated()) {
                mask = &collection->getValidated();
                state_ = VALIDATED;
            }
            else if (!collection->getPrediction().empty() && tmp.empty()) {
                mask = &collection->getPrediction();
                state_ = PREDICTED;
            }
            else if (!tmp.empty()) {
                mask = &tmp;
                state_ = CURRENT;
            }
            else {
                mask = &tmp;
                state_ = NOTHING;
            }

            image_.setImageFromHU(
                dicom_->getData()[case_idx_].data,
                (float)dicom_->getWW(),
                (float)dicom_->getWC(),
                core::Image::FILTER_NEAREST,
                mask->getData(),
                active_seg_->getMaskColor()
            );
        }
    }

    void Preview::setSegmentation(std::shared_ptr<::core::segmentation::Segmentation> segmentation) {
        active_seg_ = segmentation;
        unload_mask();
        setAndLoadMask(case_idx_);
    }

    void Preview::popup_context_menu() {
        if (ImGui::Selectable("Add to group")) {
        }
        if (ImGui::Selectable("Add to segmentation")) {
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Cropping and windowing", ImGuiTreeNodeFlags_None)) {
            ImGui::Text("Cropping:");
            ImGui::DragFloatRange2("Crop in x", &crop_x_.x, &crop_x_.y, 1.f, 0.0f, 100.0f, "Left: %.1f %%", "Right: %.1f %%");
            ImGui::DragFloatRange2("Crop in y", &crop_y_.x, &crop_y_.y, 1.f, 0.0f, 100.0f, "Top: %.1f %%", "Bottom: %.1f %%");
            set_crop(crop_x_, crop_y_, true);
            ImGui::Separator();
            ImGui::Text("Windowing: ");
            ImGui::DragInt("Window center###dicom_preview_wc", &dicom_->getWC(), 0.5, -1000, 3000, "%d HU");
            ImGui::DragInt("Window width###dicom_preview_ww", &dicom_->getWW(), 0.5, 1, 3000, "%d HU");
            set_window(dicom_->getWW(), dicom_->getWC(), true);
        }
    }

    void Preview::setCase(float percentage) {
        if (!is_valid_ || !is_loaded_)
            return;

        int idx = (int)(percentage * (float)(dicom_->size() - 1));
        if (idx != tmp_case_idx_) {
            tmp_case_idx_ = idx;
            std::cout << idx << " " << percentage << std::endl;
            set_case(idx);
        }
    }

    void Preview::set_case(int idx) {
        if (!is_valid_)
            return;

        dicom_->loadCase(idx, false, [idx, this](const core::Dicom& dicom) {
            // Preface: this is extremely bad practice, I know
            // This function may cause a segfault (if Preview is destroyed before the job is finished)
            // This happens when there is a problem when the project loads (syntax error in python file mostly)
            // If the project is not loaded, then the WelcomeView is loaded again (thus destroying the Previews
            // immediately)
            // TODO: refactor loadCase such that this never happen
            //
            // Explanation of the hack:
            // If this function is called when Preview is already destroyed, __num should contain any garbage
            // What are the chances that __num will contain exactly this sequence (defined when constructed):
            // 0100000001101101011101001111010011010010000110101101000010100010 ?
            if (__hack == 235.654885342) {
                case_idx_ = idx;
                reset_image_ = true;
                is_loaded_ = true;
                setAndLoadMask(idx);
            }
        });
    }

    void Preview::set_crop(ImVec2 crop_x, ImVec2 crop_y, bool lock) {
        if (!is_valid_)
            return;
    }

    void Preview::set_window(int width, int center, bool lock) {
        if (!is_valid_)
            return;
    }

    void Preview::setSeries(std::shared_ptr<::core::DicomSeries> dicom) {
        EventQueue::getInstance().unsubscribe(&mask_listener_);
        if (dicom != nullptr) {
            is_valid_ = true;
            mask_listener_.callback = [=](Event_ptr event) {
                if (is_loaded_)
                    setAndLoadMask(case_idx_);
            };
            mask_listener_.filter = "mask/changed/" + dicom->getId();
            EventQueue::getInstance().subscribe(&mask_listener_);
        }
        else {
            is_valid_ = false;
        }
        dicom_ = dicom;
    }
}