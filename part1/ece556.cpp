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
        stream >> rst->gx >> rst->gy;
        rst->numEdges = (rst->gx * (rst->gy - 1) + rst->gy * (rst->gx - 1));
      } else if (token == "capacity") {
        stream >> rst->cap;
      } else if (token == "num") {
        stream >> token >> rst->numNets; // Temporarily writing into "token" to ignore "net" from I/P file
        
        for (uint32_t i = 0; i < rst->numNets; i++) {
          std::getline(input_file,line);
          std::stringstream net_stream(line);
          rst->nets[i].numPins = -1;
          if(line[0] == 'n') {
            net_stream >> token >> rst->nets[i].numPins;
          } else {
            if (rst->nets[i].numPins == -1) {
              std::cout << "ERROR during parsing net : n" << i << std::endl;
              return 0;
            } else {
              for (uint32_t j = 0; j < rst->nets[i].numPins; j++) {
                net_stream >> rst->nets[i].pins[j].x >> rst->nets[i].pins[j].y;
              }
            }
          }
        }
      }
    }
    input_file.close();
    return 1;
  } else {
    input_file.close();
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
  /*********** TO BE FILLED BY YOU **********/

  return 1;
}
  

