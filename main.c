#include "mpi_util.h"

int main(int argc,char** argv){
	int myid;

	MPI_Init(&argc,&argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	double t1 = timer();

	showDistInfo(MPI_COMM_WORLD);

	double t2 = timer();
	
	if(myid == 0){
		printf("Total time : %.5f s\n",getSpan(t1,t2));
	}

	MPI_Finalize();
	return 0;
}
