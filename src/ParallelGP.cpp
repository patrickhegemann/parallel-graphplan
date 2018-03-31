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
#include "PlanVerifier.h"
#include "Settings.h"
#include "Logger.h"

#include "Planners/PlannerWithSATExtraction.h"
#include "Planners/Planner.h"

#include "ParallelGP.h"


Settings* settings;

int main(int argc, char *argv[]) {
    // Get command line arguments
    settings = new Settings(argc, (char**)argv);

    setVerbosityLevel(settings->getVerbosityLevel());
    if (settings->getInputFile() == nullptr) {
        exitError("No input file given\n");
    }

    // Parse input file
    log(0, "Parsing...\n");
    SASParser parser;

    // Switch between data structures here later
    PlanningProblem::Builder builder;
    parser.setProblemBuilder(&builder);

    IPlanningProblem *problem = parser.parse(settings->getInputFile());
    log(0, "Parsing done\n");

    // Find a plan, then verify and print it
    Plan plan;
    if (findPlan(problem, plan)) {
        printPlan(problem, plan);
        verifyPlan(problem, plan);
    }

    return 0;
}

int findPlan(IPlanningProblem *problem, Plan& plan) {
    log(0, "Searching plan...\n");

    // Call planner
    Planner *planner = nullptr;
    std::string plannerName = settings->getPlannerName();
    if (plannerName == "standard") {
        planner = new Planner(problem);
    } else if (plannerName == "satex") {
        planner = new PlannerWithSATExtraction(problem);
    }
    
    int success = 0;

    if (planner) {
        success = planner->graphplan(plan);
    } else {
        exitError("Invalid planner: %s\n", plannerName.c_str());
    }

    delete planner;

    // No plan
    if (!success) {
        log(0, "No plan found\n");
        return 0;
    }

    // Plan found
    log(0, "Plan found!\n");
    return 1;
}

int verifyPlan(IPlanningProblem *problem, Plan plan) {
    log(0, "Verifying plan...\n");

    PlanVerifier ver(problem, plan);
    int planValid = ver.verify();

    if (planValid) {
        log(0, "Plan OK\n");
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
                log(0, "%d\t%d\t%s\n", layerNumber+1, step+1, problem->getActionName(action).c_str());
                step++;
            }
        }
    }
}

