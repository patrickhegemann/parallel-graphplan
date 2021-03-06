#include <iostream>
#include <iterator>
#include <algorithm>
#include <assert.h>
#include <map>

#include <unistd.h>
#include <thread>

#include "Planners/SimpleParallelPlannerWithSAT.h"
#include "Logger.h"
#include "Settings.h"

#include "ipasir_cpp.h"



SimpleParallelPlannerWithSAT::SimpleParallelPlannerWithSAT(IPlanningProblem *problem) {
    this->problem = problem;
    problemSolved = false;
    lastFailedLayer = 0;
    horizonOffset = 0;

    // Create thread pool with 1 tag (0)
    int threadCount = settings->getThreadCount();
    threadPool = new SATSolverThreadPool(1);
    threadPool->addWorkers(0, threadCount, createSATSolver, this);

    // Get action and proposition count from problem data
    countActions = problem->getActionCount();
    countPropositions = problem->getPropositionCount();
}

SimpleParallelPlannerWithSAT::~SimpleParallelPlannerWithSAT() {
    delete threadPool;
}

int SimpleParallelPlannerWithSAT::graphplan(Plan& plan) {
    log(0, "SPPSAT algorithm using SAT Solver %s\n", ipasir_signature());

    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable()) {
        expand();
        fixedPoint = fixedPoint || checkFixedPoint();
        horizonOffset++;
    }

    // If goal is impossible to reach, this problem has no solution
    if (checkGoalUnreachable()) {
        log(1, "Goal unreachable, aborting\n");
        return false;
    }

    // Inverted horizon, used to determine how far to expand before "going idle"
    std::map<int, int> horizonInv;
    int iteration = 0;

    while (!problemSolved) {
        // Determine if graph shall be expanded
        bool doExpand = false;
        {
            // Calculation depends on the layer of the last failed extraction
            std::unique_lock<std::mutex> lck(lastFailedLayerMutex);
            // Expand far enough (so that each thread has something to work with)
            if (iteration <= horizonInv[lastFailedLayer]+settings->getThreadCount()) {
                doExpand = true;
            }
        }

        if (doExpand) {
            // Extract at horizon level
            int extractionLayer = horizon(iteration);
            horizonInv[extractionLayer] = iteration;

            // Queue an extraction job to the pool
            ThreadParameters *tp = new ThreadParameters();
            tp->planner = this;
            tp->layer = extractionLayer;
            SATSolverThreadPool::Job j;
            j.func = extractionThread;
            j.arguments = (void*)tp;
            threadPool->enqueueJob(0, j);

            // Expand the graph to the horizon
            while (problem->getLastActionLayer() < horizon(iteration+1)) {
                expand();
                fixedPoint = fixedPoint || checkFixedPoint();
            }

            iteration++;
        } else {
            sleep(1);
        }
    }

    plan = solution;

    return true;
}

// Initializes one SAT solver for one thread
void* SimpleParallelPlannerWithSAT::createSATSolver(void *args) {
	void *solver = ipasir_init();

    #ifndef PGP_NOSETLEARN
	// Set clause learning callback
    ipasir_set_learn(solver, NULL, 0, NULL);
    #endif

    return solver;
}

// Each extraction thread is running this function which extracts a plan
// starting from a given layer.
// args has to be of type SimpleParallelPlannerWithSAT::ThreadParameters
void* SimpleParallelPlannerWithSAT::extractionThread(void* solver, void* args) {
    // Get parameters for this thread
    ThreadParameters *param = (ThreadParameters*) args;
    SimpleParallelPlannerWithSAT *planner = param->planner;
    int layer = param->layer;

    // If problem has been solved in the meantime, abort prematurely
    if (planner->problemSolved) {
        delete param;
        return NULL;
    }

    // Get the last layer of clauses that have been added to the solver
    int lastLayer = 1;
    if (planner->solversLastLayer.count(solver) == 0) {
        planner->solversLastLayer[solver] = 1;
    } else {
        lastLayer = planner->solversLastLayer[solver];
    }

    // Set termination callback for SAT solver, including information of the
    // current extraction run, e.g. which layer.
	ipasir_set_terminate(solver, args, solverTerminator);

    // Add necessary clauses to this thread's SAT solver
    for (int i = lastLayer; i <= layer; i++) {
        planner->addClausesToSolver(solver, i);
        // If problem has been solved in the meantime, abort prematurely
        if (planner->problemSolved) {
            delete param;
            return NULL;
        }
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
        // Mark the problem as solved so planner can terminate
        planner->markProblemSolved(plan);
    } else {
        // Update last failed layer so other threads can terminate that work
        // on extraction from a lower layer
        std::unique_lock<std::mutex> lck(planner->lastFailedLayerMutex);
        if (planner->lastFailedLayer < layer) {
            planner->lastFailedLayer = layer;
        }
    }

    delete param;
	return NULL;
}


// Can be called by a thread to provide a solution to the planning problem.
void SimpleParallelPlannerWithSAT::markProblemSolved(Plan plan) {
    std::unique_lock<std::mutex> lck(solvedMutex);
    if (!problemSolved) {
        problemSolved = true;
        solution = plan;
    }
}

// This function is called by the SAT solvers from different threads
// to determine if they should abort solving of the formula.
// A non-zero value indicates that the solver should abort (in accordance with
// the ipasir definition).
int SimpleParallelPlannerWithSAT::solverTerminator(void* state) {
    // Get arguments (used planner and layer)
    auto *p = (SimpleParallelPlannerWithSAT::ThreadParameters*) state;
    int layer = p->layer;
    auto *planner = p->planner;
    // If problem was solved, or extraction failed at a higher layer, terminate
    int t = planner->problemSolved || planner->lastFailedLayer >= layer;
	return t;
}

void SimpleParallelPlannerWithSAT::expand() {
    // Get lock and then expand graph
    std::unique_lock<std::mutex> lck(graphMutex);
    Planner::expand();
}

void SimpleParallelPlannerWithSAT::addClausesToSolver(void *solver, int actionLayer) {
    // Get lock and then add clauses
    std::unique_lock<std::mutex> lck(graphMutex);
    PlannerWithSATExtraction::addClausesToSolver(solver, actionLayer);
}

