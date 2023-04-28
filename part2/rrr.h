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

int edgeWeightCal(routingInst *);
int newNetOrdering(routingInst *);

int compareNetOrders (const void *, const void *);

#endif // RRR_H
