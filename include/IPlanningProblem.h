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
        //virtual ~IPlanningProblem() {}

        // An builder interface for the planning problem class
        class Builder {
            public:
                Builder() {}
                virtual ~Builder() {}

                // Returns the built Planning Problem
                virtual IPlanningProblem* build() =0;
                
                // Sets the amount of variables in the planning problem
                virtual void setVariableCount(int count) =0;
                // Adds a variable to the problem and returns its number
                virtual Variable addVariable() =0;
                // Sets the size of a variable's domain (i.e. how many different values it could take)
                virtual void setVariableDomainSize(Variable v, int size) =0;
                // Sets the name of a proposition
                virtual void setPropositionName(Proposition p, std::string name) =0;
                // Finalize variables, i.e. create data structures that can't be resized later
                virtual void finalizeVariables() =0;
                // Sets two propositions mutex in every layer
                virtual void setGlobalPropMutex(Proposition p, Proposition q) =0;
                // Adds an initial proposition
                virtual void addIntialProposition(Proposition p) =0;
                // Adds a goal proposition
                virtual void addGoalProposition(Proposition p) =0;

                // Sets the amount of actions
                virtual void setActionCount(int count) =0;
                // Adds an action to the problem (and returns its number)
                virtual Action addAction() =0;
                // Sets the name of an action
                virtual void setActionName(Action a, std::string name) =0;
                // Adds a precondition to an action
                virtual void addActionPrecondition(Action a, Proposition p) =0;
                // Adds a positive effect to an action
                virtual void addActionPosEffect(Action a, Proposition p) =0;
                // Adds a negative effect to an action
                virtual void addActionNegEffect(Action a, Proposition p) =0;
        };
        
        // Gets amount of variables in this problem
        virtual int getVariableCount() =0;
        // Gets amount of actions in this problem
        virtual int getActionCount() =0;
        // Gets amount of propositions in this problem
        virtual int getPropositionCount() =0;
        // Gets the proposition number of a proposition (variable and value)
        virtual int getPropositionNumber(Proposition p) =0;

        // Gets a copy of the list of goal propositions of this problem
        virtual std::list<Proposition> getGoal() =0;

        // Properties of actions
        // Gets a list of preconditions of an action
        virtual std::list<Proposition> getActionPreconditions(Action a) =0;
        // Gets a list of positive/add effects of an action
        virtual std::list<Proposition> getActionPosEffects(Action a) =0;
        // Gets a list of negative/delete effects of an action
        virtual std::list<Proposition> getActionNegEffects(Action a) =0;
        // Gets a list of actions that have the proposition as a positive effect
        virtual std::list<Action> getPropPosActions(Proposition p) =0;

        // Gets the number of the first proposition layer
        virtual int getFirstLayer() =0;
        // Gets the number of the last proposition layer
        virtual int getLastLayer() =0;
        // Gets the number of the first action layer
        virtual int getFirstActionLayer() =0;
        // Gets the number of the last action layer
        virtual int getLastActionLayer() =0;
        // Adds a proposition layer to the planning graph and returns its number
        virtual int addPropositionLayer() =0;
        // Adds a action layer to the planning graph and returns its number
        virtual int addActionLayer() =0;

        // Propositions and actions in the planning graph
        // Whether the given proposition is enabled in the given layer
        virtual int isPropEnabled(Proposition p, int layer) =0;
        // Whether the given action is already enabled in the given layer
        virtual int isActionEnabled(Action a, int layer) =0;
        // Gets the number of the first layer where the given action is enabled
        virtual int getActionFirstLayer(Action a) =0;

        // Activates the given action in the given layer
        virtual void activateAction(Action a, int layer) =0;
        // Activates the given proposition in the given layer
        virtual void activateProposition(Proposition p, int layer) =0;

        // Gets a list of propositions in a given layer
        virtual std::list<Proposition> getLayerPropositions(int layer) =0;
        // Gets a list of actions in a given layer
        virtual std::list<Action> getLayerActions(int layer) =0;

        // Mutex Handling
        // Checks if two propositions are mutex in a given layer
        virtual int isMutexProp(Proposition p, Proposition q, int layer) =0;
        // Checks if two actions are mutex in a given layer
        virtual int isMutexAction(Action a, Action b, int layer) =0;
        // Sets two propositions mutex in a given layer
        virtual void setMutexProp(Proposition p, Proposition q, int layer) =0;
        // Sets two actions mutex in a given layer
        virtual void setMutexAction(Action a, Action b, int layer) =0;
        // Gets the amount of proposition mutexes in a given layer
        virtual int getPropMutexCount(int layer) =0;

        // Names
        // Gets the name of a proposition
        virtual std::string getPropositionName(Proposition p) =0;
        // Gets the name of an action
        virtual std::string getActionName(Action a) =0;

        // 
        // Returns whether the given action is a trivial action
        virtual int isTrivialAction(Action a) =0;
        // Returns number of the proposition layer after given action layer
        virtual int getPropLayerAfterActionLayer(int actionLayer) =0;
        // Returns number of the action layer before given proposition layer
        virtual int getActionLayerBeforePropLayer(int propLayer) =0;

        // Outputs the whole current planning graph
        virtual void dumpPlanningGraph() =0;

};

#endif /* _IPLANNING_PROBLEM_H */

