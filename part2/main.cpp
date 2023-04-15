// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"
#include <chrono>

int main(int argc, char **argv)
{

 	if(argc!=5){
 		printf("Usage : ./ROUTE.exe -d=<subnet_gen> -n=<rip&route> <input_benchmark_name> <output_file_name> \n");
 		return 1;
 	}

 	int status;
	u_int32_t totalTime = 0;
	char *subnetGen = argv[1];
	char *rrr = argv[2];
	char *inputFileName = argv[3];
 	char *outputFileName = argv[4];

 	/// create a new routing instance
 	routingInst *rst = new routingInst;
	
 	/// Time the read benchmark function
 	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	status = readBenchmark(inputFileName, rst);
	std::chrono::time_point<std::chrono::high_resolution_clock> stop = std::chrono::high_resolution_clock::now();
	std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	totalTime += duration.count();
	std::cout << std::endl << "Read Benchmark Time : " << duration.count() << "ms \n";
 	
	if(status==0) {
 		printf("ERROR: reading input file \n");
 		return 1;
 	}
	
	/// Decompose net
	if (subnetGen[3]=='1') {
		// Time the netDecompose function
		start = std::chrono::high_resolution_clock::now();
		status = netDecompose(rst);
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		totalTime += duration.count();
		std::cout << "Net Decompose Time : " << duration.count() << "ms \n";
		
		if(status==0) {
			printf("ERROR: Net decomposing failed \n");
			return 1;
 		}		
	}

 	/// run actual routing
 	start = std::chrono::high_resolution_clock::now();
 	status = solveRouting(rst);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	totalTime += duration.count();
	std::cout << "Solve Time : " << duration.count() << "ms \n";
 	
	if(status==0) {
 		printf("ERROR: running routing \n");
 		release(rst);
 		return 1;
 	}
	
	/// Rip up and Reroute
	if (rrr[3]=='1')
	{
		//Perform RRR
	}

 	/// write the result
 	start = std::chrono::high_resolution_clock::now();
 	status = writeOutput(outputFileName, rst);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Write Output Time : " << duration.count() << "ms \n";
	totalTime += duration.count();
 	
	if(status==0) {
 		printf("ERROR: writing the result \n");
 		release(rst);
 		return 1;
 	}

 	start = std::chrono::high_resolution_clock::now();
 	release(rst);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Release Time : " << duration.count() << "ms \n";
	totalTime += duration.count();
 	
	printf("\nDONE!\n");	
 	std::cout << std::endl << "Total Time : " << totalTime <<"ms \n";
	return 0;
}
