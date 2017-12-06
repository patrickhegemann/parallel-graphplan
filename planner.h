#ifndef PLANNER_H
#define PLANNER_H

#include <vector>
#include <list>

#include "problem.h"


int isNogood(int layer, std::list<int> props);
void addNogood(int layer, std::list<int> props);
int fixedPoint(Problem *problem);

int graphplan(Problem *problem, std::list<std::list<int>>& plan);
void expand(Problem *problem);
int extract(Problem *problem, std::list<int> goal, int layer, std::list<std::list<int>>& plan);
int gpSearch(Problem *problem, std::list<int> goal, std::list<int> actions, int layer, std::list<std::list<int>>& plan);

#endif
