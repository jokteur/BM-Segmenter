#include "jobscheduler.h"

#include <exception>
#include <mutex>
#include <condition_variable>

/*
 * Exceptions related to the JobScheduler class
 */

class JobSchedulerException: public std::exception
{
private:
    const char* what_;
public:
    explicit JobSchedulerException(const char* what) : what_(what) {}
    virtual const char* what() const noexcept
    {
        return what_;
    }
};

/*
 * Implementations of JobScheduler
 */

void JobScheduler::setWorkerPoolSize(int size) {
    if (size < 1) {
        throw JobSchedulerException("Cannot set thread pool size to less than 1");
    }

    // Add thread(s)
    if (size > num_active_workers_) {
        for(int i = 0;i < size - num_active_workers_;i++) {
            workers_.push_back(Worker {});
            Worker &worker = *workers_.end();
            std::thread *thread = new std::thread(&JobScheduler::worker_fct, this, std::ref(worker));
            worker.id = worker_counter_++;
            worker.thread = thread; //Is freed when killed (with the garbage collector)
        }
    }
        // Remove thread(s)
    else if (size < num_active_workers_) {
        std::lock_guard<std::mutex> guard(kill_mutex_);
        kill_x_workers_ += num_active_workers_ - size;
        // Notify the first thread to kill itself
        semaphore_.post();
    }

    num_active_workers_ = size;
}

void JobScheduler::worker_fct(JobScheduler::Worker &worker) {
    semaphore_.wait();
    while(true) {
        // If any thread must be killed, this thread will suicide itself
        {
            std::lock_guard<std::mutex> guard(kill_mutex_);
            if (kill_x_workers_ > 0) {
                worker.state = WORKER_STATE_KILLED;
                --kill_x_workers_;

                //This thread notify other potential thread to kill themselves
                if (kill_x_workers_ > 0)
                    semaphore_.post();
                break;
            }
        }

        Job *current_job;
        bool execute_job = false;
        // Search for a pending job
        {
            std::lock_guard<std::mutex> guard(jobs_mutex_);
            for (auto &job : jobs_) {
                if (job.state == Job::JOB_STATE_PENDING) {
                    if(job.abort) {
                        job.state = Job::JOB_STATE_CANCELED;
                    }
                    else {
                        current_job = &job;
                        current_job->state = Job::JOB_STATE_RUNNING;
                        execute_job = true;
                    }
                    break;
                }
            }
        }


        if(execute_job) {
            // Execute job
            bool error = false;
            bool success = false;
            std::exception exception;
            try {
                worker.state = WORKER_STATE_WORKING;
                success = current_job->fct(current_job->progress, current_job->abort);
            }
            catch (std::exception &e) {
                exception = e;
                error = true;
            }
            // Process result of job
            {

                std::lock_guard<std::mutex> guard(jobs_mutex_);
                if (error) {
                    current_job->state = Job::JOB_STATE_ERROR;
                    current_job->exception = exception;
                } else if (current_job->abort) {
                    current_job->state = Job::JOB_STATE_ABORTED;
                    current_job->success = success;
                } else {
                    current_job->state = Job::JOB_STATE_FINISHED;
                    current_job->success = success;
                }
            }
        }

        worker.state = WORKER_STATE_IDLE;
        semaphore_.wait();
    }
}

jobId JobScheduler::addJob(std::string &name, jobFct &function) {
    Job job{
            .name = name,
            .id = job_counter_++,
            .fct = function,
    };
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    jobs_.push_back(job);
    semaphore_.post();
    return job.id;
}

void JobScheduler::stopJob(jobId id) {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_) {
        if (job.id == id) {
            job.abort = true;
        }
    }
}

void JobScheduler::clean() {
    for(auto &worker : workers_) {
        if(worker.state == WORKER_STATE_KILLED) {
            worker.thread->join();
            delete worker.thread;
        }
    }
}

const Job JobScheduler::getJobInfo(jobId id) {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_) {
        if(job.id == id) {
            return job;
        }
    }
    throw JobSchedulerException("The provided jobId does not correspond to any present or past job");
}

bool JobScheduler::isBusy() {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_) {
        if(job.state == Job::JOB_STATE_PENDING || job.state == Job::JOB_STATE_RUNNING)

            return true;
    }
    return false;
}

void JobScheduler::cancelAllPendingJobs() {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_) {
        if(job.state == Job::JOB_STATE_PENDING) {
            job.state = Job::JOB_STATE_CANCELED;
        }
    }
}

