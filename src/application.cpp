//
// Created by jokte on 16.05.2020.
//

#include <exception>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include <GLFW/glfw3.h>

#include "application.h"


static void glfw_error_callback(int error, const char* description) {
    std::cout << stderr << "Glfw Error %d: %s\n" << error << description << std::endl;
}

/*
 * Exceptions related to the application class
 */

class FailedToInitializeGLFW: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Failed to initialize GLFW";
    }
};

class FailedToInitializeApp: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Failed to initialize ImGUI";
    }
};

class CannotStartLoopError: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Failed to start the loop of the application because the main window failed to initialize";
    }
};

class GetMainWindowError: public std::exception
{
    virtual const char* what() const throw()
    {
        return "Failed to return the main window because it failed to initialize";
    }
};

/*
 * Implementations of Application
 */

Rendering::Application::Application(std::string main_window_title, uint16_t main_window_width, uint16_t main_window_height)
    : scheduler_(JobScheduler::getInstance()), event_queue_(EventQueue::getInstance())
{
    init_glfw();
    if(!app_state_.error) {
        // Create window with graphics context
        Rendering::Window main_window = Rendering::Window(main_window_width, main_window_height, main_window_title);
        windows_.push_back(std::move(main_window));
        main_window_ = &windows_[0];

        init();
    }
}

void Rendering::Application::init_glfw() {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        app_state_.error = true;
        throw FailedToInitializeGLFW();
    }

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
        init_state.glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    app_state_.glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    const char* glsl_version = app_state_.glsl_version;
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
}
void Rendering::Application::init() {
    GLFWwindow* window = *main_window_->getGLFWwindow_ptr().lock().get();
    // Init imGUI
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        throw FailedToInitializeApp();
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(app_state_.glsl_version);

    float highDPIscaleFactor = 1.0;

#ifdef _WIN32
    // if it's a HighDPI monitor, try to scale everything
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    if (xscale > 1 || yscale > 1)
    {
        highDPIscaleFactor = xscale;
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }
#elif __APPLE__
    // to prevent 1200x800 from becoming 2400x1600
    // and some other weird resizings
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    io.Fonts->AddFontFromFileTTF("assets/verdana.ttf", 18.0f, NULL, NULL);
}

bool Rendering::Application::loop() {
    if (app_state_.error)
        throw CannotStartLoopError();

    GLFWwindow* main_window;
    do {
        main_window = *main_window_->getGLFWwindow_ptr().lock().get();

        glfwWaitEvents();
        event_queue_.pollEvents();

        for(auto &window : windows_) {
            window.draw();
        }

        if (glfwWindowShouldClose(main_window)) {
            scheduler_.cancelAllPendingJobs();
        }

    } while (!glfwWindowShouldClose(main_window) || scheduler_.isBusy());

    return !app_state_.error;
}

void Rendering::Application::shutdown() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

Rendering::Application::~Application() {
    if(!app_state_.error) {
        shutdown();
    }
}

Rendering::Window &Rendering::Application::getMainWindow() {
    if (!app_state_.error)
        return *main_window_;
    else
        throw GetMainWindowError();
}

