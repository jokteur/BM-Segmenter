#pragma once


// gl3w.h HAS to be included before glfw3.h
// As drawables.h is included very often, I put both includes
// here to avoid conflict.
#include "first_include.h"
#include "imgui.h"

#include <iostream>
struct Dimension {
    int width = 0;
    int height = 0;
};

struct Position {
    float x;
    float y;
};

struct Rect {
    Rect() {}
    Rect(const ImVec2& pos, const ImVec2& size) {
        xpos = pos.x;
        ypos = pos.y;
        width = size.x;
        height = size.y;
    }
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
        ImGuiID docking_id_;

        bool first_draw_ = false;
    public:
        /**
         * Default constructor, does nothing
         */
        explicit AbstractDrawable(ImGuiID docking_id = 0) : docking_id_(docking_id) {}


        virtual ~AbstractDrawable() = default;

        Rect& getDimensions() { return dimensions_; }

        void setDockID(ImGuiID id) { docking_id_ = id; }

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
        AbstractLayout(ImGuiID docking_id = 0) : AbstractDrawable(docking_id) {}

        ~AbstractLayout() override = default;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @return
         */
        void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) override = 0;
    };
}
