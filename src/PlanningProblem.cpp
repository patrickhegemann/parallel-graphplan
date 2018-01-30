#include <climits>
#include <assert.h>

#include "Logger.h"

#include "PlanningProblem.h"
#include "IPlanningProblem.h"



//PlanningProblem::PlanningProblem() {

//}

int PlanningProblem::getVariableCount() {
    return countVariables;
}

int PlanningProblem::getActionCount() {

}

std::list<Proposition> PlanningProblem::getGoal() {
    return std::list<Proposition>();
}

std::list<Proposition> PlanningProblem::getActionPreconditions(Action a) {

}

std::list<Proposition> PlanningProblem::getActionPosEffects(Action a) {

}

std::list<Proposition> PlanningProblem::getActionNegEffects(Action a) {

}

std::list<Action> PlanningProblem::getPropPosActions(Proposition p) {

}

int PlanningProblem::getFirstLayer() {
    return 1;
}

int PlanningProblem::getLastLayer() {

}

int PlanningProblem::getLastActionLayer() {

}

int PlanningProblem::addPropositionLayer() {

}

int PlanningProblem::addActionLayer() {

}

int PlanningProblem::isPropEnabled(Proposition p) {
    return propFirstLayer.count(p);
}

int PlanningProblem::isActionEnabled(Action a) {

}

int PlanningProblem::getActionFirstLayer(Action a) {

}

void PlanningProblem::activateAction(Action a, int layer) {

}

void PlanningProblem::activateProposition(Proposition p, int layer) {
    if (!isPropEnabled(p)) {    // TODO: Possible optimization: Don't check
        propFirstLayer[p] = layer;
        layerProps.push_back(p);
    }
}

std::list<Proposition> PlanningProblem::getLayerPropositions(int layer) {

}

std::list<Action> PlanningProblem::getLayerActions(int layer) {

}

inline int PlanningProblem::isMutexProp(Proposition p, Proposition q, int layer) {

}

inline int PlanningProblem::isMutexAction(Action a, Action b, int layer) {

}

inline int PlanningProblem::setMutexProp(Proposition p, Proposition q, int layer) {

}

inline int PlanningProblem::setMutexAction(Action a, Action b, int layer) {

}

int PlanningProblem::getPropMutexCount(int layer) {

}

std::string PlanningProblem::getPropositionName(Proposition p) {

}

std::string PlanningProblem::getActionName(Action a) {

}

int PlanningProblem::isTrivialAction(Action a) {

}


// ----------------------------------------------------------------------------

/**
 * Constructor.
 *
 * Creates a new Planning Problem instance to work on
 */
PlanningProblem::Builder::Builder() {
    // Allocate new Planning Problem on the heap so we can use it in the algorithm later
    problem = new PlanningProblem();
}

/**
 * Return the built problem instance.
 */
PlanningProblem* PlanningProblem::Builder::build() {
    // Finalize adjacency arrays (Add trailing dummy element)
    problem->actionPrecIndices.push_back(problem->actionPrecEdges.size());
    problem->actionPosEffIndices.push_back(problem->actionPosEffEdges.size());
    problem->actionNegEffIndices.push_back(problem->actionNegEffEdges.size());

    return problem;
}

/**
 * Set the variable count.
 *
 * Resize data structures that are dependent on this metric:
 *  - Vector containing domain size of variables
 */
void PlanningProblem::Builder::setVariableCount(int count) {
    assert(problem->countVariables == 0);

    problem->countVariables = count;
    problem->variableDomainSize.resize(count);
}

Variable PlanningProblem::Builder::addVariable() {
    return nextVariable++;
}

void PlanningProblem::Builder::setVariableDomainSize(Variable v, int size) {
    problem->variableDomainSize[v] = size;
}

void PlanningProblem::Builder::setPropositionName(Proposition p, std::string name) {
    problem->propNames[p] = name;
}

/**
 * Sets two propositions mutex to each other globally, i.e. in every layer.
 *
 * Note that this automatically calls "variable finalization", so no further
 * variables can be added after this.
 */
void PlanningProblem::Builder::setGlobalPropMutex(Proposition p, Proposition q) {
    if (!variablesFinalized) finalizeVariables();
    problem->setMutexProp(p, q, INT_MAX);
}

void PlanningProblem::Builder::addIntialProposition(Proposition p) {
    assert(variablesFinalized);

    problem->activateProposition(p, problem->getFirstLayer());
}

void PlanningProblem::Builder::addGoalProposition(Proposition p) {
    problem->goalPropositions.push_back(p);
}

void PlanningProblem::Builder::setActionCount(int count) {
    assert(variablesFinalized);
    assert(problem->countActions == 0);

    // Take trivial actions into account
    count += totalPropositionCount;

    problem->countActions = count;

    // Resize adjacency arrays
    problem->actionPrecIndices.resize(count+1);
    problem->actionPosEffIndices.resize(count+1);
    problem->actionNegEffIndices.resize(count+1);

    problem->actionFirstLayer.resize(count);
    problem->layerActions.resize(count);
    problem->actionNames.reserve(count);
    problem->actionMutexes = new int[count*count];

    // Create trivial actions
    for (int var = 0; var < problem->getVariableCount(); var++) {
        for (int val = 0; val < problem->variableDomainSize[var]; val++) {
            Action a = addAction();
            setActionName(a, "Keep " + problem->propNames[Proposition(var, val)]);
            addActionPrecondition(a, Proposition(var, val));
            addActionPosEffect(a, Proposition(var, val));
        }
    }
}

Action PlanningProblem::Builder::addAction() {
    // Set adjacency array entries. Currently no preconditions or effects.
    problem->actionPrecIndices[nextAction] = problem->actionPrecIndices.size();
    problem->actionPosEffIndices[nextAction] = problem->actionPosEffIndices.size();
    problem->actionNegEffIndices[nextAction] = problem->actionNegEffIndices.size();
    return nextAction++;
}

void PlanningProblem::Builder::setActionName(Action a, std::string name) {
    problem->actionNames[a] = name;
}

void PlanningProblem::Builder::addActionPrecondition(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionPrecIndices[a]++;
    problem->actionPrecEdges.push_back(p);
}

void PlanningProblem::Builder::addActionPosEffect(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionPosEffIndices[a]++;
    problem->actionPosEffEdges.push_back(p);
    problem->propPosActions[p].push_back(a);
}

void PlanningProblem::Builder::addActionNegEffect(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionNegEffIndices[a]++;
    problem->actionNegEffEdges.push_back(p);
}

/**
 * "Finalizes" the creation of any variables.
 *
 *  - Initializes mutex matrix
 *  - Initializes layerProps vector
 */
void PlanningProblem::Builder::finalizeVariables() {
    assert(!variablesFinalized);


    // Calculate total proposition count (i.e. how many combinations of two
    // variable/value pairs there are)
    totalPropositionCount = 0;
    for (int i = 0; i < problem->getVariableCount(); i++) {
        totalPropositionCount += problem->variableDomainSize[i];
    }

    log(2, "Finalizing variables (There are %d propositions)\n", totalPropositionCount);

    // Allocate matrix for mutexes
    problem->propMutexes = new int[totalPropositionCount*totalPropositionCount];

    // Reserve enough space for the vector that stores enabled propositions in order
    problem->layerProps.reserve(totalPropositionCount);

    // Initial proposition layer will assign every variable one value
    // TODO: push a 0 somewhere before this, so we get 1 to be the first actual layer, not 0
    problem->lastPropIndices.push_back(problem->getVariableCount() - 1);

    variablesFinalized = true;
}

