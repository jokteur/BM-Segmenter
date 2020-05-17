cmake_minimum_required(VERSION 3.12)
project(BM-Segmenter)

set(CMAKE_CXX_STANDARD 14)

set(GCC_COVERAGE_COMPILE_FLAGS "-Os -O3 -static-libgcc -static-libstdc++")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

#--- glfw3 ---
find_package(OpenGL REQUIRED)

set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)

#--- gl3w ---
include_directories(thirdparty/gl3w/include)
set(gl3w_src thirdparty/gl3w/src/gl3w.c)

#--- STB image write ---
include_directories(thirdparty/stb_image)

#--- Imgui ---
set(imgui_DIR "thirdparty/imgui")
set(imgui_src
        ${imgui_DIR}/imgui.cpp
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_widgets.cpp)
include_directories(thirdparty/imgui)
include_directories(thirdparty/imgui_bindings)

#--- Imgui bindings ---
set(imgui_DIR "thirdparty/imgui_bindings")
set(imgui_binding
        ${imgui_DIR}/imgui_impl_glfw.cpp
        ${imgui_DIR}/imgui_impl_opengl3.cpp)
include_directories(thirdparty/imgui-binding)

#Main source
file (GLOB _project_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        src/*.cpp src/*.h
        src/gui/*.cpp src/gui/*.h
        src/gui/layout/*.cpp src/gui/layout/*.h)

#All sources if for the tests
set(all_sources ${_project_src} ${GLAD_GL} ${imgui_src} ${imgui_binding} ${gl3w_src})
list(REMOVE_ITEM all_sources "src/main.cpp")

add_library(${PROJECT_NAME}_lib STATIC ${all_sources})
add_executable(${PROJECT_NAME} ${_project_src} ${GLAD_GL} ${imgui_src} ${imgui_binding} ${gl3w_src})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})

target_link_libraries(${PROJECT_NAME}_lib glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_lib ${OPENGL_gl_LIBRARY})


###############################################################
# Google test
################################################################

add_subdirectory(tests)
