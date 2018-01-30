#include "PlanningProblem.h"
#include "IPlanningProblem.h"



//PlanningProblem::PlanningProblem() {

//}

int PlanningProblem::getVariableCount() {

}

int PlanningProblem::getActionCount() {

}

std::list<Proposition> PlanningProblem::getGoal() {
    return std::list<Proposition>();
}

std::list<Proposition> PlanningProblem::getActionPreconditions(Action a) {

}

std::list<Proposition> PlanningProblem::getActionPosEffects(Action a) {

}

std::list<Proposition> PlanningProblem::getActionNegEffects(Action a) {

}

std::list<Action> PlanningProblem::getPropPosActions(Proposition p) {

}

int PlanningProblem::getLastLayer() {

}

int PlanningProblem::getLastActionLayer() {

}

int PlanningProblem::addPropositionLayer() {

}

int PlanningProblem::addActionLayer() {

}

int PlanningProblem::isPropEnabled(Proposition p) {

}

int PlanningProblem::isActionEnabled(Action a) {

}

int PlanningProblem::getActionFirstLayer(Action a) {

}

void PlanningProblem::activateAction(Action a, int layer) {

}

void PlanningProblem::activateProposition(Proposition p, int layer) {

}

std::list<Proposition> PlanningProblem::getLayerPropositions(int layer) {

}

std::list<Action> PlanningProblem::getLayerActions(int layer) {

}

inline int PlanningProblem::isMutexProp(Proposition p, Proposition q, int layer) {

}

inline int PlanningProblem::isMutexAction(Action a, Action b, int layer) {

}

inline int PlanningProblem::setMutexProp(Proposition p, Proposition q, int layer) {

}

inline int PlanningProblem::setMutexAction(Action a, Action b, int layer) {

}

int PlanningProblem::getPropMutexCount(int layer) {

}

std::string PlanningProblem::getPropositionName(Proposition p) {

}

std::string PlanningProblem::getActionName(Action a) {

}

int PlanningProblem::isTrivialAction(Action a) {

}


// ----------------------------------------------------------------------------

/*
class PlanningProblem::Builder : public IPlanningProblem::Builder {
    public:
        Builder();
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
*/


// Constructor
PlanningProblem::Builder::Builder() {
    // Allocate new Planning Problem on the heap so we can use it in the algorithm later
    problem = new PlanningProblem();
}

PlanningProblem* PlanningProblem::Builder::build() {
    return problem;
}


void PlanningProblem::Builder::setVariableCount(int count) {
    problem->countVariables = count;
}

void PlanningProblem::Builder::setPropositionName(Proposition p, std::string name) {
    problem->propNames[p] = name;
}

void PlanningProblem::Builder::setGlobalPropMutex(Proposition p, Proposition q) {
    // TODO: Implement
}

void PlanningProblem::Builder::addIntialProposition(Proposition p) {

}

void PlanningProblem::Builder::addGoalProposition(Proposition p) {

}

void PlanningProblem::Builder::setActionCount(int count) {

}

Action PlanningProblem::Builder::addAction() {
    return 0;
}

void PlanningProblem::Builder::setActionName(Action a, std::string name) {

}

void PlanningProblem::Builder::addActionPrecondition(Action a, Proposition p) {

}

void PlanningProblem::Builder::addActionPosEffect(Action a, Proposition p) {

}

void PlanningProblem::Builder::addActionNegEffect(Action a, Proposition p) {

}

