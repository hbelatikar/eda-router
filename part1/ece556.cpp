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
        rst->nets = (net*)malloc(rst->numNets*sizeof(net));
        
        //Gather all net endpoints detail and net id
        for (int i = 0; i < rst->numNets; i++) {
          std::getline(input_file,line);
          std::stringstream net_stream(line);
          if(line[0] == 'n') {
            net_stream >> token >> rst->nets[i].numPins;
            rst->nets[i].id = stoi(token.substr(1,token.size()-1));
            rst->nets[i].pins = (point*)malloc(rst->nets[i].numPins*sizeof(point));
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
  
  std::ofstream out_file(outRouteFile);
  if(out_file.is_open()){
    std::cout << "Nets: "<< rst->numNets << std::endl;
    for(int i = 0; i < rst->numNets; i++){
      out_file << "n" << rst->nets[i].id << std::endl;
      for(int j = 0; j < rst->nets[i].nroute.numSegs; j++) {
        out_file << "Edge IDs: \n";
        for(int k = 0; k < rst->nets[i].nroute.segments[j].numEdges; k++) {
          // Print p1
          // out_file << "("<<rst->nets[i].nroute.segments[j].p1.x <<","<<rst->nets[i].nroute.segments[j].p1.y << ")";
          // Print the seperator
          // out_file << "-";
          // Print p2
          // out_file << "("<<rst->nets[i].nroute.segments[j].p2.x <<","<<rst->nets[i].nroute.segments[j].p2.y << ")" << std::endl;
          out_file << rst->nets[i].nroute.segments[j].edges[k] << std::endl;
        }
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
  free(rst->edgeCaps);
  free(rst->edgeUtils);
  free(rst->nets->pins);
  free(rst->nets);
  for(int i=0; i<rst->numNets; i++) {
    for(int j = 0; j < rst->nets->nroute.numSegs; j++) 
      free(rst->nets[i].nroute.segments[j].edges);
    free(rst->nets[i].nroute.segments);
  }
  return 1;
}
  
int getEdgeID (int x1, int y1, int x2, int y2, int gx, int gy){
  if (y2 > y1)      return (y1*gx + (gx-1)*(gy) + x1); // P1 is below P2
  else if (y2 < y1) return (y2*gx + (gx-1)*(gy) + x1); // P1 is above P2
  else if (x2 > x1) return (x1 + y1*(gx-1));           // P1 is left of P2
  else if (x2 < x1) return (x2 + y1*(gx-1));           // P1 is right of P2
  else              return -1;                         // P1 and P2 are not adjacent
}