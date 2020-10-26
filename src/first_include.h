#pragma once

/*
 * gl3w and glfw3 can be in conflict when not called in the right order
 * Include this file to avoid compilation problems
 */
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>