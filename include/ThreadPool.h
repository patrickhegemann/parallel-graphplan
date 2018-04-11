#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>


/**
 * A simple thread pool.
 *
 * Author: Patrick Hegemann
 */
class ThreadPool {
    public:
        ThreadPool(int workerCount);
        ~ThreadPool();

        struct Job {
            void*(*func)(void*);    // Function for this job
            void* arguments;        // Arguments for the function
        };

        void enqueueJob(Job job);
        bool isDone();

    private:
        // An array of worker threads
        std::vector<std::thread> workers;
        // A queue for the jobs
        std::queue<Job> jobs;

        // A mutex for the job queue
        std::mutex queueMutex;
        // Condition variable that is used to notify workers when there is work
        std::condition_variable queueCondition;

        // Indicates whether the pool was force stopped
        bool stopped;

        static void* workerThread(void *args);
        Job* getNextJob();
};

#endif

