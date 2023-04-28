#include "rrr.h"

int rrr(routingInst *rst) {
  pointHash::gy = rst->gy;
  
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
  // References: 
  // Wikipedia's A* algorithm : https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode
  // Implementation/Tutorial  : https://www.youtube.com/watch?v=aKYlikFAV4k

  // Need queues for the groups 1 and 3
  // Need a min heap priorty queue for group 2
    // Need a compare function for this

  std::priority_queue<PPI, std::vector<PPI>, comparePQ> openSet;
  
  openSet.push({start,manDist(start,dest)});

  std::unordered_map<point, point, pointHash>cameFrom;
  
  // int totalPoints = rst->gx * rst->gy;
  
  std::unordered_map<point, int, pointHash>gScore;
  gScore[start] = 0;

  std::unordered_map<point, int, pointHash>fScore;
  fScore[start] = manDist(start, dest);
  point current;

  while (!openSet.empty()) {
    
    // current := the node in openSet having the lowest fScore[] value
    current = openSet.top().first;
    
    if(current == dest) {
      // Perform pathRetrace
    }
    
    //openSet.Remove(current)
    openSet.pop();

    //adjacentVertices()

    /*
      for each neighbor of current
            // d(current,neighbor) is the weight of the edge from current to neighbor
            // tentative_gScore is the distance from start to the neighbor through current
            tent_gScore = gScore[current] + rst
            tentative_gScore := gScore[current] + d(current, neighbor)
            if tentative_gScore < gScore[neighbor]
                // This path to neighbor is better than any previous one. Record it!
                cameFrom[neighbor] := current
                gScore[neighbor] := tentative_gScore
                fScore[neighbor] := tentative_gScore + h(neighbor)
                if neighbor not in openSet
                    openSet.add(neighbor)
    */
    
  }
  

  return 1;
}

// bool comparePQ (const void *a, const void *b){
//   std::pair<point,double> *p1 = (std::pair<point,double> *) a;
//   std::pair<point,double> *p2 = (std::pair<point,double> *) b;
//   return ()
// }

/*int *adjacentVertices(node){
  point adjVer[3];
  adjVer[0].x=node.x-1;
  adjVer[0].y=node.y;
  adjVer[1].x=node.x;
  adjVer[1].y=node.y+1;
  adjVer[2].x=node.x+1;
  adjVer[2].y=node.y;
  adjVer[3].x=node.x;
  adjVer[3].y=node.y-1;
  return adjVer;
}

int parent(){

}*/


//Calculating edge weights for rip up and reroute
int edgeWeightCal(routingInst *rst){
  int newcost = 0;
  rst->edgeUtilityHistory = new int [rst->numEdges];
  rst->edgeOverFlow = new int [rst->numEdges];
  rst->edgeWeight = new int [rst->numEdges];
  //Initialising the weight ordering arrays to 0 
  std::fill_n(rst->edgeUtilityHistory,rst->numEdges,0);
  std::fill_n(rst->edgeOverFlow,rst->numEdges,0);
  std::fill_n(rst->edgeWeight,rst->numEdges,0);

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