#include <algorithm>
#include <exception>
#include <memory>
#include <functional>
#include <iostream>

#include "rendering/window.h"
#include "rendering/glfw_utils.h"
#include "rendering/GLFWwindow_handler.h"
#include "settings.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

/*
 * Exceptions related to the application class
 */

class FailedToInitializeWindow: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Failed to initialize the window";
    }
};

/*
 * Implementations of Window
 */
Rendering::Window::Window(const int width, const int height, const std::string &title, int z_index) {
    // Create window with graphics context
    GLFWwindow* window_ptr = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    window_ = std::make_shared<GLFWwindow*>(window_ptr);
    error_ = !window_ ? true : false;

    if (error_)
        throw FailedToInitializeWindow();

    GLFWwindowHandler::addWindow(window_ptr, z_index);

    glfwMakeContextCurrent(*window_.get());
    glfwSwapInterval(1); // Enable vsync
}

Rendering::Window::Window(Rendering::Window &&other) noexcept {
    window_ = other.window_;
    other.window_ = nullptr;
}

void Rendering::Window::setTitle(const std::string &title) {
    if (!error_ && window_ != nullptr)
        glfwSetWindowTitle(*window_.get(), title.c_str());
}

Dimension Rendering::Window::getDimensions() {
    Dimension window_size = {-1, -1};
    if (!error_ && window_ != nullptr) {
        glfwGetWindowSize(*window_.get(), &window_size.width,&window_size.height);
    }
    return window_size;
}

Rendering::Window::~Window() {
    if (!error_ && window_ != nullptr) {
        glfwDestroyWindow(*window_.get());
        GLFWwindowHandler::removeWindow(*window_.get());
    }
}

void Rendering::Window::addDrawable(std::shared_ptr<AbstractDrawable> drawable) {
    if(std::find(drawables_.begin(), drawables_.end(), drawable) == drawables_.end())
        drawables_.push_back(drawable);
}

bool Rendering::Window::popDrawable() {
    if (!drawables_.empty())
        drawables_.pop_back();
    return !drawables_.empty();
}

void Rendering::Window::removeDrawable(std::shared_ptr<AbstractDrawable> drawable) {
    auto search = std::find(drawables_.begin(), drawables_.end(), drawable);
    if(search != drawables_.end()) {
        drawables_.erase(search);
    }
}

void Rendering::Window::draw() {
    // Take possession of the GLFWwindow pointer
    GLFWwindow* window = *window_;

    for(auto &drawable : drawables_)
        drawable->ImGuiDraw(window, dimensions);
}

void Rendering::Window::update() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Take possession of the GLFWwindow pointer
    GLFWwindow* window = *window_;

    //Make the window DPI aware of the current monitor
    GLFWmonitor* monitor = Rendering::getCurrentMonitor(window);
    glfwGetMonitorContentScale(monitor, &xscale_, &yscale_);


    if ((xscale_ != highDPIscale_ || ui_size != Settings::getInstance().getUIsize()) &&
            Settings::getInstance().getUIsize() > 50) {
        highDPIscale_ = xscale_;
        ui_size = Settings::getInstance().getUIsize();

        if (ui_size > 300)
            ui_size = 300;

        float new_scale = highDPIscale_ * (float)ui_size/100.f;

        Settings::getInstance().setScale(new_scale);

        // Hack for now, font manager is coming later
        io.Fonts->Clear();
        ImFont* font = io.Fonts->AddFontFromFileTTF("assets/verdana.ttf", 16.0f *  new_scale, NULL, NULL);
        io.Fonts->AddFontFromFileTTF("assets/UbuntuMono-R.ttf", 16.f * new_scale, NULL, NULL);
        io.Fonts->Build();

        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();

    }

    int width, height, xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);
    glfwGetWindowSize(window, &width, &height);
    // Adapt for DPI
    dimensions.xpos = (float)xpos / xscale_;
    dimensions.ypos = (float)ypos / xscale_;
    dimensions.width = (float)width / xscale_;
    dimensions.height = (float)height / xscale_;

    for(auto &drawable : drawables_)
        drawable->update(window, dimensions);

}