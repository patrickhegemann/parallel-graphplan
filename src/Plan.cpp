#include <iterator>

#include "Plan.h"


void Plan::addLayer(std::list<Action> actions) {
    this->actions.push_back(actions);
}

int Plan::getLayerCount() {
    return actions.size();
}

std::list<Action> Plan::getLayerActions(int layer) {
    std::list<std::list<Action>>::iterator it = actions.begin();
    std::advance(it, layer);
    return *it;
}

void Plan::clear() {
    actions.clear();
}

