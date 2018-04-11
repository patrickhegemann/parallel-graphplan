#include "SATSolverThreadPool.h"
#include "Logger.h"
extern "C" {
    #include "ipasir.h"
}


/**
 * Initializes the thread pool with the given amount of tags.
 */
SATSolverThreadPool::SATSolverThreadPool(int tagCount) {
    stopped = false;
    this->tagCount = tagCount;

    // Allocate mutexes and condition variables
    queueMutexes = (std::mutex**) malloc(tagCount * sizeof(std::mutex*));
    queueConditions = (std::condition_variable**)
        malloc(tagCount * sizeof(std::condition_variable*));

    for (int i = 0; i < tagCount; i++) {
        queueMutexes[i] = new std::mutex();
        queueConditions[i] = new std::condition_variable();
    }

    // Resize worker and job vectors
    workers.resize(tagCount);
    jobs.resize(tagCount);
}

/**
 * Stops all worker threads and joins them. This doesn't kill the threads,
 * but rather let's them finish their current jobs.
 */
SATSolverThreadPool::~SATSolverThreadPool() {
    // Wake and join all worker threads
    // First lock all the mutexes
    for (int i = 0; i < tagCount; i++) {
        queueMutexes[i]->lock();
    }
    // Then we can set the stopped variable to true
    stopped = true;
    // Notify workers from all queues
    for (int i = 0; i < tagCount; i++) {
        queueConditions[i]->notify_all();
    }
    // Unlock all the mutexes
    for (int i = 0; i < tagCount; i++) {
        queueMutexes[i]->unlock();
    }

    // Finally join all workers of all tags
    for (auto& w : workers) {
        for (auto& thr : w) {
            thr.join();
        }
    }

    for (int i = 0; i < tagCount; i++) {
        delete queueMutexes[i];
        delete queueConditions[i];
    }
    free(queueMutexes);
    free(queueConditions);
}

/**
 * Adds the given amount of workers to the given tag's pool. SAT solvers will 
 * be initialized using the given function with the given arguments.
 */
void SATSolverThreadPool::addWorkers(int tag, int amount,
    void*(*solverInit)(void*), void *initArgs) {
    // Spawn the specified number of threads
    for (int i = 0; i < amount; i++) {
        // Initialize worker arguments
        WorkerThreadArguments *workerArgs = new WorkerThreadArguments();
        workerArgs->pool = this;
        workerArgs->tag = tag;
        workerArgs->solver = solverInit(initArgs);  // Initialize the solver

        // Spawn and add the worker
        workers[tag].push_back(std::thread(workerThread, workerArgs));
    }
}

/**
 * Enqueue a new job with the specified tag
 */
void SATSolverThreadPool::enqueueJob(int tag, Job job) {
    // Acquire queue mutex and add job if possible
    std::unique_lock<std::mutex> lck(*(queueMutexes[tag]));
    
    if (!stopped) {
        jobs[tag].push(job);
        queueConditions[tag]->notify_one();
    }
}

/**
 * Checks whether there are no more jobs in the queue.
 */
bool SATSolverThreadPool::isDone() {
    // Check each queue if it's empty
    for (int i = 0; i < tagCount; i++) {
        // Lock the queue mutex and then check
        auto& m = *(queueMutexes[i]);
        std::unique_lock<std::mutex> lck(m);
        if (!jobs[i].empty()) {
            return false;
        }
        // Note: mutexes will be unlocked automatically after each iteration
    }
    return true;
}


/**
 * Gets the next job out of the tag's queue
 */
SATSolverThreadPool::Job* SATSolverThreadPool::getNextJob(int tag) {
    // Acquire queue mutex
    std::unique_lock<std::mutex> lck(*(queueMutexes[tag]));

    // Return 0 if pool was stopped meanwhile
    SATSolverThreadPool::Job *job = 0;

    if (!stopped) {
        // Wait for wakeup if there is no job currently
        while (jobs[tag].empty() && !stopped) {
            queueConditions[tag]->wait(lck);
        }

        // Check if pool stopped in the meantime
        if (!stopped) {
            // Get the next job from the queue
            job = new SATSolverThreadPool::Job();
            job->func = jobs[tag].front().func;
            job->arguments = jobs[tag].front().arguments;
            jobs[tag].pop();
        }
    }

    // Return the job (Lock will automatically be released)
    return job;
}

/**
 * The main function for each worker thread.
 * Waits for and then executes jobs with its tag.
 */
void* SATSolverThreadPool::workerThread(void *args) {
    // Get arguments
    WorkerThreadArguments *wargs = (WorkerThreadArguments*) args;
    SATSolverThreadPool *pool = wargs->pool;
    int tag = wargs->tag;
    void *solver = wargs->solver;

    SATSolverThreadPool::Job *job;
    while ((job = pool->getNextJob(tag))) {
        job->func(solver, job->arguments);
        delete job;
    }

    log(1, "calling ipasir release now\n");
    // TODO: Make this generic
    ipasir_release(solver);
    
    return 0;
}

