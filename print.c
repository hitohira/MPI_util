#include "mpi_util.h"

// nodeInfo[n], gitmearr[n*n]
static void printData(int n,int dataSize,int repTimes,int* nodeInfo,double* gtimearr){
	printf("Average of comm time\nData size : %d\nRepeat time : %d\n",dataSize,repTimes);
	printf("# of process: %d\n\n",n);
	for(int i = 0; i < n; i++){
		printf("process: %d\n",i);
		printf("\tnode group: %d\n",nodeInfo[i]);
		printf("\tcomm time\n");
		for(int j = 0; j < n; j++){
			printf("\t\t%d : %.4f\n",j,gtimearr[i*n+j]);
		}
	}
}

void showDistInfo(MPI_Comm comm){
	int dataSize = 1024;
	int repTimes = 20;
	int err;

	MPI_Comm splited;
	MPI_Status status;
	int* nodeInfo = NULL;
	int* gtimearr = NULL;
	int* timearr = NULL;

	int numprocs,myid;

	MPI_Comm_rank(comm,&myid);
	MPI_Comm_size(comm,&numprocs);

	err = calcDistance(comm,dataSize,repTimes,timearr);
	if(err == -1){
		fprintf(stderr,"calcDist error\n");
		goto fine;
	}


	nodeSplit(comm,&splited);
	err = gatherSplitInfoTo0(comm,splited,nodeInfo);
	if(err == -1){
		fprintf(stderr,"gather split info error\n");
		goto fine;
	}

	gtimearr = (double*)malloc(numprocs*numprocs*sizeof(double));
	if(gtimearr == -1){
		fprintf(stderr,"mallc error\n");
		goto fine;
	}

	MPI_Gather(timearr,numprocs,MPI_DOUBLE,gtimearr,numprocs,MPI_DOUBLE,0,comm);
	
	printData(numprocs,dataSize,repTimes,nodeInfo,gtimearr);

fine:
	if(gtimearr) free(gtimearr);
	if(timearr) free(timearr);
	if(nodeInfo) free(nodeInfo);
}
