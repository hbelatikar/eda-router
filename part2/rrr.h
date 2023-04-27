#ifndef RRR_H
#define RRR_H

#include <map>
#include <vector>
#include <set>
#include <queue>
#include "ece556.h"

int rrr (routingInst *rst);

int singleNetReroute(routingInst *, point, point);

bool pqComp (int, int);

#endif // RRR_H
