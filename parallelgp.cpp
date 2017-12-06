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
    std::list<std::list<int>> plan;
    int success = graphplan(problem, plan);

    std::cout << "=========================" << std::endl;

    // No plan
    if (!success) {
        std::cout << "No plan found" << std::endl;
        return 0;
    }

    std::cout << "Plan found!" << std::endl;

    // Output plan
    int layerNumber = 1;
    for (auto const& layer : plan) {
        std::cout << "Actions in layer " << layerNumber << ":" << std::endl;
        for (auto const& action : layer) {
            std::cout << problem->actionNames[action] << std::endl;
        }
        std::cout << "--------------------" << std::endl;
        layerNumber++;
    }

    return 0;
}

