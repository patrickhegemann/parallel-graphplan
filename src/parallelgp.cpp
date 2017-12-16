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



int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "No input .sas file given\n";
        exit(1);
    }

    // Parse input file
    char *inputFile = argv[1];
    SASParser *parser = new SASParser();
    Problem *problem = parser->parse(inputFile);
    delete parser;

    std::cout << "=========================" << std::endl;

    // Call planner
    Planner *planner = new Planner(problem);
    std::list<std::list<int>> plan;
    int success = planner->graphplan(plan);

    std::cout << "=========================" << std::endl;

    // No plan
    if (!success) {
        std::cout << "No plan found" << std::endl;
        return 0;
    }

    std::cout << "Plan found!" << std::endl;

    // TODO: remove
    std::cout << problem->actionNames[15] << " x " << problem->actionNames[7] << std::endl;
    std::cout << "XX " << problem->actionMutexes[15*problem->countActions + 7] << std::endl;
    std::cout << "XX " << problem->actionMutexes[7*problem->countActions + 15] << std::endl;

    for (int i : problem->lastActionIndices) {
        std::cout << i << "a ";
    }
    std::cout << std::endl;

    for (int i : problem->lastPropIndices) {
        std::cout << i << "p ";
    }
    std::cout << std::endl;

    // Output plan
    int layerNumber = 1;
    for (auto const& layer : plan) {
        std::cout << "Actions in layer " << layerNumber << ":" << std::endl;
        for (auto const& action : layer) {
            std::cout << "\t" << problem->actionNames[action] << std::endl;

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
        }
        std::cout << "--------------------" << std::endl;
        layerNumber++;
    }

    return 0;
}

