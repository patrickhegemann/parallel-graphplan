#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include <list>

#include "problem.h"


int plan(Problem *problem, std::list<std::list<int>>& plan);
void expand(Problem *problem);

int extract(Problem *problem, std::vector<int> goal, int layer, std::list<std::list<int>>& plan);

int gpSearch(Problem *problem, std::vector<int> goal, std::vector<int> actions, int layer, std::list<std::list<int>>& plan);

#endif
