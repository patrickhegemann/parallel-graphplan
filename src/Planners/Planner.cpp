#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>
#include <set>
#include <list>

#include "Planners/Planner.h"
#include "Logger.h"
#include "Settings.h"
#include "pgp_utility.h"


Planner::Planner(IPlanningProblem *problem) {
    this->problem = problem;
}




int Planner::isNogood(int layer, std::list<Proposition> props) {
    log(2, "Checking for nogood in layer %d\n", layer);
    for (Proposition prop : props) {
        log(4, "\t%d = %d\n", prop.first, prop.second);
    }

    std::set<Proposition> propSet(props.begin(), props.end());

    // No nogoods added for specified layer -> can't be a nogood
    if (nogoods.size() <= (unsigned int) layer) return false;

    unsigned int propsFoundInNogood = 0;

    int resetState = 0;

    for (unsigned int i = 0; i < nogoods[layer].size(); i+=2) {
        int variable = nogoods[layer][i];
        int value = nogoods[layer][i+1];

        if ((variable != -1 || value != -1) && resetState) continue;
        resetState = 0;

        Proposition p(variable, value);

        // Next nogood -> reset
        if (variable == NOGOOD_SEPARATOR) {
            // Exactly the propositions found in this nogood
            if (propsFoundInNogood >= propSet.size()) {
                return true;
            }
            propsFoundInNogood = 0;
            continue;
        }

        // Check if the proposition is one of our wanted propositions
        if (std::find(propSet.begin(), propSet.end(), p) != propSet.end()) {
            propsFoundInNogood++;
        } else {
            // Otherwise reset
            propsFoundInNogood = 0;
            resetState = 1;
            continue;
        }
    }

    return false;
}

/*
int Planner::isNogood(int layer, std::list<Proposition> props) {
    if (nogoods.size() <= (unsigned int) layer) return false;

    Proposition p(-1,-1), q(-1,-1);
    Proposition *lastActualProp = &p;
    Proposition *lastSearchedProp = &q;

    for (std::list<Proposition>& nogood : nogoods[layer]) {
        auto i = nogood.begin();
        auto j = props.begin();

        while (i != nogood.end() || j != props.end()) {
            
            if ((i == nogood.end() && *j != *lastSearchedProp) || (j == props.end() && *i != *lastActualProp)) {
                break;
            }
            
            if (*i == *lastActualProp) {
                *lastActualProp = *i;
                ++i;
            } else if (*j == *lastSearchedProp) {
                *lastSearchedProp = *j;
                ++j;
            } else if (*i == *j) {
                *lastActualProp = *i;
                *lastSearchedProp = *j;
                ++i;
                ++j;
            } else {
                // *i != *j and no duplicates
                // -> check next nogood
                break;
            }

        }

        if (i == nogood.end() && j == props.end()) {
            return true;
        }
    }

    return false;
}
*/

/*

void Planner::addNogood(int layer, std::list<Proposition> props) {
    log(2, "Adding a nogood in layer %d\n", layer);

    props.sort();

    // Add vectors to the nogood table until we have sufficiently many layers
    while (nogoods.size() <= (unsigned int) layer) {
        nogoods.push_back(std::list<std::list<Proposition>>());
    }

    nogoods[layer].push_back(props);

    countNogoods[layer]++;    
}
*/


void Planner::addNogood(int layer, std::list<Proposition> props) {
    log(2, "Adding a nogood in layer %d\n", layer);

    // Add vectors to the nogood table until we have sufficiently many layers
    while (nogoods.size() <= (unsigned int) layer) {
        nogoods.push_back(std::vector<int>());
    }

    // Add the nogood to the specified layer
    for (Proposition p : props) {
        nogoods[layer].push_back(p.first);
        nogoods[layer].push_back(p.second);
    }

    // Push two nogood separators so they are always aligned to two ints
    nogoods[layer].push_back(NOGOOD_SEPARATOR);
    nogoods[layer].push_back(NOGOOD_SEPARATOR);

    countNogoods[layer]++;
}



void Planner::dumpNogoods() {
    int layer = 0;
    for (auto n : nogoods) {
        log(0, "Layer %d: ", layer);
        for (auto x : n) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
        layer++;
    }
}


/**
 * Check if a fixed point in the planning graph is reached
 */
int Planner::checkFixedPoint() {
    log(4, "Checking for fixed point\n");

    int lastLayer = problem->getLastLayer();

    // Checking only makes sense if we have at least two existing layers.
    if (lastLayer - problem->getFirstLayer() < 1) return false;

    log(4, "last prop#: %d, before that: %d, last mutex#: %d, before that: %d\n",
            problem->getLayerPropositions(lastLayer).size(),
            problem->getLayerPropositions(lastLayer-1).size(),
            problem->getPropMutexCount(lastLayer),
            problem->getPropMutexCount(lastLayer-1)
        );

    // If the last two layers have equal amounts of propositions and proposition
    // mutexes then we have reached a fixed point.
    if (problem->getLayerPropositions(lastLayer).size() == problem->getLayerPropositions(lastLayer-1).size()
            && problem->getPropMutexCount(lastLayer) == problem->getPropMutexCount(lastLayer-1)) {
        log(1, "Fixed point reached\n");
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
int Planner::checkGoalUnreachable() {
    log(4, "Checking if goal is unreachable\n");

    std::list<Proposition> problemGoal = problem->getGoal();

    // Check if all goals are enabled already. If not, return true.
    for (Proposition goal : problemGoal) {
        log(4, "Checking if %s is unreachable\n", problem->getPropositionName(goal).c_str());
        if (!problem->isPropEnabled(goal, problem->getLastLayer())) {
            log(2, "Not all goal propositions enabled yet\n");
            return true;
        }
    }

    // Check if any pair of goals is mutex in the current layer. If so, return true.
    for (Proposition goal1 : problemGoal) {
        for (Proposition goal2 : problemGoal) {
            if (goal1 == goal2) break;
            if (problem->isMutexProp(goal1, goal2, problem->getLastLayer())) {
                log(2, "Pair of goals still mutex: %d, %d\n", goal1, goal2);
                return true;
            }
        }
    }

    // None of the above are true -> Goal reachable
    log(4, "Goal is reachable.\n");
    return false;
}


int Planner::graphplan(Plan& plan) {
    log(4, "Entering graphplan algorithm\n");

    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable()) {
        expand();
        fixedPoint = fixedPoint || checkFixedPoint();
    }

    // If goal is impossible to reach, this problem has no solution
    if (checkGoalUnreachable()) {
        log(1, "Goal unreachable, aborting\n");
        return false;
    }

    // Do backwards search with given goal propositions
    log(4, "Preparing goal list\n");
    std::list<Proposition> goal = problem->getGoal();
    log(4, "Calling first extract\n");
    // If fixed point is reached, we have theoretically expanded beyond it, just to find out.
    // So we subtract that additional layer again
    int lastLayer = problem->getLastLayer();
    //if (fixedPoint) lastLayer--;
    int success = extract(goal, lastLayer, plan);

    // Keep track of how many nogoods exist, so we can determine if any are added during an iteration
    int lastNogoodCount = 0;

    while(!success) {
        // Expand for one more layer
        expand();
        fixedPoint = checkFixedPoint();

        // Clean up, start with a fresh empty plan
        plan.clear();

        // Do backwards search with given goal propositions
        lastLayer = problem->getLastLayer();
        //if (fixedPoint) lastLayer--;    // about the -1 see above
        success = extract(goal, lastLayer, plan);

        if ((!success) && fixedPoint) {
            if (lastNogoodCount == countNogoods[problem->getLastLayer()]) {
                return 0;
            }
            lastNogoodCount = countNogoods[problem->getLastLayer()];
        }
    }

    return success;
}

void Planner::expand() {
    log(0, "Expanding graph\n");

    int lastPropositionLayer = problem->getLastLayer();

    int newPropositionLayer = problem->addPropositionLayer();
    log(4, "New proposition layer is %d\n", newPropositionLayer);
    int newActionLayer = problem->addActionLayer();
    log(4, "New action layer is %d\n", newActionLayer);

    // TODO: Possibly remove. Still useful if giving up optimality
    // If we reached a fixed point we don't need to do anything else.
    // No adding of actions or propositions needed, since those will be carried
    // over implicitly.
    //if (fixedPoint && !fixedMutexes) {
    //    updateNewLayerMutexes(currentPropLayer);
    //    return;
    //}

    // No nogoods for this layer yet
    countNogoods.push_back(0);

    // Add actions
    // TODO: Not a very clean loop, use a list of unused actions instead
    for(Action action = 0; action < problem->getActionCount(); action++) {
        // Only check disabled actions
        if (problem->isActionEnabled(action, newActionLayer-1)) continue;

        bool enable = true;

        // Check if preconditions already present and abort if not
        std::list<Proposition> preconds = problem->getActionPreconditions(action);
        for (Proposition p : preconds) {
            if (!problem->isPropEnabled(p, lastPropositionLayer)) {
                enable = false;
                break;
            }

            // Check for precondition mutexes and abort if mutex was found
            for (Proposition q : preconds) {
                if (p == q) break;
                if (problem->isMutexProp(p, q, lastPropositionLayer)) {
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
        problem->activateAction(action, newActionLayer);
        // Check for general mutexes that are independent on the layer
        for (Action b : problem->getLayerActions(newActionLayer)) {
            if (checkActionsMutex(action, b)) {
                problem->setMutexAction(action, b, INT_MAX);
            }
        }
    }

    updateActionLayerMutexes(lastPropositionLayer, newActionLayer);
    updatePropLayerMutexes(newPropositionLayer, newActionLayer);

    log(0, "Done expanding graph\n");

    if (settings->getDumpPlanningGraph()) {
        problem->dumpPlanningGraph();
    }
}

/**
 * Updates the mutexes of the specified action layer.
 *
 * @param prevPropLayer
 *      Previous proposition layer, ie the one right before the new action layer
 *
 * @param actionLayer
 *      The action that is to be updated
 *
 * // TODO: The following feature is only useful when expanding beyond the fixed point
 * // If a fixed point is reached, the mutexes will be added (one last time) for every layer.
 * // Then fixedMutexes will be set to 1, which means this function _should_ never be called
 * // again.
 * 
 */
void Planner::updateActionLayerMutexes(int prevPropLayer, int actionLayer) {
    // TODO: See above
    // If we reached a fixed point then all current mutexes will be there forever.
    /*
    if (fixedPoint) {
        newMutexActionLayer = INT_MAX;
        newMutexPropLayer = INT_MAX;
        fixedMutexes = 1;
    }
    */

    // Perform various checks for each pair of actions present
    std::list<Action> actions = problem->getLayerActions(actionLayer);
    for (Action a : actions) {
        for (Action b : actions) {
            if (a == b || problem->isMutexAction(a, b, actionLayer)) break;
            if (checkActionPrecsMutex(a, b, prevPropLayer)) {
                problem->setMutexAction(a, b, actionLayer);
            }
        }
    }
}


/**
 * Updates the mutexes of the specified proposition layer.
 *
 * @param newPropLayer
 *      The proposition layer that is to be updated
 *
 * @param actionLayer
 *      The last action layer, ie the one right before the new proposition layer
 */
void Planner::updatePropLayerMutexes(int newPropLayer, int actionLayer) {
    // Update proposition mutexes
    std::list<Proposition> props = problem->getLayerPropositions(newPropLayer);
    for (Proposition p : props) {
        for (Proposition q : props) {
            if (p == q) break;
            if (checkPropsMutex(p, q, actionLayer)) {
                // Set a new mutex
                problem->setMutexProp(p, q, newPropLayer);
            }
        }
    }
}


/**
 * Checks if two propositions will be mutex in the proposition layer following
 * the given action layer
 */
int Planner::checkPropsMutex(Proposition p, Proposition q, int actionLayer) {
    // Iterate over all pairs of p's and q's preconditions
    for (int a : problem->getPropPosActions(p)) {
        for (int b : problem->getPropPosActions(q)) {
            if (problem->isActionEnabled(a, actionLayer)
                    && problem->isActionEnabled(b, actionLayer)
                    && !(problem->isMutexAction(a, b, actionLayer))) {
                // Two non-mutex actions exist which resp. enable p and q
                return false;
            }
        }
    }

    return true;
}

/**
 * Checks if two actions are mutex due to colliding effects
 */
int Planner::checkActionsMutex(Action a, Action b) {
    auto& precA = problem->getActionPreconditions(a);
    auto& posA = problem->getActionPosEffects(a);
    auto& negA = problem->getActionNegEffects(a);
    auto& precB = problem->getActionPreconditions(b);
    auto& posB = problem->getActionPosEffects(b);
    auto& negB = problem->getActionNegEffects(b);

    if (!empty_intersection(negA, precB) || !empty_intersection(negA, posB)
            || !empty_intersection(negB, precA) || !empty_intersection(negB, posA)) {
        return true;
    }

    // Check if positive effects coĺlide
    for (Proposition p : posA) {
        for (Proposition q : posB) {
            if (problem->isMutexProp(p, q, INT_MAX)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Checks if the preconditions of two actions are mutex in the given layer
 *
 * @param layer
 *      The proposition layer where the preconditions of the actions are in
 */
int Planner::checkActionPrecsMutex(int a, int b, int propLayer) {
    for (Proposition p : problem->getActionPreconditions(a)) {
        for (Proposition q : problem->getActionPreconditions(b)) {
            if (p == q) continue;
            if (problem->isMutexProp(p, q, propLayer)) {
                return true;
            }
        }
    }

    return false;
}


int Planner::extract(std::list<Proposition> goal, int layer, Plan& plan) {
    log(2, "Extracting in layer %d\n", layer);

    for (Proposition g : goal) {
        log(4, "\t%s\n", problem->getPropositionName(g).c_str());
        //std::cout << "(" << g.first << "," << g.second << "), ";
    }
    //std::cout << "in layer " << layer << std::endl;

    // Trivial success
    if (layer == problem->getFirstLayer()) {
        return 1;
    }

    // This sub-goal has failed before
    // TODO: share with other threads (receive)
    if (isNogood(layer, goal)) {
        log(3, "Nogood found\n");
        return 0;
    }

    // Perform the graphplan search
    std::list<Action> actions;
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

int Planner::gpSearch(std::list<Proposition> goal, std::list<Action> actions, int layer, Plan& plan) {
    log(2, "Performing gpSearch %d\n", goal.size());

    int actionLayer = problem->getActionLayerBeforePropLayer(layer);

    for (auto g : goal) {
        //std::cout << problem->propNames[g] << ", ";
        std::cout << "(" << g.first << "," << g.second << ") " << problem->getPropositionName(g) << ", ";
    }
    std::cout << std::endl;
    
    
    // All actions already chosen
    if (goal.empty()) {
        // Extract plan for preconditions of chosen actions
        // First, get all preconditions into one list
        std::list<Proposition> preconds;
        for (Action a : actions) {
            // TODO: Duplicate detection ?
            std::list<Proposition>& ap = problem->getActionPreconditions(a);
            preconds.insert(preconds.end(), ap.begin(), ap.end());
        }

        // Try satisfying preconditions in previous layer
        int success = extract(preconds, layer-1, plan);
        if (!success) return 0;
        
        // Plan found, add actions to plan and return success
        plan.addLayer(actions);
        return 1;
    }


    // TODO: select one possibly in a different way
    Proposition nextProp = goal.front();

    // Get providers (actions) of p
    std::list<Action> providers;
    for (Action provider : problem->getPropPosActions(nextProp)) {
        log(2, "ppa %s\n", problem->getActionName(provider).c_str());
        // Check if provider is enabled and if not, skip provider
        if (!(problem->isActionEnabled(provider, actionLayer))
                || problem->getActionFirstLayer(provider) > actionLayer) {
            continue;
        }

        // Check if provider is mutex with any already chosen action
        bool mut = false;
        for (Action act : actions) {
            if (problem->isMutexAction(provider, act, actionLayer)) {
                mut = true;
                break;
            }
        }

        // Add provider to the list
        if (!mut) {
            // Small hack: Add trivial actions to the back and others to the front, so they'd get chosen first
            log(2, "doot\n");
            if (problem->isTrivialAction(provider)) {
                providers.push_back(provider);
            } else {
                providers.push_front(provider);
            }
        }
    }

    log(2, "size %d\n", providers.size());


    // No providers for goal => no plan
    if (providers.size() == 0) {
        return 0;
    }
    
    // Add a providing action
    // TODO: Add some different variants (possibly with heuristics) to choose one provider
    /* Select provider for chosen proposition. Non-deterministic choice point (=> Backtrack here)
     * Simple method: Just try every provider one after another
     * TODO: Parallelize here
     */
    for (Action provider : providers) {
        // Copy goal and action list for next recursive call
        std::list<Proposition> newGoal(goal);
        std::list<Action> newActions(actions);

        // Add action to action list
        newActions.push_back(provider);

        // Remove action effects from goal list
        log(2, "action: %s\n", problem->getActionName(provider).c_str());
        for (Proposition posEff : problem->getActionPosEffects(provider)) {
            log(2, "NOOT %d,%d %s\n", posEff.first, posEff.second, problem->getPropositionName(posEff).c_str());
            newGoal.remove(posEff);
        }

        // Call recursively
        int success = gpSearch(newGoal, newActions, layer, plan);

        if (success) return 1;
    }

    return 0;
}

