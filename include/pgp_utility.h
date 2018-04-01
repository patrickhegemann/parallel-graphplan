#ifndef _PGP_UTILITY_H
#define _PGP_UTILITY_H

#include <list>

#include "common.h"

// Checks if the intersection of two sorted (!) lists is empty
inline bool empty_intersection(const std::list<Proposition>& x, const std::list<Proposition>& y) {
    auto i = x.begin();
    auto j = y.begin();
    while (i != x.end() && j != y.end()) {
        if (*i == *j) {
            return false;
        } else if (*i < *j)  {
            ++i;
        } else {
            ++j;
        }
    }
    return true;
}

#endif
 
