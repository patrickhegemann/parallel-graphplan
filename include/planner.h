#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include <list>

#include "problem.h"


class Planner {
    public:
        Planner(Problem *problem);
        int graphplan(std::list<std::list<int>>& plan);

    private:
        Problem *problem;

        int fixedPoint();

        void expand();
        int checkPropsMutex(int p, int q, int actionLayer);
        int checkActionsMutex(int a, int b);
        int checkActionPrecsMutex(int a, int b, int layer);
            
        int extract(std::list<int> goal, int layer,
                std::list<std::list<int>>& plan);
        int gpSearch(std::list<int> goal, std::list<int> actions,
                int layer, std::list<std::list<int>>& plan);

        int isNogood(int layer, std::list<int> props);
        void addNogood(int layer, std::list<int> props);
};

#endif
