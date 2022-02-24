#pragma once

/*
 * gl3w and glfw3 can be in conflict when not called in the right order
 * Include this file to avoid compilation problems
 */

#include <glad/glad.h>
#define _AVOID_FORMAT_INCLUDE_REORDERING
#include <GLFW/glfw3.h>