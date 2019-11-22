#include "mpi_util.h"

int main(int argc,char** argv){
	int myid;

	MPI_Init(&argc,&argv);
	
	MPI_Comm_Rank(MPI_COMM_WORLD,&myid);

	showDistInfo(MPI_COMM_WORLD);

	MPI_Finalize();
	return 0;
}
