// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

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
  int horizontalEdgesInSegment, verticalEdgesInSegment; 
  int edgeIndex = -1;
  int pinIndex = -1;           // Number of pins of the net
  
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
      
      horizontalEdgesInSegment = std::abs(rst->nets[i].nroute.segments[j].p1.x - rst->nets[i].nroute.segments[j].p2.x);
      verticalEdgesInSegment = std::abs(rst->nets[i].nroute.segments[j].p1.y - rst->nets[i].nroute.segments[j].p2.y);
      rst->nets[i].nroute.segments[j].numEdges = horizontalEdgesInSegment + verticalEdgesInSegment;
      rst->nets[i].nroute.segments[j].edges = new int[rst->nets[i].nroute.segments[j].numEdges];
      
      // Using temporary nodes to traverse between nodes
      currentNode = P1;
      nextNode = currentNode;

      // Traversing horizontally
      while(currentNode.x != P2.x) {
        if(currentNode.x < P2.x)
			    nextNode.x++;
		    else
			    nextNode.x--;
        rst->nets[i].nroute.segments[j].edges[edgeIndex] = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);;
        currentNode = nextNode;
        edgeIndex++;
      }
      // Traversing vertically
      while(currentNode.y != P2.y) {
        if(currentNode.y < P2.y) 
			    nextNode.y++;
		    else
			    nextNode.y--;
        rst->nets[i].nroute.segments[j].edges[edgeIndex] = getEdgeID(currentNode.x, currentNode.y, nextNode.x, nextNode.y, rst->gx, rst->gy);
        currentNode = nextNode;
        edgeIndex++;
      }
    } 
  }
  return 1;
}

int writeOutput(const char *outRouteFile, routingInst *rst){
  //point *edge1;
  point printPoints[1];
  int prevEdgeID, currentEdgeID, edgeDifference;
  std::ofstream out_file(outRouteFile);
  if(out_file.is_open()){
    for(int i = 0; i < rst->numNets; i++){
      out_file << "n" << rst->nets[i].id << std::endl;
      for(int j = 0; j < rst->nets[i].nroute.numSegs; j++) {
        prevEdgeID = rst->nets[i].nroute.segments[j].edges[0];
        printPoints[0]= rst->nets[i].nroute.segments[j].p1;
        printPoints[1] = nextPoint(printPoints[0], prevEdgeID, rst);
        for(int k = 1; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
          currentEdgeID = rst->nets[i].nroute.segments[j].edges[k];
          edgeDifference = std::abs(currentEdgeID - prevEdgeID);
					//Check if the next edge is the bend then assign the pivot as the next startpoint
					if (edgeDifference != 1 && edgeDifference != rst->gx)
					{
		        writePtToFile(out_file, printPoints);
						printPoints[0] = printPoints[1];
					}
					//If its a stright edge find the end point
					printPoints[1] = nextPoint(printPoints[1], currentEdgeID, rst);
					prevEdgeID = currentEdgeID;
          // edge1 = revEdgeID(rst->nets[i].nroute.segments[j].edges[k], rst->gx, rst->gy, rst); 
          // if( prevEdgeID == -1) {
          //   printPoints[0] = edge1[0];
          //   printPoints[1] = edge1[1];
          // } else if ((rst->nets[i].nroute.segments[j].edges[k] != prevEdgeID + 1)  &&  
          //            (rst->nets[i].nroute.segments[j].edges[k] != prevEdgeID + rst->gx)) {
          //   writePtToFile(out_file, printPoints);
          //   printPoints[0] = edge1[0];
          //   printPoints[1] = edge1[1];
          // } else {
          //   printPoints[1] = edge1[1];
          // }
          // prevEdgeID = rst->nets[i].nroute.segments[j].edges[k];
        }
        writePtToFile(out_file, printPoints);
      }
      out_file << "!" << std::endl;
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
      if(rst->nets->nroute.segments != NULL) {
        for(int j = 0; j < rst->nets[i].nroute.numSegs; j++) {
          if(rst->nets[i].nroute.segments[j].edges != NULL) {
            delete rst->nets[i].nroute.segments[j].edges;  
          }
        }
        delete rst->nets[i].nroute.segments;
      }
      delete rst->nets[i].pins;
    }
  }
  delete rst->nets;
  delete rst->edgeCaps;
  delete rst->edgeUtils;
  return 1;
}
  
int getEdgeID (int x1, int y1, int x2, int y2, int gx, int gy){
  if      (y1 != y2) return (std::min(y1,y2) * gx + (gx-1) * (gy) + x1); // Points are vertical
  else if (x1 != x2) return (std::min(x1,x2) + y1 * (gx-1)); // Points are horizontal
  else return -1; // Points are not adjacent
}
point nextPoint(point p1, int edgeid, routingInst *rst)
{
  //If the given edge is the horizontal edge
	if(edgeid < ((rst->gx - 1) * rst->gy))
	{	
		//if the edge is going right or the left direction 
		point leftSidePoint = {p1.x - 1, p1.y};
		point rightSidePoint = {p1.x + 1, p1.y};
		if(getEdgeID(p1.x, p1.y, leftSidePoint.x, leftSidePoint.y, rst->gx, rst->gy) == edgeid)
		{
			return(leftSidePoint);
		}
		else if(getEdgeID(p1.x, p1.y, rightSidePoint.x, rightSidePoint.y, rst->gx, rst->gy) == edgeid)
		{
			return(rightSidePoint);
		}
	}
	//If the given edge is the vertical edge
	else {
		//Find if the edge is going top or the bottom
		point bottomPoint = {p1.x, p1.y - 1};
		point topPoint = {p1.x, p1.y + 1};
		if(getEdgeID(p1.x, p1.y, bottomPoint.x, bottomPoint.y, rst->gx, rst->gy) == edgeid)
		{
			return(bottomPoint);
		}
		else if(getEdgeID(p1.x, p1.y, topPoint.x, topPoint.y, rst->gx, rst->gy) == edgeid)
		{
			return(topPoint);
		}
	}
}
point* revEdgeID(int edgeID, int gx, int gy, routingInst *rst){
  static point endPoints[2]; //Store the 2 endpoints here
  int bottom, up, left, right;
  int edgeDiff = (((gx-1) * gy) - 1) - edgeID; // Calculate the edge diffrence with the max horizontal edge
  up = endPoints[0].y + 1;
  bottom = endPoints[0].y - 1;
  if(edgeDiff < 0) {
    edgeDiff = std::abs(edgeDiff);
    // This is a vertical edge
    endPoints[0].x = endPoints[1].x = (edgeDiff-1)%(gx);  // P1.x = P2.x -> Extra edges after the x-axis
    endPoints[0].y = (edgeDiff/(gx+1)); // Y cord based on how many V edges are present through x axis 
    if(getEdgeID(endPoints[0].x,endPoints[0].y,endPoints[1].x, up, rst->gx,rst->gy)==edgeID)
    {
      endPoints[1].y = endPoints[0].y + 1;
    }
    else if(getEdgeID(endPoints[0].x,endPoints[0].y,endPoints[1].x, bottom, rst->gx,rst->gy)==edgeID)
    {
      endPoints[1].y = endPoints[0].y - 1;
    }
  } else if (edgeDiff >= 0){
    // This is a horizontal edges
    endPoints[0].x = edgeID%(gx-1);
    endPoints[0].y = endPoints[1].y = (edgeID)/(gx - 1);  // P1.y = P2.y
    left = endPoints[0].x - 1;
    right = endPoints[0].x + 1;
    if(getEdgeID(endPoints[0].x,endPoints[0].y, right, up, rst->gx,rst->gy)==edgeID)
    {
      endPoints[1].x = endPoints[0].x + 1;
    }
    else if(getEdgeID(endPoints[0].x,endPoints[0].y, left , bottom, rst->gx,rst->gy)==edgeID)
    {
      endPoints[1].x = endPoints[0].x - 1;
    }
  } else {
    // Error in input
    endPoints[0].x = endPoints[1].x = -1;
    endPoints[0].y = endPoints[1].y = -1;
  }
  return endPoints;
}

void writePtToFile (std::ofstream &outFile, point *P){
  // Print points as per specification
  outFile << "(" << P[0].x <<","<< P[0].y << ")";
  outFile << "-";
  outFile << "(" << P[1].x <<","<< P[1].y << ")" << std::endl;
}