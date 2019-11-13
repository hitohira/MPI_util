#include "mpi_util.h"

double timer(){
	return MPI_Wtime();	
}

double getSpan(double t1,double t2){
	return t2 - t1;
}

