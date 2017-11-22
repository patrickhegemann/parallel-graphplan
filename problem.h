#ifndef problem_h
#define problem_h

#include <vector>
#include <list>

typedef struct _Problem {
    // Amount of propositions and actions
    int countPropositions;
    int countActions;

    // Goal propositions of the problem
    std::vector<int> goalPropositions;

    // Planning graph edges (note that they are always the same in every layer, if
    // they appear, so we only need to define them once. They are all implemented
    // with adjacency arrays
    //
    // Would it be good if the idle actions were implicit and thus did not occupy
    // any space in the edge arrays? Performance?
    //
    // Precondition edges ("From actions to their preconditions")
    std::vector<int> actionPrecIndices;
    std::vector<int> actionPrecEdges;
    // Positive effect edges ("From actions to their positive effects")
    std::vector<int> actionPosEffIndices;
    std::vector<int> actionPosEffEdges;
    // Negative effect edges ("From actions to their negative effects")
    std::vector<int> actionNegEffIndices;
    std::vector<int> actionNegEffEdges;

    // Positive effect edges 2 ("From positive effects to actions")
    // This is needed when determining proposition mutexes
    // Implemented as an adjacency list
    //std::vector<int> propPosActIndices;
    //std::vector<int> propPosActEdges;
    std::vector<std::list<int>> propPosActions;

    // Proposition groups (sets of propositions that are together a finite domain
    // variable and as such are always mutex)
    std::vector<int> propGroups;

    // Arrays that indicate whether a proposition or action did already occur in
    // any layer of the planning graph. Values are always 0 or 1
    std::vector<char> propEnabled;
    std::vector<char> actionEnabled;

    // Arrays that store propositions/actions that are already used in some layer
    std::vector<int> layerProps;
    std::vector<int> layerActions;

    // Lists that hold an index for each layer, indicating the point up to which a
    // layer contains propositions/actions from the layerProps/layerActions arrays
    std::list<int> lastPropIndices;
    std::list<int> lastActionIndices;


    // Action names
    std::vector<std::string> actionNames;
} Problem;

#endif
