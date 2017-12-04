#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <climits>


#include "parser.h"
#include "problem.h"


// Define the strings that mark sections in the SAS file
#define VERSION_HEADER "begin_version"
#define VERSION_FOOTER "end_version"
#define METRIC_HEADER "begin_metric"
#define METRIC_FOOTER "end_metric"
#define VARIABLE_HEADER "begin_variable"
#define VARIABLE_FOOTER "end_variable"
#define MUTEX_HEADER "begin_mutex_group"
#define MUTEX_FOOTER "end_mutex_group"
#define INITIALSTATE_HEADER "begin_state"
#define INITIALSTATE_FOOTER "end_state"
#define GOALSTATE_HEADER "begin_goal"
#define GOALSTATE_FOOTER "end_goal"
#define OPERATION_HEADER "begin_operator"
#define OPERATION_FOOTER "end_operator"
#define AXIOM_HEADER "begin_rule"
#define AXIOM_FOOTER "end_rule"

#define MAGIC_ERROR -1000


/**
 * Constructor
 */
SASParser::SASParser() {}


/**
 * Parse a file and output problem struct
 */
Problem* SASParser::parse(char *filename) {
    // Initialize Problem struct
    problem = new Problem();

    // TODO: maybe modify this so it takes a stringstream
    // Prepare file stream
    file.open(filename);
    if (!file.is_open()) {
        std::cout << "Error: SAS file could not be opened" << std::endl;
        return nullptr;
    }

    // Start parsing!
    nextLine();
    
    // Parse all sections
    version();
    metric();
    variables();
    mutexes();
    initialstate();
    goalstate();
    operators();
    //axioms();

    std::cout << "Done parsing!" << std::endl;

    file.close();

    return problem;
}


/**
 * Reads next line for parsing
 */
void SASParser::nextLine() {
    if (!std::getline(file, curLine)) {
        // error handling
    }
}

/**
 * Outputs errors
 */
void SASParser::error(std::string err) {
    std::cout << "Parser Error: " << err << std::endl;
}

// ----------------------------------------------------------------------------

/**
 * Checks whether the current line matches the given line.
 * Invokes nextLine() in that case.
 */
int SASParser::accept(std::string line) {
    if (curLine == line) {
        nextLine();
        return 1;
    }
    return 0;
}

/**
 * Same as accept but throws an error if expected token is not given
 */
int SASParser::expect(std::string line) {
    if (accept(line)) {
        return 1;
    }
    std::string msg = "Expected " + line + ", got " + curLine;
    error(msg);
    return 0;
}

// ----------------------------------------------------------------------------

/**
 * Converts the current line into an integer and returns it.
 * Invokes nextLine().
 */
int SASParser::acceptAnyInt() {
    int number = stoi(curLine);
    nextLine();
    return number;
}

/**
 * Returns the current line and invokes nextLine().
 */
std::string SASParser::acceptAnyLine() {
    std::string line = curLine;
    nextLine();
    return line;
}

/**
 * Initializes the line tokenizer for reading tokens and invokes nextToken()
 * and nextLine().
 */
void SASParser::acceptTokenLine() {
    lineStream = std::stringstream(curLine);
    nextToken();
    nextLine();
}

// ----------------------------------------------------------------------------

/**
 * Reads next token
 */
void SASParser::nextToken() {
    lineStream >> curToken;
}

/**
 * Converts the current token to an integer and returns it.
 * Invokes nextToken().
 */
int SASParser::acceptAnyIntToken() {
    int number = std::stoi(curToken);
    nextToken();
    return number;
}

/**
 * Converts current token to integer and checks if its in the given range,
 * including min and max. In that case, invokes nextToken() and return the
 * number.
 * Returns MAGIC_ERROR otherwise.
 */
int SASParser::acceptIntTokenRange(int min, int max) {
    int number = std::stoi(curToken);
    if (min <= number && number <= max) {
        nextToken();
        return number;
    }
    return MAGIC_ERROR;
}

/**
 * Same as acceptIntTokenRange, but outputs an error if the integer is not
 * in the range.
 */
int SASParser::expectIntTokenRange(int min, int max) {
    int number = acceptIntTokenRange(min, max);
    if (number != MAGIC_ERROR) {
        return number;
    }

    std::string msg = "Expected int between " + std::to_string(min) + " and ";
    msg += std::to_string(max) + " inclusively, got " + curToken;
    error(msg);
    return MAGIC_ERROR;
}

/**
 * Expects a pair of variable and its value as tokens, both may not be out of bounds.
 * Returns 0 in the error case, 1 otherwise.
 */
int SASParser::expectVarValuePair(int *variable, int *value) {
    *variable = expectIntTokenRange(0, countVariables-1);
    int range = varFirstProps[*variable+1] - varFirstProps[*variable];
    *value = expectIntTokenRange(0, range-1);

    if (*variable == MAGIC_ERROR || *value == MAGIC_ERROR) {
        // Error case
        return 0;
    }
    return 1;
}



// ----------------------------------------------------------------------------

/**
 * Parses the version section of a SAS file
 */
void SASParser::version() {
    std::cout << "Parsing version section ..." << std::endl;
    expect(VERSION_HEADER);
    expect("3");
    expect(VERSION_FOOTER);
}

/**
 * Parses the metric section
 */
void SASParser::metric() {
    std::cout << "Parsing metric section ..." << std::endl;
    expect(METRIC_HEADER);

    // If there are action costs, inform that we are going to ignore them
    if (!accept("0")) {
        acceptAnyLine();
        std::cout << "WARNING: Problem has action costs, but they will be ignored." << std::endl;
    }

    expect(METRIC_FOOTER);
}

/**
 * Parses the variables section
 */
void SASParser::variables() {
    std::cout << "Parsing variables section ..." << std::endl;
    
    countVariables = acceptAnyInt();
    varFirstProps.reserve(countVariables+1);
    std::cout << "There are " << countVariables << " finite domain variables" << std::endl;

    // Initialize problem
    problem->countPropositions = 0;

    for (int i = 0; i < countVariables; i++) {
        expect(VARIABLE_HEADER);

        // variable name; we don't need to store that since it's mostly useless
        acceptAnyLine();

        // axiom layer; we're not concerned with that for now
        acceptAnyLine();

        // Now it gets interesting: range of the variable
        int range = acceptAnyInt();
        varFirstProps[i] = problem->countPropositions;
        //varRanges[i] = acceptAnyInt();

        //int oldPropCount = problem->countPropositions;
        problem->countPropositions += range;
        problem->propGroups.reserve(problem->countPropositions);

        // proposition names; maybe later for debugging
        for (int j = varFirstProps[i]; j < varFirstProps[i] + range; j++) {
            acceptAnyLine();
            // We can also use this loop to fill the propGroup vector
            problem->propGroups[j] = i;
        }

        expect(VARIABLE_FOOTER);
    }

    // Additional element in the varFirstProps vector, so we can simplify the
    // calculation of the actual size of the value range of each variable
    // during the parsing
    varFirstProps[countVariables] = problem->countPropositions;
}


/**
 * Parses the mutex section
 */
void SASParser::mutexes() {
    std::cout << "Parsing mutex section ..." << std::endl;
    
    int numMutexes = acceptAnyInt();
    problem->propMutexes = new int[problem->countPropositions*problem->countPropositions];

    std::cout << "There are " << numMutexes << " mutex groups" << std::endl;

    for (int i = 0; i < numMutexes; i++) {
        expect(MUTEX_HEADER);
        
        // number of facts in the mutex group
        int numFacts = acceptAnyInt();
        int *facts = new int[numFacts];

        // Read all the facts of the group that are mutex
        for (int j = 0; j < numFacts; j++) {
            // Tokenize this line and read individual tokens
            acceptTokenLine();
            int variable, value;
            expectVarValuePair(&variable, &value);

            // Translate variable and value to proposition number
            int prop = varFirstProps[variable] + value;
            facts[j] = prop;

            // Set these propositions mutex to each other in *every* layer
            for (int k = 0; k < j; k++) {
                setPropMutex(problem, prop, facts[k], INT_MAX);
            }
        }

        delete[] facts;

        expect(MUTEX_FOOTER);
    }
    
}


/**
 * Parses the initial state section
 */
void SASParser::initialstate() {
    std::cout << "Parsing initial state section ..." << std::endl;

    // Initialize problem vectors
    //problem->propEnabled = std::vector<char>(problem->countPropositions, 0);
    problem->propEnabled.resize(problem->countPropositions, 0);
    //.reserve(problem->countPropositions);
    problem->layerProps.reserve(problem->countPropositions);
    
    problem->lastPropIndices.push_back(problem->countPropositions);


    expect(INITIALSTATE_HEADER);
    
    // Parse initial variable states
    for (int i = 0; i < countVariables; i++) {
        int value = acceptAnyInt();

        // Translate variable and value to proposition number
        int prop = varFirstProps[i] + value;
        
        // Update problem:
        // Enable the specified proposition
        problem->propEnabled[prop] = 1;
        // Put the proposition in the next spot of the layerProps array,
        // indicating that the proposition exists now in the planning graph
        problem->layerProps[i] = prop;
    }

    expect(INITIALSTATE_FOOTER);
}


/**
 * Parses the goal state section
 */
void SASParser::goalstate() {
    std::cout << "Parsing goal state section ..." << std::endl;

    expect(GOALSTATE_HEADER);
    
    int countGoalProps = acceptAnyInt();
    problem->goalPropositions.reserve(countGoalProps);
    
    for (int i = 0; i < countGoalProps; i++) {
        // Tokenize this line and read individual tokens
        acceptTokenLine();
        int variable, value;
        expectVarValuePair(&variable, &value);

        // Translate variable and value to proposition number
        int prop = varFirstProps[variable] + value;
        std::cout << "goal prop: " << prop << " (variable " << variable << " has value " << value << ")" << std::endl;

        problem->goalPropositions[i] = prop;
    }
    
    expect(GOALSTATE_FOOTER);
}


/**
 * Parse the operator section
 */
void SASParser::operators() {
    std::cout << "Parsing operator section ..." << std::endl;

    int countOperators = acceptAnyInt();
    problem->countActions = countOperators;

    // Initialize problem vectors
    problem->actionPrecIndices.reserve(countOperators);
    problem->actionPosEffIndices.reserve(countOperators);
    problem->actionNegEffIndices.reserve(countOperators);
    //problem->propPosActIndices.reserve(problem->countPropositions);
    problem->propPosActions.resize(problem->countPropositions);
    problem->actionEnabled.resize(countOperators, 0);
    problem->layerActions.reserve(countOperators);
    problem->actionNames.reserve(countOperators);

    for (int i = 0; i < countOperators; i++) {
        singleOperator(i);
    }
}

/**
 * Parse an operator of the operators section
 */
void SASParser::singleOperator(int operatorNumber) {
    expect(OPERATION_HEADER);

    // Get operator name
    problem->actionNames[operatorNumber] = acceptAnyLine();

    // Update basic graph structure of problem
    problem->actionPrecIndices[operatorNumber] = problem->actionPrecEdges.size();
    problem->actionPosEffIndices[operatorNumber] = problem->actionPosEffEdges.size();
    problem->actionNegEffIndices[operatorNumber] = problem->actionNegEffEdges.size();
    /*
    std::cout << "operator \"" << problem->actionNames[operatorNumber];
    std::cout << "\" negeff index " << problem->actionPosEffIndices[operatorNumber];
    std::cout << std::endl;
    */

    // Prevail conditions
    int countPrevailConditions = acceptAnyInt();
    for (int j = 0; j < countPrevailConditions; j++) {
        acceptTokenLine();
        int variable, value;
        expectVarValuePair(&variable, &value);

        // Translate variable and value to proposition number
        int prop = varFirstProps[variable] + value;
        // Add prevail conditions as precondition and positive effect!
        problem->actionPrecEdges.push_back(prop);
        problem->actionPosEffEdges.push_back(prop);
        // Also register the positive effect for the proposition
        problem->propPosActions[prop].push_back(operatorNumber);
    }

    // Effects
    int countEffects = acceptAnyInt();
    for (int j = 0; j < countEffects; j++) {
        acceptTokenLine();
        operatorEffect(operatorNumber);
    }

    // Operator cost (we don't expect this to play a role, since we only expect
    // "metric" problems)
    //acceptAnyInt();
    acceptAnyLine();

    expect(OPERATION_FOOTER);
}

/**
 * Parse an effect of an operator
 */
void SASParser::operatorEffect(int operatorNumber) {
    int countEffConditions = acceptAnyIntToken();

    // For STRIPS, there are usually no effect conditions
    if (countEffConditions != 0) {
        std::cout << "WARNING: amount of effect conditions is not zero!" << std::endl;
    }

    for (int i = 0; i < countEffConditions; i++) {
        int variable, value;
        expectVarValuePair(&variable, &value);
        // Since we usually don't have this case, nothing useful happens here

        // Translate variable and value to proposition number
        // int prop = varFirstProps[variable] + value;
    }

    // Get affected variable, "precondition" and "effect"
    int variable = expectIntTokenRange(0, countVariables-1);
    int range = varFirstProps[variable+1] - varFirstProps[variable];
    int preValue = expectIntTokenRange(-1, range-1);
    int postValue = expectIntTokenRange(0, range-1);

    // If the effect needs the variable to have a certain value, then
    // add that as both a precondition and a negative effect
    if (preValue != -1) {
        // Translate variable and value to proposition number
        int preProp = varFirstProps[variable] + preValue;
        problem->actionPrecEdges.push_back(preProp);
        problem->actionNegEffEdges.push_back(preProp);
    }

    // Add positive effect for both action and variable
    int postProp = varFirstProps[variable] + postValue;
    problem->actionPosEffEdges.push_back(postProp);
    problem->propPosActions[postProp].push_back(operatorNumber);
    
    /*
    std::cout << "xyz " << postProp << " ab " << problem->propPosActions[postProp].size();
    std::cout << std::endl;
    */

    /*
    std::cout << "Operator " << problem->actionNames[operatorNumber] << " sets ";
    std::cout << "variable " << variable << " from " << preValue << " to " << postValue;
    std::cout << std::endl;
    */
}


/**
 * Parse the axiom section
 */
void SASParser::axioms() {
    // We don't expect the inputs to have axioms...
}

