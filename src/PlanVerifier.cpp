#include <list>
#include <set>
#include <iostream>
#include <algorithm>

#include "Logger.h"

#include "PlanVerifier.h"


PlanVerifier::PlanVerifier(IPlanningProblem *problem, Plan plan) {
    this->problem = problem;
    this->plan = plan;
    
    std::list<Proposition> initialState = problem->getLayerPropositions(problem->getFirstLayer());
    for (Proposition p : initialState) {
        log(4, "Inserting %s into initial state\n", problem->getPropositionName(p).c_str());
        state.insert(p);
    }
}

/**
 * Checks if the given plan is a valid solution to the problem by simulating
 * the given plan while checking if all steps and actions are valid and then
 * checking if the goal conditions are met.
 */
int PlanVerifier::verify() {
    //for (int layerNumber = problem->getFirstLayer(); layerNumber <= problem->getLastLayer(); layerNumber++) {
    for (int layerNumber = 0; layerNumber < plan.getLayerCount(); layerNumber++) {
        log(4, "Verifying layer number %d\n", layerNumber+1);
        std::list<Action> layer = plan.getLayerActions(layerNumber);
        for (auto a : layer) {
            log(4, "\t%s\n", problem->getActionName(a).c_str());
        }
        // +1 because of mutex checking, in the real algorithm layer 1 is the first layer, not 0
        if (!simulateStep(layer, problem->getFirstLayer()+layerNumber)) return false;
    }

    return checkGoal();
}

/**
 * Simulates a step of the plan
 */
int PlanVerifier::simulateStep(std::list<int> step, int layerNumber) {
    log(4, "Simulating a step\n");

    std::list<Proposition> adding;
    std::list<Proposition> removing;

    for (Action action : step) {
        // Check preconditions and return false if not met
        for (Proposition precondition : problem->getActionPreconditions(action)) {
            log(4, "Checking for precondition %s\n", problem->getPropositionName(precondition).c_str());
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
        log(4, "Adding proposition \"%s\" (%d,%d)\n",
                problem->getPropositionName(a).c_str(), a.first, a.second);

        // Set the value of a variable to the new value, i.e. erase all propositions that
        // have the variable set to a different value
        for (Proposition b : state) {
            if (b.first == a.first && b.second != a.second) {
                state.erase(b);
            }
        }
        state.insert(a);
    }

    for (Proposition r : removing) {
        log(4, "Removing proposition \"%s\" (%d,%d)\n",
                problem->getPropositionName(r).c_str(), r.first, r.second);
        state.erase(r);
    }

    // Check if any 2 propositions in the state are mutex
    for (Proposition p : state) {
        for (Proposition q : state) {
            if (p == q) break;
            int propLayer = problem->getPropLayerAfterActionLayer(layerNumber);
            if (problem->isMutexProp(p, q, propLayer)) {
                log(4, "Verifier Error: Propositions \"%s\" (%d,%d) and \"%s\" (%d,%d) are mutex in layer %d\n",
                        problem->getPropositionName(p).c_str(), p.first, p.second,
                        problem->getPropositionName(q).c_str(), q.first, q.second,
                        propLayer
                   );
                return false;
            }
        }
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

