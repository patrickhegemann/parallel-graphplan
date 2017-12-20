// Includes for data structure
#include <list>
#include <vector>

// Includes for I/O
#include <iostream>
#include <fstream>

// Other includes
#include <string>
#include <list>


#include "problem.h"
#include "parser.h"
#include "planner.h"
#include "planVerifier.h"



int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "No input .sas file given\n";
        exit(1);
    }

    // Parse input file
    
    std::cout << "Parsing ... ";
    
    char *inputFile = argv[1];
    SASParser *parser = new SASParser();
    Problem *problem = parser->parse(inputFile);
    delete parser;

    std::cout << "DONE" << std::endl;

    // ========================================================================

    std::cout << "Searching plan ... ";

    // Call planner
    Planner *planner = new Planner(problem);
    std::list<std::list<int>> plan;
    int success = planner->graphplan(plan);

    // No plan
    if (!success) {
        std::cout << "No plan found" << std::endl;
        return 0;
    }

    std::cout << "Plan found!" << std::endl;

    delete planner;

    // ========================================================================

    /*
    for (int i : problem->lastActionIndices) {
        std::cout << i << "a ";
    }
    std::cout << std::endl;

    for (int i : problem->lastPropIndices) {
        std::cout << i << "p ";
    }
    std::cout << std::endl;
    */

    // Output plan (long version)
    /*
    int layerNumber = 1;
    for (auto const& layer : plan) {
        std::cout << "Actions in layer " << layerNumber << ":" << std::endl;
        for (auto const& action : layer) {
            std::cout << "\t" << problem->actionNames[action] << std::endl;
            
            */
            /*
            std::cout << "\t\t";
            for (int k = problem->actionPrecIndices[action];
                    k < problem->actionPrecIndices[action+1]; k++) {
                std::cout << "." << problem->propNames[problem->actionPrecEdges[k]] << " ";
            }
            std::cout << std::endl;

            std::cout << "\t\t";
            for (int k = problem->actionPosEffIndices[action];
                    k < problem->actionPosEffIndices[action+1]; k++) {
                std::cout << "+" << problem->propNames[problem->actionPosEffEdges[k]] << " ";
            }
            std::cout << std::endl;

            std::cout << "\t\t";
            for (int k = problem->actionNegEffIndices[action];
                    k < problem->actionNegEffIndices[action+1]; k++) {
                std::cout << "-" << problem->propNames[problem->actionNegEffEdges[k]] << " ";
            }
            std::cout << std::endl;
            */
            /*
        }
        std::cout << "--------------------" << std::endl;
        layerNumber++;
    }
    */

    // ========================================================================

    // Output plan (short version)
    int step = 0;
    std::cout << std::endl << "step";
    for (auto layer : plan) {
        for (auto action : layer) {
            if (action >= problem->countPropositions) {
                std::cout << "\t" << step << " : " << problem->actionNames[action] << std::endl;
                step++;
            }
        }
    }
    std::cout << std::endl;

    // ========================================================================

    std::cout << "Verifying plan ... ";
    PlanVerifier ver(problem, plan);
    int planValid = ver.verify();

    if (planValid) {
        std::cout << "Plan OK";
    } else {
        std::cout << "Plan INVALID";
    }
    std::cout << std::endl;

    return 0;
}

