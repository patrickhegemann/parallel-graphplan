#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>
#include <assert.h>

#include <unistd.h>
#include <thread>

#include "Planners/SimpleParallelPlannerWithSAT.h"
#include "Logger.h"
#include "Settings.h"

extern "C" {
    #include "ipasir.h"
}



SimpleParallelPlannerWithSAT::SimpleParallelPlannerWithSAT(IPlanningProblem *problem) {
    this->problem = problem;

    // Create thread pool with 1 tag (0)
    int threadCount = settings->getThreadCount();
    threadPool = new SATSolverThreadPool(1);
    threadPool->addWorkers(0, threadCount, createSATSolver, this);

    // Get action and proposition count from problem data
    countActions = problem->getActionCount();
    countPropositions = problem->getPropositionCount();
}

SimpleParallelPlannerWithSAT::~SimpleParallelPlannerWithSAT() {
    //delete threadPool;
}

int SimpleParallelPlannerWithSAT::graphplan(Plan& plan) {
    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable()) {
        expand();
        fixedPoint = fixedPoint || checkFixedPoint();
    }

    // If goal is impossible to reach, this problem has no solution
    if (checkGoalUnreachable()) {
        log(1, "Goal unreachable, aborting\n");
        return false;
    }

    int nextExtractionLayer = problem->getLastActionLayer();

    while (!problemSolved) {
        // 50 is just a random number
        if (nextExtractionLayer < 50) {
            // Queue an extraction job to the pool
            ThreadParameters *tp = new ThreadParameters();
            tp->planner = this;
            tp->layer = nextExtractionLayer;
            SATSolverThreadPool::Job j;
            j.func = extractionThread;
            j.arguments = (void*)tp;
            threadPool->enqueueJob(0, j);

            // Expand the graph for one more layer
            // TODO: mutex for the graph (-> segfault)
            expand();
            fixedPoint = fixedPoint || checkFixedPoint();
            nextExtractionLayer++;
        }

        sleep(1);
    }

    plan = solution;

    return true;
}

// Initializes one SAT solver for one thread
void* SimpleParallelPlannerWithSAT::createSATSolver(void *args) {
	void *solver = ipasir_init();

	// Set termination and clause learning callbacks
	ipasir_set_terminate(solver, args, solverTerminator);
    ipasir_set_learn(solver, NULL, 0, NULL);

    return solver;
}

// Each extraction thread is running this function which 
void* SimpleParallelPlannerWithSAT::extractionThread(void* solver, void* args) {
    // Get parameters for this thread
    ThreadParameters *param = (ThreadParameters*) args;
    SimpleParallelPlannerWithSAT *planner = param->planner;
    int layer = param->layer;

    // Get the last layer of clauses that have been added to the solver
    // TODO: Is this thread safe?
    int lastLayer = 1;
    if (planner->solversLastLayer.count(solver) == 0) {
        planner->solversLastLayer[solver] = 1;
    } else {
        lastLayer = planner->solversLastLayer[solver];
    }

    // Add necessary clauses to this thread's SAT solver
    for (int i = lastLayer; i <= layer; i++) {
        planner->addClausesToSolver(solver, i);
    }
    // Update solver information
    planner->solversLastLayer[solver] = layer;

    // Extract plan
    Plan plan;
    int success = planner->extract(solver,
            planner->problem->getGoal(),
            planner->problem->getPropLayerAfterActionLayer(layer),
            plan);

    if (success) {
        log(1, "success\n");
        planner->markProblemSolved(plan);
    }

    delete param;
	return NULL;
}


// Can be called by a thread to provide a solution to the planning problem.
void SimpleParallelPlannerWithSAT::markProblemSolved(Plan plan) {
    log(1, "waiting for mutex\n");
    solvedMutex.lock();
    if (!problemSolved) {
        problemSolved = true;
        solution = plan;
    }
    solvedMutex.unlock();
    log(1, "unlocked mutex\n");
}

// This function is called by the SAT solvers from different threads
// to determine if they should abort solving of the formula.
// A non-zero value indicates that the solver should abort (in accordance with
// the ipasir definition).
int SimpleParallelPlannerWithSAT::solverTerminator(void* state) {
    // TODO: Implement
    int t = ((SimpleParallelPlannerWithSAT*) state)->problemSolved;
    log(1, "solverTerminator called, result: %d\n", t);
	return t;
}

void SimpleParallelPlannerWithSAT::expand() {
    Planner::expand();
}

