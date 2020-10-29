#pragma once

#include <string>
#include <vector>
#include <memory>
#include <GLFW/glfw3.h>

#include "drawables.h"
#include "dimensions.h"

namespace Rendering {
    typedef std::weak_ptr<AbstractDrawable> drawable_weak_ptr;

    class Window {
    private:
        std::shared_ptr<GLFWwindow*> window_;
        int width_;
        int height_;
        std::string title_;

        // DPI related
        float highDPIscale_ = 1.f;
        float xscale_;
        float yscale_;
        int ui_size = 100;

        Rect dimensions;
        std::vector<AbstractDrawable*> drawables_;

        bool error_ = false;

    public:
        /**
         * Empty constructor, initializes no window
         */
        Window() : error_(true) {}

        /**
         * Initializes a new GLFW window
         * @param width width of the window (DPI aware)
         * @param height height of the window (DPI aware)
         * @param title title of the window
         * @param z_index of the window (lower will ge on background)
         */
        Window(const int width, const int height, const std::string &title, int z_index=0);

        /**
         * Move constructor which hands the pointer of the GLFW window to the other Window object
         * @param other
         */
        Window(Window&& other) noexcept;

        /**
         * Copy constructor is forbidden, as we don't want to have a dandling pointer
         */
        Window(Window&) = delete;

        /**
         * Function to call before ImGui::NewFrame();
         */
        void update();

        /**
         * Draws the window
         */
        void draw();

        /**
         * Sets a new title for the window
         * @param title
         */
        void setTitle(const std::string &title);

        /**
         * Adds a drawable to the list of drawables to be drawn to the window
         * @param drawable pointer
         */
        void addDrawable(AbstractDrawable* drawable);

        /**
         * Pops the last drawable on the end of the drawing list
         * @return true if list not empty, false if empty
         */
        bool popDrawable();

        /**
         * Removes a specific drawable from drawing list
         * @param drawable pointer
         */
        void removeDrawable(AbstractDrawable* drawable);

        /**
         * If something failed while initializing the window, this function will return true
         * @return true if could not initialize the window, false success
         */
        bool getError() const { return error_; }

        /**
         * @return current width and height of the window if it is initialized, 0 otherwise
         */
        Dimension getDimensions();


        /**
         * @return the GLFW window pointer (necessary for GLFW operations)
         */
        std::weak_ptr<GLFWwindow*> getGLFWwindow_ptr() { return window_; }

        /**
         * Destroys the GLFW window if it was active
         */
        ~Window();
    };
}
