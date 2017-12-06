#include <iostream>

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
    // TODO: Initialize nogood table
    int layer = 0;

    // TODO: Fixed point iteration

    std::list<std::list<int>> p;
    std::list<int> goal(problem->goalPropositions.begin(), problem->goalPropositions.end());

    int success = extract(problem, goal, layer, p);
    while(!success) {
        layer++;
        expand(problem);
        p = *(new std::list<std::list<int>>);
        success = extract(problem, goal, layer, p);

        // TODO:
        if ((!success) && fixedPoint(problem)) {
            // if (TODO) return 0;
            // TODO
        }
    }

    plan = p;
    return 1;
}

void expand(Problem *problem) {
    std::cout << "Expanding graph" << std::endl;
    
}

int extract(Problem *problem, std::list<int> goal, int layer, std::list<std::list<int>>& plan) {
    std::cout << "Extracting" << std::endl;

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

int gpSearch(Problem *problem, std::list<int> goal, std::list<int> actions, int layer, std::list<std::list<int>>& plan) {
    std::cout << "Performing gpSearch" << std::endl;

    // All actions already chosen
    if (goal.empty()) {
        // Extract plan for preconditions of chosen actions
        std::list<int> preconds;
        //for (std::vector<int>::size_type i = 0; i < actions.size(); i++) {
        for (int a : actions) {
            //int a = actions[i];
            // TODO: adjacency arrays start or end at specified index?
            int first = problem->actionPrecIndices[a];
            int last = problem->actionPrecIndices[a+1] - 1;
            for (std::vector<int>::size_type j = first; j < last; j++) {
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
        // Check if provider is mutex with any action
        bool mut = false;
        for (int act : actions) {
            if (getActionMutex(problem, provider, act) >= layer) {
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
    actions.push_back(a);
    
    // Remove action effects from goal list
    int first = problem->actionPosEffIndices[a];
    int last = problem->actionPosEffIndices[a+1] - 1;
    for (std::vector<int>::size_type i = first; i < last; i++) {
        goal.remove(problem->actionPosEffEdges[i]);
    }

    // Call recursively
    return gpSearch(problem, goal, actions, layer, plan);
}

