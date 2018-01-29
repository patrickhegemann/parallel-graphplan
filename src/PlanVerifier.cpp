#include <list>
#include <set>
#include <iostream>
#include <algorithm>

#include "PlanVerifier.h"


PlanVerifier::PlanVerifier(IPlanningProblem *problem, Plan plan) {
    this->problem = problem;
    this->plan = plan;
    
    std::list<Proposition> initialState = problem->getLayerPropositions(0);
    for (Proposition p : initialState) {
        state.insert(p);
    }
}

/**
 * Checks if the given plan is a valid solution to the problem by simulating
 * the given plan while checking if all steps and actions are valid and then
 * checking if the goal conditions are met.
 */
int PlanVerifier::verify() {
    for (int layerNumber = 0; layerNumber < plan.getLayerCount(); layerNumber++) {
        std::list<Action> layer = plan.getLayerActions(layerNumber);
        if (!simulateStep(layer)) return false;
    }

    return checkGoal();
}

/**
 * Simulates a step of the plan
 */
int PlanVerifier::simulateStep(std::list<int> step) {
    std::list<Proposition> adding;
    std::list<Proposition> removing;

    for (Action action : step) {
        // Check preconditions and return false if not met
        for (Proposition precondition : problem->getActionPreconditions(action)) {
            if (state.count(precondition) == 0) return false;
        }
       
        // Add positive effects
        for (Proposition posEff : problem->getActionPosEffects(action)) {
            // Return false if positive effect is also negative effect of other action
            if (std::find(removing.begin(), removing.end(), posEff) != removing.end()) return false;
            adding.push_back(posEff);
        }
        
        // Remove negative effects
        for (Proposition negEff : problem->getActionNegEffects(action)) {
            // Return false if negative effect is also positive effect of other action
            if (std::find(adding.begin(), adding.end(), negEff) != adding.end()) return false;
            removing.push_back(negEff);
        }
    }

    // Applying effects of actions to state
    for (Proposition a : adding) {
        state.insert(a);
    }
    for (Proposition r : removing) {
        state.erase(r);
    }

    return true;
}


/**
 * Checks if the current state contains the goal state of the problem
 */
int PlanVerifier::checkGoal() {
    for (Proposition p : problem->getGoal()) {
        if (state.count(p) == 0) return false;
    }
    return true;
}

