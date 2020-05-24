#include "jobscheduler.h"

#include <exception>
#include <mutex>
#include <condition_variable>
#include <chrono>

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
    std::lock_guard<std::mutex> guard(kill_mutex_);
    if (size > num_active_workers_) {
        for(int i = 0;i < size - num_active_workers_;i++) {
            workers_.push_back(Worker {});
            Worker &worker = *(--workers_.end());
            worker.id = worker_counter_++;
            std::thread *thread = new std::thread(&JobScheduler::worker_fct, this, std::ref(worker));
            worker.thread = thread; //Is freed when killed (with the garbage collector)
        }
    }
        // Remove thread(s)
    else if (size < num_active_workers_) {
        kill_x_workers_ += num_active_workers_ - size;
        // Notify the first thread to kill itself
        for (int i = 0;i < num_active_workers_ - size;i++)
            semaphore_.post();
    }

    num_active_workers_ = size;
}

void JobScheduler::worker_fct(JobScheduler::Worker &worker) {
    while(true) {
        worker.state = WORKER_STATE_IDLE;
        semaphore_.wait();
        // If any thread must be killed, this thread will commit suicide
        {
            std::lock_guard<std::mutex> guard(kill_mutex_);
            if (kill_x_workers_ > 0) {
                worker.state = WORKER_STATE_KILLED;
                --kill_x_workers_;
                break;
            }
        }

        Job *current_job;
        bool execute_job = false;
        // Search for a pending job
        {
            std::lock_guard<std::mutex> guard(jobs_mutex_);
            // Get the most urgent job from the priority queue
            Job &job = *priority_queue_.top().it;
            if(job.abort) {
                job.state = Job::JOB_STATE_CANCELED;
                post_event(job);
            }
            else {
                current_job = &job;
                current_job->state = Job::JOB_STATE_RUNNING;
                execute_job = true;
            }
            priority_queue_.pop();
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
                post_event(*current_job);
            }
        }
    }
}

JobReference JobScheduler::addJob(std::string name, jobFct &function, Job::jobPriority priority) {
    Job job{
        .name = name,
        .id = job_counter_++,
        .fct = function,
        .priority = priority,
    };
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    jobs_list_.push_back(job);

    JobReference jobReference {
        .it = --(jobs_list_.end())
    };

    priority_queue_.push(jobReference);
    semaphore_.post();
    return jobReference;
}

void JobScheduler::stopJob(JobReference& jobReference) {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    jobReference.it->abort = true;
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
    for (auto &job : jobs_list_) {
        if(job.id == id) {
            return job;
        }
    }
    // Did not found any job
    Job job{
        .name = "",
        .state = Job::JOB_STATE_NOTEXISTING,
    };
    return job;
}

bool JobScheduler::isBusy() {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_list_) {
        if(job.state == Job::JOB_STATE_PENDING || job.state == Job::JOB_STATE_RUNNING)

            return true;
    }
    return false;
}

void JobScheduler::cancelAllPendingJobs() {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_list_) {
        if(job.state == Job::JOB_STATE_PENDING) {
            job.abort = true;
        }
    }
}

void JobScheduler::abortAll() {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    for (auto &job : jobs_list_) {
        job.abort = true;
    }
}


void JobScheduler::post_event(Job &job) {
    std::string event_name = std::string("jobs/ids/") + std::to_string(job.id);
    std::string event_name2 = std::string("jobs/names/") + job.name;

    event_queue_.post(Event_ptr(new JobEvent(event_name, job)));
    event_queue_.post(Event_ptr(new JobEvent(event_name2, job)));
}

void JobScheduler::remove_job_from_list(JobReference &jobReference) {
    jobs_list_.erase(jobReference.it);
}

void JobScheduler::removeJob(JobReference &jobReference) {
    std::lock_guard<std::mutex> guard(jobs_mutex_);
    Job &job = *jobReference.it;
    if (job.state == Job::JOB_STATE_PENDING || job.state == Job::JOB_STATE_RUNNING)
        throw JobSchedulerException("Tried to call removeJob() on a Job before it terminated");

    remove_job_from_list(jobReference);
}

