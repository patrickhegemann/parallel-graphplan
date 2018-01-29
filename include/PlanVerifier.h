#ifndef _PLANVERIFIER_H
#define _PLANVERIFIER_H

#include <set>

#include "IPlanningProblem.h"
#include "Plan.h"
#include "common.h"


class PlanVerifier {
    public:
        PlanVerifier(IPlanningProblem *problem, Plan plan);
        int verify();
        int simulateStep(std::list<Action> step);
        int checkGoal();

    private:
        IPlanningProblem *problem;
        Plan plan;
        std::set<Proposition> state;
};

#endif
