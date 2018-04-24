#ifndef _SIMPLE_PARALLEL_PLANNER_SAT
#define _SIMPLE_PARALLEL_PLANNER_SAT

#include <vector>
#include <list>
#include <mutex>
#include <map>

#include "common.h"
#include "IPlanningProblem.h"
#include "Planners/Planner.h"
#include "Planners/PlannerWithSATExtraction.h"
#include "SATSolverThreadPool.h"



/**
 * Planner class that implements the Graphplan algorithm in parallel, using SAT
 * Solving for the extraction step.
 *
 * Author: Patrick Hegemann
 */
class SimpleParallelPlannerWithSAT : public PlannerWithSATExtraction {
    public:
        SimpleParallelPlannerWithSAT(IPlanningProblem *problem);
        ~SimpleParallelPlannerWithSAT();
        int graphplan(Plan& plan);

        static void* createSATSolver(void *args);
        static int solverTerminator(void* state);
        static void* extractionThread(void* solver, void *args);

    protected:
        // A thread pool
        SATSolverThreadPool *threadPool;

        // Indicates which layers have been added to a solver
        std::map<void*, int> solversLastLayer;

        void expand();
        void addClausesToSolver(void *solver, int actionLayer);
        //int extract(void* solver, std::list<Proposition> goal, int layer, Plan& plan);

    private:
        // Struct that is given as a parameter to each thread
        struct ThreadParameters {
            SimpleParallelPlannerWithSAT *planner;
            int layer;
        };

        // Method for threads to signal that they solved the problem
        void markProblemSolved(Plan plan);
        // A mutex for the plan
        std::mutex solvedMutex;
        // Plan for the solved problem
        Plan solution;
        // Indicates whether the problem has been solved
        bool problemSolved;

        // The last layer where extraction has failed
        int lastFailedLayer;
        // Mutex for lastFailedLayer
        std::mutex lastFailedLayerMutex;

        // Copy of the relevant parts of planning graph for synchronisation purposes
        std::vector<std::list<Proposition>> layerPropsCopy;
        std::vector<std::list<Action>> layerActionsCopy;
        // Mutex for the action and proposition lists
        std::mutex graphCopyMutex;
};


#endif

