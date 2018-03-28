#ifndef _PLANNER_SATEX_H
#define _PLANNER_SATEX_H

#include <vector>
#include <list>

#include "common.h"
#include "IPlanningProblem.h"
#include "Planners/Planner.h"



/**
 * Main Planner class that implements the Graphplan algorithm in parallel.
 *
 * Author: Patrick Hegemann
 */
class PlannerWithSATExtraction : public Planner {
    public:
        PlannerWithSATExtraction(IPlanningProblem *problem);
        ~PlannerWithSATExtraction();
        int graphplan(Plan& plan);

    protected:
        void *solver;

        int countPropositions;
        int countActions;

        void expand();
        int extract(std::list<Proposition> goal, int layer, Plan& plan);
        
        int propositionAtLayer(Proposition p, int layer);
        int actionAtLayer(Action a, int layer);
};

#endif

