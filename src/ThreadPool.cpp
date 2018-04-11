#include "ThreadPool.h"


/**
 * Initializes the thread pool with the given amount of worker threads
 */
ThreadPool::ThreadPool(int workerCount) {
    stopped = false;

    // Spawn the specified number of threads
    for (int i = 0; i < workerCount; i++) {
        workers.push_back(std::thread(workerThread, this));
    }
}

/**
 * Stops all worker threads and joins them. This doesn't kill the threads,
 * but rather let's them finish their current jobs.
 */
ThreadPool::~ThreadPool() {
    // Wake and join all worker threads
    std::unique_lock<std::mutex> lck(queueMutex);
    stopped = true;
    queueCondition.notify_all();
    lck.unlock();

    for (std::thread& t : workers) {
        t.join();
    }
}

/**
 * Enqueue a new job.
 */
void ThreadPool::enqueueJob(Job job) {
    // Acquire queue mutex and add job if possible
    std::unique_lock<std::mutex> lck(queueMutex);
    
    if (!stopped) {
        jobs.push(job);
        queueCondition.notify_one();
    }
}

/**
 * Checks whether there are no more jobs in the queue.
 */
bool ThreadPool::isDone() {
    std::unique_lock<std::mutex> lck(queueMutex);
    bool done = jobs.empty();
    return done;
}


/**
 * Gets the next job out of the queue
 */
ThreadPool::Job* ThreadPool::getNextJob() {
    // Acquire queue mutex
    std::unique_lock<std::mutex> lck(queueMutex);

    // Return 0 if pool was stopped meanwhile
    ThreadPool::Job *job = 0;

    if (!stopped) {
        // Wait for wakeup if there is no job currently
        while (jobs.empty() && !stopped) {
            queueCondition.wait(lck);
        }

        // Check if pool stopped in the meantime
        if (!stopped) {
            // Get the next job from the queue
            job = new ThreadPool::Job();
            job->func = jobs.front().func;
            job->arguments = jobs.front().arguments;
            jobs.pop();
        }
    }

    // Unlock the queue mutex and return the job
    lck.unlock();
    return job;
}

/**
 * The main function for each worker thread.
 * Waits for and then executes jobs.
 */
void* ThreadPool::workerThread(void *args) {
    ThreadPool *pool = (ThreadPool*) args;
    ThreadPool::Job *job;

    while ((job = pool->getNextJob())) {
        job->func(job->arguments);
        delete job;
    }

    return 0;
}

