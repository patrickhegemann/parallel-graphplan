#ifndef _PARALLELGP_H
#define _PARALLELGP_H

int findPlan(IPlanningProblem *problem, Plan& plan);
int verifyPlan(IPlanningProblem *problem, Plan plan);
void printPlan(IPlanningProblem *problem, Plan plan);

#endif

