#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <climits>

#include "PlanningProblem.h"
#include "Logger.h"

#include "Parser.h"


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
IPlanningProblem* SASParser::parse(const char *filename) {
    // Initialize Planning Problem Builder
    //PlanningProblem::Builder problemBuilder();

    // TODO: maybe modify this so it takes a stringstream
    // Prepare file stream
    file.open(filename);
    if (!file.is_open()) {
        error("SAS file could not be opened");
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

    file.close();

    return problemBuilder.build();
}


/**
 * Reads next line for parsing
 */
void SASParser::nextLine() {
    if (!std::getline(file, curLine)) {
        error("Unexpected EOF");
    }
}

/**
 * Outputs errors
 */
void SASParser::error(std::string err) {
    exitError("Parser Error: %s\n", err.c_str());
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
    int range = variableDomainSizes[*variable];
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
    log(2, "Parsing version section\n");
    expect(VERSION_HEADER);
    expect("3");
    expect(VERSION_FOOTER);
}

/**
 * Parses the metric section
 */
void SASParser::metric() {
    log(2, "Parsing metric section\n");
    expect(METRIC_HEADER);

    // If there are action costs, inform that we are going to ignore them
    if (!accept("0")) {
        acceptAnyLine();
        log(1, "WARNING: Problem has action costs, but they will be ignored.\n");
    }

    expect(METRIC_FOOTER);
}

/**
 * Parses the variables section
 */
void SASParser::variables() {
    log(2, "Parsing variables section\n");
    
    // Variable count in the problem
    countVariables = acceptAnyInt();
    problemBuilder.setVariableCount(countVariables);

    // Keep track of how big each variable's domain is
    variableDomainSizes.resize(countVariables);

    for (int i = 0; i < countVariables; i++) {
        expect(VARIABLE_HEADER);
        // Variable name (mostly useless, so we will skip it)
        acceptAnyLine();
        // axiom layer; we're not concerned with that for now
        acceptAnyLine();

        // Domain size of the variable
        variableDomainSizes[i] = acceptAnyInt();

        // Proposition names
        for (int j = 0; j < variableDomainSizes[i]; j++) {
            problemBuilder.setPropositionName(Proposition(i, j), acceptAnyLine());
        }

        expect(VARIABLE_FOOTER);
    }
}


/**
 * Parses the mutex section
 */
void SASParser::mutexes() {
    log(2, "Parsing mutex section\n");

    // Amount of predefined mutexes
    int numMutexes = acceptAnyInt();
    log(3, "There are %d mutex groups\n", numMutexes);

    for (int i = 0; i < numMutexes; i++) {
        expect(MUTEX_HEADER);
        
        // number of facts in the mutex group
        int numFacts = acceptAnyInt();
        Proposition *facts = new Proposition[numFacts];

        // Read all the facts of the group that are mutex
        for (int j = 0; j < numFacts; j++) {
            // Tokenize this line and read individual tokens
            acceptTokenLine();
            int variable, value;
            expectVarValuePair(&variable, &value);

            // Translate variable and value to proposition number
            facts[j] = Proposition(variable, value);

            // Set these propositions mutex to each other in every layer
            for (int k = 0; k < j; k++) {
                problemBuilder.setGlobalPropMutex(facts[j], facts[k]);
            }
        }
  
        // Clean up
        delete[] facts;

        expect(MUTEX_FOOTER);
    }
}


/**
 * Parses the initial state section
 */
void SASParser::initialstate() {
    log(2, "Parsing initial state section\n");

    // Initialize problem vectors
    //TODO: Move to PlanningProblem class constructor
    //problem->propEnabled.resize(problem->countPropositions, 0);
    //problem->layerProps.reserve(problem->countPropositions);
    //problem->lastPropIndices.push_back(countVariables-1);

    expect(INITIALSTATE_HEADER);
    
    // Parse initial variable states
    for (int variable = 0; variable < countVariables; variable++) {
        int value = acceptAnyInt();
        problemBuilder.addIntialProposition(Proposition(variable, value));
        //TODO: Move this stuff to PlanningProblem
        // Update problem:
        // Enable the specified proposition
        //problem->propEnabled[prop] = 1;
        // Put the proposition in the next spot of the layerProps array,
        // indicating that the proposition exists now in the planning graph
        //problem->layerProps[i] = prop;
    }

    expect(INITIALSTATE_FOOTER);
}


/**
 * Parses the goal state section
 */
void SASParser::goalstate() {
    log(2, "Parsing goal state section\n");

    expect(GOALSTATE_HEADER);
    
    int countGoalProps = acceptAnyInt();
    //problem->goalPropositions.resize(countGoalProps);
    
    for (int i = 0; i < countGoalProps; i++) {
        // Tokenize this line and read individual tokens
        acceptTokenLine();
        int variable, value;
        expectVarValuePair(&variable, &value);

        // TODO: Move to PlanningProblem::Builder
        //problem->goalPropositions[i] = prop;
        problemBuilder.addGoalProposition(Proposition(variable, value));
    }
    
    expect(GOALSTATE_FOOTER);
}


/**
 * Parse the operator section
 */
void SASParser::operators() {
    log(2, "Parsing operator section\n");

    // Read and set action count
    int countOperators = acceptAnyInt(); // + problem->countPropositions;
    problemBuilder.setActionCount(countOperators);

    // TODO: Move all this to the PlanningProblem builder
    // Initialize problem data structure
    /*
    problem->actionPrecIndices.resize(countOperators+1);
    problem->actionPosEffIndices.resize(countOperators+1);
    problem->actionNegEffIndices.resize(countOperators+1);
    problem->propPosActions.resize(problem->countPropositions);
    problem->actionEnabled.resize(countOperators, 0);
    problem->actionFirstLayer.resize(countOperators);
    problem->layerActions.resize(countOperators);
    problem->actionNames.reserve(countOperators);
    problem->actionMutexes = new int[countOperators*countOperators];
    */

    // Trivial actions
    // TODO: Move to PlanningProblem builder
    /*
    for (int i = 0; i < problem->countPropositions; i++) {
        // Adjacency array indices
        problem->actionPrecIndices[i] = i;
        problem->actionPosEffIndices[i] = i;
        problem->actionNegEffIndices[i] = 0;

        // Edges
        problem->actionPrecEdges.push_back(i);
        problem->actionPosEffEdges.push_back(i);
        problem->propPosActions[i].push_back(i);

        // Already enabled? Depends on initial state
        //problem->actionEnabled[i] = problem->propEnabled[i];
        //if (problem->actionEnabled[i]) {
        //    problem->layerActions.push_back(i);
        //}

        //problem->actionNames[i] = ("Noop " + std::to_string(i));
        problem->actionNames[i] = ("Keep " + problem->propNames[i]);
    }
    */

    // Parse actions
    //for (int i = problem->countPropositions; i < countOperators; i++) {
    for (int i = 0; i < countOperators; i++) {
        expect(OPERATION_HEADER);

        Action action = problemBuilder.addAction();

        // Get operator name
        //problem->actionNames[operatorNumber] = acceptAnyLine();
        problemBuilder.setActionName(action, acceptAnyLine());

        // Update basic graph structure of problem
        // TODO: Move to builder
        /*
           problem->actionPrecIndices[operatorNumber] = problem->actionPrecEdges.size();
           problem->actionPosEffIndices[operatorNumber] = problem->actionPosEffEdges.size();
           problem->actionNegEffIndices[operatorNumber] = problem->actionNegEffEdges.size();
           */

        // Prevail conditions
        int countPrevailConditions = acceptAnyInt();
        for (int j = 0; j < countPrevailConditions; j++) {
            acceptTokenLine();
            int variable, value;
            expectVarValuePair(&variable, &value);

            problemBuilder.addActionPrecondition(action, Proposition(variable, value));
            problemBuilder.addActionPosEffect(action, Proposition(variable, value));
            /* TODO: Move to builder
            // Add prevail conditions as precondition and positive effect!
            problem->actionPrecEdges.push_back(prop);
            problem->actionPosEffEdges.push_back(prop);
            // Also register the positive effect for the proposition
            problem->propPosActions[prop].push_back(operatorNumber);
            */
        }

        // Effects
        int countEffects = acceptAnyInt();
        for (int j = 0; j < countEffects; j++) {
            acceptTokenLine();
            operatorEffect(action);
        }

        // Operator cost (we don't expect this to play a role, since we only expect
        // "metric" problems)
        //acceptAnyInt();
        acceptAnyLine();

        expect(OPERATION_FOOTER);
    }

    // Prepare an additional last element for the adjacency arrays
    // TODO: Move to planning problem builder
    /*
    problem->actionPrecIndices[problem->countActions] = problem->actionPrecEdges.size();
    problem->actionPosEffIndices[problem->countActions] = problem->actionPosEffEdges.size();
    problem->actionNegEffIndices[problem->countActions] = problem->actionNegEffEdges.size();
    */
}


/**
 * Parse an effect of an operator
 */
void SASParser::operatorEffect(Action action) {
    int countEffConditions = acceptAnyIntToken();

    // For STRIPS, there are usually no effect conditions
    if (countEffConditions != 0) {
        log(0, "WARNING: amount of effect conditions is not zero!\n");
    }

    for (int i = 0; i < countEffConditions; i++) {
        int variable, value;
        expectVarValuePair(&variable, &value);
        // We usually don't have this case, so nothing useful implemented here
    }

    // Get affected variable, "precondition" and "effect"
    int variable = expectIntTokenRange(0, countVariables-1);
    int range = variableDomainSizes[variable];
    int preValue = expectIntTokenRange(-1, range-1);
    int postValue = expectIntTokenRange(0, range-1);

    // If the effect needs the variable to have a certain value, then
    // add that as both a precondition and a negative effect
    if (preValue != -1) {
        /* TODO: Move to builder
        problem->actionPrecEdges.push_back(preProp);
        problem->actionNegEffEdges.push_back(preProp);
        */
        problemBuilder.addActionPrecondition(action, Proposition(variable, preValue));
        problemBuilder.addActionNegEffect(action, Proposition(variable, preValue));
    }

    // Add positive effect for both action and variable
    /* TODO: Move to builder
    problem->actionPosEffEdges.push_back(postProp);
    problem->propPosActions[postProp].push_back(operatorNumber);
    */
    problemBuilder.addActionPosEffect(action, Proposition(variable, postValue));
}


/**
 * Parse the axiom section
 */
void SASParser::axioms() {
    // We don't expect the inputs to have axioms...
}

