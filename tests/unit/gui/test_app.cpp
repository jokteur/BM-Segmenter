//
// Created by jokte on 16.05.2020.
//
#include <exception>
#include <complex>

#include "gui/application.h"
#include <gtest/gtest.h>

using namespace GUI;
TEST(Application, initWindow) {

    // Test a simple initialization with an empty window
    Application app("TestApp", 1280, 720);
}