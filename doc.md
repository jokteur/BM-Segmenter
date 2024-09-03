# Pyhton

Some features of the program, including importing dicom images, automatic computing of segmentations using the machine learning model, and computing of the surfaces of the segmentations, are implemented in Python. The C++ code calls the appropriate python scripts to perform these actions.

The Python scripts used by the program are located at `src/python/scripts`. This directory contains various py files.
Additionally, `src/python/scripts/project_edition` contains the package `bms_project_edition` which provides the machine learning and surfaces calculations features.

Our goal here is to document the management of the Python module search path (sys.path), which allows the python `import` instructions to correctly import these scripts when the program is executed.

When the program is installed, the program installation directory includes a `python/` directory, and the `scripts` directory discussed above is copied into `pyhton/scripts` in the installation directory.

Initially, the Python module search path includes the `python/` directory of the installation directory (see `py_api.h`).

At the beginning of the program, the function `PyAPI::init()` from `init_python.h` is called, which makes python run an import of `python.scripts.__init__`. This file contains code which makes `python/scripts/project_edition` being added to the Python search path. As a consequence, both `python/` and `python/scripts/project_edition` are in the Python search path during the program execution.

The various scripts located at the root of `python/scripts` are able to import each other as they use relative imports.
The package `bms_project_edition` located in `python/scripts/project_edition` uses absolute imports, which correctly work as `python/scripts/project_edition` is in the Python search path.

