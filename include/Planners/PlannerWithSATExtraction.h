#ifndef _PLANNER_SATEX_H
#define _PLANNER_SATEX_H

#include <vector>
#include <list>

#include "common.h"
#include "IPlanningProblem.h"
#include "Planners/Planner.h"


#define IPASIR_INTERRUPT 0
#define IPASIR_IS_SAT 10
#define IPASIR_IS_UNSAT 20


/**
 * Planner class that implements the Graphplan algorithm using SAT Solving for
 * the extraction step.
 *
 * Author: Patrick Hegemann
 */
class PlannerWithSATExtraction : public Planner {
    public:
        PlannerWithSATExtraction() {}
        PlannerWithSATExtraction(IPlanningProblem *problem);
        ~PlannerWithSATExtraction();
        int graphplan(Plan& plan);

    protected:
        int countPropositions;
        int countActions;

        void expand();
        void addClausesToSolver(void *solver, int actionLayer);
        int extract(void *solver, std::list<Proposition> goal, int layer, Plan& plan);
        
        int propositionAtLayer(Proposition p, int layer);
        int actionAtLayer(Action a, int layer);

    private:
        bool solverInitialized = false;
        void *solver;
};

#endif

