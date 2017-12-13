#include <iostream>

// TODO
//#include <stdlib.h>
//#include <time.h>

#include "planner.h"


int isNogood(int layer, std::list<int> props) {
    // TODO: Implement
    return 0;
}

void addNogood(int layer, std::list<int> props) {
    // TODO: Implement
}

int fixedPoint(Problem *problem) {
    // TODO: Implement
}


int graphplan(Problem *problem, std::list<std::list<int>>& plan) {
    // TODO
    //srand ( time(NULL) ); //initialize the random seed

    // TODO: Initialize nogood table
    int layer = 0;

    // TODO: Fixed point iteration
    expand(problem);
    expand(problem);
    expand(problem);
    expand(problem);
    expand(problem);
    expand(problem);
    expand(problem);
    layer = 7;

    std::list<std::list<int>> p;
    std::list<int> goal(problem->goalPropositions.begin(),
            problem->goalPropositions.end());

    int success = extract(problem, goal, layer, p);

    /* 
    while(!success) {
        layer++;
        expand(problem);
        std::cout << "=======" << std::endl;
        p = *(new std::list<std::list<int>>);
        std::list<int> goal(problem->goalPropositions.begin(),
            problem->goalPropositions.end());
        success = extract(problem, goal, layer, p);

        // TODO:
        if ((!success) && fixedPoint(problem)) {
            // if (TODO) return 0;
            // TODO
        }
    }
    */
    

    plan = p;
    return 1;
}

void expand(Problem *problem) {
    std::cout << std::endl << "Expanding graph" << std::endl;
    
    // Note that we always generate an action layer number (i) and a prop layer (i+1)
    int currentPropLayer = problem->lastPropIndices.size();
    int nextPropLayer = currentPropLayer + 1;
    int nextActionLayer = currentPropLayer;

    // Copy last proposition and action indices for next layer
    int currentLastPropIndex, currentLastActionIndex;
    currentLastPropIndex = problem->lastPropIndices.back();
    if (!problem->lastActionIndices.empty()) {
        currentLastActionIndex = problem->lastActionIndices.back();
    } else{
        currentLastActionIndex = -1;
    }
    problem->lastPropIndices.push_back(currentLastPropIndex);
    problem->lastActionIndices.push_back(currentLastActionIndex);

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
                
                //std::cout << "Action " << problem->actionNames[action];
                //std::cout << " preconditions " << prec << std::endl;

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
                //std::cout << problem->actionNames[action] << " enables " << prop << std::endl;
                addedProps.push_back(prop);
            }

            // Update action mutexes: TODO: Need to move outside?
            // Iterate other new actions in this layer
            //for (int i = currentLastActionIndex;
            for (int i = 0;
                    i < problem->lastActionIndices.back(); i++) {
                int action2 = problem->layerActions[i];
                
                if (checkActionsMutex(problem, action, action2) ||
                        checkActionsMutex(problem, action2, action) ||
                        checkActionPrecsMutex(problem, action, action2, currentPropLayer)) {
                    setActionMutex(problem, action, action2, nextActionLayer);
                }
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

    // Update proposition mutexes
    for (int i = 0; i <= problem->lastPropIndices.back(); i++) {
        int p = problem->layerProps[i];
        for (int j = 0; j < i; j++) {
            int q = problem->layerProps[j];
            if (checkPropsMutex(problem, p, q, nextActionLayer)) {
                setPropMutex(problem, p, q, nextPropLayer);
            }
        }
    }


    // debug:
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
}


/**
 * Checks if two propositions will be mutex in the proposition layer following
 * the given action layer
 */
int checkPropsMutex(Problem *problem, int p, int q, int actionLayer) {
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
int checkActionsMutex(Problem *problem, int a, int b) {
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
int checkActionPrecsMutex(Problem *problem, int a, int b, int layer) {
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


int extract(Problem *problem, std::list<int> goal, int layer,
        std::list<std::list<int>>& plan) {
    std::cout << "Extracting" << std::endl;

    for (int g : goal) {
        std::cout << g << ", ";
    }
    std::cout << std::endl;

    // Trivial success
    if (layer == 0) {
        return 1;
    }

    // This sub-goal has failed before
    // TODO: share with other threads (receive)
    if (isNogood(layer, goal)) return 0;

    // Perform the graphplan search
    std::list<int> actions;
    int success = gpSearch(problem, goal, actions, layer, plan);

    // Plan found
    if (success) {
        return 1;
    }

    // Update nogoods and fail
    // TODO: share with other threads (send)
    addNogood(layer, goal);
    return 0;
}

int gpSearch(Problem *problem, std::list<int> goal, std::list<int> actions,
        int layer, std::list<std::list<int>>& plan) {
    std::cout << "Performing gpSearch" << std::endl;

    for (int g : goal) {
        std::cout << problem->propNames[g] << ", ";
    }
    std::cout << std::endl;

    // All actions already chosen
    if (goal.empty()) {
        // Extract plan for preconditions of chosen actions
        std::list<int> preconds;
        //for (std::vector<int>::size_type i = 0; i < actions.size(); i++) {
        for (int a : actions) {
            //int a = actions[i];
            int first = problem->actionPrecIndices[a];
            int last = problem->actionPrecIndices[a+1] - 1;
            for (std::vector<int>::size_type j = first; j <= last; j++) {
                // TODO: Duplicate detection ?
                preconds.push_back(problem->actionPrecEdges[j]);
            }
        }

        // Try satisfying preconditions in previous layer
        int success = extract(problem, preconds, layer-1, plan);
        if (!success) return 0;
        
        // Plan found, add actions to plan and return success
        plan.push_back(actions);
        return 1;
    }

    // TODO: select one
    int p = goal.front();

    // Get providers (actions) of p
    std::list<int> providers;
    for (int provider : problem->propPosActions[p]) {
        // Check if provider is enabled
        if (!(problem->actionEnabled[provider]) || problem->actionFirstLayer[provider] > layer) continue;

        // Check if provider is mutex with any action
        bool mut = false;
        for (int act : actions) {
            if ((act == 15 && provider == 7) || (act == 7 && provider == 15)) {
        std::cout << problem->actionNames[15] << " x " << problem->actionNames[7] << std::endl;
        std::cout << "XX " << problem->actionMutexes[15*problem->countActions + 7] << std::endl;
        std::cout << "XX " << problem->actionMutexes[7*problem->countActions + 15] << std::endl;
        std::cout << "l" << layer << std::endl;
            }

            // TODO:                                         v off by one?
            if (getActionMutex(problem, provider, act, layer+1)) {
                std::cout << "ACTION MUTEX " << provider << " " << act << std::endl;
                mut = true;
                break;
            }
        }

        // Add provider
        if (!mut) {
            providers.push_back(provider);
        }
    }

    // No providers for goal => no plan
    if (providers.empty()) return 0;
    
    // TODO: choose one
    // Add a providing action
    int a = providers.front();

    /*
    int a; 
    int randIndex = rand() % providers.size();
    std::cout << randIndex << std::endl;
    int counter;
    for (int i : providers) {
        if (counter == randIndex) {
            a = i;
            break;
        }
        counter++;
    }
    */
    actions.push_back(a);

    
    // Remove action effects from goal list
    int first = problem->actionPosEffIndices[a];
    int last = problem->actionPosEffIndices[a+1] - 1;
    for (std::vector<int>::size_type i = first; i <= last; i++) {
        goal.remove(problem->actionPosEffEdges[i]);
    }

    // Call recursively
    return gpSearch(problem, goal, actions, layer, plan);
}

