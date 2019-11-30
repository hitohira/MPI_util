#include "mpi_util.h"

int main(int argc,char** argv){
	int myid,numprocs;

	MPI_Init(&argc,&argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

	int dataSize = 1 << 18;
	int repTimes = 100;
	
	double t1 = timer();
	for(int width = 1; width <= numprocs / 2; width++){
//	showDistInfo(MPI_COMM_WORLD);
		ping_pong(MPI_COMM_WORLD,dataSize,repTimes,width);
	}
	double t2 = timer();

	if(myid == 0){
		printf("Total time : %.5f s\n",getSpan(t1,t2));
	}

	MPI_Finalize();
	return 0;
}
