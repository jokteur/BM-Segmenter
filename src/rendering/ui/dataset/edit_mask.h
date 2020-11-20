#pragma once

#include <string>
#include <vector>

#include "python/py_api.h"
#include "opencv2/opencv.hpp"

#include "core/image.h"
#include "core/dataset/explore.h"
#include "core/dicom.h"
#include "events.h"
#include "rendering/drawables.h"
#include "rendering/ui/widgets/image_simple.h"
#include "rendering/ui/widgets/image_button.h"

namespace Rendering {
    struct Line {
        ImVec2 start;
        ImVec2 end;
    };
    /**
     * Little widget class for visualizing DICOM images
     */
    class EditMask : public AbstractLayout {
    private:
        static int instance_number;
        int my_instance_;

        std::string identifier_;
        bool is_open_ = true;

        ::core::Image image_;
        SimpleImage image_widget_;

        ImageButton lasso_select_b_;
        ImageButton box_select_b_;
        ImageButton brush_select_b_;
        ImageButton* active_button_ = nullptr;
        std::vector<ImageButton*> buttons_list_;

        Listener listener_;
        Listener reset_viewer_listener_;

        bool reset_image_ = false;

        bool active_dragging_ = false;
        ImVec2 drag_delta_;

        std::shared_ptr<::core::DicomSeries> dicom_ = nullptr;

        int case_select_ = 1;
        int previous_select_ = 0;
        int image_size_ = 0;

        void loadDicom(const std::shared_ptr<::core::DicomSeries> dicom);
        void loadCase(int idx);

        void button_logic();

        void accept_drag_and_drop();

    public:
        EditMask();
        ~EditMask();

        /**
         * Draws the viewer image widget
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
    };
}