find_package(Threads REQUIRED)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# Set variable sfor find_package(GTest)
set(GTEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-src/googletest/include)

set(GTEST_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/googletest-build/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(GTEST_LIBRARY gtest)
set_target_properties(${GTEST_LIBRARY} PROPERTIES
        "IMPORTED_LOCATION" "${GTEST_LIBRARY_PATH}"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")

set(GTEST_MAIN_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR}/googletest-build/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
set(GTEST_MAIN_LIBRARY gtest_main)
set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES
        "IMPORTED_LOCATION" "${GTEST_MAIN_LIBRARY_PATH}"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}")
# "INTERFACE_INCLUDE_DIRECTORIES" "${GTEST_INCLUDE_DIR}" does not work


find_package(GTest)
include(GoogleTest)

# Unit testing
add_subdirectory(unit)
