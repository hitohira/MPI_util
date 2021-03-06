#include "mpi_util.h"

// nodeInfo[n], gitmearr[n*n]
static void printData(int n,int dataSize,int repTimes,int* nodeInfo,double* gtimearr){
	printf("Average and SD of comm time\nData size : %d\nRepeat time : %d\n",dataSize,repTimes);
	printf("# of process: %d\n\n",n);
	printf("ave sd min max mode\n");
	for(int i = 0; i < n; i++){
		printf("process: %d\n",i);
		printf("\tnode group: %d\n",nodeInfo[i]);
		printf("\tcomm time\n");
		for(int j = 0; j < n; j++){
			printf("\t\t%4d : %.6f\t%.6f\t%.6f\t%.6f\t%6f\n",j,gtimearr[5*i*n+j],gtimearr[(5*i+1)*n+j],gtimearr[(5*i+2)*n+j],gtimearr[(5*i+3)*n+j],gtimearr[(5*i+4)*n+j]);
		}
	}
}

void showDistInfo(MPI_Comm comm){
	int dataSize = 1 << 18;
	int repTimes = 100;
	int err;

	MPI_Comm splited;
	MPI_Status status;
	int* nodeInfo = NULL;
	double* gtimearr = NULL;
	double* timearr = NULL;

	int numprocs,myid;

	MPI_Comm_rank(comm,&myid);
	MPI_Comm_size(comm,&numprocs);

	err = calcDistanceP2P(comm,dataSize,repTimes,&timearr);
	if(err == -1){
		fprintf(stderr,"calcDist error\n");
		goto fine;
	}


	nodeSplit(comm,&splited);
	err = gatherSplitInfoTo0(comm,splited,&nodeInfo);
	if(err == -1){
		fprintf(stderr,"gather split info error\n");
		goto fine;
	}

	gtimearr = (double*)malloc(5*numprocs*numprocs*sizeof(double));
	if(gtimearr == NULL){
		fprintf(stderr,"mallc error\n");
		goto fine;
	}

	MPI_Gather(timearr,5*numprocs,MPI_DOUBLE,gtimearr,5*numprocs,MPI_DOUBLE,0,comm);

	if(myid == 0){	
		printData(numprocs,dataSize,repTimes,nodeInfo,gtimearr);
	}
fine:
	if(gtimearr) free(gtimearr);
	if(timearr) free(timearr);
	if(nodeInfo) free(nodeInfo);
}
