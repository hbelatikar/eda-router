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
	char *subnet_gen = argv[1];
	char *rrr = argv[2];
	char *inputFileName = argv[3];
 	char *outputFileName = argv[4];

 	/// create a new routing instance
 	routingInst *rst = new routingInst;
	
 	/// read benchmark
 	auto start = std::chrono::high_resolution_clock::now();
	status = readBenchmark(inputFileName, rst);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Read Benchmark Time : " << duration.count() << "ms \n";
 	
	if(status==0) {
 		printf("ERROR: reading input file \n");
 		return 1;
 	}
	
 	/// run actual routing
 	start = std::chrono::high_resolution_clock::now();
 	status = solveRouting(rst);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Solve Time : " << duration.count() << "ms \n";
 	
	if(status==0) {
 		printf("ERROR: running routing \n");
 		release(rst);
 		return 1;
 	}
	
 	/// write the result
 	start = std::chrono::high_resolution_clock::now();
 	status = writeOutput(outputFileName, rst);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "Write Output Time : " << duration.count() << "ms \n";
 	
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
 	
	printf("\nDONE!\n");	
 	return 0;
}
