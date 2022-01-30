#include <iostream>
#include <iterator>
#include <climits>
#include <algorithm>
#include <cmath>

#include "Planners/PlannerWithSATExtraction.h"
#include "Logger.h"
#include "Settings.h"

#include "ipasir_cpp.h"



PlannerWithSATExtraction::PlannerWithSATExtraction(IPlanningProblem *problem) : Planner(problem) {
    solver = ipasir_init();
    solverInitialized = true;
    horizonOffset = 0;

    ipasir_set_terminate(solver, NULL, NULL);
    #ifndef PGP_NOSETLEARN
    ipasir_set_learn(solver, NULL, 0, NULL); 
    #endif

    countActions = problem->getActionCount();
    countPropositions = problem->getPropositionCount();
}

PlannerWithSATExtraction::~PlannerWithSATExtraction() {
    if (solverInitialized) {
        ipasir_release(solver);
    }
}

int PlannerWithSATExtraction::graphplan(Plan& plan) {
    log(0, "SATEx algorithm using SAT Solver %s\n", ipasir_signature());

    // Expand the graph until we hit a fixed-point level or we find out that
    // the problem is unsolvable.
    while (!fixedPoint && checkGoalUnreachable()) {
        expand();
        fixedPoint = fixedPoint || checkFixedPoint();
        horizonOffset++;
    }

    // If goal is impossible to reach, this problem has no solution
    if (checkGoalUnreachable()) {
        log(1, "Goal unreachable, aborting\n");
        return false;
    }

    // Do backwards search with given goal propositions
    std::list<Proposition> goal = problem->getGoal();
    // If fixed point is reached, we have theoretically expanded beyond it, just to find out.
    // So we subtract that additional layer again
    int lastLayer = problem->getLastLayer();
    int success = extract(solver, goal, lastLayer, plan);

    // How many iteration of the main loop have been done, to calculate horizon
    int iteration = 0;

    while(!success) {
        // Expand until horizon
        while (problem->getLastActionLayer() < horizon(iteration+1)) {
            expand();
            fixedPoint = fixedPoint || checkFixedPoint();
        }

        // Clean up, start with a fresh empty plan
        plan.clear();

        // Do backwards search with given goal propositions
        lastLayer = problem->getLastLayer();
        success = extract(solver, goal, lastLayer, plan);

        iteration++;
    }

    return success;
}

/**
 * Adds necessary clauses for one action layer to the given SAT solver.
 */
void PlannerWithSATExtraction::addClausesToSolver(void *solver, int actionLayer) {
    log(0, "Adding clauses to SAT solver %p\n", solver);

    int prevPropLayer = problem->getPropLayerBeforeActionLayer(actionLayer);
    int nextPropLayer = problem->getPropLayerAfterActionLayer(actionLayer);

    for (Action a : problem->getLayerActions(actionLayer)) {
        // Add precondition clauses to the SAT solver
        // If an action is done in layer i, the precondition has to be true in layer i-1
        if (actionLayer != problem->getFirstActionLayer()) {
            for (Proposition prec : problem->getActionPreconditions(a)) {
                ipasir_add(solver, -actionAtLayer(a, actionLayer));
                ipasir_add(solver, propositionAtLayer(prec, prevPropLayer));
                ipasir_add(solver, 0);
            }
        }

        // Add positive effect clauses to the SAT solver
        // If an action is done in layer i, the positive effect has to be true in layer i+1
        for (Proposition pos : problem->getActionPosEffects(a)) {
            ipasir_add(solver, -actionAtLayer(a, actionLayer));
            ipasir_add(solver, propositionAtLayer(pos, nextPropLayer));
            ipasir_add(solver, 0);
        }
        
        // Add negative effect clauses to the SAT solver
        // If an action is done in layer i, the negative effect has to be false in layer i+1
        for (Proposition neg : problem->getActionNegEffects(a)) {
            ipasir_add(solver, -actionAtLayer(a, actionLayer));
            ipasir_add(solver, -propositionAtLayer(neg, nextPropLayer));
            ipasir_add(solver, 0);
        }

        // Action mutexes
        for (Action b : problem->getLayerActions(problem->getLastActionLayer())) {
            if (a == b) break;
            if (problem->isMutexAction(a, b, actionLayer)) {
                ipasir_add(solver, -actionAtLayer(a, actionLayer));
                ipasir_add(solver, -actionAtLayer(b, actionLayer));
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
            if (problem->isActionEnabled(a, actionLayer)) {
                ipasir_add(solver, actionAtLayer(a, actionLayer));
            }   
        }
        ipasir_add(solver, 0);
    }

    log(0, "Done adding clauses\n");
}

void PlannerWithSATExtraction::expand() {
    Planner::expand();
    addClausesToSolver(solver, problem->getLastActionLayer());
}

int PlannerWithSATExtraction::extract(void *solver, std::list<Proposition> goal, int layer, Plan& plan) {
    log(0, "Extracting in layer %d with SAT Extraction\n", layer);

    // Assume that the goal is true in this layer
    for (Proposition p : goal) {
        ipasir_assume(solver, propositionAtLayer(p, layer));
    }

    if (ipasir_solve(solver) == IPASIR_IS_SAT) {
        for (int i = problem->getFirstActionLayer(); i <= problem->getActionLayerBeforePropLayer(layer); i++) {
            std::list<Action> actions;
            for (Action a : problem->getLayerActions(i)) {
                int lit = actionAtLayer(a, i);
                if (ipasir_val(solver, lit) == lit) {
                    actions.push_back(a);
                }
            }
            plan.addLayer(actions);
        }
        log (0, "Done extracting: success\n");
        return 1;
    } else {
        log (0, "Done extracting: failure/terminated\n");
        return 0;
    }
}

/*
 * Returns the variable number for SAT solving of a proposition being true in a given layer
 */
int PlannerWithSATExtraction::propositionAtLayer(Proposition p, int layer) {
    int r = (countPropositions + countActions) * (layer - 2) + countActions + 1 + problem->getPropositionNumber(p);
    return r;
}

/*
 * Returns the variable number for SAT solving of an action being used in a given layer
 */
int PlannerWithSATExtraction::actionAtLayer(Action a, int layer) {
    int r = (countPropositions + countActions) * (layer - 1) + 1 + a;
    return r;
}


int PlannerWithSATExtraction::horizon(int n) {
    // Get parameters
    int horizonType = settings->getHorizonType();
    double factor = settings->getHorizonFactor();

    int hor = 0;

    // Calculate horizon depending on type, factor and input (n)
    switch (horizonType) {
        case HORIZON_LINEAR:
            // Linear Horizon: f*n
            hor = ((int)factor)*n;
            break;

        case HORIZON_EXPONENTIAL:
            // Exponential horizon: f^n
            hor = ceil(pow(factor, n));
            break;
    }

    return hor + horizonOffset;
}

