#ifndef _PLANNER_H
#define _PLANNER_H

#include <vector>
#include <list>

#include "common.h"
#include "IPlanningProblem.h"
#include "Plan.h"


#define NOGOOD_SEPARATOR -1


/**
 * Main Planner class that implements the Graphplan algorithm in parallel.
 *
 * Author: Patrick Hegemann
 */
class Planner {
    public:
        Planner(IPlanningProblem *problem);
        int graphplan(Plan& plan);

    private:
        IPlanningProblem *problem;

        // Is the fixed point reached?
        int fixedPoint = 0;
        // TODO: Only needed if expanding beyond fixed point
        //int fixedMutexes = 0;

        // Nogoods
        // Count of nogoods per Layer
        std::vector<int> countNogoods;
        // Representation of nogoods is in this case a vector of vectors for
        // easier sharing between planner threads
        std::vector<std::vector<int>> nogoods;

        // Check if the fixed point in the planning graph is reached
        int checkFixedPoint();
        // Check if the goal is unreachable or goal propositions are mutex
        int checkGoalUnreachable();

        // Expand the planning graph by one (action & proposition) layer
        void expand();
        // Updates the action mutexes of a layer
        void updateActionLayerMutexes(int prevPropLayer, int actionLayer);
        // Updates the proposition mutexes of a layer
        void updatePropLayerMutexes(int newPropLayer, int actionLayer);
        // Checks if two propositions will be mutex in the given layer
        int checkPropsMutex(Proposition p, Proposition q, int actionLayer);
        // Checks if effects of two actions collide
        int checkActionsMutex(Action a, Action b);
        // Checks if preconditions of actions are mutex in the given layer
        int checkActionPrecsMutex(Action a, Action b, int propLayer);
        
        // Extract a plan for the given goal, starting at the specified layer
        // Calls gpSearch recursively
        int extract(std::list<Proposition> goal, int layer, Plan& plan);
        // Looks for actions to take in order to achieve the given goal
        // Calls itself and extract recursively
        int gpSearch(std::list<Proposition> goal, std::list<Action> actions, int layer, Plan& plan);

        // Returns if the given combination of propositions is a nogood at the specified layer
        int isNogood(int layer, std::list<Proposition> props);
        // Set the given combination of propositions as a new nogood at the specified layer
        void addNogood(int layer, std::list<Proposition> props);
};

#endif

