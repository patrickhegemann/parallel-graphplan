#include <climits>
#include <assert.h>
#include <iostream>

#include "Logger.h"

#include "PlanningProblem.h"
#include "IPlanningProblem.h"



int PlanningProblem::getVariableCount() {
    return countVariables;
}

int PlanningProblem::getActionCount() {
    return countActions;
}

int PlanningProblem::getPropositionCount() {
    return totalPropositionCount;
}

int PlanningProblem::getPropositionNumber(Proposition p) {
    return variableMutexIndex[p.first] + p.second;
}

std::list<Proposition> PlanningProblem::getGoal() {
    return std::list<Proposition>(goalPropositions);
}

std::list<Proposition>& PlanningProblem::getActionPreconditions(Action a) {
    return actionPrecs[a];
}

std::list<Proposition>& PlanningProblem::getActionPosEffects(Action a) {
    return actionPosEffs[a];
}

std::list<Proposition>& PlanningProblem::getActionNegEffects(Action a) {
    return actionNegEffs[a];
}

std::list<Action>& PlanningProblem::getPropPosActions(Proposition p) {
    return propPosActions[p];
}

int PlanningProblem::getFirstLayer() {
    return 1;
}

int PlanningProblem::getLastLayer() {
    return lastPropLayer;
}

int PlanningProblem::getFirstActionLayer() {
    return 1;
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
        // Update lists representations of layerActions
        auto end = layerActions.begin() + lastActionIndices[lastActionLayer] + 1;
        std::list<Action> acts(layerActions.begin(), end);
        layerActionsLists.push_back(acts);
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
    layerActionsLists[layer-1].push_back(a);
    actionFirstLayer[a] = layer;

    // Add positive effects of action to next proposition layer
    for (auto& p : getActionPosEffects(a)) {
        activateProposition(p, getPropLayerAfterActionLayer(layer));
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
    // + 1 because the last proposition needs to be included here
    auto end = layerProps.begin() + lastPropIndices[layer] + 1;
    return std::list<Proposition>(layerProps.begin(), end);
}

std::list<Action>& PlanningProblem::getLayerActions(int layer) {
    return layerActionsLists[layer-1];
}

int PlanningProblem::isMutexProp(Proposition p, Proposition q, int layer) {
    int pMutexNumber = variableMutexIndex[p.first]+p.second;
    int qMutexNumber = variableMutexIndex[q.first]+q.second;
    if (p == q) return false;
    return (propMutexes[pMutexNumber*totalPropositionCount + qMutexNumber] >= layer ||
        p.first == q.first);
}

int PlanningProblem::isMutexAction(Action a, Action b, int layer) {
    if (a == b) return false;
    return actionMutexes[a*countActions + b] >= layer;
}

void PlanningProblem::setMutexProp(Proposition p, Proposition q, int layer) {
    if (p == q) return;
    if (layer < INT_MAX && !isMutexProp(p, q, layer)) {
        layerPropMutexCount[layer]++;
    }
    if (!isMutexProp(p, q, layer)) {
        int pMutexNumber = variableMutexIndex[p.first]+p.second;
        int qMutexNumber = variableMutexIndex[q.first]+q.second;
        propMutexes[pMutexNumber*totalPropositionCount + qMutexNumber] = layer;
        propMutexes[qMutexNumber*totalPropositionCount + pMutexNumber] = layer;
    }
}

void PlanningProblem::setMutexAction(Action a, Action b, int layer) {
    if (a == b) return;
    actionMutexes[a*countActions + b] = layer;
    actionMutexes[b*countActions + a] = layer;
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

int PlanningProblem::getPropLayerBeforeActionLayer(int actionLayer) {
    return actionLayer;
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
    problem->totalPropositionCount = this->totalPropositionCount;

    // Sorting action precondition and effect lists
    for (Action a = 0; a < problem->countActions; a++) {
        problem->actionPrecs[a].sort();
        problem->actionPosEffs[a].sort();
        problem->actionNegEffs[a].sort();
    }

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
    problem->actionPrecs.resize(count);
    problem->actionPosEffs.resize(count);
    problem->actionNegEffs.resize(count);

    problem->actionFirstLayer.resize(count);
    problem->layerActions.resize(count);
    problem->actionNames.resize(count);
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
    return nextAction++;
}

void PlanningProblem::Builder::setActionName(Action a, std::string name) {
    problem->actionNames[a] = name;
}

void PlanningProblem::Builder::addActionPrecondition(Action a, Proposition p) {
    assert(a == nextAction-1);
    problem->actionPrecs[a].push_back(p);
}

void PlanningProblem::Builder::addActionPosEffect(Action a, Proposition p) {
    assert(a == nextAction-1);
    problem->actionPosEffs[a].push_back(p);
    problem->propPosActions[p].push_back(a);
}

void PlanningProblem::Builder::addActionNegEffect(Action a, Proposition p) {
    assert(a == nextAction-1);
    problem->actionNegEffs[a].push_back(p);
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
    problem->totalPropositionCount = totalPropositionCount;

    // Allocate matrix for mutexes
    problem->propMutexes = new int[totalPropositionCount*totalPropositionCount];

    problem->addPropositionLayer();

    variablesFinalized = true;
}

