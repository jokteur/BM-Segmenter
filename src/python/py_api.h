#ifndef BM_SEGMENTER_PY_API_H
#define BM_SEGMENTER_PY_API_H

// Workaround for pybind bug in debug. See https://github.com/microsoft/onnxruntime/issues/9735
#define _STL_CRT_SECURE_INVALID_PARAMETER(expr) _CRT_SECURE_INVALID_PARAMETER(expr)

#ifdef _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif

#include <pybind11/embed.h>
#include "cv2np.h"
#include <string>
#include <locale>
#include <codecvt>
#include <iostream>

namespace PyAPI {
    namespace py = pybind11;

    /**
     * This class is to keep the Python interpreter alive during the whole
     * execution of the program.
     *
     * This class sets the correct path for the python execution.
     * Calling Handler::getInstance for the first time will initialize
     * the Python interpreter.
     */
    class Handler {
    private:

        Handler() {
            // wchar_t *home_dir = Py_DecodeLocale("python", nullptr);
            // Py_SetPythonHome(home_dir);
            // std::cout << "HEllo" << *home_dir << std::endl;
            std::string pythonHome = "/Users/jokteur/miniconda3/envs/bmseg/";
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wide_pythonHome = converter.from_bytes(pythonHome);
            // std::cout << "Gello world" << std::endl;
            
            Py_SetPythonHome(wide_pythonHome.data());
            
            py::initialize_interpreter();
            // PyEval_SaveThread();
        }
    public:
        /**
         * Copy constructors stay empty, because of the Singleton
         */
        Handler(Handler const &) = delete;
        void operator=(Handler const &) = delete;

        /**
         * @return instance of the Singleton of the EventQueue
         */
        static Handler& getInstance () {
            static Handler instance;
            return instance;
        }


        ~Handler() {
//            PyGILState_Ensure();
//            py::finalize_interpreter();
        }

    };
}
#endif //BM_SEGMENTER_PY_API_H
