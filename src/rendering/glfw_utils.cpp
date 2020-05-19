#include "glfw_utils.h"

static int min(int x, int y) {
    return x < y ? x : y;
}
static int max(int x, int y) {
    return x > y ? x : y;
}

// Adapted from https://stackoverflow.com/a/31526753
GLFWmonitor* Rendering::getCurrentMonitor(GLFWwindow *window) {
    int nmonitors, i;
    int wx, wy, ww, wh;
    int mx, my, mw, mh;
    int overlap, bestoverlap;
    GLFWmonitor *bestmonitor;
    GLFWmonitor **monitors;
    const GLFWvidmode *mode;

    bestoverlap = 0;
    bestmonitor = glfwGetPrimaryMonitor();

    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++) {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        overlap = max(0, min(wx + ww, mx + mw) - max(wx, mx)) *
                  max(0, min(wy + wh, my + mh) - max(wy, my));

        if (bestoverlap < overlap) {
            bestoverlap = overlap;
            bestmonitor = monitors[i];
        }
    }

    return bestmonitor;

}