#include <exception>
#include <iostream>

#include <GL/gl3w.h>    // Initialize with gl3wInit()
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include "application.h"
#include "settings.h"


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
        return "Failed to start the loop of the application because the app is not initialized";
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
        Rendering::Window main_window = Rendering::Window(main_window_width, main_window_height, main_window_title, -1);
        windows_.push_back(std::move(main_window));
        main_window_ = &windows_[0];
    }

    push_animation_.filter = "push_animation";
    push_animation_.callback = [=](Event_ptr event) {
        last_time_ = std::chrono::system_clock::now();
    };
    event_queue_.subscribe(&push_animation_);
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
    GLFWwindow* window = *main_window_->getGLFWwindow_ptr().lock();

    // Init imGUI
    glfwMakeContextCurrent(window);

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        throw FailedToInitializeApp();
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= configFlags_;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(app_state_.glsl_version);

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

//    io.Fonts->AddFontFromFileTTF("assets/verdana.ttf", 16.0f, NULL, NULL);

    ImGuiStyle& style = ImGui::GetStyle();

    // Hack to make the ImGui windows look like normal windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    app_state_.imgui_init = true;
}

bool Rendering::Application::loop() {
    if (!app_state_.imgui_init)
        throw CannotStartLoopError();

    GLFWwindow* main_window;
    do {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        main_window = *main_window_->getGLFWwindow_ptr().lock();

        auto time_since_push = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_time_);
        if (time_since_push.count() < 500) {
            glfwPollEvents();
        }
        else {
            glfwWaitEvents();
        }
        event_queue_.pollEvents();
        KeyboardShortCut::dispatchShortcuts();

        for(auto &window : windows_) {
            window.update();
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        for(auto &window : windows_) {
            window.draw();
        }
        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(main_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.5, 0.5, 0.5, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        JobScheduler::getInstance().finalizeJobs();

        glfwSwapBuffers(main_window);

        if (glfwWindowShouldClose(main_window)) {
            scheduler_.abortAll();
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
    event_queue_.unsubscribe(&push_animation_);
}

Rendering::Window &Rendering::Application::getMainWindow() {
    if (!app_state_.error)
        return *main_window_;
    else
        throw GetMainWindowError();
}

void Rendering::Application::addImGuiFlags(int configFlags) {
    configFlags_ = configFlags;
}

