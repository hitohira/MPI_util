#ifndef MPI_UTIL_H
#define MPI_UTIL_H

#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// timer.c
double timer();
double getSpan(double t1,double t2);

// distance.c
int calcDistance(MPI_Comm comm,int dataSize,int repTimes,double** timearr);
int calcDistanceP2P(MPI_Comm comm,int dataSize,int repTimes,double** timearr);

// split.c
void nodeSplit(MPI_Comm comm,MPI_Comm* splited);
int gatherSplitInfoTo0(MPI_Comm comm,MPI_Comm splited,int** info);

// print.c
void showDistInfo(MPI_Comm);

#endif
