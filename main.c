#include "mpi_util.h"

int main(int argc,char** argv){
	int myid,numprocs;

	MPI_Init(&argc,&argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

	int dataSize = 128*128;
	int repTimes = 100;
	
	double t1 = timer();
	/*
	for(int width = 1; width <= numprocs / 2; width++){
//		showDistInfo(MPI_COMM_WORLD);
		ping_pong(MPI_COMM_WORLD,dataSize,repTimes,width);
	}
	*/
	//now bcast mode!!!!!!!!!!!!!!!!!!!!!
//	ping_pong(MPI_COMM_WORLD,dataSize,repTimes,1);

	MPI_Comm comm_cart;
	int dims[3] = {4,6,2};
	int period[3] = {0,0,0};
	MPIMY_Cart_create2(MPI_COMM_WORLD,3,dims,period,&comm_cart);
	int newid;
	MPI_Comm_rank(comm_cart,&newid);
	double t2 = timer();
	printf("%d %d\n",myid,newid);

//	int rank;
//	MPI_Comm_rank(comm_cart,&rank);
//	printf("%d\t%d\n",myid,rank);

	if(myid == 0){
		printf("Total time : %.5f s\n",getSpan(t1,t2));
	}

	MPI_Finalize();
	return 0;
}
