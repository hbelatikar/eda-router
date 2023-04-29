#ifndef RRR_H
#define RRR_H

#define PPI std::pair<point,int>

#include "ece556.h"
#include <unordered_map>
#include <vector>
#include <set>
#include <queue>

int rrr (routingInst *rst);

int singleNetReroute(routingInst *, point, point);


int edgeWeightCal(routingInst *);
int newNetOrdering(routingInst *);

int compareNetOrders (const void *, const void *);

std::vector<point> findNeighbors(point, routingInst*);

// Ref: https://www.geeksforgeeks.org/how-to-create-an-unordered_map-of-user-defined-class-in-cpp/
// Ref: https://www.techiedelight.com/use-struct-key-std-unordered_map-cpp/
namespace{
  class pointHash {
    public:
      static int gy;
      std::size_t operator()(const point& p) const {
          return gy * p.x + p.y;
      }
  };
  int pointHash::gy = 0;
}

// Ref: https://www.geeksforgeeks.org/custom-comparator-in-priority_queue-in-cpp-stl/
class comparePQ {
    public:
       bool operator()(std::pair<point,int> a, std::pair<point,int>  b){
          // Swap if the fScore of first pair is greater than second pair because we want min heap
          return a.second > b.second;
    }
};
#endif // RRR_H
