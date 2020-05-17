# GUI testing

list(TRANSFORM all_sources PREPEND "../../")

include(GoogleTest)
enable_testing()
find_package(GTest)

if (GTEST_FOUND)
    message(${GTEST_INCLUDE_DIR})

    add_executable(unit_tests_gui gui/test_app.cpp ${all_sources})
    target_include_directories(unit_tests_gui PRIVATE "../../src")
    target_link_libraries(unit_tests_gui ${PROJECT_NAME}_lib gtest_main pthread)
    gtest_discover_tests(unit_tests_gui)
endif()