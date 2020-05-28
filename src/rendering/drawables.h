//
// Created by jokte on 17.05.2020.
//

#ifndef BM_SEGMENTER_LAYOUT_H
#define BM_SEGMENTER_LAYOUT_H

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
        Rect dimensions;
    public:
        /**
         * Default constructor, does nothing
         */
        AbstractDrawable() {};


        virtual ~AbstractDrawable() = default;

        Rect& getDimensions() { return dimensions; }

        /**
         * Updates the class, before any ImGui::NewFrame() is called
         * @param window
         * @param parent_dimension
         */
        void update(GLFWwindow *window, Rect &parent_dimension) {}

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @return
         */
        virtual void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) = 0;

    };

    class AbstractLayout : public AbstractDrawable {
    protected:
    public:
        AbstractLayout() = default;

        virtual ~AbstractLayout() = default;

        /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @return
         */
        virtual void ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) = 0;
    };
}

#endif //BM_SEGMENTER_LAYOUT_H
