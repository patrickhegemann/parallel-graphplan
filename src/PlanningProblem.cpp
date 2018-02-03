#include <climits>
#include <assert.h>
#include <iostream>

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
    return std::list<Proposition>(goalPropositions);
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
    log(4, "addPropositionLayer. lastPropLayer=%d\n", lastPropLayer);
    // No mutexes in this layer yet
    layerPropMutexCount.push_back(0);

    // At least same propositions as previous layer are available
    if (lastPropIndices.empty()) {
        lastPropIndices.push_back(-1);
    } else {
        //lastPropIndices.push_back(lastPropIndices[lastPropLayer]);
        for (auto x : lastPropIndices) {
            log(4, "x = %d\n", x);
        }
        lastPropIndices.push_back(lastPropIndices.back());
    }
    lastPropLayer++;

    log(4, "new value of lastPropLayer = %d\n", lastPropLayer);

    return lastPropLayer;
}

int PlanningProblem::addActionLayer() {
    // At least same actions as previous layer are available
    if (lastActionIndices.empty()) {
        lastActionIndices.push_back(-1);
    } else {
        lastActionIndices.push_back(lastActionIndices.back());
    }
    lastActionLayer++;

    return lastActionLayer;
}

int PlanningProblem::isPropEnabled(Proposition p, int layer) {
    return (propFirstLayer[p] <= layer && propFirstLayer[p] > 0);
}

int PlanningProblem::isActionEnabled(Action a, int layer) {
    return (actionFirstLayer[a] <= layer && actionFirstLayer[a] > 0);
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
    log(4, "activateProposition: %s in layer %d\n", propNames[p].c_str(), layer);
    if (!isPropEnabled(p, layer)) {
        log(4, "That proposition will now be added\n");
        propFirstLayer[p] = layer;
        layerProps.push_back(p);
        lastPropIndices[layer]++;
    }
}

std::list<Proposition> PlanningProblem::getLayerPropositions(int layer) {
    log(4, "getLayerPropositions\n");
    log(4, "Layer: %d\n", layer);
    log(4, "lastPropIndices[%d] = %d\n", layer, lastPropIndices[layer]);

    // + 1 because the last proposition needs to be included here
    auto end = layerProps.begin() + lastPropIndices[layer] + 1;
    return std::list<Proposition>(layerProps.begin(), end);
}

std::list<Action> PlanningProblem::getLayerActions(int layer) {
    auto end = layerActions.begin() + lastActionIndices[layer] + 1;
    std::list<Action> acts(layerActions.begin(), end);
    log(4, "Hello?\n");
    for (auto a : acts) {
        log(4, "a = %s\n", actionNames[a].c_str());
    }
    return acts;
    //return std::list<Action>(layerActions.begin(), end);
}

int PlanningProblem::isMutexProp(Proposition p, Proposition q, int layer) {
    int pMutexNumber = variableMutexIndex[p.first]+p.second;
    int qMutexNumber = variableMutexIndex[q.first]+q.second;
    return (propMutexes[pMutexNumber*totalPropositionCount + qMutexNumber] >= layer ||
        p.first == q.first);
}

int PlanningProblem::isMutexAction(Action a, Action b, int layer) {
    log(5, "Checking if actions \"%s\" and \"%s\" are mutex in layer %d\n", actionNames[a].c_str(), actionNames[b].c_str(), layer);
    log(5, "result: %d\n", (actionMutexes[a*countActions + b] >= layer));
    return actionMutexes[a*countActions + b] >= layer;
}

void PlanningProblem::setMutexProp(Proposition p, Proposition q, int layer) {
    if (layer < INT_MAX && !isMutexProp(p, q, layer)) {
        layerPropMutexCount[layer]++;
    }
    if (!isMutexProp(p, q, layer)) {
        int pMutexNumber = variableMutexIndex[p.first]+p.second;
        int qMutexNumber = variableMutexIndex[q.first]+q.second;
        propMutexes[pMutexNumber*totalPropositionCount + qMutexNumber] = layer;
        propMutexes[qMutexNumber*totalPropositionCount + pMutexNumber] = layer;
        log(2, "Propositions \"%s\" and \"%s\" are now mutex in layer %d\n", propNames[p].c_str(), propNames[q].c_str(), layer);
    }
}

void PlanningProblem::setMutexAction(Action a, Action b, int layer) {
    actionMutexes[a*countActions + b] = layer;
    actionMutexes[b*countActions + a] = layer;
    log(2, "Actions \"%s\" and \"%s\" are now mutex in layer %d\n", actionNames[a].c_str(), actionNames[b].c_str(), layer);
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

void PlanningProblem::dumpPlanningGraph() {
    // Output format:
    // 1. Node labels (proposition names, action names) & edges:
    //      [PROP/ACTION]LABEL <labelnr> <label>
    //          PROPLABEL 1 robby-at-rooma
    //          PROBLABEL 2 robby-at-roomb
    //          ACTIONLABEL 1 move-a-b
    //      EDGE [PREC/POS/NEG] <propnr> <actionnr>
    //          EDGE PREC 1 1
    //          EDGE POS 1 1
    //
    // 2.For each layer:
    // 2a. Layer
    //      [PROP/ACTION]LAYER <layernr> <prop/action count>
    //          PROPLAYER 1 4
    //          ACTIONLAYER 1 5
    //
    // 2b. Nodes
    //      [PROP/ACTION]NODE <labelnr>
    //          PROPNODE 1
    //          ACTIONNODE 4
    
    std::cout << "PLANNING GRAPH" << std::endl;

    int labelcounter = 0;
    for (int var = 0; var < countVariables; var++) {
        for (int val = 0; val < variableDomainSize[var]; val++) {
            std::cout << "PROPLABEL " << labelcounter << " " << propNames[Proposition(var, val)] << std::endl;
            labelcounter++;
        }
    }

    for (Action a = 0; a < countActions; a++) {
        std::cout << "ACTIONLABEL " << a << " " << actionNames[a] << std::endl;
        std::list<Proposition> precs = getActionPreconditions(a);
        std::list<Proposition> pos = getActionPosEffects(a);
        std::list<Proposition> neg = getActionNegEffects(a);

        for (Proposition p : precs) {
            int propNumber = variableMutexIndex[p.first]+p.second;
            std::cout << "EDGE PREC " << propNumber << " " << a << std::endl;
        }        

        for (Proposition p : pos) {
            int propNumber = variableMutexIndex[p.first]+p.second;
            std::cout << "EDGE POS " << propNumber << " " << a << std::endl;
        }        

        for (Proposition p : neg) {
            int propNumber = variableMutexIndex[p.first]+p.second;
            std::cout << "EDGE NEG " << propNumber << " " << a << std::endl;
        }        
    }


    for (int i = getFirstLayer(); i <= getLastLayer(); i++) {
        std::list<Proposition> props = getLayerPropositions(i);
        std::cout << "PROPLAYER " << i << " " << props.size() << std::endl;
        for (Proposition p : props) {
            int propNumber = variableMutexIndex[p.first]+p.second;
            std::cout << "PROPNODE " << propNumber << std::endl;
        }
        
        
        if (i <= getLastActionLayer()) {
            std::list<Action> actions = getLayerActions(i);
            std::cout << "ACTIONLAYER " << i << " " << actions.size() << std::endl;
            for (Action a : actions) {
                std::cout << "ACTIONNODE " << a << std::endl;
            }
        }
    }

    std::cout << "END PLANNING GRAPH" << std::endl;
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
    
    problem->layerPropMutexCount.push_back(0);
    problem->lastPropIndices.push_back(-1);
    problem->lastActionIndices.push_back(-1);
}

/**
 * Return the built problem instance.
 */
PlanningProblem* PlanningProblem::Builder::build() {
    // Finalize adjacency arrays (Add trailing dummy element)
    problem->actionPrecIndices[problem->actionPrecIndices.size()-1] = problem->actionPrecEdges.size();
    problem->actionPosEffIndices[problem->actionPosEffIndices.size()-1] = problem->actionPosEffEdges.size();
    problem->actionNegEffIndices[problem->actionNegEffIndices.size()-1] = problem->actionNegEffEdges.size();
    problem->totalPropositionCount = this->totalPropositionCount;


    // Experimental output of structure so far
    log(4, "Dumping problem data\n");
    log(4, "Variables and propositions:\n");
    log(4, "Variable count: %d\n", problem->countVariables);
    for (int var = 0; var < problem->countVariables; var++) {
        log(4, "Variable %d:\n", var);
        for (int val = 0; val < problem->variableDomainSize[var]; val++) {
            //std::cout << problem->propNames[Proposition(var, val)] << std::endl;
            log(4, "\tVal %d: %s\n", val, problem->getPropositionName(Proposition(var, val)).c_str());
        }
    }

    log(4, "Actions:\n");
    for (Action a = 0; a < problem->countActions; a++) {
        log(4, "Action %d: %s\n", a, problem->actionNames[a].c_str());
        for (Proposition prec : problem->getActionPreconditions(a)) {
            log(4, "\tPrec: %s\n", problem->getPropositionName(prec).c_str());
        }
        for (Proposition pos : problem->getActionPosEffects(a)) {
            log(4, "\t+ Eff: %s\n", problem->getPropositionName(pos).c_str());
        }
        for (Proposition neg : problem->getActionNegEffects(a)) {
            log(4, "\t- Eff: %s\n", problem->getPropositionName(neg).c_str());
        }
    }

    log(4, "Layers:\n");
    for (int i = problem->getFirstLayer(); i <= problem->getLastLayer(); i++) {
        log(4, "Layer %d:\n", i);
        for (Proposition p : problem->getLayerPropositions(i)) {
            log(4, "Prop %s is enabled\n", problem->getPropositionName(p).c_str());
        }
    }
    // End experimental output

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
    if (problem->variableMutexIndex.empty()) {
        problem->variableMutexIndex.push_back(0);
    } else {
        problem->variableMutexIndex.push_back(problem->variableMutexIndex.back()+problem->variableDomainSize[nextVariable-1]);
    }

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
    log(4, "count = %d\n", count);
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
    problem->actionPrecIndices[nextAction] = problem->actionPrecEdges.size();
    problem->actionPosEffIndices[nextAction] = problem->actionPosEffEdges.size();
    problem->actionNegEffIndices[nextAction] = problem->actionNegEffEdges.size();
    return nextAction++;
}

void PlanningProblem::Builder::setActionName(Action a, std::string name) {
    problem->actionNames[a] = name;
}

void PlanningProblem::Builder::addActionPrecondition(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionPrecIndices[a+1]++;
    problem->actionPrecEdges.push_back(p);
}

void PlanningProblem::Builder::addActionPosEffect(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionPosEffIndices[a+1]++;
    problem->actionPosEffEdges.push_back(p);
    problem->propPosActions[p].push_back(a);
}

void PlanningProblem::Builder::addActionNegEffect(Action a, Proposition p) {
    assert(a == nextAction-1);

    problem->actionNegEffIndices[a+1]++;
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
    //problem->layerProps.reserve(totalPropositionCount);

    // TODO: move this somewhere else?
    // add dummy layers so we get 1 to be the first actual layer, not 0 (also for actions layer)
    //problem->layerPropMutexCount.push_back(0);
    //problem->lastPropIndices.push_back(-1);

    //problem->addActionLayer();//lastPropIndices.push_back(-1);
    problem->addPropositionLayer();//lastActionIndices.push_back(-1);

    variablesFinalized = true;
}

