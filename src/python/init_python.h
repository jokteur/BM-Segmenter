#include "py_api.h"
#include "jobscheduler.h"


namespace PyAPI {
    void init() {
        jobFct job = [](float& progress, bool& abort) -> std::shared_ptr<JobResult> {
            auto job_result = std::make_shared<JobResult>();
            auto state = PyGILState_Ensure();
            try {
                py::module scripts = py::module::import("python.scripts.__init__");
                job_result->success = true;
            }
            catch (const std::exception& e) {
                job_result->success = false;
            }

            PyGILState_Release(state);

            return job_result;
        };

        jobResultFct result_fct = [](std::shared_ptr<JobResult> result) {
            if (!result->success)
                std::cerr << "Could not load and initialize python" << std::endl;
        };
        JobScheduler::getInstance().addJob("python_init", job, result_fct);
    }
}