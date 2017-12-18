#include <list>
#include <set>
#include <iostream>
#include <algorithm>

#include "planVerifier.h"
#include "problem.h"


PlanVerifier::PlanVerifier(Problem *problem, std::list<std::list<int>> plan) {
    this->problem = problem;
    this->plan = plan;

    for (int i = 0; i <= problem->lastPropIndices.front(); i++) {
        state.insert(problem->layerProps[i]);
    }
}

/**
 * Checks if the given plan is a valid solution to the problem by simulating
 * the given plan while checking if all steps and actions are valid and then
 * checking if the goal conditions are met.
 */
int PlanVerifier::verify() {
    for (std::list<int> step : plan) {
        if (!simulateStep(step)) return false;
    }

    return checkGoal();
}

/**
 * Simulates a step of the plan
 */
int PlanVerifier::simulateStep(std::list<int> step) {
    std::list<int> adding;
    std::list<int> removing;

    for (int action : step) {
        // Check requirements (iterate preconditions and return false if not met)
        int first = problem->actionPrecIndices[action];
        int last = problem->actionPrecIndices[action+1] - 1;
        for (int i = first; i <= last; i++) {
            int prec = problem->actionPrecEdges[i];
            // Return false if precondition not met
            if (state.count(prec) == 0) return false;
        }
       
        // Add positive effects
        first = problem->actionPosEffIndices[action];
        last = problem->actionPosEffIndices[action+1] - 1;
        for (int i = first; i <= last; i++) {
            int posEff = problem->actionPosEffEdges[i];
            // Return false if positive effect is also negative effect of other action
            if (std::find(removing.begin(), removing.end(), posEff) != removing.end()) return false;
            adding.push_back(posEff);
        }
        
        // Remove negative effects
        first = problem->actionNegEffIndices[action];
        last = problem->actionNegEffIndices[action+1] - 1;
        for (int i = first; i <= last; i++) {
            int negEff = problem->actionNegEffEdges[i];
            // Return false if negative effect is also positive effect of other action
            if (std::find(adding.begin(), adding.end(), negEff) != adding.end()) return false;
            removing.push_back(negEff);
        }
    }

    // Applying effects of actions to state
    for (int a : adding) {
        state.insert(a);
    }
    for (int r : removing) {
        state.erase(r);
    }

    return true;
}


/**
 * Checks if the current state contains the goal state of the problem
 */
int PlanVerifier::checkGoal() {
    for (unsigned int i = 0; i < problem->goalPropositions.size(); i++) {
        if (state.count(problem->goalPropositions[i]) == 0) return false;
    }
    return true;
}

