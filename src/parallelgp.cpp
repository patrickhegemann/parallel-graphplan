// Includes for data structure
#include <list>
#include <vector>

// Includes for I/O
#include <iostream>
#include <fstream>

// Other includes
#include <string>
#include <list>

#include "parallelgp.h"

#include "problem.h"
#include "parser.h"
#include "planner.h"
#include "planVerifier.h"
#include "Settings.h"
#include "Logger.h"


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
    Problem *problem = parser.parse(settings.getInputFile());
    log(1, "Parsing done\n");

    // Find a plan, then verify and print it
    std::list<std::list<int>> plan;
    if (findPlan(problem, plan)) {
        printPlan(problem, plan);
        verifyPlan(problem, plan);
    }

    return 0;
}

int findPlan(Problem *problem, std::list<std::list<int>>& plan) {
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

int verifyPlan(Problem *problem, std::list<std::list<int>> plan) {
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

void printPlan(Problem *problem, std::list<std::list<int>> plan) {
    // Output plan (short version)
    int layerNumber = 0;
    int step = 0;
    log(0, "LAYER\tSTEP\tACTION\n");
    for (auto layer : plan) {
        for (auto action : layer) {
            if (action >= problem->countPropositions) {
                log(0, "%d\t%d\t%s\n", layerNumber, step, problem->actionNames[action].c_str());
                step++;
            }
        }
        layerNumber++;
    }
}

