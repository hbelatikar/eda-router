#include "rrr.h"

int rrr(routingInst *rst) {
  int status = 0;
  int presentedge = 0;
  // Compute Edge Weights
  edgeWeightCal(rst);
  // Reorder the Nets
  newNetOrdering(rst);
  for (int i = 0; i < rst->numNets; i++){
    net presentNet = rst->nets[i];
    route presentRoute = presentNet.nroute;
    // Rip up the net
    for(int k = 0; k<rst->nets[i].nroute.numSegs; k++) {
      segment presentSegment = presentRoute.segments[k];
      for (int l = 0; l < rst->nets[i].nroute.segments[k].numEdges; l++){
        presentedge = presentSegment.edges[l];
        rst->edgeUtils[presentedge] = rst->edgeUtils[presentedge]-1;
        printf("Edgeutility:%d \n",rst->edgeUtils[presentedge]);
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
  int newcost;
  rst->edgeUtilityHistory = new int [rst->numEdges];
  rst->edgeOverFlow = new int [rst->numEdges];
  rst->edgeWeight = new int [rst->numEdges];
  //Initialising the weight ordering arrays to 0 
  std::fill_n(rst->edgeUtilityHistory,rst->numEdges,0);
  std::fill_n(rst->edgeOverFlow,rst->numEdges,0);
  std::fill_n(rst->edgeWeight,rst->numEdges,0);
  for(int i=0; i<rst->numNets; i++) { 
    newcost = 0;
    int presentedge = 0;
    int presentEdgeUtility = 0;
    net presentNet = rst->nets[i];
    route presentRoute = presentNet.nroute;
    for(int j=0; j<rst->nets[i].nroute.numSegs; j++) {
      segment presentSegment = presentRoute.segments[j];
      for(int k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
        presentedge = presentSegment.edges[k];
        presentEdgeUtility = rst->edgeUtils[presentedge];
        printf("Edgeid:%d, EdgeUtility:%d, ", presentedge, presentEdgeUtility);
        rst->edgeOverFlow[presentedge] = std::max(rst->edgeUtils[presentedge] - rst->edgeCaps[presentedge],0);
        printf("points:%d, %d, %d, %d ", presentNet.nroute.segments->p1.x, presentNet.nroute.segments->p1.y, presentNet.nroute.segments->p2.x, presentNet.nroute.segments->p2.y);
        printf("edgeoverflow:%d,  ",rst->edgeOverFlow[presentedge]);
        if(rst->edgeOverFlow[presentedge]>0){
          rst->edgeUtilityHistory[presentedge] = rst->edgeUtilityHistory[presentedge] + 1;
        }
        printf("edgeUtilityHistory:%d, ",rst->edgeUtilityHistory[presentedge]);
        rst->edgeWeight[presentedge] = rst->edgeOverFlow[presentedge] * rst->edgeUtilityHistory[presentedge];
        printf("edgeWeight:%d \n",rst->edgeWeight[presentedge]);
        newcost = newcost + rst->edgeWeight[presentedge];
        printf("new cost of edge:%d \n",newcost);
      }
    }
  printf("new cost of net:%d\n",newcost);
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