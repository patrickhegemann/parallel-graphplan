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

        std::list<Proposition> getGoal();

        // Properties of actions
        std::list<Proposition> getActionPreconditions(Action a);
        std::list<Proposition> getActionPosEffects(Action a);
        std::list<Proposition> getActionNegEffects(Action a);
        std::list<Action> getPropPosActions(Proposition p);
        
        int getLastLayer();
        int getLastActionLayer();
        int addPropositionLayer();
        int addActionLayer();

        // Propositions and actions in the planning graph
        int isPropEnabled(Proposition p);
        int isActionEnabled(Action a);
        int getActionFirstLayer(Action a);

        void activateAction(Action a, int layer);
        void activateProposition(Proposition p, int layer);

        std::list<Proposition> getLayerPropositions(int layer);
        std::list<Action> getLayerActions(int layer);

        // Mutex Handling
        inline int isMutexProp(Proposition p, Proposition q, int layer);
        inline int isMutexAction(Action a, Action b, int layer);
        inline int setMutexProp(Proposition p, Proposition q, int layer);
        inline int setMutexAction(Action a, Action b, int layer);
        int getPropMutexCount(int layer);

        std::string getPropositionName(Proposition p);
        std::string getActionName(Action a);

        int isTrivialAction(Action a);
    
    private:
        int countVariables;
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

        // Positive effect edges from positive effects to actions
        // This is needed when determining proposition mutexes
        // Implemented as an adjacency list
        std::vector<std::list<int>> propPosActions;

        // Mutexes, here implemented as matrixes. The matrix entries specify the
        // *last* layer in which the propositions/actions are mutex with each other
        int *propMutexes;
        int *actionMutexes;

        // Arrays that indicate in which layer a proposition/action first shows up
        std::map<Proposition, int> propFirstLayer;
        std::vector<int> actionFirstLayer;

        // Arrays that store propositions/actions that are already used in some layer
        std::vector<Proposition> layerProps;
        std::vector<Action> layerActions;

        // Lists that hold an index for each layer, indicating the point up to which a
        // layer contains propositions/actions from the layerProps/layerActions arrays
        std::list<int> lastPropIndices;
        std::list<int> lastActionIndices;

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
        void setPropositionName(Proposition p, std::string name);
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
};


#endif /* _PLANNING_PROBLEM_H */
