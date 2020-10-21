cmake_minimum_required(VERSION 3.14)
project(BM-Segmenter)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_EXE_LINKER_FLAGS -static)
set(GCC_COVERAGE_COMPILE_FLAGS "-s -O3 -fvisibility=hidden")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/install)

# Python and TOML11 do not like each other
# This flag avoids interopabilities problems
add_compile_definitions(HAVE_SNPRINTF)

# -----
# glfw3
# -----
find_package(OpenGL REQUIRED)

set(ENKITS_BUILD_EXAMPLES OFF CACHE BOOL  "Build basic example applications" )
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_TESTS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set(GLFW_BUILD_INSTALL OFF CACHE BOOL  "GLFW lib only" )

set(INSTALL_DIR "0.1")

add_subdirectory(thirdparty/glfw)
include_directories(thirdparty/glfw/include)

# ----
# gl3w
# ----
include_directories(thirdparty/gl3w/include)
set(gl3w_src thirdparty/gl3w/src/gl3w.c)

# ---------------
# STB image write
# ---------------
include_directories(thirdparty/stb_image)

# ------
# OpenCV
# ------
find_package(OpenCV 4.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)

# -------
# PyTorch
# -------
find_package(Torch REQUIRED)
#message("${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# The following code block is suggested to be used on Windows.
# According to https://github.com/pytorch/pytorch/issues/25457,
# the DLLs need to be copied to avoid memory errors.
if (WIN32)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    install(
        FILES ${TORCH_DLLS}
        DESTINATION ${INSTALL_DIR}
    )
endif (WIN32)

# --------------------------------
# Python installation and pybind11
# --------------------------------

add_definitions(-DPy_ENABLE_SHARED)

option(Python_ROOT_DIR "")

find_package(Python REQUIRED Interpreter Development)

if((NOT ${Python_FOUND}))
    message(FATAL_ERROR
            "Missing Python installation\n"
            "Make sure that Python is installed on the system "
            "or to set the cache entry 'Python_ROOT_DIR' to point "
            "to a valid python installation"
            )
endif()
# Ensure that the python header is found
if(NOT EXISTS "${Python_INCLUDE_DIRS}/Python.h")
    message(FATAL_ERROR
            "Missing: \"${Python_INCLUDE_DIRS}/Python.h\",\n"
            "Set the cache entry 'Python_INCLUDE_DIRS' to point "
            "to a valid python include path. Containing "
            "Python.h for python version \"${Python_VERSION}\""
            )
endif()

include_directories(${Python_INCLUDE_DIRS})
add_subdirectory(src/python)

add_subdirectory(thirdparty/pybind11)
include_directories(thirdparty/pybind11/include)

# ----------
# Dear Imgui
# ----------

include_directories(thirdparty/imgui/misc/cpp)
set(imgui_DIR "thirdparty/imgui")
set(imgui_src
        ${imgui_DIR}/imgui.cpp
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_widgets.cpp
        ${imgui_DIR}/misc/cpp/imgui_stdlib.cpp
        ${imgui_DIR}/misc/cpp/imgui_stdlib.h)
include_directories(thirdparty/imgui)

# For GLFWwindowHandler
include_directories(src/rendering)
set(window_handler src/rendering/GLFWwindow_handler.h)

# --- Imgui bindings ---
set(imgui_bindings_DIR "thirdparty/imgui_bindings")
set(imgui_binding
        ${imgui_bindings_DIR}/imgui_impl_glfw.cpp
        ${imgui_bindings_DIR}/imgui_impl_glfw.h
        ${imgui_bindings_DIR}/imgui_impl_opengl3.cpp
        ${imgui_bindings_DIR}/imgui_impl_opengl3.h)
include_directories(thirdparty/imgui_bindings)

# ----------------
# nativefiledialog
# ----------------
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

# ------
# TOML11
# ------
include_directories(thirdparty/toml11)

# -----------
# Main source
# -----------
include_directories(src)
file (GLOB _project_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        src/*.cpp src/*.h
        src/python/*.h
        src/python/*.cpp
        src/rendering/*.cpp src/rendering/*.h
        src/rendering/ui/*.cpp src/rendering/ui/*.h
        src/rendering/ui/modales/*.h
        src/rendering/ui/modales/*.cpp
        src/rendering/ui/widgets/*.h
        src/core/project/*.cpp src/core/project/*.h
        src/core/ml/*.cpp src/core/ml/*.h
        src/core/dataset/*.cpp src/core/dataset/*.h
        src/core/editor/*.cpp src/core/editor/*.h)

set(all_sources ${_project_src} ${GLAD_GL} ${imgui_src} ${imgui_binding} ${gl3w_src})
list(REMOVE_ITEM all_sources "src/main.cpp")

add_library(${PROJECT_NAME}_lib STATIC ${all_sources})

target_link_libraries(${PROJECT_NAME}_lib glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_lib ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME}_lib nfd)
target_link_libraries(${PROJECT_NAME}_lib ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME}_lib ${Python_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_lib "${TORCH_LIBRARIES}")


add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})
install(DIRECTORY assets DESTINATION ${INSTALL_DIR})

###############################################################
# Google test
###############################################################

#add_subdirectory(tests)
