#ifndef PARALLELGP_H
#define PARALLELGP_H

#include <list>
#include "problem.h"

int findPlan(Problem *problem, std::list<std::list<int>>& plan);
int verifyPlan(Problem *problem, std::list<std::list<int>> plan);
void printPlan(Problem *problem, std::list<std::list<int>> plan);

#endif

