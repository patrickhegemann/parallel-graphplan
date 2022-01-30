#include <iostream>
#include <iterator>
#include <algorithm>
#include <assert.h>
#include <map>

#include <unistd.h>
#include <thread>

#include "Planners/LPEPEPlanner.h"
#include "Logger.h"
#include "Settings.h"
#include "common.h"

#ifdef IPASIRCPP
#include "ipasir.h"
#else
extern "C" {
    #include "ipasir_cpp.h"
}
#endif



LPEPEPlanner::LPEPEPlanner(IPlanningProblem *problem) {
    this->problem = problem;
    problemSolved = false;

    // Create thread pool with 1 tag (0)
    int threadCount = settings->getThreadCount();
    threadPool = new SATPriorityThreadPool(1);
    threadPool->addWorkers(0, threadCount, createSATSolver, this);

    // Get action and proposition count from problem data
    countActions = problem->getActionCount();
    countPropositions = problem->getPropositionCount();
}

LPEPEPlanner::~LPEPEPlanner() {
    delete threadPool;
}

int LPEPEPlanner::graphplan(Plan& plan) {
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
            SATPriorityThreadPool::Job j;
            j.func = extractionThread;
            j.arguments = (void*)tp;
            threadPool->enqueueJob(extractionLayer, 0, j);

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
void* LPEPEPlanner::createSATSolver(void *args) {
	void *solver = ipasir_init();

    #ifndef PGP_NOSETLEARN
	// Set clause learning callback
    ipasir_set_learn(solver, NULL, 0, NULL);
	ipasir_set_terminate(solver, args, solverTerminator);
    #endif

    return solver;
}

// Each extraction thread is running this function which extracts a plan
// starting from a given layer.
// args has to be of type LPEPEPlanner::ThreadParameters
void* LPEPEPlanner::extractionThread(void* solver, void* args) {
    // Get parameters for this thread
    ThreadParameters *param = (ThreadParameters*) args;
    LPEPEPlanner *planner = param->planner;
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

    // Add necessary clauses to this thread's SAT solver
    for (int i = lastLayer; i <= layer; i++) {
        // TODO: only add new mutexes planner->addClausesToSolver(solver, i);
        // If problem has been solved in the meantime, abort prematurely
        if (planner->problemSolved) {
            delete param;
            return NULL;
        }
    }

    planner->solversLastLayer[solver] = layer;

    // Set termination callback for SAT solver, including information of the
    // current extraction run, e.g. which layer.
	ipasir_set_terminate(solver, args, solverTerminator);

    // Extract plan
    Plan plan;
    int success = planner->extract(solver,
            planner->problem->getGoal(),
            planner->problem->getPropLayerAfterActionLayer(layer),
            plan);

    if (success) {
        // Mark the problem as solved so planner can terminate
        planner->markProblemSolved(plan);
    }

    delete param;
	return NULL;
}


// Can be called by a thread to provide a solution to the planning problem.
void LPEPEPlanner::markProblemSolved(Plan plan) {
    std::unique_lock<std::mutex> lck(solvedMutex);
    if (!problemSolved) {
        problemSolved = true;
        solution = plan;
    }
}

// This function is called by the SAT solvers from different threads
// to determine if they should abort solving of the formula.
// A non-zero value indicates that the solver should abort.
int LPEPEPlanner::solverTerminator(void* state) {
    // Get arguments (planner)
    auto *planner = (LPEPEPlanner*) state;
    // If problem was solved, or extraction failed at a higher layer, terminate
    int t = planner->problemSolved;
	return t;
}

void LPEPEPlanner::expand() {
    // Get lock and then expand graph
    std::unique_lock<std::mutex> lck(graphMutex);
    Planner::expand();
}

void LPEPEPlanner::addClausesToSolver(void *solver, int actionLayer) {
    // Get lock and then add clauses
    std::unique_lock<std::mutex> lck(graphMutex);
    PlannerWithSATExtraction::addClausesToSolver(solver, actionLayer);
}

void LPEPEPlanner::addNewMutexesToSolver(void *solver, int actionLayer) {

}


int LPEPEPlanner::extract(void* solver, std::list<Proposition> goal, int layer, Plan& plan) {
    log(0, "Extracting in layer %d with LPEPE\n", layer);

    int packSize = settings->getLayerPackSize();
    // Layer at which the goals will be assumed
    int goalPackLayer = ((layer - 1) % packSize) + 1;

    // Assume that the goal is true in this layer
    for (Proposition p : goal) {
        ipasir_assume(solver, propositionAtLayer(p, goalPackLayer));
    }

    if (ipasir_solve(solver) == IPASIR_IS_SAT) {
        for (int i = problem->getFirstActionLayer(); i <= problem->getActionLayerBeforePropLayer(layer); i++) {
            std::list<Action> actions;
            for (Action a : problem->getLayerActions(i)) {
                int lit = actionAtLayer(a, i);
                if (ipasir_val(solver, lit) == lit) {
                    actions.push_back(a);
                }
            }
            plan.addLayer(actions);
        }
        log (0, "Done extracting: success\n");
        return 1;
    } else {
        log (0, "Done extracting: failure/terminated\n");
        return 0;
    }
}

