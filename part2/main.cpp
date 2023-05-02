// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include "rrr.h"
#include <chrono>
#include <thread>
// #define ENDTIME 900000 //Define the exit time as 900s (15 mins)
#define ENDTIME 10000

int main(int argc, char **argv)
{

 	if(argc!=5){
 		printf("Usage : ./ROUTE.exe -d=<subnet_gen> -n=<rip&route> <input_benchmark_name> <output_file_name> \n");
 		return 1;
 	}
	
	std::chrono::time_point<std::chrono::high_resolution_clock> globalStart = std::chrono::high_resolution_clock::now();

 	int status;
	u_int32_t totalTime = 0;
	char *subnetGen = argv[1];
	char *rrrEn = argv[2];
	char *inputFileName = argv[3];
 	char *outputFileName = argv[4];

 	/// create a new routing instance
 	routingInst *rst = new routingInst;
	
 	/// Time the read benchmark function
 	std::chrono::time_point<std::chrono::high_resolution_clock> functionStart = std::chrono::high_resolution_clock::now();
	status = readBenchmark(inputFileName, rst);
	std::chrono::time_point<std::chrono::high_resolution_clock> functionStop = std::chrono::high_resolution_clock::now();
	std::chrono::milliseconds functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
	totalTime += functionDuration.count();
	std::cout << std::endl << "Read Benchmark Time : " << functionDuration.count() << " ms \n";
 	
	if(status==0) {
 		printf("ERROR: reading input file \n");
 		return 1;
 	}
	
	/// Decompose net
	if (subnetGen[3]=='1') {
		// Time the netDecompose function
		functionStart = std::chrono::high_resolution_clock::now();
		status = netDecompose(rst);
		functionStop = std::chrono::high_resolution_clock::now();
		functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
		totalTime += functionDuration.count();
		std::cout << "Net Decompose Time : " << functionDuration.count() << " ms \n";
		
		if(status==0) {
			printf("ERROR: Net decomposing failed \n");
			return 1;
 		}		
	}
 	/// run initial routing
 	functionStart = std::chrono::high_resolution_clock::now();
 	status = solveRouting(rst);
	functionStop = std::chrono::high_resolution_clock::now();
	functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
	totalTime += functionDuration.count();
	std::cout << "Solve Time : " << functionDuration.count() << " ms \n";
 	
	if(status==0) {
 		printf("ERROR: running routing \n");
 		release(rst);
 		return 1;
 	}
	
	/// write the result
 	functionStart = std::chrono::high_resolution_clock::now();
 	status = writeOutput(outputFileName, rst);
	functionStop = std::chrono::high_resolution_clock::now();
	functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
	std::cout << "Write Output Time After Initial Solution : " << functionDuration.count() << " ms \n";
	totalTime += functionDuration.count();
 	
	if(status==0) {
 		printf("ERROR: writing the result \n");
 		release(rst);
 		return 1;
 	}


	/// Rip up and Reroute
	if (rrrEn[3]=='1')
	{	
		functionStart = std::chrono::high_resolution_clock::now();
	 	// Take current time
		std::chrono::time_point<std::chrono::high_resolution_clock> rrrTime = std::chrono::high_resolution_clock::now();
		std::chrono::milliseconds rrrDuration = std::chrono::duration_cast<std::chrono::milliseconds>(rrrTime - globalStart);
		
		std::chrono::time_point<std::chrono::high_resolution_clock> wrStart, wrStop;
		std::chrono::milliseconds wrDuration;
		int wrStatus = 0;		
		u_int32_t rrrIter = 1;
		long rrrUpdatedCost, bestCost;
		// Perform the edge weight calculation once
		edgeWeightCal(rst);
		bestCost = rst->totalRoutingCost;
		while (rrrDuration.count() < ENDTIME)
		{	
			std::cout << "RRR Iteration: " << rrrIter << "\n";
			
			// Perform RRR
			status = rrr(rst, rrrIter);
			if (status == 0) {
				std::cout << "Could not perform RRR" << std::endl;
				release(rst);
				return 0;
			} else {
				// Calc total routing inst cost
				rrrUpdatedCost = rst->totalRoutingCost;
				if(rrrUpdatedCost < bestCost){
					bestCost = rrrUpdatedCost;
					std::cout << "Best cost is " << bestCost << "\n";

					wrStart = std::chrono::high_resolution_clock::now();
					wrStatus = writeOutput(outputFileName, rst);
					wrStop = std::chrono::high_resolution_clock::now();
					wrDuration = std::chrono::duration_cast<std::chrono::milliseconds>(wrStop - wrStart);
					std::cout << "Write Output Time after Best Cost: " << wrDuration.count() << " ms \n";
					totalTime += wrDuration.count();
					
					if(wrStatus==0) {
						printf("ERROR: writing the result \n");
						release(rst);
						return 1;
					}
				}
			}
			
			/////////////// REMOVE THIS ONCE DONE /////////////////////////////
			//  std::this_thread::sleep_for(std::chrono::milliseconds(100));  //
			/////////////// REMOVE THIS ONCE DONE /////////////////////////////

			rrrTime = std::chrono::high_resolution_clock::now(); 
			rrrDuration = std::chrono::duration_cast<std::chrono::milliseconds>(rrrTime - globalStart);
			rrrIter++;
		}
		
		functionStop = std::chrono::high_resolution_clock::now();
		functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
		totalTime += functionDuration.count();
		std::cout << "RRR Time : " << functionDuration.count() << " ms \n";
	}

 	// /// write the result
 	// functionStart = std::chrono::high_resolution_clock::now();
 	// status = writeOutput(outputFileName, rst);
	// functionStop = std::chrono::high_resolution_clock::now();
	// functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
	// std::cout << "Write Output Time : " << functionDuration.count() << " ms \n";
	// totalTime += functionDuration.count();
 	
	// if(status==0) {
 	// 	printf("ERROR: writing the result \n");
 	// 	release(rst);
 	// 	return 1;
 	// }

 	functionStart = std::chrono::high_resolution_clock::now();
 	release(rst);
	functionStop = std::chrono::high_resolution_clock::now();
	functionDuration = std::chrono::duration_cast<std::chrono::milliseconds>(functionStop - functionStart);
	std::cout << "Release Time : " << functionDuration.count() << " ms \n";
	totalTime += functionDuration.count();
 	
	printf("\nDONE!\n");	
 	std::cout << std::endl << "Total Time : " << totalTime <<" ms \n";
	return 0;
}
