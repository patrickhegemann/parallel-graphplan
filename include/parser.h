#ifndef parser_h
#define parser_h

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "problem.h"

/**
 * Recursive descent parser for SAS as output by FastDownward
 */
class SASParser {
    public:
        SASParser();
        Problem* parse(const char *filename);
        
    private:
        // State
        Problem *problem;

        std::ifstream file;
        std::string curLine;    // Current line

        std::stringstream lineStream;
        std::string curToken;   // Current token (if a line contains multiple ints)

        // Ranges of each variable. This is needed to translate variables and
        // their values to propositions
        int countVariables;
        std::vector<int> varFirstProps;

        
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
        void singleOperator(int operatorNumber);
        void operatorEffect(int operatorNumber);

};

#endif
