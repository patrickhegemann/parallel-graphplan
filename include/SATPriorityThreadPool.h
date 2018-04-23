#ifndef SATSOLVERPRIORITYPOOL_H_
#define SATSOLVERPRIORITYPOOL_H_

#include <vector>
#include <queue>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>


/**
 * A thread pool with multiple priority queues, for SAT solving applications.
 * The same as SATSolverThreadPool but jobs have priorities.
 *
 * Author: Patrick Hegemann
 */
class SATPriorityThreadPool {
    public:
            SATPriorityThreadPool(int tagCount);
            ~SATPriorityThreadPool();

        void addWorkers(int tag, int amount, void*(*solverInit)(void*), void *initArgs);

        // Arguments passed to each worker thread
        struct WorkerThreadArguments {
            SATPriorityThreadPool* pool;  // Reference to the thread pool
            int tag;                    // This thread's tag
            void* solver;               // Reference to the SAT solver
        };

        // Job that can be queued for a pool
        struct Job {
            // Priority - a lower value means higher priority
            int priority;
            // Function for this job. First argument is the SAT solver
            // Second argument is the arguments member of this struct.
            void*(*func)(void*, void*);
            void* arguments;               // Arguments for the function
        };

        // Comparator for job priorities
        class JobComparator {
            public:
                bool operator() (Job a, Job b) {
                    return a.priority < b.priority;
                }
        };

        void enqueueJob(int tag, int priority, Job job);
        bool isDone();

    private:
        // Amount of tags in this pool
        int tagCount;

        // A vector of vectors of worker threads
        // One vector for each tag
        std::vector<std::vector<std::thread>> workers;
        // A queue for jobs of each tag
        std::vector<std::priority_queue<Job, std::vector<Job>, JobComparator>> jobs;

        // Mutexes for each job queue
        std::mutex **queueMutexes;
        // Condition variables that are used to notify workers
        std::condition_variable **queueConditions;

        // Indicates whether the pool was force stopped
        bool stopped;

        static void* workerThread(void *args);
        Job* getNextJob(int tag);

};

#endif

