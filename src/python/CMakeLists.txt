
set(_PYTHON_VERSION_NO_DOTS "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")

# Install the python scripts
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/src/scripts
    DESTINATION ${INSTALL_DIR}/python
    PATTERN ".git" EXCLUDE
    PATTERN ".gitignore" EXCLUDE
    PATTERN ".arcconfig" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
)

install(
    PROGRAMS ${Python_EXECUTABLE}
    DESTINATION ${INSTALL_DIR}/python/bin
)

# Check for the required packages
file(STRINGS ../scripts/requirements.txt PYTHON_REQUIREMENTS)
foreach(_package IN LISTS PYTHON_REQUIREMENTS)
    if(NOT IS_DIRECTORY "${Python_STDLIB}/site-packages/${_package}")
        message(FATAL_ERROR
            "Missing '${_package}' installation in the Python environment.\n"
            "Make sure that sites-packages contains a valid '${_package}' installation."
            )
    endif()
endforeach()

install(
    DIRECTORY ${Python_STDLIB}
    DESTINATION ${INSTALL_DIR}/python/
    PATTERN "__pycache__" EXCLUDE               # * any cache *
    PATTERN "config-${Python_VERSION}m/*.a" EXCLUDE  # static lib
    PATTERN "lib2to3" EXCLUDE                   # ./lib2to3
    PATTERN "site-packages/pydicom/data/test_files" EXCLUDE           # useless test data from pydicom
    PATTERN "tkinter" EXCLUDE                   # ./tkinter
    PATTERN "lib-dynload/_tkinter.*" EXCLUDE    # ./lib-dynload/_tkinter.co
    PATTERN "idlelib" EXCLUDE                   # ./idlelib
    PATTERN "test" EXCLUDE                      # ./test
    PATTERN "turtledemo" EXCLUDE                # ./turtledemo
    PATTERN "turtle.py" EXCLUDE                 # ./turtle.py
    PATTERN "wininst*.exe" EXCLUDE              # from distutils, avoid malware false positive
)
#
## Needed for distutils/pip
## get the last part of the include dir, will be 'python{version}{abiflag}',
install(
        FILES ${Python_INCLUDE_DIRS}/pyconfig.h
        DESTINATION ${INSTALL_DIR}/python
)

install(
    DIRECTORY ${Python_RUNTIME_LIBRARY_DIRS}/DLLs
    DESTINATION ${INSTALL_DIR}/python
    CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
    PATTERN "*.pdb" EXCLUDE
    PATTERN "*_d.*" EXCLUDE
)
#
install(
    DIRECTORY ${Python_RUNTIME_LIBRARY_DIRS}/DLLs
    DESTINATION ${INSTALL_DIR}/python
    CONFIGURATIONS Debug
)


install(
    FILES ${Python_RUNTIME_LIBRARY_DIRS}/python${_PYTHON_VERSION_NO_DOTS}.dll
        ${Python_RUNTIME_LIBRARY_DIRS}/python${Python_VERSION_MAJOR}.dll
    DESTINATION ${INSTALL_DIR}
)