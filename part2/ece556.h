// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.


#ifndef ECE556_H
#define ECE556_H

#include <stdio.h>
#include <fstream>
#include <iostream>

 /**
  * A class to represent a 2D Point. 
  */
class point {
  public:
    int x;
    int y;

    // Declaring constructors and overloading operators for pointHashing
    point (): x(-1), y(-1) {}
    point (int new_x, int new_y) : x(new_x), y(new_y) {}
    
    bool operator!=(const point &p) const {
      return ((x != p.x) || (y != p.y));
    }
    
    bool operator==(const point &p) const {
      return ((x == p.x) && (y == p.y));
    }
    
    bool operator<(const point &p) const {
      return (x < p.x);
    }
    
    bool operator>(const point &p) const {
      return (x > p.x);
    }
    
    friend std::ostream &operator<<(std::ostream& os, const point& p) {
      os <<"(" << p.x << "," << p.y << ")";
      return os;
    }
};

/**
* A structure to represent a segment
*/
 typedef struct
 {
   point p1 ; 	/* start point of a segment */
   point p2 ; 	/* end point of a segment */
   
   int numEdges ; /* number of edges in the segment*/
   int *edges ;  	/* array of edges representing the segment*/
   
 } segment ;
 
 
  /**
  * A structure to represent a route
  */
  typedef struct
  {
    int numSegs ;  	/* number of segments in a route*/
    segment *segments ;  /* an array of segments (note, a segment may be flat, L-shaped or any other shape, based on your preference */

  } route ;
 
 
  /**
  * A structure to represent nets
  */
  typedef struct
  {

   int id ; 		/* ID of the net */
   int numPins ; 		/* number of pins (or terminals) of the net */
   point *pins ; 		/* array of pins (or terminals) of the net. */
   route nroute ;		/* stored route for the net. */
   int cost ;        /*variable introduced to store the cost of a net in order to reorder the nets*/
  } net ;
  
  /**
  * A structure to represent the routing instance
  */
  typedef struct
  {
    int gx ;		/* x dimension of the global routing grid */
    int gy ;		/* y dimension of the global routing grid */
    
    int cap ;

    long totalRoutingCost;

    int numNets ;	/* number of nets */
    net *nets ;		/* array of nets */
    
    int numEdges ; 	/* number of edges of the grid */
    int *edgeCaps; 	/* array of the actual edge capacities after considering for blockages */
    int *edgeUtils;	/* array of edge utilizations */  
    int *edgeUtilityHistory;
    int *edgeOverFlow;
    int *edgeWeight;  
  
  } routingInst ;
  

/* int readBenchmark(const char *fileName, routingInst *rst)
   Read in the benchmark file and initialize the routing instance.
   This function needs to populate all fields of the routingInst structure.
   input1: fileName: Name of the benchmark input file
   input2: pointer to the routing instance
   output: 1 if successful
*/
int readBenchmark(const char *fileName, routingInst *rst);

  
/* int solveRouting(routingInst *rst)
   This function creates a routing solution
   input: pointer to the routing instance
   output: 1 if successful, 0 otherwise (e.g. the data structures are not populated) 
*/
int solveRouting(routingInst *rst);
  
/* int writeOutput(const char *outRouteFile, routingInst *rst)
   Write the routing solution obtained from solveRouting(). 
   Refer to the project link for the required output format.

   Finally, make sure your generated output file passes the evaluation script to make sure
   it is in the correct format and the nets have been correctly routed. The script also reports
   the total wirelength and overflow of your routing solution.

   input1: name of the output file
   input2: pointer to the routing instance
   output: 1 if successful, 0 otherwise 
  */
  int writeOutput(const char *outRouteFile, routingInst *rst);
  
  /* int release(routingInst *rst)
    Release the memory for all the allocated data structures. 
    Failure to release may cause memory problems after multiple runs of your program. 
    Need to recursively delete all memory allocations from bottom to top 
    (starting from segments then routes then individual fields within a net struct, 
    then nets, then the fields in a routing instance, and finally the routing instance)

    output: 1 if successful, 0 otherwise 
  */
 int release(routingInst *rst);

  /* int getEdgeID(int x1, int y1, int x2, int y2, int gx, int gx)
    Provides the edge ID based on the cartesian end points of the grid.
    Requries the two end points and grid size.
    For the 3x3 grid, the edge IDs are as
    (0,2) - 4 - (1,2) - 5 - (2,2)
      |           |           |
      9           10          11
      |           |           |
    (0,1) - 2 - (1,1) - 3 - (2,1)
      |           |           |
      6           7           8
      |           |           |
    (0,0) - 0 - (1,0) - 1 - (2,0)
  */
  int getEdgeID(int, int, int, int, int, int);

  int getEdgeIDthruPts(point, point, routingInst*);

  point* getEdgePts(int edgeID, routingInst* rst);

  /* point nextPoint(point, int, routingInst*)
    
  */
  point nextPoint(point , int, routingInst *);
  
  /* void writePtToFile(ofstream &, point);
      Write a point to the file pointer according
      to the project requirement
  */
  void writePtToFile(std::ofstream &, point *);
  
  /* int netDecompose(routingInst *);
     Decomposes the net points and swaps the position of 
     points so that the two points are the closest to 
     each other 
  */
  int netDecompose(routingInst *);

  int manDist(point, point);

  int compareX(const void *, const void *);
  int compareY(const void *, const void *);
#endif // ECE556_H
