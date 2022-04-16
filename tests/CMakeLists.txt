include_directories(../src)


file (GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
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

set(test_all
    search.cpp)


add_executable(${PROJECT_NAME}_tests ${sources} ${test_all})
target_link_libraries(${PROJECT_NAME}_tests doctest::doctest ${PROJECT_NAME}_lib)

add_custom_command(TARGET ${PROJECT_NAME}_tests PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:${PROJECT_NAME}_tests>) 