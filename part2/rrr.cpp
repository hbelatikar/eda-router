#include "rrr.h"

int rrr(routingInst *rst) {
  int status = 0;
  // Compute Edge Weights
  // Reorder the Nets
  for (int i = 0; i < rst->numNets; i++){
    for (int j = 0; j < rst->nets[i].numPins - 1; j++){
      // Reroute the net
      status = singleNetReroute(rst,rst->nets[i].pins[j],rst->nets[i].pins[j+1]);
      if (status == 0) {
        std::cout << "Could not reroute net n" << rst->nets[i].id << "\n";
        return 0;
      }
    }
  }
  return 1;
}

int singleNetReroute(routingInst *rst, point start, point dest) {

  // Need queues for the groups 1 and 3
  // Need a min heap priorty queue for group 2
    // Need a compare function for this

  return 1;
}

bool pqComp (int a, int b){
  return true;
}