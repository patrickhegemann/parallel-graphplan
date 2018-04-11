#ifndef _PARSER_H 
#define _PARSER_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "PlanningProblem.h"

/**
 * Recursive descent parser for SAS as output by FastDownward translator
 *
 * Author: Patrick Hegemann
 */
class SASParser {
    public:
        SASParser();
        IPlanningProblem* parse(const char *filename);
        void setProblemBuilder(IPlanningProblem::Builder *builder);
        
    private:
        // State
        IPlanningProblem::Builder *problemBuilder;
        int countVariables;
        std::vector<int> variableDomainSizes;
        std::vector<std::string> variableNames;

        // I/O
        std::ifstream file;
        std::string curLine;    // Current line

        std::vector<std::string> lineVector;
        unsigned int currentTokenIndex;
        std::string curToken;   // Current token (if a line contains multiple ints)

        // Basic Parser functions
        void nextLine();
        void error(std::string err);
        
        int accept(std::string line);
        int expect(std::string line);

        // Some extra parser functions
        int acceptAnyInt();
        std::string acceptAnyLine();
        void acceptTokenLine();

        // Token functions
        void nextToken();
        int acceptAnyIntToken();
        int acceptIntTokenRange(int min, int max);
        int expectIntTokenRange(int min, int max);
        int expectVarValuePair(int *variable, int *value);

        // Sections
        void version();
        void metric();
        void variables();
        void mutexes();
        void initialstate();
        void goalstate();
        void operators();
        void axioms();

        // Methods for operator details
        void operatorEffect(int operatorNumber);
};

#endif
