#ifndef _IPLANNING_PROBLEM_H
#define _IPLANNING_PROBLEM_H

#include <list>
#include <utility>
#include <string>

#include "common.h"


/**
 * Interface for planning problems and their planning graphs.
 *
 * Author: Patrick Hegemann
 */
class IPlanningProblem {
    public:
        virtual ~IPlanningProblem() {}

        // An builder interface for the planning problem class
        class Builder {
            public:
                virtual ~Builder() {}
                virtual IPlanningProblem* build();
                
                // Sets the amount of variables in the planning problem
                virtual void setVariableCount(int count);
                // Sets the name of a proposition
                virtual void setPropositionName(Proposition p, std::string name);
                // Sets two propositions mutex in every layer
                virtual void setGlobalPropMutex(Proposition p, Proposition q);
                // Adds an initial proposition
                virtual void addIntialProposition(Proposition p); 
                // Adds a goal proposition
                virtual void addGoalProposition(Proposition p);
                // Sets the amount of actions
                virtual void setActionCount(int count);

                // Adds an action to the problem (and returns its number)
                virtual Action addAction();
                // Sets the name of an action
                virtual void setActionName(Action a, std::string name);
                // Adds a precondition to an action
                virtual void addActionPrecondition(Action a, Proposition p);
                // Adds a positive effect to an action
                virtual void addActionPosEffect(Action a, Proposition p);
                // Adds a negative effect to an action
                virtual void addActionNegEffect(Action a, Proposition p);
        };
        
        // Gets amount of variables in this problem
        virtual int getVariableCount();
        // Gets amount of actions in this problem
        virtual int getActionCount();

        // Gets a list of goal propositions of this problem
        virtual std::list<Proposition> getGoal();

        // Properties of actions
        // Gets a list of preconditions of an action
        virtual std::list<Proposition> getActionPreconditions(Action a);
        // Gets a list of positive/add effects of an action
        virtual std::list<Proposition> getActionPosEffects(Action a);
        // Gets a list of negative/delete effects of an action
        virtual std::list<Proposition> getActionNegEffects(Action a);
        // Gets a list of actions that have the proposition as a positive effect
        virtual std::list<Action> getPropPosActions(Proposition p);

        // Gets the number of the last proposition layer
        virtual int getLastLayer();
        // Gets the number of the last action layer
        virtual int getLastActionLayer();
        // Adds a proposition layer to the planning graph and returns its number
        virtual int addPropositionLayer();
        // Adds a action layer to the planning graph and returns its number
        virtual int addActionLayer();

        // Propositions and actions in the planning graph
        // Whether the given proposition is already enabled in the planning graph
        virtual int isPropEnabled(Proposition p);
        // Whether the given action is already enabled in the planning graph
        virtual int isActionEnabled(Action a);
        // Gets the number of the first layer where the given action is enabled
        virtual int getActionFirstLayer(Action a);

        // Activates the given action in the given layer
        virtual void activateAction(Action a, int layer);
        // Activates the given proposition in the given layer
        // (TODO: Mabye not necessary, can be done in activateAction()?)
        virtual void activateProposition(Proposition p, int layer);

        // Gets a list of propositions in a given layer
        virtual std::list<Proposition> getLayerPropositions(int layer);
        // Gets a list of actions in a given layer
        virtual std::list<Action> getLayerActions(int layer);

        // Mutex Handling
        // Checks if two propositions are mutex in a given layer
        virtual inline int isMutexProp(Proposition p, Proposition q, int layer);
        // Checks if two actions are mutex in a given layer
        virtual inline int isMutexAction(Action a, Action b, int layer);
        // Sets two propositions mutex in a given layer
        virtual inline int setMutexProp(Proposition p, Proposition q, int layer);
        // Sets two actions mutex in a given layer
        virtual inline int setMutexAction(Action a, Action b, int layer);
        // Gets the amount of proposition mutexes in a given layer
        virtual int getPropMutexCount(int layer);

        // Names
        // Gets the name of a proposition
        virtual std::string getPropositionName(Proposition p);
        // Gets the name of an action
        virtual std::string getActionName(Action a);

        // 
        // Returns whether the given action is a trivial action
        virtual int isTrivialAction(Action a);

};

#endif /* _IPLANNING_PROBLEM_H */

