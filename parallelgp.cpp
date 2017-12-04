// Includes for data structure
#include <list>
#include <vector>

// Includes for I/O
#include <iostream>
#include <fstream>

// Other includes
#include <string>


#include "parallelgp.h"
#include "problem.h"
#include "parser.h"
#include "planner.h"



int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "No input .sas file given\n";
        exit(1);
    }

    char *inputFile = argv[1];
    std::cout << inputFile << std::endl;

    SASParser *parser = new SASParser();
    Problem *problem = parser->parse(inputFile);
    //parseSAS(inputFile);
    /*
    propGroups.reserve(10);
    propGroups[3] = 4;
    propGroups[4] = 9;
    std::cout << propGroups[3] << " " << propGroups[4] << std::endl;
    */

    return 0;
}


/*
void parseSAS(char *inputFile) {
    std::ifstream file(inputFile);
    std::string line;
    while (std::getline(file, line)) {
        //parseLine(line);
    }
}
*/
