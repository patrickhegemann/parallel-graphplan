#ifndef PLANVERIFIER_H
#define PLANVERIFIER_H

#include <set>

#include "problem.h"


class PlanVerifier {
    public:
        PlanVerifier(Problem *problem, std::list<std::list<int>> plan);
        int verify();
        int simulateStep(std::list<int> step);
        int checkGoal();

    private:
        Problem *problem;
        std::list<std::list<int>> plan;
        std::set<int> state;
};

#endif
