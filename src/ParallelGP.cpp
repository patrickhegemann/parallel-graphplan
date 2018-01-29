// Includes for data structure
#include <list>
#include <vector>

// Includes for I/O
#include <iostream>
#include <fstream>

// Other includes
#include <string>
#include <list>

#include "common.h"
#include "Plan.h"
#include "PlanningProblem.h"
#include "Parser.h"
#include "Planner.h"
#include "PlanVerifier.h"
#include "Settings.h"
#include "Logger.h"

#include "ParallelGP.h"


int main(int argc, char *argv[]) {
    // Get command line arguments
    Settings settings(argc, (char**)argv);

    setVerbosityLevel(settings.getVerbosityLevel());
    if (settings.getInputFile() == nullptr) {
        exitError("No input file given\n");
    }

    // Parse input file
    log(1, "Parsing...\n");
    SASParser parser;
    IPlanningProblem *problem = parser.parse(settings.getInputFile());
    log(1, "Parsing done\n");

    // Find a plan, then verify and print it
    Plan plan;
    if (findPlan(problem, plan)) {
        printPlan(problem, plan);
        verifyPlan(problem, plan);
    }

    return 0;
}

int findPlan(IPlanningProblem *problem, Plan& plan) {
    log(1, "Searching plan...\n");

    // Call planner
    Planner planner(problem);
    int success = planner.graphplan(plan);

    // No plan
    if (!success) {
        log(0, "No plan found\n");
        return 0;
    }

    // Plan found
    log(1, "Plan found!\n");
    return 1;
}

int verifyPlan(IPlanningProblem *problem, Plan plan) {
    log(1, "Verifying plan...\n");

    PlanVerifier ver(problem, plan);
    int planValid = ver.verify();

    if (planValid) {
        log(1, "Plan OK\n");
    } else {
        exitError("Plan INVALID\n");
    }

    return planValid;
}

void printPlan(IPlanningProblem *problem, Plan plan) {
    // Output plan (short version)
    int step = 0;
    log(0, "LAYER\tSTEP\tACTION\n");
    for (int layerNumber = 0; layerNumber < plan.getLayerCount(); layerNumber++) {
        std::list<Action> layer = plan.getLayerActions(layerNumber);
        for (Action action : layer) {
            if (!problem->isTrivialAction(action))  {
                log(0, "%d\t%d\t%s\n", layerNumber, step, problem->getActionName(action).c_str());
                step++;
            }
        }
    }
}

