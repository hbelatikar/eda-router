#include "rrr.h"

int rrr(routingInst *rst) {
  int status = 0;
  // Compute Edge Weights
  edgeWeightCal(rst);
  // Reorder the Nets
  newNetOrdering(rst);

  for (int i = 0; i < rst->numNets; i++){
    // Rip up the net
    for(int k = 0; k<rst->nets[i].nroute.numSegs; k++) {
      for (int l = 0; l < rst->nets[i].nroute.segments[k].numEdges; l++){
        rst->edgeUtils[rst->nets[i].nroute.segments[k].edges[l]]--;
      }
    }

    for (int j = 0; j < rst->nets[i].numPins - 1; j++){
      // Reroute the net segments
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


//Calculating edge weights for rip up and reroute
int edgeWeightCal(routingInst *rst){
  int newcost = 0;
  for(int i=0; i<rst->numNets; i++) { 
    for(int j=0; j<rst->nets[i].nroute.numSegs; j++) {
      for(int k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
        rst->edgeOverFlow[k] = std::max(rst->edgeUtils[k] - rst->edgeCaps[k],0);
        if(rst->edgeOverFlow[k]>0){
          rst->edgeUtilityHistory[k] = rst->edgeUtilityHistory[k] + 1;
        }
        rst->edgeWeight[k] = rst->edgeOverFlow[k] * rst->edgeUtilityHistory[k];
        newcost = newcost + rst->edgeWeight[k];
      }
    }
  rst->nets[i].cost = newcost;
  }
  return 1;
}

int newNetOrdering(routingInst *rst){
  qsort(rst->nets,rst->numNets,sizeof(net),compareNetOrders);
  return 1;
}

int compareNetOrders (const void *a, const void *b){
  net *n1 = (net *) a;
  net *n2 = (net *) b;
  return ((n2->cost) - (n1->cost));
}