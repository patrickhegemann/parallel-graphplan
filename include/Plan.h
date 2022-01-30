#ifndef _PLAN_H
#define _PLAN_H

#include <list>

#include "common.h"


/**
 * Class for representing a plan for a planning problem
 *
 * Author: Patrick Hegemann
 */
class Plan {
    public:
        // Adds a layer of actions
        void addLayer(std::list<Action> actions);
        // Get amount of layers in the plan
        int getLayerCount();
        // Returns all the actions of a layer in a list
        std::list<Action> getLayerActions(int layer);
        // Clear the plan (Remove everything)
        void clear();

    private:
        std::list<std::list<Action>> actions;
};

#endif /* _PLAN_H */
