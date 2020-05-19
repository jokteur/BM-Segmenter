//
// Created by jokte on 17.05.2020.
//

#ifndef BM_SEGMENTER_LAYOUT_H
#define BM_SEGMENTER_LAYOUT_H

#include <GLFW/glfw3.h>

#include "dimensions.h"

namespace Rendering {
    /**
     * @brief Abstract class for GUI drawables
     */
    class AbstractDrawable {
    public:
        /**
         * Default constructor, does nothing
         */
        AbstractDrawable() {};


        virtual ~AbstractDrawable() = default;

        /**
         * Draws to the window
         * @return
         */
        virtual void draw(GLFWwindow* window) = 0;

    };

    class AbstractLayout : public AbstractDrawable {
    protected:
        //
    public:
        AbstractLayout() {};

        virtual ~AbstractLayout() = default;
        /**
         * Draws to the window
         * @return
         */
        virtual void draw(GLFWwindow* window) = 0;
    };
}

#endif //BM_SEGMENTER_LAYOUT_H
