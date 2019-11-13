#ifndef MPI_UTIL_H
#define MPI_UTIL_H

#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>

// timer.c
double timer();
double getSpan(double t1,double t2);

// distance.c
int calcDistance(MPI_Comm,int numprocs,int myid,int dataSize,int repTimes,double* timearr);

#endif
