cmake_minimum_required(VERSION 3.10)
project(BM-Segmenter)

set(CMAKE_CXX_STANDARD 14)

set(GCC_COVERAGE_COMPILE_FLAGS "-Os -O3 -static-libgcc -static-libstdc++")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# --- glfw3 ---
find_package(OpenGL REQUIRED)

set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )


add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)

# --- gl3w ---
include_directories(thirdparty/gl3w/include)
set(gl3w_src thirdparty/gl3w/src/gl3w.c)

# --- STB image write ---
include_directories(thirdparty/stb_image)

# --- Imgui ---
set(imgui_DIR "thirdparty/imgui")
set(imgui_src
        ${imgui_DIR}/imgui.cpp
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_widgets.cpp)
include_directories(thirdparty/imgui)

# For GLFWwindowHandler
include_directories(src/rendering)
set(window_handler src/rendering/GLFWwindow_handler.h)

# --- Imgui bindings ---
set(imgui__bindings_DIR "thirdparty/imgui_bindings")
set(imgui_binding
        ${imgui__bindings_DIR}/imgui_impl_glfw.cpp
        ${imgui__bindings_DIR}/imgui_impl_glfw.h
        ${imgui__bindings_DIR}/imgui_impl_opengl3.cpp
        ${imgui__bindings_DIR}/imgui_impl_opengl3.h)
include_directories(thirdparty/imgui_bindings)

# --- nativefiledialog ---
if (WIN32)
    set(native_DIR "thirdparty/nativefiledialog/src")
    set(native_src
            ${native_DIR}/common.h
            ${native_DIR}/include/nfd.h
            ${native_DIR}/nfd_common.h
            ${native_DIR}/nfd_common.c
            ${native_DIR}/nfd_win.cpp
            ${native_DIR}/common.h)

    add_library(nfd ${native_src})

    target_link_libraries(nfd comctl32.lib)
endif()
# TODO implement for other OS
include_directories(thirdparty/nativefiledialog/src/include)

# --- Main source ---
file (GLOB _project_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        src/*.cpp src/*.h
        src/rendering/*.cpp src/rendering/*.h
        src/rendering/ui/*.cpp src/rendering/ui/*.h
        src/core/project/*.cpp src/core/project/*.h
        src/core/ml/*.cpp src/core/ml/*.h
        src/core/dataset/*.cpp src/core/dataset/*.h
        src/core/editor/*.cpp src/core/editor/*.h)


# --- All sources for the tests ---
set(all_sources ${_project_src} ${GLAD_GL} ${imgui_src} ${imgui_binding} ${gl3w_src})
add_executable(${PROJECT_NAME} ${all_sources})
list(REMOVE_ITEM all_sources "src/main.cpp")
add_library(${PROJECT_NAME}_lib STATIC ${all_sources})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} nfd)

target_link_libraries(${PROJECT_NAME}_lib glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_lib ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME}_lib nfd)

# --- Add all assets ---
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

###############################################################
# Google test
################################################################

add_subdirectory(tests)
