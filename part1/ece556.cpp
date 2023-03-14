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
        rst->edgeCaps = (int*)malloc(rst->numEdges*sizeof(int));
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
            if (edgeID <= 0) rst->edgeCaps[edgeID] = new_cap;
            else {
              std::cout << "ERROR: Points not adjacent while updating capacities" << std::endl;
              return 0;
            }
          }
        }
        for (int i = rst->numEdges - 1; i >= 0; i--) 
          std::cout << "Edge :" << i << " Cap :" << rst->edgeCaps[i] << "\n";
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
  /*********** TO BE FILLED BY YOU **********/

  return 1;
}

int writeOutput(const char *outRouteFile, routingInst *rst){
  /*********** TO BE FILLED BY YOU **********/

  return 1;
}


int release(routingInst *rst){
  
  free(rst->edgeCaps);
  // free(rst->edgeUtils);
  free(rst->nets->pins);
  free(rst->nets);
  return 1;
}
  
int getEdgeID (int x1, int y1, int x2, int y2, int gx, int gy){
  if (y2 > y1)      return (y1*gx + (gx-1)*(gy) + x1); // P1 is below P2
  else if (y2 < y1) return (y2*gx + (gx-1)*(gy) + x1); // P1 is above P2
  else if (x2 > x1) return (x1 + y1*(gx-1));               // P1 is left of P2
  else if (x2 < x1) return (x2 + y1*(gx-1));               // P1 is right of P2
  else              return -1;                                 // P1 and P2 are not adjacent
}