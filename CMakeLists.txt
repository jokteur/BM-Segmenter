cmake_minimum_required(VERSION 3.14)
project(BM_Segmenter)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXE_LINKER_FLAGS -static)
set(GCC_COVERAGE_COMPILE_FLAGS "-s -O3 -fvisibility=hidden")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build/install/0.1)


# Python and TOML11 do not like each other
# This flag avoids interopabilities problems
add_compile_definitions(HAVE_SNPRINTF)

set(APP_LOG_DEBUG OFF CACHE BOOL "Collect log for debugging")
set(APP_PRINT_DEBUG OFF CACHE BOOL "If LOG_DEBUG is on, print the log to the std")

if (${APP_LOG_DEBUG})    
    add_compile_definitions(LOG_DEBUG)
endif()
if (${APP_PRINT_DEBUG})    
    add_compile_definitions(PRINT_DEBUG)
endif()

# --------
# TempoApp
# --------

add_subdirectory(external/TempoApp)
include_directories(external/TempoApp/include)
include_directories(external/TempoApp/external/imgui/imgui)

# -----
# Fonts
# -----
include_directories(external/IconFontCppHeaders)

# ----
# Json
# ----
add_subdirectory(external/json)
include_directories(external/json/single_include)

# ------
# ImPlot
# ------
include_directories(external/ImPlot)

# ------------
# Fuzzy Search
# ------------
add_subdirectory(external/rapidfuzz-cpp)

# ------------
# ICU (unicode)
# ------------
set(ICU_ROOT "C:\\libs\\icu")
find_package(ICU COMPONENTS uc io in dt tu test REQUIRED)
message(${ICU_LIBRARIES})

include_directories(${ICU_INCLUDE_DIRS})

# -----------
# Main source
# -----------
include_directories(src)

file (GLOB source_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        external/ImPlot/*.cpp
        src/*.cpp
        src/python/*.cpp
        src/core/*.cpp
        src/ui/*.cpp
        src/ui/navigation/*.cpp
        src/ui/dataset/*.cpp
        src/ui/import/*.cpp
        src/ui/measurements/*.cpp
        src/ui/ml/*.cpp
        src/ui/translations/*.cpp
        src/ui/segmentation/*.cpp
        src/ui/widgets/*.cpp
        src/ui/search/*.cpp
    )

add_executable(${PROJECT_NAME} ${source_list} src/main.cpp ${APP_ICON_RESOURCE_WINDOWS})
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json Tempo rapidfuzz::rapidfuzz ${Python_LIBRARIES} ${ICU_LIBRARIES})

# Copy the data (such as fonts) in build directory
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:${PROJECT_NAME}>) 

# add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
#     ${CMAKE_SOURCE_DIR}/src/scripts $<TARGET_FILE_DIR:${PROJECT_NAME}>/python/scripts) 

# Set compiler options
if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_INSTALL_PREFIX})