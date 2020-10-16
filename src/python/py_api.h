#ifndef BM_SEGMENTER_PY_API_H
#define BM_SEGMENTER_PY_API_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pybind11/embed.h>

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
            wchar_t *home_dir = Py_DecodeLocale("python", NULL);
            Py_SetPythonHome(home_dir);

            py::initialize_interpreter();
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

        void testFct() {
            py::module scripts = py::module::import("python.scripts");
        }


        ~Handler() {
            pybind11::finalize_interpreter();
        }

    };
}

#endif //BM_SEGMENTER_PY_API_H
