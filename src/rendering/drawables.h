#pragma once


// gl3w.h HAS to be included before glfw3.h
// As drawables.h is included very often, I put both includes
// here to avoid conflict.
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

struct Dimension {
    int width = 0;
    int height = 0;
};

struct Position {
    float x;
    float y;
};

struct Rect {
    float xpos = 0;
    float ypos = 0;
    float width = 0;
    float height = 0;
};

namespace Rendering {
    /**
     * @brief Abstract class for GUI drawables
     */
    class AbstractDrawable {
    protected:
        Rect dimensions_;
    public:
        /**
         * Default constructor, does nothing
         */
        AbstractDrawable() = default;;


        virtual ~AbstractDrawable() = default;

        Rect& getDimensions() { return dimensions_; }

        /**
         * Updates the class, before any ImGui::NewFrame() is called
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        void update(GLFWwindow *window, Rect &parent_dimension) {}

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
        virtual void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) = 0;

    };

    class AbstractLayout : public AbstractDrawable {
    protected:
    public:
        AbstractLayout() = default;

        ~AbstractLayout() override = default;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @return
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override = 0;
    };
}
