// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include <string>
#include <sstream>
#include <math.h>

int readBenchmark(const char *fileName, routingInst *rst){
  std::ifstream input_file(fileName);
  if(input_file.is_open()){
    std::string line;

    while(std::getline(input_file,line)){
      std::stringstream stream(line);
      std::string token;

      stream >> token;
      if (token == "grid") {
        // Parse grid size and calculate num of edges
        stream >> rst->gx >> rst->gy;
        rst->numEdges = (rst->gx * (rst->gy - 1) + rst->gy * (rst->gx - 1));
        rst->edgeCaps = new int[rst->numEdges];
        
        rst->edgeUtils = new int[rst->numEdges];
        rst->edgeWeight = new int [rst->numEdges];
        rst->edgeOverFlow = new int [rst->numEdges];
        rst->edgeUtilityHistory = new int [rst->numEdges];
        //Initialising the weight ordering arrays to 0 
        std::fill_n(rst->edgeUtils,rst->numEdges,0);
        std::fill_n(rst->edgeWeight,rst->numEdges,0);
        std::fill_n(rst->edgeOverFlow,rst->numEdges,0);
        std::fill_n(rst->edgeUtilityHistory,rst->numEdges,0);
      }
      else if (token == "capacity") {
        // Parse the default capacity
        stream >> rst->cap;
        std::fill_n(rst->edgeCaps,rst->numEdges,rst->cap);
      }
      else if (token == "num") {
        // Parse all the number of nets
        stream >> token >> rst->numNets; 
        rst->nets = new net[rst->numNets];
        
        //Gather all net endpoints detail and net id
        for (int i = 0; i < rst->numNets; i++) {
          std::getline(input_file,line);
          std::stringstream net_stream(line);
          if(line[0] == 'n') {
            net_stream >> token >> rst->nets[i].numPins;
            rst->nets[i].id = stoi(token.substr(1,token.size()-1));
            rst->nets[i].pins = new point[rst->nets[i].numPins];
            for (int j = 0; j < rst->nets[i].numPins; j++) {
              std::getline(input_file,line);
              std::stringstream net_stream(line);
              net_stream >> rst->nets[i].pins[j].x >> rst->nets[i].pins[j].y;
            }
          } else {
            std::cout << "ERROR: Could not parse net" << std::endl;
            input_file.close();
            return 0;
          }
        }

        //Read in the restricted edge capacities
        std::getline(input_file,line);
        std::stringstream net_stream(line);
        if(line[0]=='n') {
          std::cout << "ERROR: Observed more nets than expected" << std::endl;
          input_file.close();
          return 0;
        } else {
          int blockages = 0;
          int x1,y1,x2,y2,new_cap;
          int edgeID;
          net_stream >> blockages;
          for (int i = 0; i < blockages ; ++i){
            std::getline(input_file,line);
            std::stringstream net_stream(line);
            net_stream >> x1 >> y1 >> x2 >> y2 >> new_cap;
            edgeID = getEdgeID(x1,y1,x2,y2,rst->gx,rst->gy);
            if (edgeID >= 0) {
              rst->edgeCaps[edgeID] = new_cap;
            } else {
              std::cout << "ERROR: Points not adjacent while updating capacities" << std::endl;
              return 0;
            }
          }
        }
      }
    }
    input_file.close();
    return 1;
  } else {
    input_file.close();
    std::cout << "ERROR: Could not open file!" << std::endl;
    return 0;
  }
}

int solveRouting(routingInst *rst){
  point P1, P2, currentNode, nextNode;                  // Points to store pins of the nets
  int hEdgesInSeg, vEdgesInSeg; 
  int edgeIndex = -1;
  int pinIndex = -1;           // Number of pins of the net
  int edgeID = -1;
  //Iterating through all the nets 
  for(int i=0; i<rst->numNets; i++) {  
    pinIndex = 0;
    //Number of segments = Number of pins - 1
    rst->nets[i].nroute.numSegs = rst->nets[i].numPins-1;
    rst->nets[i].nroute.segments = new segment[rst->nets[i].nroute.numSegs];
    //Iterating through all the segments 
    for(int j=0; j<rst->nets[i].nroute.numSegs; j++) {
      edgeIndex = 0;
      //assigning x and y value of the net to the segment
      P1 = rst->nets[i].pins[pinIndex];
      pinIndex++;
      P2 = rst->nets[i].pins[pinIndex];
      rst->nets[i].nroute.segments[j].p1 = P1;
      rst->nets[i].nroute.segments[j].p2 = P2;
      hEdgesInSeg = std::abs(rst->nets[i].nroute.segments[j].p1.x - rst->nets[i].nroute.segments[j].p2.x);
      vEdgesInSeg = std::abs(rst->nets[i].nroute.segments[j].p1.y - rst->nets[i].nroute.segments[j].p2.y);
      rst->nets[i].nroute.segments[j].numEdges = hEdgesInSeg + vEdgesInSeg;
      rst->nets[i].nroute.segments[j].edges = new int[rst->nets[i].nroute.segments[j].numEdges];
      // Using temporary nodes to traverse between nodes
      currentNode = P1;
      nextNode = currentNode;
      // Even-Odd implementation
      if (j%2==0) {
        // If loop index is even traverse horizontally first
        while(currentNode.x != P2.x) {
          if(currentNode.x < P2.x)
            nextNode.x++;
          else
            nextNode.x--;
          edgeID = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);
          rst->nets[i].nroute.segments[j].edges[edgeIndex] = edgeID;
          rst->edgeUtils[edgeID]++;
          currentNode = nextNode;
          edgeIndex++;
        }
        // Traversing vertically
        while(currentNode.y != P2.y) {
          if(currentNode.y < P2.y) 
            nextNode.y++;
          else
            nextNode.y--;
          edgeID = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);
          rst->nets[i].nroute.segments[j].edges[edgeIndex] = edgeID;
          rst->edgeUtils[edgeID]++;
          currentNode = nextNode;
          edgeIndex++;
        }
      } else {
        // Else if loop index is odd traverse vertically first
        while(currentNode.y != P2.y) {
          if(currentNode.y < P2.y) 
            nextNode.y++;
          else
            nextNode.y--;
          edgeID = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);
          rst->nets[i].nroute.segments[j].edges[edgeIndex] = edgeID;
          rst->edgeUtils[edgeID]++;
          currentNode = nextNode;
          edgeIndex++;
        }
        // Traversing horizontally
        while(currentNode.x != P2.x) {
          if(currentNode.x < P2.x)
            nextNode.x++;
          else
            nextNode.x--;
          edgeID = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);
          rst->nets[i].nroute.segments[j].edges[edgeIndex] = edgeID;
          rst->edgeUtils[edgeID]++;
        currentNode = nextNode;
        edgeIndex++;
        }
      }
    } 
  }
  return 1;
}

int writeOutput(const char *outRouteFile, routingInst *rst){
  point printPoints[2];
  point *startPoints;
  int prevEdgeID, currentEdgeID, edgeDifference;
  std::ofstream out_file(outRouteFile);
  if(out_file.is_open()){
    for(int i = 0; i < rst->numNets; i++){
      out_file << "n" << rst->nets[i].id << "\n";
      for(int j = 0; j < rst->nets[i].nroute.numSegs; j++) {
        //for(int k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
          //startPoints = getEdgePts(rst->nets[i].nroute.segments[j].edges[k], rst);
          //out_file << *(startPoints) << "," << *(startPoints + 1) << "\n";
        //}
        segment presentsegment = rst->nets[i].nroute.segments[j];
        int *edges = presentsegment.edges;
        prevEdgeID     = edges[0];
        startPoints = getEdgePts(prevEdgeID, rst);
        if (rst->nets[i].nroute.segments[j].p1 == *(startPoints) ||  
            rst->nets[i].nroute.segments[j].p1 == *(startPoints + 1) ) {
          printPoints[0] = rst->nets[i].nroute.segments[j].p1;
        } else {
          printPoints[0] = rst->nets[i].nroute.segments[j].p2;
        }
        printPoints[1] = nextPoint(printPoints[0], prevEdgeID, rst);
        if((printPoints[1].x < 0) || (printPoints[1].y < 0)){
          std::cout << "ERROR: nextPoint function call returned negative value \n";
          std::cout << "ERROR: P1: "<< printPoints[0] << " P2: " << printPoints[1] << " edgeID: "<<  prevEdgeID <<"\n";
          return 0;
        }
        for(int k = 1; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
          currentEdgeID  = rst->nets[i].nroute.segments[j].edges[k];
          edgeDifference = std::abs(currentEdgeID - prevEdgeID);
    			//Check if the next edge is a bend then assign the pivot as the next startpoint
					if (edgeDifference != 1 && edgeDifference != rst->gx) {
		        out_file << printPoints[0] << "," << printPoints[1] << "\n";
						printPoints[0] = printPoints[1];
					}
    			//If its a stright edge find the end point
					printPoints[1] = nextPoint(printPoints[1], currentEdgeID, rst);
          if((printPoints[1].x < 0) || (printPoints[1].y < 0)){
            std::cout << "ERROR: nextPoint function call returned negative value\n";
            std::cout << "ERROR: P1: "<< printPoints[0] << " P2: " << printPoints[1] << " edgeID: "<<  currentEdgeID <<"\n";
            return 0;
          }
					prevEdgeID = currentEdgeID;
        }
        out_file << printPoints[0] << "," << printPoints[1] << "\n";
      }
      out_file << "!\n";
    }
    out_file.close();
    return 1;
  } else {  // Else if output file is not open
    std::cout << "ERROR: Unable to write to output file.\n";
    out_file.close();
    return 0;
  }
}

int release(routingInst *rst){
  if(rst->nets != NULL){
    for(int i=0; i<rst->numNets; i++) {
      if(rst->nets[i].nroute.segments != NULL) {
        for(int j = 0; j < rst->nets[i].nroute.numSegs; j++) {
          // if(rst->nets[i].nroute.segments[j].edges != NULL) {
          //   delete[] rst->nets[i].nroute.segments[j].edges;  
          // }
          try {
            delete[] rst->nets[i].nroute.segments[j].edges;  
          } catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
          }
        }
        delete[] rst->nets[i].nroute.segments;
      }
      delete[] rst->nets[i].pins;
    }
  }
  delete[] rst->nets;
  delete[] rst->edgeCaps;
  delete[] rst->edgeUtils;
  delete[] rst->edgeOverFlow;
  delete[] rst->edgeUtilityHistory;
  return 1;
}
  
int getEdgeID (int x1, int y1, int x2, int y2, int gx, int gy){
  if      (y1 != y2) return (std::min(y1,y2) * gx + (gx-1) * (gy) + x1); // Points are vertical
  else if (x1 != x2) return (std::min(x1,x2) + y1 * (gx-1)); // Points are horizontal
  else return -1; // Points are not adjacent
}

int getEdgeIDthruPts(point a, point b, routingInst* rst){
  return getEdgeID(a.x,a.y,b.x,b.y,rst->gx,rst->gy);
}

point* getEdgePts(int edgeID, routingInst* rst){
  static point endPts[2];
  // Reverse the edgeID to get the 2 points
  // Find if the edgeID is vertical or horizontal
  int isVert = (edgeID - ((rst->gx - 1) * rst->gy));
  
  // isVert >= 0 -> Vertical or else horizontal
  if(isVert >= 0) {
    // Vertical Edge ID
    endPts[0].y = floor(isVert/rst->gx);
    endPts[1].y = endPts[0].y + 1;
    endPts[0].x = isVert - (endPts[0].y * rst->gx); 
    endPts[1].x = endPts[0].x;
  } else {
    // Horizontal Edge ID
    endPts[0].y = floor((edgeID) / (rst->gx-1));
    endPts[1].y = endPts[0].y;
    endPts[0].x = edgeID - (endPts[0].y * (rst->gx - 1));
    endPts[1].x = endPts[0].x + 1;
  }
  return endPts;
}

point nextPoint(point p1, int edgeID, routingInst *rst) {
  point ptToReturn;
  //If the given edge is the horizontal edge
	if(edgeID < ((rst->gx - 1) * rst->gy)) {	
		//if the edge is going right or the left direction 
		point leftPoint(p1.x - 1, p1.y);
		point rightPoint(p1.x + 1, p1.y);
		if(getEdgeID(p1.x, p1.y, leftPoint.x, leftPoint.y, rst->gx, rst->gy) == edgeID) {
			ptToReturn = leftPoint;
		}
		else if(getEdgeID(p1.x, p1.y, rightPoint.x, rightPoint.y, rst->gx, rst->gy) == edgeID) {
      ptToReturn = rightPoint;
		}
	} else {  
    // If the given edge is the vertical edge
		// Find if the edge is going top or the bottom
		point bottomPoint(p1.x, p1.y - 1);
		point topPoint(p1.x, p1.y + 1);
		if(getEdgeID(p1.x, p1.y, bottomPoint.x, bottomPoint.y, rst->gx, rst->gy) == edgeID) {
			ptToReturn = bottomPoint;
		}
		else if(getEdgeID(p1.x, p1.y, topPoint.x, topPoint.y, rst->gx, rst->gy) == edgeID) {
      ptToReturn = topPoint;
		}
	}
  return ptToReturn;
}

void writePtToFile (std::ofstream &outFile, point *P){
  // Print points as per specification
  outFile << "(" << P[0].x <<","<< P[0].y << ")";
  outFile << "-";
  outFile << "(" << P[1].x <<","<< P[1].y << ")\n";
}

int netDecompose (routingInst* rst) {
  int minManDist, calcManDist;
  // Iterate through all nets
  for(int i = 0; i < rst->numNets; i++){
    if (rst->nets[i].numPins > 2) {
      if(i&1) {
        // If iterator is odd quicksort the nets according to the X axis
        qsort(rst->nets[i].pins, rst->nets[i].numPins, sizeof(point), compareX);
      } else {
        // Quicksort the nets according to the Y axis
        qsort(rst->nets[i].pins, rst->nets[i].numPins, sizeof(point), compareY);
      }
      
      // BRUTE FORCE THE SOLUTION
      // TODO: Find a better approach than Bubble
      minManDist = manDist(rst->nets[i].pins[0], rst->nets[i].pins[1]);

      // Go through all the pins to calc the manDist
      for (int j = 0; j < rst->nets[i].numPins - 1; j++){
        minManDist = manDist(rst->nets[i].pins[j], rst->nets[i].pins[j+1]);
        for (int k = j+2; k < rst->nets[i].numPins; k++){
          calcManDist = manDist(rst->nets[i].pins[j], rst->nets[i].pins[k]);
          if (calcManDist < minManDist){
            minManDist = calcManDist;
            std::swap(rst->nets[i].pins[j+1], rst->nets[i].pins[k]);
          }
        }
      }
    }
    
    // DEBUG ONLY //
    // std::cout << "net: n" << rst->nets[i].id << " numPins: " << rst->nets[i].numPins << std::endl;
    
    // for (int z = 0 ; z < rst->nets[i].numPins; z++) {
    //   std::cout << "(" << rst->nets[i].pins[z].x << "," << rst->nets[i].pins[z].y << ")\n";
    // }
    // DEBUG ONLY ENDS //

  }
  return 1;
}

int manDist (point p1, point p2){
  return (std::abs((p2.x) - (p1.x)) + std::abs((p2.y) - (p1.y)));
}

int compareX (const void *a, const void *b){
  point *p1 = (point *) a;
  point *p2 = (point *) b;
  return ((p1->x) - (p2->x));
}

int compareY (const void *a, const void *b){
  point *p1 = (point *) a;
  point *p2 = (point *) b;
  return ((p1->y) - (p2->y));
}
