SYSTEM     = x86-64_sles10_4.1
LIBFORMAT  = static_pic

# ---------------------------------------------------------------------         
# Compiler selection                                                            
# ---------------------------------------------------------------------         

CCC = g++

# ---------------------------------------------------------------------         
# Compiler options                                                              
# ---------------------------------------------------------------------         

CCOPT = -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -std=c++11 -g -Wall  

# ---------------------------------------------------------------------         
# Link options and libraries                                                    
# ---------------------------------------------------------------------         

CCFLAGS = $(CCOPT) 
CCLNFLAGS = -lm -pthread 

#------------------------------------------------------------                   
#  make all      : to compile.                                     
#  make execute  : to compile and execute.                         
#------------------------------------------------------------    

ROUTE.exe: main.o ece556.o rrr.o
	/bin/rm -f ROUTE.exe
	$(CCC) $(LINKFLAGS) $(CCFLAGS) main.o ece556.o rrr.o $(CCLNFLAGS) -o ROUTE.exe

main.o: main.cpp ece556.h rrr.h
	/bin/rm -f main.o
	$(CCC) $(CCFLAGS) main.cpp -c

ece556.o: ece556.cpp ece556.h
	/bin/rm -f ece556.o
	$(CCC) $(CCFLAGS) ece556.cpp -c

rrr.o: rrr.cpp ece556.h rrr.h
	/bin/rm -f rrr.o
	$(CCC) $(CCFLAGS) rrr.cpp -c

clean:
	/bin/rm -f *~ *.o ROUTE.exe 

.PHONY: all

all: ROUTE.exe

.PHONY: execute

execute: ROUTE.exe
	./ROUTE.exe -d=1 -n=1 ../benchmarks/in.gr outputs/out.txt

.PHONY: execute_all

execute_all: ROUTE.exe
	/bin/rm -f /outputs/adaptec*_out.txt
	./ROUTE.exe -d=1 -n=0 ../benchmarks/adaptec1.gr outputs/adaptec1_out.txt
	./ROUTE.exe -d=1 -n=0 ../benchmarks/adaptec2.gr outputs/adaptec2_out.txt
	./ROUTE.exe -d=1 -n=0 ../benchmarks/adaptec3.gr outputs/adaptec3_out.txt

