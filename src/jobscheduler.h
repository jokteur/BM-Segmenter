#ifndef BM_SEGMENTER_JOBSCHEDULER_H
#define BM_SEGMENTER_JOBSCHEDULER_H

#include <thread>
#include <string>
#include <mutex>
#include <list>
#include <map>
#include <queue>
#include <iostream>
#include <functional>
#include <condition_variable>

#include "events.h"


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

/**
 * Typedef for the lambda function that will be executed
 *
 * First argument is the progress of the function
 * Second argument is the abort bool of the function
 * If this is set to true, the function should abort itself
 *
 * The function should return true if successful, false if not
 */
typedef std::function<bool (float &, bool &)> jobFct;
typedef uint64_t jobId;
typedef uint64_t workerId;

/*
 * Job description
 */
struct Job {
    enum jobState {JOB_STATE_PENDING, JOB_STATE_RUNNING, JOB_STATE_FINISHED,
            JOB_STATE_ERROR, JOB_STATE_CANCELED, JOB_STATE_ABORTED, JOB_STATE_NOTEXISTING};
    enum jobPriority {JOB_PRIORITY_LOWEST, JOB_PRIORITY_LOW, JOB_PRIORITY_NORMAL, JOB_PRIORITY_HIGH, JOB_PRIORITY_HIGHEST};

    std::string name;
    jobId id;

    jobFct fct;
    jobState state = JOB_STATE_PENDING;
    jobPriority priority = JOB_PRIORITY_NORMAL;
    float progress = 0.f;

    std::exception exception;
    bool abort = false;
    bool success = false;
};

class JobEvent : public Event {
private:
    Job &job_;
public:
    JobEvent(std::string &name, Job &job) : Event(name), job_(job) {}
    Job getJob() { return job_; }
};
#define JOBEVENT_PTRCAST(job) (reinterpret_cast<JobEvent*>((job)))

/**
 * Custom Job reference to give ability to compare priorities between
 * operators
 */

struct JobReference {
    std::list<Job>::iterator it;

    const Job& getJob() {
        return *it;
    }

    bool operator()(const JobReference& lhs, const JobReference& rhs) {
        if (lhs.it->priority == rhs.it->priority)
            return lhs.it->id > rhs.it->id;
        else
            return lhs.it->priority < rhs.it->priority;
    }
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
 * Here is a sample code using the scheduler :
 * @code{.cpp}
 *
 * namespace Rendering {
 *     class MyWindow : public AbstractLayout {
 *     private:
 *         JobScheduler &scheduler_;
 *         int counter_ = 0;
 *         jobId job_id_;
 *         std::vector<jobId> jobs_;
 *     public:
 *         MyWindow() : scheduler_(JobScheduler::getInstance()) {
 *             scheduler_.setWorkerPoolSize(3);
 *         }
 *
 *         void draw(GLFWwindow* window) override {
 *             ImGui::Begin("My Window");
 *             jobFct job;
 *             if(ImGui::Button("Launch job")) {
 *                 std::string name = "myJob";
 *                 int counter = counter_;
 *
 *                 // Dummy job
 *                 jobFct job = [counter] (float &progress, bool &abort) -> bool {
 *                     // Simulate a progression of some kind, update every 0.5 second
 *                     for(int i = 0;i < 20;i++) {
 *                         usleep(0.5*1e6);
 *                         glfwPostEmptyEvent();
 *                         if (abort)
 *                             return false;
 *                         progress = float(i+1)/20.;
 *                     }
 *                     return true ;
 *                 };
 *                 jobs_.push_back(scheduler_.addJob(name, job));
 *                 counter_++;
 *             }
 *             ImGui::End();
 *         }
 *     }
 * }
 * @endcode
 *
 * @note if you are changing the Worker pool size often, it is important to regularly call the method clean()
 * of the JobScheduler, because once a thread has been killed, its respective pointer is not automatically
 * freed by the class.
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

    std::list<Job> jobs_list_;
    std::mutex jobs_mutex_;
    std::priority_queue<JobReference, std::vector<JobReference>, JobReference> priority_queue_;
    Semaphore semaphore_;
    std::list<Worker> workers_;

    EventQueue& event_queue_;

    /**
     * Post an JobEvent to the event queue
     * If nobody was listening to the event corresponding of this job, the function returns false
     */
    void post_event(Job & job);

    /**
     * Removes the job from the list
     * Should only be called when jobs_mutex_ is already hold
     * @param job
     */
    inline void remove_job_from_list(JobReference &jobReference);

    JobScheduler() : event_queue_(EventQueue::getInstance()) {
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
     *
     * Once a job is launched, whenever a job stops (FINISHED, ABORTED, CANCELED, ERROR),
     * the JobScheduler will send two events : `jobs/names/[name]` and `jobs/ids/[job_id]`
     * The user can subscribe to either of these events
     *
     * @param name name of the job
     * @param function lambda function to be executed by the job. The function should be in this format :
     * std::function<bool (float &progress, bool &abort)>, progress should be between 0 and 1 and indicate
     * to outsiders the progress of the job, and abort can be read to see if an abort command has been
     * carried on. It is recommended to implement these two arguments for efficient execution
     * @param expect_acknowledge if it is set to true, then the job will retire under the condition
     * that all listeners have acknowledged the JobEvent. If there are no listeners, then the job
     * retires automatically
     * @return id of the given job
     */
    JobReference addJob(std::string name, jobFct &function, Job::jobPriority priority = Job::JOB_PRIORITY_NORMAL);

    /**
     * Stops the job with the given JobReference (if the jobs has implemented bool &abort of the lambda function)
     * @param id id of the job
     */
    void stopJob(JobReference &jobReference);

    /**
     * Removes the job from the history of the JobScheduler
     * After that, the job can never be accessed again with getJobInfo() or JobReference
     *
     * This function can only be called if the job terminated (by any mean), otherwise it will throw
     * an JobSchedulerException(). It is recommended to do this via a listener that listens to the
     * posted Event of the JobScheduler
     * @param jobReference
     */
    void removeJob(JobReference &jobReference);

    /**
     * Get the information about a certain job at a given time (copy of the job)
     *
     * If there is no job in the queue with the given id, the function will return a job with
     * a state of JOB_STATE_NOTEXISTING
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

    void abortAll();

    /**
     * Garbage collector of the workers
     * Once threads have been killed, the JobScheduler does not automatically frees the pointer on the thread
     * This function should be called regularly to clean the dandling pointers of the killed threads
     * TODO : avoid garbage collecting
     */
    void clean();

    ~JobScheduler() {
        abortAll();
        clean();
    }
};

#endif //BM_SEGMENTER_JOBSCHEDULER_H
