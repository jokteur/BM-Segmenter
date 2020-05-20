#include <exception>
#include <complex>

#include "rendering/application.h"
#include <gtest/gtest.h>

using namespace Rendering;
TEST(Application, initWindow) {

    // Test a simple initialization with an empty window
    Application app("TestApp", 1280, 720);
}