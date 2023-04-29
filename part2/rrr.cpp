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
  
  std::priority_queue<PPI, std::vector<PPI>, comparePQ> openSet;
  std::unordered_map<point, point, pointHash> cameFrom;
  std::unordered_map<point, int, pointHash> gScore;
  std::unordered_map<point, int, pointHash> fScore;

  // Adding a map based openset to check if points are presnet in queue
  std::unordered_map<point, int, pointHash>openSet_mapped;
  
  int tentGScore;
  point current;
  std::vector<point> neighbors;

  openSet.push({start,manDist(start,dest)});
  openSet_mapped[start] = manDist(start, dest);
  gScore[start] = 0;
  fScore[start] = manDist(start, dest);
  // cameFrom[start] = ;
  while (!openSet.empty()) {
    
    // Get point with lowest fScore value
    current = openSet.top().first;
    
    if(current == dest) {
      // Perform pathRetrace
      return 1;
    }
    
    //openSet.Remove(current)
    openSet.pop();
    openSet_mapped.erase(start);

    neighbors = findNeighbors(current,rst);
    // std::cout << "The node is (" << current.x << "," << current.y << ")\n";
    
    for(auto & neighbor : neighbors){
      tentGScore = gScore[current] + rst->edgeWeight[getEdgeIDthruPts(current,neighbor,rst)];
      if(tentGScore < gScore[neighbor]){
        cameFrom[neighbor] = current;
        gScore[neighbor] = tentGScore;
        fScore[neighbor] = tentGScore + manDist(neighbor,dest);
        if(openSet_mapped.count(neighbor) == 0) {
          openSet.push({neighbor,fScore[neighbor]});
          openSet_mapped[neighbor] = fScore[neighbor];
        }
      }
    }
  }
  return 0;
}

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

std::vector<point> findNeighbors(point p, routingInst* rst){
  std::vector<point> neighbors;
  // Max point is rst->gx -1 & rst->gy -1
  if (p.x < (rst->gy - 1)) {
    neighbors.push_back({p.x+1,p.y});
  }
  if (p.x > 0) {
    neighbors.push_back({p.x-1,p.y});
  }
  if (p.y < (rst->gx - 1)) {
    neighbors.push_back({p.x,p.y+1});
  }
  if (p.y > 0) {
    neighbors.push_back({p.x,p.y-1});
  }

  return neighbors;
}