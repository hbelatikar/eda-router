// ECE556 - Copyright 2014 University of Wisconsin-Madison.  All Rights Reserved.

#include "ece556.h"

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
 	status = readBenchmark(inputFileName, rst);
 	if(status==0) {
 		printf("ERROR: reading input file \n");
 		return 1;
 	}
	
 	/// run actual routing
 	status = solveRouting(rst);
 	if(status==0) {
 		printf("ERROR: running routing \n");
 		release(rst);
 		return 1;
 	}
	
 	/// write the result
 	status = writeOutput(outputFileName, rst);
 	if(status==0) {
 		printf("ERROR: writing the result \n");
 		release(rst);
 		return 1;
 	}

 	release(rst);
 	printf("\nDONE!\n");	
 	return 0;
}
