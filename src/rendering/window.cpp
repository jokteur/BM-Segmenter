#include <algorithm>
#include <exception>
#include <memory>

#include "window.h"
#include "glfw_utils.h"

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
Rendering::Window::Window(const int width, const int height, const std::string &title) {
    // Create window with graphics context
    GLFWwindow* window_ptr = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    window_ = std::make_shared<GLFWwindow*>(window_ptr);
    error_ = !window_ ? true : false;

    if (error_)
        throw FailedToInitializeWindow();

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
    if (!error_ && window_ != nullptr)
        glfwDestroyWindow(*window_.get());
}

void Rendering::Window::addDrawable(Rendering::AbstractDrawable* drawable) {
    if(std::find(drawables_.begin(), drawables_.end(), drawable) == drawables_.end())
        drawables_.push_back(drawable);
}

bool Rendering::Window::popDrawable() {
    if (!drawables_.empty())
        drawables_.pop_back();
    return !drawables_.empty();
}

void Rendering::Window::removeDrawable(Rendering::AbstractDrawable* drawable) {

}

void Rendering::Window::draw() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for(auto &drawable : drawables_)
        drawable->draw(*window_.get());

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(*window_.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(*window_.get());
}
