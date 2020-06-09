#ifndef BM_SEGMENTER_ERROR_MESSAGE_H
#define BM_SEGMENTER_ERROR_MESSAGE_H

#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    void show_error_modal(const char* title, const char* short_descr, const char* detailed = "", float size_x = 300, float size_y = 200);
}

#endif //BM_SEGMENTER_ERROR_MESSAGE_H
