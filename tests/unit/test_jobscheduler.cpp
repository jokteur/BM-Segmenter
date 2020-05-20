
#include <exception>
#include <complex>
#include <future>
#include <memory>
#include <unistd.h>
#include <string>

#include "jobscheduler.h"
#include <gtest/gtest.h>

/*
 * This tests if the number of workers can be allocated during runtime
 */
TEST(JobScheduler, WorkerPoolSize) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
        std::launch::async, [this] {
            JobScheduler& jobScheduler = JobScheduler::getInstance();

            EXPECT_ANY_THROW(jobScheduler.setWorkerPoolSize(0))
                << "Setting the worker pool size to 0 did not throw an error";

            // Augment the worker pool size
            int i = 1;
            for(; i < 10; ++i) {
                jobScheduler.setWorkerPoolSize(i);
                EXPECT_EQ(i, jobScheduler.getNumberOfWorkers())
                                            << "Number of active workers does not correspond to number that has been set";
            }

            // Diminish the worker pool size
            for(; i > 0; --i) {
                jobScheduler.setWorkerPoolSize(i);
                EXPECT_EQ(i, jobScheduler.getNumberOfWorkers())
                                            << "Number of active workers does not correspond to number that has been set";;
            }

            jobScheduler.clean();
        }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(50)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}

/*
 * This tests if jobs are executed correctly by collecting info and do
 * some timing measurements
 */
TEST(JobScheduler, ExecuteJobsSingleWorker) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
        std::launch::async, [this] {
            JobScheduler& jobScheduler = JobScheduler::getInstance();

            // Define a fake job that could be aborted, and that shows its progression
            int counter = 0;
            jobFct job_fct = [&counter] (float &progress, bool &abort) -> bool {
                // Simulate a progression of some kind, update every 10 ms
                counter++;
                for(int i = 0;i < 10;i++) {
                    usleep(1e4);
                    progress = float(i+1)/10.;
                }
                return true ;
            };

            std::string name("100ms job");
            jobId job_id_1 = jobScheduler.addJob(name, job_fct);
            jobId job_id_2 = jobScheduler.addJob(name, job_fct);
            jobId job_id_3 = jobScheduler.addJob(name, job_fct);

            // After 60ms, see if progress of the first job is more than 0.5
            usleep(6*1e4);
            Job job1 = jobScheduler.getJobInfo(job_id_1);

            EXPECT_GE(job1.progress, 0.5)
                << "Reported progress of the job is not correct";

            // Sleep 350ms to wait for all jobs to finish
            usleep(3.5e5);

            job1 = jobScheduler.getJobInfo(job_id_1);
            Job job2 = jobScheduler.getJobInfo(job_id_2);
            Job job3 = jobScheduler.getJobInfo(job_id_3);

            EXPECT_EQ(job1.state, Job::JOB_STATE_FINISHED)
                << "Job 1 did not finish correctly or in time";
            EXPECT_EQ(job2.state, Job::JOB_STATE_FINISHED)
                << "Job 2 did not finish correctly or in time";
            EXPECT_EQ(job3.state, Job::JOB_STATE_FINISHED)
                << "Job 3 did not finish correctly or in time";

            EXPECT_EQ(counter, 3)
                << "A data race has occured";


            jobScheduler.clean();
        }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(420)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}

/*
 * This tests if jobs can be aborted or cancel correctly within a given time frame
 */
TEST(JobScheduler, StopJobSingleWorker) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
        std::launch::async, [this] {
            JobScheduler& jobScheduler = JobScheduler::getInstance();

            // Define a fake job that could be aborted, and that shows its progression
            jobFct job_fct = [] (float &progress, bool &abort) -> bool {
                // Simulate a progression of some kind, update every 10 ms, aborts then asked
                for(int i = 0;i < 10;i++) {
                    usleep(1e4);
                    if (abort)
                        return false;
                }
                return true ;
            };

            std::string name("100ms job");
            jobId job_id_1 = jobScheduler.addJob(name, job_fct);
            jobId job_id_2 = jobScheduler.addJob(name, job_fct);
            jobId job_id_3 = jobScheduler.addJob(name, job_fct);

            // After 50ms, stop the job1 (should be active) and job3 (should be pending)
            usleep(5e4);
            jobScheduler.stopJob(job_id_1);
            jobScheduler.stopJob(job_id_3);

            // As the abort is read every 10 ms, give it at least 20ms to stop the job1
            usleep(2e4);
            Job job1 = jobScheduler.getJobInfo(job_id_1);

            EXPECT_EQ(job1.state, Job::JOB_STATE_ABORTED)
                << "Job 1 has not been aborted in time";

            // Sleep 150ms to wait for job 2 to finish
            usleep(1.5e5);

            Job job2 = jobScheduler.getJobInfo(job_id_2);
            Job job3 = jobScheduler.getJobInfo(job_id_3); //Pending jobs that have been canceled often get removed at a later time

            EXPECT_EQ(job2.state, Job::JOB_STATE_FINISHED)
                << "Job 2 did not finish correctly or in time";
            EXPECT_EQ(job3.state, Job::JOB_STATE_CANCELED)
                << "Job 3 has not been canceled correctly";

            jobScheduler.clean();
        }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(230)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}

/*
 * This tests if multiple workers are able to work concurrently
 */
TEST(JobScheduler, AddJobMultipleWorkers) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
        std::launch::async, [this] {
            JobScheduler& jobScheduler = JobScheduler::getInstance();

            jobScheduler.setWorkerPoolSize(4);

            // Define a fake job that could be aborted, and that shows its progression
            jobFct job_fct = [] (float &progress, bool &abort) -> bool {
                // Simulate a progression of some kind, update every 10 ms
                for(int i = 0;i < 10;i++) {
                    usleep(1e4);
                }
                return true ;
            };

            std::string name("100ms job");
            jobId job_id_1 = jobScheduler.addJob(name, job_fct);
            jobId job_id_2 = jobScheduler.addJob(name, job_fct);
            jobId job_id_3 = jobScheduler.addJob(name, job_fct);
            jobId job_id_4 = jobScheduler.addJob(name, job_fct);

            // After 100ms, all the jobs should have executed. For the margin, let us give 120ms
            usleep(1.2e5);

            Job job1 = jobScheduler.getJobInfo(job_id_1);
            Job job2 = jobScheduler.getJobInfo(job_id_2);
            Job job3 = jobScheduler.getJobInfo(job_id_3);
            Job job4 = jobScheduler.getJobInfo(job_id_4);

            EXPECT_EQ(job1.state, Job::JOB_STATE_FINISHED)
                << "Job 1 has not finished in time";
            EXPECT_EQ(job2.state, Job::JOB_STATE_FINISHED)
                << "Job 2 has not finished in time";
            EXPECT_EQ(job3.state, Job::JOB_STATE_FINISHED)
                << "Job 3 has not finished in time";
            EXPECT_EQ(job4.state, Job::JOB_STATE_FINISHED)
                << "Job 4 has not finished in time";

            jobScheduler.clean();
            }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(130)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}

/*
 * This tests a worker pool size that shrinks over time
 */
TEST(JobScheduler, DynamicWorkerPool) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
        std::launch::async, [this] {
            JobScheduler& jobScheduler = JobScheduler::getInstance();

            jobScheduler.setWorkerPoolSize(4);

            // Define a fake job that is 100ms long
            jobFct job_fct = [] (float &progress, bool &abort) -> bool {
                // Simulate a progression of some kind, update every 10 ms
                for(int i = 0;i < 10;i++) {
                    usleep(1e4);
                }
                return true ;
            };

            std::string name("100ms job");
            std::vector<jobId> job_ids;

            // Add 16 jobs with a worker pool size of 4
            // In this state, with each job taking 100ms, it should take 400 ms for all jobs to complete
            for(int i = 0;i < 16;++i) {
                jobId job_id = jobScheduler.addJob(name, job_fct);
                job_ids.push_back(job_id);
            }

            usleep(1e4); // Give a chance to the workers to take the jobs

            // Immediately cancel all pending jobs (only 4 active jobs should remain -> 100ms until completion)
            jobScheduler.cancelAllPendingJobs();

            // Push 4 new jobs
            // In this state, it should take approximately 200 ms for all jobs to complete
            for(int i = 0;i < 16;++i) {
                jobId job_id = jobScheduler.addJob(name, job_fct);
                job_ids.push_back(job_id);
            }

            // Diminsh the worker pool size to 2
            // The first 4 jobs should terminate with the 4 Workers, but then the next jobs should only be
            // executed by 2 Workers, which means for all jobs to terminate, it should take 100ms + 200ms = 300ms
            jobScheduler.setWorkerPoolSize(2);
            usleep(3.3e5); // Let us give 320 ms for all jobs to complete

            // Now, the situation is the following :
            // - the first 4 jobs should have been completed by 4 Workers
            // - the next 12 jobs have been cancelled
            // - the last 4 jobs should have been completed by 2 Workers

            for(int i = 0;i < 20;++i) {
                Job job = jobScheduler.getJobInfo(job_ids[i]);
                if (i >= 4 && i < 16) {
                    std::string expect_msg = std::string("Job ") + std::to_string(i) +
                            std::string(" has not been canceled as expected");
                    EXPECT_EQ(job.state, Job::JOB_STATE_CANCELED) << expect_msg;
                }
                else {
                    std::string expect_msg = std::string("Job ") + std::to_string(i) +
                            std::string(" has not finished in time");
                    EXPECT_EQ(job.state, Job::JOB_STATE_FINISHED) << expect_msg;
                }
            }

            jobScheduler.clean();
        }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(360)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}

/*
 * This test is designed to launch many jobs with lots of workers, to see
 * if there is not too much overhead
 */
TEST(JobScheduler, StressTest) {
    // Using async allows to avoid having deadlocks with the main thread while running the test
    auto asyncFuture = std::async(
            std::launch::async, [this] {
                JobScheduler& jobScheduler = JobScheduler::getInstance();

                int num_workers = 8;
                int num_jobs = 8000;

                jobScheduler.setWorkerPoolSize(num_workers);

                // Define a fake job that is 100ms long
                jobFct job_fct = [] (float &progress, bool &abort) -> bool {
                    // Simulate a progression of some kind, update every 10 ms
                    for(int i = 0;i < 10;i++) {
                        usleep(1e2);
                    }
                    return true ;
                };

                std::string name("1ms job");
                std::vector<jobId> job_ids;

                // Add 8000 jobs, with 8 Workers, it should theorically terminate in 1 second
                for(int i = 0;i < num_jobs;++i) {
                    jobId job_id = jobScheduler.addJob(name, job_fct);
                    job_ids.push_back(job_id);
                }

                float overhead = 1.1;
                usleep(overhead*1e6); // Wait for all jobs to finish within a given overhead

                EXPECT_FALSE(jobScheduler.isBusy()) << "Not all jobs have finished";


                for(int i = 0;i < num_jobs;++i) {
                    Job job = jobScheduler.getJobInfo(job_ids[i]);
                    std::string expect_msg = std::string("Job ") + std::to_string(i) +
                                                 std::string(" has not finished in time");
                    EXPECT_EQ(job.state, Job::JOB_STATE_FINISHED) << expect_msg;
                }

                jobScheduler.clean();
            }
    );

    ASSERT_TRUE(asyncFuture.wait_for(std::chrono::milliseconds(1300)) != std::future_status::timeout)
        << "Test did not terminate within the expect time.";
}