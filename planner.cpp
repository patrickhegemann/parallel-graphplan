

#include "planner.h"


int isNogood(int layer, std::vector<int> props) {
    // TODO: Implement
    return 0;
}

void addNogood(int layer, std::vector<int> props) {
    // TODO: Implement
}



int plan(Problem *problem, std::list<std::list<int>>& plan) {

}

void expand(Problem *problem) {
    
}

int extract(Problem *problem, std::vector<int> goal, int layer, std::list<std::list<int>>& plan) {
    // Trivial success
    if (layer == 0) {
        // ? plan = new std::list<std::list<int>>();
        return 1;
    }

    // This sub-goal has failed before
    // TODO: share with other threads (receive)
    if (isNogood(layer, goal)) return 0;

    // Perform the graphplan search
    std::vector<int> actions;
    int success = gpSearch(problem, goal, actions, layer, plan);

    if (success) {
        return 1;
    }

    // Update nogoods and fail
    // TODO: share with other threads (send)
    addNogood(layer, goal);
    return 0;
}

int gpSearch(Problem *problem, std::vector<int> goal, std::vector<int> actions, int layer, std::list<std::list<int>>& plan) {
    // All actions already chosen
    if (goal.empty()) {
        std::vector<int> preconds;
        for (std::vector<int>::size_type i = 0; i < actions.size(); i++) {
            int a = actions[i];

            // TODO: where do adjacency arrays start ?
            int first = problem->actionPrecIndices[a];
            int last = problem->actionPrecIndices[a+1] - 1;
            for (std::vector<int>::size_type j = first; j < last; j++) {
                // TODO: Duplicate detection ?
                preconds.push_back(actionPrecEdges[j]);
            }
        }

        int success = extract(problem, preconds, layer-1);
        if (!success) return 0;
        
        // TODO:
    }
}
