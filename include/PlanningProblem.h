#ifndef _PLANNING_PROBLEM_H
#define _PLANNING_PROBLEM_H

#include <list>
#include <utility>
#include <string>
#include <vector>
#include <map>

#include "IPlanningProblem.h"


/**
 * Class representating a planning problem instance and its planning graph
 *
 * Author: Patrick Hegemann
 */
class PlanningProblem : public IPlanningProblem {
    public:
        class Builder;

        int getVariableCount();
        int getActionCount();
        int getPropositionCount();
        int getPropositionNumber(Proposition p);

        std::list<Proposition> getGoal();

        // Properties of actions
        std::list<Proposition>& getActionPreconditions(Action a);
        std::list<Proposition>& getActionPosEffects(Action a);
        std::list<Proposition>& getActionNegEffects(Action a);
        std::list<Action>& getPropPosActions(Proposition p);
        
        int getFirstLayer();
        int getLastLayer();
        int getFirstActionLayer();
        int getLastActionLayer();
        int addPropositionLayer();
        int addActionLayer();

        // Propositions and actions in the planning graph
        int isPropEnabled(Proposition p, int layer);
        int isActionEnabled(Action a, int layer);
        int getActionFirstLayer(Action a);

        void activateAction(Action a, int layer);
        void activateProposition(Proposition p, int layer);

        std::list<Proposition> getLayerPropositions(int layer);
        std::list<Action>& getLayerActions(int layer);

        // Mutex Handling
        int isMutexProp(Proposition p, Proposition q, int layer);
        int isMutexAction(Action a, Action b, int layer);
        void setMutexProp(Proposition p, Proposition q, int layer);
        void setMutexAction(Action a, Action b, int layer);
        int getPropMutexCount(int layer);

        std::string getPropositionName(Proposition p);
        std::string getActionName(Action a);

        int isTrivialAction(Action a);
    
        int getPropLayerAfterActionLayer(int actionLayer);
        int getActionLayerBeforePropLayer(int propLayer);
        int getPropLayerBeforeActionLayer(int actionLayer);

        void dumpPlanningGraph();

    private:
        // Amount of variables in the problem
        int countVariables;
        // Size of each variable domain
        std::vector<int> variableDomainSize;
        // Total amount of propositions (variable value pairs)
        int totalPropositionCount;

        // Amount of actions defined in the problem
        int countActions;

        // List of goal propositions
        std::list<Proposition> goalPropositions;

        // Planning graph edges (note that they are always the same in every layer, if
        // they appear, so we only need to define them once.) They are all implemented
        // with adjacency arrays
        //
        // Precondition edges ("From actions to their preconditions")
        std::vector<int> actionPrecIndices;
        std::vector<Proposition> actionPrecEdges;
        // Positive effect edges ("From actions to their positive effects")
        std::vector<int> actionPosEffIndices;
        std::vector<Proposition> actionPosEffEdges;
        // Negative effect edges ("From actions to their negative effects")
        std::vector<int> actionNegEffIndices;
        std::vector<Proposition> actionNegEffEdges;

        // And again as vectors of lists to avoid copying
        std::vector<std::list<Proposition>> actionPrecs;
        std::vector<std::list<Proposition>> actionPosEffs;
        std::vector<std::list<Proposition>> actionNegEffs;


        // Positive effect edges from positive effects to actions
        // This is needed when determining proposition mutexes
        // Implemented as an adjacency list
        std::map<Proposition, std::list<Action>> propPosActions;

        // Number of last proposition layer
        int lastPropLayer;
        // Number of last action layer
        int lastActionLayer;

        // Mutexes, here implemented as matrixes. The matrix entries specify the
        // *last* layer in which the propositions/actions are mutex with each other
        int *propMutexes;
        int *actionMutexes;

        // Each proposition needs to have a unique number that can be used to check
        // mutexes. Each variable has its "starting number", that the value of the
        // proposition will be added to in order to get this unique number.
        // In this case the starting number of a variable depends on the domain size
        // of all the variables before it.
        std::vector<int> variableMutexIndex;

        // Arrays that indicate in which layer a proposition/action first shows up
        std::map<Proposition, int> propFirstLayer;
        std::vector<int> actionFirstLayer;

        // Arrays that store propositions/actions that are already used in some layer
        std::vector<Proposition> layerProps;
        std::vector<Action> layerActions;

        std::vector<std::list<Action>> layerActionsLists;

        // Lists that hold an index for each layer, indicating the point up to which a
        // layer contains propositions/actions from the layerProps/layerActions arrays
        std::vector<int> lastPropIndices;
        std::vector<int> lastActionIndices;

        // Array indicating the amount of proposition mutexes in each proposition layer
        // for calculating if a fixed-point level is reached
        std::vector<int> layerPropMutexCount;

        // Names
        std::vector<std::string> actionNames;
        std::map<Proposition, std::string> propNames;
};


// Builder class
class PlanningProblem::Builder : public IPlanningProblem::Builder {
    public:
        Builder();
        ~Builder() {}
        PlanningProblem* build();
        void setVariableCount(int count);
        Variable addVariable();
        void setVariableDomainSize(Variable v, int size);
        void setPropositionName(Proposition p, std::string name);
        void finalizeVariables();
        void setGlobalPropMutex(Proposition p, Proposition q);
        void addIntialProposition(Proposition p);
        void addGoalProposition(Proposition p);

        void setActionCount(int count);
        Action addAction();
        void setActionName(Action a, std::string name);
        void addActionPrecondition(Action a, Proposition p);
        void addActionPosEffect(Action a, Proposition p);
        void addActionNegEffect(Action a, Proposition p);

    private:
        PlanningProblem* problem;
        int nextVariable = 0;
        int nextAction = 0;

        int variablesFinalized = 0;
        int totalPropositionCount = 0;
};


#endif /* _PLANNING_PROBLEM_H */
