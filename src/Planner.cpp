#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>

#include "Planner.h"
#include "Logger.h"


Planner::Planner(IPlanningProblem *problem) {
    this->problem = problem;
}

int Planner::isNogood(int layer, std::list<Proposition> props) {
    // No nogoods added for specified layer -> can't be a nogood
    if (nogoods.size() <= (unsigned int) layer) return false;

    unsigned int propsFoundInNogood = 0;

    for (unsigned int i = 0; i < nogoods[layer].size(); i++) {
        int variable = nogoods[layer][2*i];
        int value = nogoods[layer][2*i+1];
        Proposition p(variable, value);

        // Next nogood -> reset
        if (variable == 0) {
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

void Planner::addNogood(int layer, std::list<Proposition> props) {
    //TODO: Proper logging
    /*
    std::cout << "New nogood in layer " << layer << ": ";
    for (int p : props) {
        std::cout << p << ", ";
    }
    std::cout << std::endl;
    */

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

/**
 * Check if a fixed point in the planning graph is reached
 */
int Planner::checkFixedPoint() {
    int lastLayer = problem->getLastLayer();

    // Checking only makes sense if we have at least two existing layers.
    if (lastLayer < 2) return false;

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
    std::list<Proposition> problemGoal = problem->getGoal();

    // Check if all goals are enabled already. If not, return true.
    for (Proposition goal : problemGoal) {
        if (!problem->isPropEnabled(goal)) {
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
    return false;
}


int Planner::graphplan(Plan& plan) {
    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable()) {
        //layer++;
        expand();
        fixedPoint = fixedPoint || checkFixedPoint();
    }
    // If goal is impossible to reach, this problem has no solution
    if (checkGoalUnreachable()) {
        log(1, "Goal unreachable, aborting\n");
        return false;
    }

    // Do backwards search with given goal propositions
    std::list<Proposition> goal(problem->getGoal().begin(), problem->getGoal().end());
    int success = extract(goal, problem->getLastLayer(), plan);

    // Keep track of how many nogoods exist, so we can determine if any are added during an iteration
    int lastNogoodCount = 0;

    while(!success) {
        // Expand for one more layer
        // TODO: Proper logging -- std::cout << "Layer " << layer << std::endl;
        if (!fixedPoint) {
            //layer++;
            expand();
            fixedPoint = checkFixedPoint();
        }

        // Clean up, start with a fresh empty plan
        plan.clear();

        // Do backwards search with given goal propositions
        std::list<Proposition> goal(problem->getGoal().begin(), problem->getGoal().end());
        int success = extract(goal, problem->getLastLayer(), plan);

        if ((!success) && fixedPoint) {
            if (lastNogoodCount == countNogoods[problem->getLastLayer()]) {
                return 0;
            }
            lastNogoodCount = countNogoods[problem->getLastLayer()];
        }
    }

    // TODO: Proper logging -- std::cout << "Final number of layers: " << layer << std::endl;
    return success;
}

void Planner::expand() {
    log(2, "Expanding graph\n");
    
    int lastPropositionLayer = problem->getLastLayer();

    int newPropositionLayer = problem->addPropositionLayer();
    int newActionLayer = problem->addActionLayer();

    /* TODO: Move to planning problem class
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
    */

    /* TODO: Possibly remove. Still useful if giving up optimality, i.e.
     * expanding beyond the fixed point
     * /
    // If we reached a fixed point we don't need to do anything else.
    // No adding of actions or propositions needed, since those will be carried
    // over implicitly.
    if (fixedPoint && !fixedMutexes) {
        updateNewLayerMutexes(currentPropLayer);
        return;
    }
    */

    // No nogoods for this layer yet
    countNogoods.push_back(0);

    /* TODO: Move to planning problem class
    // Add a new entry in the layerPropMutexCount vector to see if we reached a
    // fixed-point level
    problem->layerPropMutexCount.push_back(0);
    */

    std::list<Proposition> addedProps;

    // Add actions
    // TODO: Not a very clean loop
    for(Action action = 0; action < problem->getActionCount(); action++) {
        // Only check disabled actions
        // TODO: Potential for optimization: use a list of unused actions
        if (problem->isActionEnabled(action)) continue;

        bool enable = true;

        // Check if preconditions already present and abort if not
        std::list<Proposition> preconds = problem->getActionPreconditions(action);
        for (Proposition p : preconds) {
            if (!problem->isPropEnabled(p)) {
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
        /* TODO: Move to planning problem class
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
        */
    }

    // Add propositions
    for (Proposition prop : addedProps) {
        problem->activateProposition(prop, newPropositionLayer);
        /* TODO: Move to planning problem class
        if (!problem->propEnabled[prop]) {
            problem->lastPropIndices.back()++;
            problem->layerProps[problem->lastPropIndices.back()] = prop;
            problem->propEnabled[prop] = 1;
        }
        */
    }

    updateActionLayerMutexes(lastPropositionLayer, newActionLayer);
    updatePropLayerMutexes(newPropositionLayer, newActionLayer);
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
            if (a == b) break;
            if (checkActionsMutex(a, b)
                    || checkActionsMutex(b, a)
                    || checkActionPrecsMutex(a, b, prevPropLayer)) {
                // Set a new mutex
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
            if (problem->isActionEnabled(a) && problem->isActionEnabled(b) &&
                    !(problem->isMutexAction(a, b, actionLayer))) {
                // Two non-mutex actions exist which resp. enable p and q
                return false;
            }
        }
    }

    return true;
}

/**
 * Checks if two actions are mutex due to colliding effects
 * Has to be called twice: once with actions (a,b), then with (b,a).
 */
int Planner::checkActionsMutex(Action a, Action b) {
    // Iterate the action's negative effects
    for (Proposition neg : problem->getActionNegEffects(a)) {
        // Check if negative effects collide with preconditions
        for (Proposition prec : problem->getActionPreconditions(b)) {
            if (neg == prec) return true;
        }

        // Check if negative effects collide with positive effects
        for (Proposition pos : problem->getActionPosEffects(b)) {
            if (neg == pos) return true;
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
            if (problem->isMutexProp(p, q, propLayer)) {
                return true;
            }
        }
    }

    return false;
}


int Planner::extract(std::list<Proposition> goal, int layer, Plan& plan) {
    log(2, "Extracting\n");
    /*
    if (verbosityLevelSetting >= 3) {
        for (int g : goal) {
            //log(2, "%d, ", g);
            std::cout << g << ", ";
        }
        //log(2, "in layer %d\n", layer);
        std::cout << "in layer " << layer << std::endl;
    } else {
        std::cout << std::endl;
    }
    */
    
    // Trivial success
    if (layer == 0) {
        return 1;
    }

    // This sub-goal has failed before
    // TODO: share with other threads (receive)
    if (isNogood(layer, goal)) return 0;

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
    log(2, "Performing gpSearch\n");

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
        // First, get all preconditions into one list
        std::list<Proposition> preconds;
        for (Action a : actions) {
            // TODO: Duplicate detection ?
            std::list<Proposition> ap = problem->getActionPreconditions(a);
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
        // Check if provider is enabled and if not, skip provider
        if (!(problem->isActionEnabled(provider))
                || problem->getActionFirstLayer(provider) > layer) {
            continue;
        }

        // Check if provider is mutex with any already chosen action
        bool mut = false;
        for (Action act : actions) {
            if (problem->isMutexAction(provider, act, layer)) {
                mut = true;
                break;
            }
        }

        // Add provider to the list
        if (!mut) {
            // Small hack: Add trivial actions to the back and others to the front, so they'd get chosen first
            if (problem->isTrivialAction(provider)) {
                providers.push_back(provider);
            } else {
                providers.push_front(provider);
            }
        }
    }


    // No providers for goal => no plan
    if (providers.empty()) return 0;
    
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
        for (Proposition posEff : problem->getActionPosEffects(provider)) {
            newGoal.remove(posEff);
        }

        // Call recursively
        int success = gpSearch(newGoal, newActions, layer, plan);

        if (success) return 1;
    }

    return 0;
}

