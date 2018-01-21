#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>

#include "planner.h"


Planner::Planner(Problem *problem) {
    this->problem = problem;
}

int Planner::isNogood(int layer, std::list<int> props) {
    // No nogoods added for specified layer -> can't be a nogood
    if (nogoods.size() <= (unsigned int) layer) return false;

    unsigned int propsFoundInNogood = 0;

    //std::cout << "layer " << layer << std::endl;

    for (unsigned int i = 0; i < nogoods[layer].size(); i++) {
        int p = nogoods[layer][i];
        // Next nogood -> reset
        if (p == 0) {
            // Exactly the propositions found in this nogood
            if (propsFoundInNogood == props.size()) {
                //std::cout << "nogood found" << std::endl;
                return true;
            }
            propsFoundInNogood = 0;
            continue;
        }

        // Check if the proposition is one of our wanted propositions
        if (std::find(props.begin(), props.end(), p) != props.end()) {
            //std::cout << p << std::endl;
            propsFoundInNogood++;
        }
    }

    return false;
}

void Planner::addNogood(int layer, std::list<int> props) {
    std::cout << "New nogood in layer " << layer << ": ";
    for (int p : props) {
        std::cout << p << ", ";
    }
    std::cout << std::endl;

    // Add vectors to the nogood table until we have sufficiently many layers
    while (nogoods.size() <= (unsigned int) layer) {
        nogoods.push_back(std::vector<int>());
    }

    // Add the nogood to the specified layer
    for (int p : props) {
        nogoods[layer].push_back(p);
    }
    // Push a 0 for separating nogoods
    nogoods[layer].push_back(0);

    countNogoods++;
}

/**
 * Check if a fixed point in the planning graph is reached
 */
int Planner::checkFixedPoint() {
    // Checking only makes sense if we have at least two existing layers.
    if (problem->lastPropIndices.size() < 2) return false;

    // If the last two layers have equal amounts of propositions and proposition
    // mutexes then we reached a fixed point.
    std::list<int>::iterator lastLastPropIndex = std::prev(problem->lastPropIndices.end());
    std::vector<int>::iterator lastLayerPropMutexCount = std::prev(problem->layerPropMutexCount.end());

    if (*lastLastPropIndex == *(std::prev(lastLastPropIndex)) &&
            *lastLayerPropMutexCount == *(std::prev(lastLayerPropMutexCount))) {
        return true;
    }

    // No fixed point
    return false;
}


/**
 * Checks if there are still goals missing in the planning graph or, in case
 * they are all there, if any pair of goals is still mutex.
 * Formally: NOT(g in P) OR NOT((g^2 intersect mP) = {})
 * where g is the goal set, P the current layer, mP the mutexes of the current layer
 *
 * This function is needed while doing fixed-point iteration and expanding to detect
 * if the problem is unsolvable.
 */
int Planner::checkGoalUnreachable(int layer) {
    // Check if all goals are enabled already. If not, return true.
    for (auto const& goal: problem->goalPropositions) {
        if (!problem->propEnabled[goal]) return true;
    }

    // Check if any pair of goals is mutex in the current layer. If so, return true.
    for (unsigned int i = 0; i < problem->goalPropositions.size(); i++) {
        for (unsigned int j = 0; j < i; j++) {
            int goal1 = problem->goalPropositions[i];
            int goal2 = problem->goalPropositions[j];
            if (getPropMutex(problem, goal1, goal2, layer)) {
                return true;
            }
        }
    }

    // None of the above are true -> Goal reachable
    return false;
}


int Planner::graphplan(std::list<std::list<int>>& plan) {
    // Initialize the random seed
    srand(time(NULL));

    int layer = 0;

    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable(layer)) {
        layer++;
        expand();

        fixedPoint = fixedPoint || checkFixedPoint();
        // If goal is impossible to reach, this problem has no solution
    }
    if (checkGoalUnreachable(layer)) return false;

    std::list<int> goal(problem->goalPropositions.begin(),
            problem->goalPropositions.end());

    int success = extract(goal, layer, plan);

    while(!success) {
        // Expand for one more layer
        //getchar();
        std::cout << "Layer " << layer << std::endl;
        if (!fixedPoint) {
            layer++;
            expand();
            fixedPoint = checkFixedPoint();
        }

        // Do backwards search with given goal propositions
        plan.clear();
        std::list<int> goal(problem->goalPropositions.begin(),
            problem->goalPropositions.end());
        success = extract(goal, layer, plan);

        if ((!success) && fixedPoint) {
            // TODO: Check if nogood table not growing -> no plan
        }
    }

    //plan = p;
    std::cout << "Final number of layers: " << layer << std::endl;
    return success;
}

void Planner::expand() {
    //std::cout << "Expanding graph" << std::endl;
    
    // TODO: make layer a property of the planner so this doesn't have to be so ugly
    // Note that we always generate an action layer number (i) and a prop layer (i+1)
    int currentPropLayer = problem->lastPropIndices.size();
    int nextActionLayer = currentPropLayer;

    // Copy last proposition and action indices from current layer to next layer
    // These values will be adjusted during the expanding in order to add new
    // actions and propositions
    int currentLastPropIndex, currentLastActionIndex;
    currentLastPropIndex = problem->lastPropIndices.back();
    if (!problem->lastActionIndices.empty()) {
        currentLastActionIndex = problem->lastActionIndices.back();
    } else{
        currentLastActionIndex = -1;
    }
    problem->lastPropIndices.push_back(currentLastPropIndex);
    problem->lastActionIndices.push_back(currentLastActionIndex);

    // TODO: Remove this because we won't expand past the fixed point
    // If we reached a fixed point we don't need to do anything else.
    // No adding of actions or propositions needed, since those will be carried
    // over implicitly.
    if (fixedPoint && !fixedMutexes) {
        updateNewLayerMutexes(currentPropLayer);
        return;
    }

    // Add a new entry in the layerPropMutexCount vector to see if we reached a
    // fixed-point level
    problem->layerPropMutexCount.push_back(0);

    std::list<int> addedProps;

    // Add actions
    for(int action = 0; action < problem->countActions; action++) {
        // Only check disabled actions
        // Potential for optimization: use a list of unused actions
        
        if (!problem->actionEnabled[action]) {
            bool enable = true;

            // Check if preconditions already present and abort if not
            for (int j = problem->actionPrecIndices[action];
                    j < problem->actionPrecIndices[action+1]; j++) {
                int prec = problem->actionPrecEdges[j];
                
                if (!problem->propEnabled[prec]) {
                    enable = false;
                    break;
                }

                // Check for precondition mutexes and abort if mutex was found
                for (int k = problem->actionPrecIndices[action]; k < j; k++) {
                    int prec2 = problem->actionPrecEdges[k];
                    if (getPropMutex(problem, prec, prec2, currentPropLayer)) {
                        enable = false;
                        break;
                    }
                }

                if (!enable) break;
            }

            // Action not enabled -> continue with next action
            if (!enable) continue;

            // Add the action
            // Enable action in next layer
            problem->lastActionIndices.back()++;
            problem->layerActions[problem->lastActionIndices.back()] = action;
            problem->actionEnabled[action] = 1;
            problem->actionFirstLayer[action] = nextActionLayer;

            // Add positive effects of action to next proposition layer
            for (int i = problem->actionPosEffIndices[action];
                    i < problem->actionPosEffIndices[action+1]; i++) {
                int prop = problem->actionPosEffEdges[i];
                addedProps.push_back(prop);
            }

        }
    }

    // Add propositions
    for (int prop : addedProps) {
        if (!problem->propEnabled[prop]) {
            problem->lastPropIndices.back()++;
            problem->layerProps[problem->lastPropIndices.back()] = prop;
            problem->propEnabled[prop] = 1;
        }
    }

    updateNewLayerMutexes(currentPropLayer);

    // debug:
    /*
    std::cout << "Enabled actions: " << std::endl;
    for (int i = 0; i <= problem->lastActionIndices.back(); i++) {
        std::cout << "\t" << problem->actionNames[problem->layerActions[i]] << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Enabled propositions: ";
    for (int i = 0; i <= problem->lastPropIndices.back(); i++) {
        std::cout << problem->layerProps[i] << ", ";
    }
    std::cout << std::endl;
    */
}

/**
 * Updates the mutexes of the newly added specified layer.
 * If a fixed point is reached, the mutexes will be added (one last time) for every layer.
 * Then fixedMutexes will be set to 1, which means this function _should_ never be called
 * again.
 */
void Planner::updateNewLayerMutexes(int currentPropLayer) {
    int nextActionLayer = currentPropLayer;
    int nextPropLayer = currentPropLayer + 1;

    // Number of the layers where the mutexes will be added
    int newMutexActionLayer = nextActionLayer;
    int newMutexPropLayer = nextPropLayer;

    // If we reached a fixed point then all current mutexes will be there forever.
    if (fixedPoint) {
        newMutexActionLayer = INT_MAX;
        newMutexPropLayer = INT_MAX;
        fixedMutexes = 1;
    }

    // Update action mutexes
    // Iterate other new actions in this layer
    for (int i = 0; i < problem->lastActionIndices.back(); i++) {
        int action = problem->layerActions[i];

        for (int j = 0; j < i; j++) {
            int action2 = problem->layerActions[j];

            if (checkActionsMutex(action, action2) ||
                    checkActionsMutex(action2, action) ||
                    checkActionPrecsMutex(action, action2, currentPropLayer)) {
                setActionMutex(problem, action, action2, newMutexActionLayer);
            }
        }
    }

    // Update proposition mutexes
    for (int i = 0; i <= problem->lastPropIndices.back(); i++) {
        int p = problem->layerProps[i];

        for (int j = 0; j < i; j++) {
            int q = problem->layerProps[j];

            if (checkPropsMutex(p, q, nextActionLayer)) {
                setPropMutex(problem, p, q, newMutexPropLayer);
            }
        }
    }
}


/**
 * Checks if two propositions will be mutex in the proposition layer following
 * the given action layer
 */
int Planner::checkPropsMutex(int p, int q, int actionLayer) {
    for (int a : problem->propPosActions[p]) {
        for (int b : problem->propPosActions[q]) {
            if (problem->actionEnabled[a] && problem->actionEnabled[b] &&
                    !getActionMutex(problem, a, b, actionLayer)) {
                // Two non-mutex actions exist which resp. enable p and q
                return false;
            }
        }
    }

    return true;
}

/**
 * Checks if two actions will be mutex in the next layer.
 * Has to be called twice: once with actions (a,b), then with (b,a).
 */
int Planner::checkActionsMutex(int a, int b) {
    // Iterate the action's negative effects
    for (int j = problem->actionNegEffIndices[a];
            j < problem->actionNegEffIndices[a+1]; j++) {
        int negEff = problem->actionNegEffEdges[j];

        // Check if negative effects collide with preconditions
        for (int k = problem->actionPrecIndices[b];
                k < problem->actionPrecIndices[b+1]; k++) {
            if (negEff == problem->actionPrecEdges[k]) {
                return true;
            }
        }

        // Check if negative effects collide with positive effects
        for (int k = problem->actionPosEffIndices[b];
                k < problem->actionPosEffIndices[b+1]; k++) {
            if (negEff == problem->actionPosEffEdges[k]) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Checks if the preconditions of two actions are mutex in the given layer
 */
int Planner::checkActionPrecsMutex(int a, int b, int layer) {
    // Iterate a's preconditions
    for (int i = problem->actionPrecIndices[a];
            i < problem->actionPrecIndices[a+1]; i++) {
        int prec = problem->actionPrecEdges[i];

        // Iterate b's preconditions
        for (int j = problem->actionPrecIndices[b];
                j < problem->actionPrecIndices[b+1]; j++) {
            int prec2 = problem->actionPrecEdges[j];

            // Check if mutex
            if (getPropMutex(problem, prec, prec2, layer)) {
                return true;
            }
        }
    }

    return false;
}


int Planner::extract(std::list<int> goal, int layer, std::list<std::list<int>>& plan) {
    std::cout << "Extracting ";
    for (int g : goal) {
        std::cout << g << ", ";
    }
    std::cout << "in layer " << layer << std::endl;
    

    // Trivial success
    if (layer == 0) {
        return 1;
    }

    // This sub-goal has failed before
    // TODO: share with other threads (receive)
    if (isNogood(layer, goal)) return 0;

    // Perform the graphplan search
    std::list<int> actions;
    int success = gpSearch(goal, actions, layer, plan);

    // Plan found
    if (success) {
        return 1;
    }

    // Update nogoods and fail
    // TODO: share with other threads (send)
    addNogood(layer, goal);
    return 0;
}

int Planner::gpSearch(std::list<int> goal, std::list<int> actions, int layer, std::list<std::list<int>>& plan) {
    //std::cout << "Performing gpSearch" << std::endl;

    /*
    for (int g : goal) {
        //std::cout << problem->propNames[g] << ", ";
        std::cout << g << ", ";
    }
    std::cout << std::endl;
    */
    

    // All actions already chosen
    if (goal.empty()) {
        // Extract plan for preconditions of chosen actions
        std::list<int> preconds;
        //for (std::vector<int>::size_type i = 0; i < actions.size(); i++) {
        for (int a : actions) {
            //int a = actions[i];
            int first = problem->actionPrecIndices[a];
            unsigned int last = problem->actionPrecIndices[a+1] - 1;
            for (std::vector<int>::size_type j = first; j <= last; j++) {
                // TODO: Duplicate detection ?
                preconds.push_back(problem->actionPrecEdges[j]);
            }
        }

        // Try satisfying preconditions in previous layer
        int success = extract(preconds, layer-1, plan);
        if (!success) return 0;
        
        // Plan found, add actions to plan and return success
        plan.push_back(actions);
        return 1;
    }

    // TODO: select one possibly in a different way
    int p = goal.front();

    // Get providers (actions) of p
    std::list<int> providers;
    for (int provider : problem->propPosActions[p]) {
        // Check if provider is enabled
        if (!(problem->actionEnabled[provider]) || problem->actionFirstLayer[provider] > layer) continue;

        // Check if provider is mutex with any action
        bool mut = false;
        for (int act : actions) {
            if (getActionMutex(problem, provider, act, layer)) {
                // std::cout << "ACTION MUTEX " << provider << " " << act << std::endl;
                mut = true;
                break;
            }
        }

        // Add provider
        if (!mut) {
            // Hack: Add "keep" actions to the back and others to the front, so they get chosen first
            if (provider >= problem->countPropositions) {
                providers.push_front(provider);
            } else {
                providers.push_back(provider);
            }
        }
    }

    // No providers for goal => no plan
    if (providers.empty()) return 0;
    
    // TODO: choose one
    // Add a providing action
    //int a = providers.front();
    /* // Choose a single action randomly
    int a = 0;
    int rnd = rand();
    int randIndex = rnd % providers.size();
    // std::cout << "RANDOM " << rnd << " % " << providers.size() << "=" << randIndex << std::endl;
    int counter = 0;
    for (int i : providers) {
        if (counter == randIndex) {
            a = i;
            break;
        }
        counter++;
    }

    actions.push_back(a);
    */
    
    /* Select provider for chosen proposition. Non-deterministic choice point (=> Backtrack here)
     * Simple method: Just try every provider one after another
     * TODO: Parallelize here
     */
    for (int prov : providers) {
        // Copy goal and action list for next recursive call
        std::list<int> newGoal(goal);
        std::list<int> newActions(actions);

        // Add action to action list
        newActions.push_back(prov);

        // Remove action effects from goal list
        int first = problem->actionPosEffIndices[prov];
        unsigned int last = problem->actionPosEffIndices[prov+1] - 1;
        for (std::vector<int>::size_type i = first; i <= last; i++) {
            newGoal.remove(problem->actionPosEffEdges[i]);
        }

        // Call recursively
        int success = gpSearch(newGoal, newActions, layer, plan);

        if (success) return success;
    }

    return 0;
    
    /*
    // Remove action effects from goal list
    int first = problem->actionPosEffIndices[a];
    unsigned int last = problem->actionPosEffIndices[a+1] - 1;
    for (std::vector<int>::size_type i = first; i <= last; i++) {
        goal.remove(problem->actionPosEffEdges[i]);
    }

    // Call recursively
    return gpSearch(goal, actions, layer, plan);
    */
}

