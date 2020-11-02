#pragma once
#include "rendering/drawables.h"
#include "rendering/ui/modales/modals.h"

#include <string>
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace Rendering {
    /**
     * Show an error modal
     * The error message will show on top off whatever modal is active
     * @param title title of the error window
     * @param short_descr short description of the error
     * @param detailed detailed description of the error (user has to click on it)
     */
    void show_error_modal(const std::string& title, const std::string& short_descr, const std::string& detailed = "");
}