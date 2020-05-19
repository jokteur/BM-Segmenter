#ifndef BM_SEGMENTER_JOBSCHEDULER_H
#define BM_SEGMENTER_JOBSCHEDULER_H

#include <thread>
#include <string>
#include <mutex>
#include <list>
#include <map>
#include <iostream>
#include <functional>
#include <condition_variable>

/**
 * Semaphore for waking the worker up when a new job is available
 */
class Semaphore {
private:
    std::mutex mutex;
    unsigned int counter = 0;
    std::condition_variable cv;
public:
    Semaphore() = default;
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        while (!counter) {
            cv.wait(lock);
        }
        --counter;
    }
    void post() {
        std::lock_guard<std::mutex> guard(mutex);
        ++counter;
        cv.notify_one();
    }
};

typedef std::function<bool (float &, bool &)> jobFct;
typedef int jobId;
typedef int workerId;

/*
 * Job description
 */
struct Job {
    enum jobState {JOB_STATE_PENDING, JOB_STATE_RUNNING, JOB_STATE_FINISHED, JOB_STATE_ERROR, JOB_STATE_CANCELED, JOB_STATE_ABORTED};
    std::string name;
    jobId id;

    jobFct fct;
    jobState state = JOB_STATE_PENDING;
    float progress = 0.f;

    std::exception exception;
    bool abort = false;
    bool success = false;
};


/**
 * @brief The JobScheduler class is there to allow multi-threading in the app\n
 *
 * We don't want that the GUI freeze whenever a heavy calculation is done.
 * The JobScheduler helps by launching jobs in other thread(s), called Workers. First the user
 * must define a number of workers that will always wait for new jobs to execute.
 * The class allows for a syntax that permits to read the progress of a job, or send
 * a command to abort a job. \n
 *
 * Here is a sample code with some sample jobs and a sample UI :
 * @code{.cpp}
 *
 * @endcode
 *
 */
class JobScheduler {
private:
    enum workerState {WORKER_STATE_IDLE, WORKER_STATE_WORKING, WORKER_STATE_KILLED};
    struct Worker {
        workerState state = WORKER_STATE_IDLE;
        workerId id;
        std::thread *thread;
    };

    jobId job_counter_ = 0;
    workerId worker_counter_ = 0;
    int num_active_workers_ = 0;

    int thread_pool_size_ = 0;

    int kill_x_workers_ = 0;
    std::mutex kill_mutex_;

    std::list<Job> jobs_;
    std::mutex jobs_mutex_;
    Semaphore semaphore_;
    std::list<Worker> workers_;

    JobScheduler() {
        setWorkerPoolSize(1);
    }


public:
    /**
     * Copy constructors stay empty, because of the Singleton
     */
    JobScheduler(JobScheduler const &) = delete;
    void operator=(JobScheduler const &) = delete;

    /**
     * @return instance of the Singleton of the Job Scheduler
     */
    static JobScheduler& getInstance () {
        static JobScheduler instance;
        return instance;
    }

    /**
     * Sets the number of threads (workers) available
     * If the given size is less than the number of active jobs, the function will first wait
     * that some of the jobs are finished before killing the excess workers
     * @param size of the worker pool
     */
    void setWorkerPoolSize(int size);

    /**
     * Adds a new job to the scheduler
     * The job starts whenever a thread is available and search for a new job
     * Job fairness is not guaranteed
     * @param name name of the job
     * @param function lambda function to be executed by the job. The function should be in this format :
     * std::function<bool (float &progress, bool &abort)>, progress should be between 0 and 1 and indicate
     * to outsiders the progress of the job, and abort can be read to see if an abort command has been
     * carried on. It is recommended to implement these two arguments for efficient execution
     * @return id of the given job
     */
    jobId addJob(std::string &name, jobFct &function);

    /**
     * Stops the job with the given id (if the jobs has implemented bool &abort of the lambda function)
     * @param id id of the job
     */
    void stopJob(jobId id);

    /**
     * Get the information about a certain job at a given time (copy of the job)
     * @param id id of the job
     * @return a copy of the Job structure which should contain informations about the job's state, success, ...
     */
    const Job getJobInfo(jobId id);

    /**
     * @return number of active workers
     */
    const int getNumberOfWorkers() { return num_active_workers_; }

    /**
     * Function to check if there are any pending or running jobs
     * @return true if any job is pending or running
     */
    bool isBusy();

    /**
     * Cancels all jobs that are still in pending
     */
    void cancelAllPendingJobs();

    /**
     * Function that is executed by each thread to look, wait and execute new jobs
     * This function should have been private, but had to be made public for std::thread
     */
    void worker_fct(Worker &worker);

    /**
     * Garbage collector of the workers
     * Once threads have been killed, they JobScheduler does not automatically frees the pointer on the thread
     * This function should be called regularly to clean the dandling pointers of the killed threads
     * TODO : avoid garbage collecting
     */
    void clean();

    ~JobScheduler() {}
};


#endif //BM_SEGMENTER_JOBSCHEDULER_H
