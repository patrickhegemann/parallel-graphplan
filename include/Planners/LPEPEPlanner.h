#ifndef _LPEPE_PLANNER
#define _LPEPE_PLANNER

#include <vector>
#include <list>
#include <mutex>
#include <map>

#include "common.h"
#include "IPlanningProblem.h"
#include "Planners/Planner.h"
#include "Planners/PlannerWithSATExtraction.h"
#include "SATPriorityThreadPool.h"



class LPEPEPlanner : public PlannerWithSATExtraction {
    public:
        LPEPEPlanner(IPlanningProblem *problem);
        ~LPEPEPlanner();
        int graphplan(Plan& plan);

        static void* createSATSolver(void *args);
        static int solverTerminator(void* state);
        static void* extractionThread(void* solver, void *args);

    protected:
        // A thread pool
        SATPriorityThreadPool *threadPool;

        // Indicates which layers have been added to a solver
        std::map<void*, int> solversLastLayer;

        void expand();
        void addClausesToSolver(void *solver, int actionLayer);
        void addNewMutexesToSolver(void *solver, int actionLayer);
        int extract(void* solver, std::list<Proposition> goal, int layer, Plan& plan);

    private:
        // Struct that is given as a parameter to each thread
        struct ThreadParameters {
            LPEPEPlanner *planner;
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

        // Mutex for the problem/planning graph
        std::mutex graphMutex;
};


#endif

