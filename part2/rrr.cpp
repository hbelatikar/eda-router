#include "rrr.h"

int rrr(routingInst *rst) {
  pointHash::gx = rst->gx;
  
  int status = 0;
  int relaxer = 0;
  // Compute Edge Weights
  edgeWeightCal(rst);
  // Reorder the Nets
  newNetOrdering(rst);
  
  // Rip up ALL the nets
  for (int i = 0; i < rst->numNets; i++){
    // std::cout << "Ripping up the i:"<<i<<"th net n"<<rst->nets[i].id<<"\n";
    for(int j = 0; j<rst->nets[i].nroute.numSegs; j++) {
      // std::cout << "Ripping up the seg with "<<rst->nets[i].nroute.segments[k].numEdges<<" edges\n";
      // std::cout << "Ripping up the edge "<<rst->nets[i].nroute.segments[k].edges[l]<<"\n";
      // rst->edgeUtils[rst->nets[i].nroute.segments[k].edges[l]]--;
      delete[] rst->nets[i].nroute.segments[j].edges;
      }
    delete[] rst->nets[i].nroute.segments;
  }
  // Reset the edge utilisation
  std::fill_n(rst->edgeUtils,rst->numEdges,0);
  
  // Reroute ALL the nets
  for (int i = 0; i < rst->numNets; i++){
    // Allocate the net infos before rerouting
    rst->nets[i].nroute.numSegs = rst->nets[i].numPins - 1;
    rst->nets[i].nroute.segments = new segment[rst->nets[i].nroute.numSegs];
    
    for (int j = 0; j < rst->nets[i].nroute.numSegs; j++){
      
      rst->nets[i].nroute.segments[j].p1 = rst->nets[i].pins[j];
      rst->nets[i].nroute.segments[j].p2 = rst->nets[i].pins[j+1];

      // Reroute the net segments
      relaxer = 0;
      do {
        status = singleNetReroute(rst, rst->nets[i].pins[j], rst->nets[i].pins[j+1], i, j, relaxer);
        relaxer += 2;
      } while (status == 0);
      // if (status == 0) {
      //   std::cout << "Could not reroute net n" << rst->nets[i].id << "\n";
      //   return 0;
      // }
    }
    std::cout << "Rerouted net:"<<i<<"\n";
  }
  return 1;
}

int singleNetReroute(routingInst *rst, point start, point dest, int netIdx, int segIdx, int relaxer) {
  // References: 
  // Wikipedia's A* algorithm : https://en.wikipedia.org/wiki/A*_search_algorithm#Pseudocode
  // Implementation/Tutorial  : https://www.youtube.com/watch?v=aKYlikFAV4k
  
  std::priority_queue<PPI, std::vector<PPI>, comparePQ> openSet;
  std::unordered_map<point, point, pointHash> cameFrom;
  std::unordered_map<point, int, pointHash> gScore;
  std::unordered_map<point, int, pointHash> fScore;

  // Adding a map based openset to check if points are present in queue
  std::unordered_map<point, int, pointHash> openSet_map;
  // Adding a closedSet set since Wikipedias algo revisits processed nodes
  std::unordered_map<point, int, pointHash> closedSet;

  int status = 0;
  int edgeID = -1;
  int edgeIdx = -1;
  int fScoreVal;
  int tentGScore = INT32_MAX;
  bool firstTimeFlag = false;
  point current;
  point tempRetrace;
  point nullStart(-2,-2);
  std::vector<point> neighbors;
  std::vector<point> path;
  // std::vector<point>::size_type pathLen;
  
  int topBound = std::min(start.y,dest.y)   + relaxer; 
  int botBound = std::max(start.y,dest.y)   - relaxer; 
  int leftBound = std::min(start.x,dest.x)  - relaxer; 
  int rightBound = std::max(start.x,dest.x) + relaxer;

  rst->nets[netIdx].nroute.segments[segIdx].numEdges = manDist(start,dest);
  rst->nets[netIdx].nroute.segments[segIdx].edges = new int[rst->nets[netIdx].nroute.segments[segIdx].numEdges];
        
  openSet.push(std::make_pair(start,manDist(start,dest)));
  openSet_map[start] = manDist(start, dest);
  gScore[start] = 0;
  fScore[start] = manDist(start, dest);
  cameFrom[start] = nullStart;
   
  while (!openSet.empty()) {
    
    // Get point with lowest fScore value
    current = openSet.top().first;
    
    if(current == dest) {
      status = 0;
      edgeIdx = 0;
      path.push_back(current);
      // Perform path retrace
      while (current != start) {
        tempRetrace = cameFrom.at(current);
        edgeID = getEdgeIDthruPts(current, tempRetrace, rst);
        rst->nets[netIdx].nroute.segments[segIdx].edges[edgeIdx] = edgeID;
        rst->edgeUtils[edgeID]++;
        path.push_back(tempRetrace);
        current = tempRetrace;
        edgeIdx++;
        status = 1;
      }
      if(status == 1) {
        // pathLen = (int)path.size();
        rst->nets[netIdx].nroute.segments[segIdx].numEdges = edgeIdx;
        // rst->nets[netIdx].nroute.segments[segIdx].edges = new int[rst->nets[netIdx].nroute.segments[segIdx].numEdges];
        
        // for (int i=0; i<pathLen-1; i++){
        //   edgeID = getEdgeIDthruPts(path.at(i),path.at(i+1), rst);
        //   rst->nets[netIdx].nroute.segments[segIdx].edges[i] = edgeID;
        //   rst->edgeUtils[edgeID]++;
        // }
        return 1;
      } else {
        std::cout << "Could not retrace path!\n";
        return 0;
      }
    }
    
    //Remove the current from openSet
    openSet.pop();
    openSet_map.erase(current);
    closedSet[current] = 1;

    neighbors = findNeighbors(current, topBound, botBound, leftBound, rightBound, rst);

    // std::cout << "The node is << current << "\n";
    
    for(auto & neighbor : neighbors) {
      // If we have not processed this neighbor before process it
      if(closedSet.count(neighbor) == 0) {
        firstTimeFlag = false;
        if(openSet_map.count(neighbor) == 0) {
          fScore[neighbor] = INT32_MAX;
          gScore[neighbor] = INT32_MAX;
          firstTimeFlag = true;
        }
        tentGScore = gScore[current] + rst->edgeWeight[getEdgeIDthruPts(current,neighbor,rst)];
        if(tentGScore < gScore[neighbor]){
          fScoreVal = tentGScore + manDist(neighbor,dest);
          cameFrom[neighbor] = current;
          gScore[neighbor] = tentGScore;
          fScore[neighbor] = fScoreVal;
          if(firstTimeFlag) {
            openSet.push(std::make_pair(neighbor,fScoreVal));
            openSet_map[neighbor] = fScoreVal;
          }
        }
      }
    }
  }
  return 0;
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
    net presentNet = rst->nets[i];
    route presentRoute = presentNet.nroute;
    for(int j=0; j<rst->nets[i].nroute.numSegs; j++) {
      segment presentSegment = presentRoute.segments[j];
      for(int k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
        presentedge = presentSegment.edges[k];
        rst->edgeOverFlow[presentedge] = std::max(rst->edgeUtils[presentedge] - rst->edgeCaps[presentedge],0);
        if(rst->edgeOverFlow[presentedge]>0){
          rst->edgeUtilityHistory[presentedge] = rst->edgeUtilityHistory[presentedge] + 1;
        }
        rst->edgeWeight[presentedge] = rst->edgeOverFlow[presentedge] * rst->edgeUtilityHistory[presentedge];
        newcost = newcost + rst->edgeWeight[presentedge];
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

std::vector<point> findNeighbors(point p, int topBound, int botBound, int leftBound, int rightBound, routingInst* rst){
  std::vector<point> neighbors;
  point ptToPush;
  // Max point is rst->gx -1 & rst->gy -1
  if ((p.x < (rst->gx - 1)) && (p.x < rightBound)) {
    ptToPush.x = p.x+1;
    ptToPush.y = p.y;
    neighbors.push_back(ptToPush);
  }
  if ((p.x > 0) && (p.x > leftBound)) {
    ptToPush.x = p.x-1;
    ptToPush.y = p.y;
    neighbors.push_back(ptToPush);
  }
  if ((p.y < (rst->gy - 1)) && (p.y < topBound)) {
    ptToPush.x = p.x;
    ptToPush.y = p.y+1;
    neighbors.push_back(ptToPush);
  }
  if ((p.y > 0) && (p.y > botBound)) {
    ptToPush.x = p.x;
    ptToPush.y = p.y-1;
    neighbors.push_back(ptToPush);
  }
  return neighbors;
}
