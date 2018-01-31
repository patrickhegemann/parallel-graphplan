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
    return countActions;
}

std::list<Proposition> PlanningProblem::getGoal() {
    return std::list<Proposition>();
}

std::list<Proposition> PlanningProblem::getActionPreconditions(Action a) {
    auto begin = actionPrecEdges.begin() + actionPrecIndices[a];
    auto end = actionPrecEdges.begin() + actionPrecIndices[a + 1];
    return std::list<Proposition>(begin, end);
}

std::list<Proposition> PlanningProblem::getActionPosEffects(Action a) {
    auto begin = actionPosEffEdges.begin() + actionPosEffIndices[a];
    auto end = actionPosEffEdges.begin() + actionPosEffIndices[a + 1];
    return std::list<Proposition>(begin, end);
}

std::list<Proposition> PlanningProblem::getActionNegEffects(Action a) {
    auto begin = actionNegEffEdges.begin() + actionNegEffIndices[a];
    auto end = actionNegEffEdges.begin() + actionNegEffIndices[a + 1];
    return std::list<Proposition>(begin, end);
}

std::list<Action> PlanningProblem::getPropPosActions(Proposition p) {
    return propPosActions[p];
}

int PlanningProblem::getFirstLayer() {
    return 1;
}

int PlanningProblem::getLastLayer() {
    return lastPropLayer;
}

int PlanningProblem::getLastActionLayer() {
    return lastActionLayer;
}

int PlanningProblem::addPropositionLayer() {
    // No mutexes in this layer yet
    layerPropMutexCount.push_back(0);

    // At least same propositions as previous layer are available
    if (lastPropIndices.empty()) {
        lastPropIndices.push_back(-1);
    } else {
        lastPropIndices.push_back(lastPropIndices[lastPropLayer]);
    }
    lastPropLayer++;

    return lastPropLayer;
}

int PlanningProblem::addActionLayer() {
    // At least same actions as previous layer are available
    if (lastActionIndices.empty()) {
        lastActionIndices.push_back(-1);
    } else {
        lastActionIndices.push_back(lastActionIndices[lastActionLayer]);
    }
    lastActionLayer++;

    return lastActionLayer;
}

int PlanningProblem::isPropEnabled(Proposition p, int layer) {
    return (propFirstLayer[p] <= layer);
}

int PlanningProblem::isActionEnabled(Action a, int layer) {
    return (actionFirstLayer[a] <= layer);
}

int PlanningProblem::getActionFirstLayer(Action a) {
    return actionFirstLayer[a];
}

void PlanningProblem::activateAction(Action a, int layer) {
    lastActionIndices[layer]++;
    layerActions[lastActionIndices[layer]] = a;
    actionFirstLayer[a] = layer;

    // Add positive effects of action to next proposition layer
    for (int i = actionPosEffIndices[a]; i < actionPosEffIndices[a+1]; i++) {
        activateProposition(actionPosEffEdges[i], getPropLayerAfterActionLayer(layer));
    }
}

void PlanningProblem::activateProposition(Proposition p, int layer) {
    if (!isPropEnabled(p, layer)) {
        propFirstLayer[p] = layer;
        layerProps.push_back(p);
        lastPropIndices[layer]++;
    }
}

std::list<Proposition> PlanningProblem::getLayerPropositions(int layer) {
    auto end = layerProps.begin() + lastPropIndices[layer];
    return std::list<Proposition>(layerProps.begin(), end);
}

std::list<Action> PlanningProblem::getLayerActions(int layer) {
    auto end = layerActions.begin() + lastActionIndices[layer];
    return std::list<Action>(layerActions.begin(), end);
}

int PlanningProblem::isMutexProp(Proposition p, Proposition q, int layer) {
    // TODO 
    // return (propMutexes[a*p->countPropositions + b] >= layer) ||
    return 0;
}

int PlanningProblem::isMutexAction(Action a, Action b, int layer) {
    // TODO: implement
    return 0;
}

void PlanningProblem::setMutexProp(Proposition p, Proposition q, int layer) {
    // TODO: implement
}

void PlanningProblem::setMutexAction(Action a, Action b, int layer) {
    // TODO: implement
}

int PlanningProblem::getPropMutexCount(int layer) {
    return layerPropMutexCount[layer];
}

std::string PlanningProblem::getPropositionName(Proposition p) {
    return propNames[p];
}

std::string PlanningProblem::getActionName(Action a) {
    return actionNames[a];
}

int PlanningProblem::isTrivialAction(Action a) {
    return (a < totalPropositionCount);
}

int PlanningProblem::getPropLayerAfterActionLayer(int actionLayer) {
    return actionLayer + 1;
}

int PlanningProblem::getActionLayerBeforePropLayer(int propLayer) {
    return propLayer - 1;
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
    problem->totalPropositionCount = this->totalPropositionCount;

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

    // TODO: move this somewhere else?
    // add dummy layers so we get 1 to be the first actual layer, not 0 (also for actions layer)
    problem->addPropositionLayer();//lastActionIndices.push_back(-1);
    problem->addActionLayer();//lastPropIndices.push_back(-1);
    // Initial proposition layer will assign every variable one value
    problem->lastPropIndices.push_back(problem->getVariableCount() - 1);

    variablesFinalized = true;
}

