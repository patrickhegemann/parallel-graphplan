#ifndef SATSOLVERTHREADPOOL_H_
#define SATSOLVERTHREADPOOL_H_

#include <vector>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>


/**
 * A thread pool with multiple queues, for SAT solving applications.
 * In a SATSolverThreadPool there are a number of tags. Each worker and job has
 * a tag. Many workers and many jobs can have the same tag respectively. Jobs
 * will only be scheduled to workers with the same tag. Additionally, each
 * worker has its own SAT solver instance, which is initialized using a function
 * specified by the user.
 *
 * Author: Patrick Hegemann
 */
class SATSolverThreadPool {
    public:
        SATSolverThreadPool(int tagCount);
        ~SATSolverThreadPool();

        void addWorkers(int tag, int amount, void*(*solverInit)(void*), void *initArgs);

        // Arguments passed to each worker thread
        struct WorkerThreadArguments {
            SATSolverThreadPool* pool;  // Reference to the thread pool
            int tag;                    // This thread's tag
            void* solver;               // Reference to the SAT solver
        };

        // Job that can be queued for a pool
        struct Job {
            // Function for this job. First argument is the SAT solver
            // Second argument is the arguments member of this struct.
            void*(*func)(void*, void*);
            void* arguments;               // Arguments for the function
        };

        void enqueueJob(int tag, Job job);
        bool isDone();

    private:
        // Amount of tags in this pool
        int tagCount;

        // A vector of vectors of worker threads
        // One vector for each tag
        std::vector<std::vector<std::thread>> workers;
        // A queue for jobs of each tag
        std::vector<std::queue<Job>> jobs;

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

