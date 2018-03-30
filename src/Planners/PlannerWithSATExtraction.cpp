#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>

#include "Planners/PlannerWithSATExtraction.h"
#include "Logger.h"
#include "Settings.h"
extern "C" {
    #include "ipasir.h"
}


#define IPASIR_INTERRUPT 0
#define IPASIR_IS_SAT 10
#define IPASIR_IS_UNSAT 20


PlannerWithSATExtraction::PlannerWithSATExtraction(IPlanningProblem *problem) : Planner(problem) {
    //solver = ipasir_init();
    solver = ipasir_init();

    ipasir_set_terminate (solver, NULL, NULL);
    ipasir_set_learn (solver, NULL, 0, NULL); 

    countActions = problem->getActionCount();
    countPropositions = problem->getPropositionCount();

    /*
    int firstLayer = problem->getFirstLayer();
    for (Proposition p : problem->getLayerPropositions(firstLayer)) {
        ipasir_add(solver, propositionAtLayer(p, firstLayer));
        ipasir_add(0);
    }
    */
}

PlannerWithSATExtraction::~PlannerWithSATExtraction() {
    ipasir_release(solver);
}

int PlannerWithSATExtraction::graphplan(Plan& plan) {
    log(4, "Entering graphplan algorithm with SAT Extraction\n");

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
    //int lastNogoodCount = 0;

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

        // When extracting with SAT, the nogoods are implicitly handled by the SAT Solver,
        // thus we cannot count them and have to go on until we find a plan
        // So this solver can only be used for problems with a solution.
    }

    return success;
}

void PlannerWithSATExtraction::expand() {
    int previousPropLayer = problem->getLastLayer();

    Planner::expand();

    int lastActionLayer = problem->getLastActionLayer();
    int nextPropLayer = problem->getLastLayer();

    for (Action a : problem->getLayerActions(lastActionLayer)) {
        // Add precondition clauses to the SAT solver
        // If an action is done in layer i, the precondition has to be true in layer i-1
        if(lastActionLayer != problem->getFirstActionLayer()) {
            for (Proposition prec : problem->getActionPreconditions(a)) {
                ipasir_add(solver, -actionAtLayer(a, lastActionLayer));
                ipasir_add(solver, propositionAtLayer(prec, previousPropLayer));
                ipasir_add(solver, 0);
            }
        }

        // Add positive effect clauses to the SAT solver
        // If an action is done in layer i, the positive effect has to be true in layer i+1
        for (Proposition pos : problem->getActionPosEffects(a)) {
            ipasir_add(solver, -actionAtLayer(a, lastActionLayer));
            ipasir_add(solver, propositionAtLayer(pos, nextPropLayer));
            ipasir_add(solver, 0);
        }
        
        // Add negative effect clauses to the SAT solver
        // If an action is done in layer i, the negative effect has to be false in layer i+1
        for (Proposition neg : problem->getActionNegEffects(a)) {
            ipasir_add(solver, -actionAtLayer(a, lastActionLayer));
            ipasir_add(solver, -propositionAtLayer(neg, nextPropLayer));
            ipasir_add(solver, 0);
        }

        // Action mutexes
        for (Action b : problem->getLayerActions(problem->getLastActionLayer())) {
            if (a == b) break;
            if (problem->isMutexAction(a, b, lastActionLayer)) {
                ipasir_add(solver, -actionAtLayer(a, lastActionLayer));
                ipasir_add(solver, -actionAtLayer(b, lastActionLayer));
                ipasir_add(solver, 0);
            }
        }
    }

    // If a proposition is true in a (non-initial) layer, it must have been enabled
    // by an action:
    // p -> a or b or c or ... in previous layer, where a,b,c.. are providers of p
    for (Proposition p: problem->getLayerPropositions(nextPropLayer)) {
        ipasir_add(solver, -propositionAtLayer(p, nextPropLayer));
        for (Action a: problem->getPropPosActions(p)) {
            if (problem->isActionEnabled(a, lastActionLayer)) {
                ipasir_add(solver, actionAtLayer(a, lastActionLayer));
            }   
        }
        ipasir_add(solver, 0);
    }

    // These are implicit, but can be made explicit for experiments
    /*
    // Proposition mutexes
    for (Proposition p : problem->getLayerPropositions(nextPropLayer)) {
        for (Proposition q : problem->getLayerPropositions(nextPropLayer)) {
            if (p == q) break;
            if (problem->isMutexProp(p, q, nextPropLayer)) {
                ipasir_add(solver, -propositionAtLayer(p, nextPropLayer));
                ipasir_add(solver, -propositionAtLayer(q, nextPropLayer));
                ipasir_add(solver, 0);
            }
        }
    }
    */
}

int PlannerWithSATExtraction::extract(std::list<Proposition> goal, int layer, Plan& plan) {
    log(2, "Extracting in layer %d with SAT Extraction\n", layer);

    // Assume that the goal is true in this layer
    for (Proposition p : goal) {
        ipasir_assume(solver, propositionAtLayer(p, layer));
    }

    if (ipasir_solve(solver) == IPASIR_IS_SAT) {
        for (int i = problem->getFirstActionLayer(); i <= problem->getLastActionLayer(); i++) {
            std::list<Action> actions;
            for (Action a : problem->getLayerActions(i)) {
                int lit = actionAtLayer(a, i);
                if (ipasir_val(solver, lit) == lit) {
                    actions.push_back(a);
                }
            }
            plan.addLayer(actions);
        }
        return 1;
    } else {
        return 0;
    }
}

/*
 * Returns the variable number for SAT solving of a proposition being true in a given layer
 */
int PlannerWithSATExtraction::propositionAtLayer(Proposition p, int layer) {
    int r = (countPropositions + countActions) * (layer - 2) + countActions + 1 + problem->getPropositionNumber(p);
    //log(4, "prop at layer %d, %s = %d\n", layer, problem->getPropositionName(p).c_str(), r);
    return r;
}

/*
 * Returns the variable number for SAT solving of an action being used in a given layer
 */
int PlannerWithSATExtraction::actionAtLayer(Action a, int layer) {
    int r = (countPropositions + countActions) * (layer - 1) + 1 + a;
    //log(4, "action at layer %d, %s = %d\n", layer, problem->getActionName(a).c_str(), r);
    return r;
}

