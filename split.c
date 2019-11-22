#include "mpi_util.h"

static void setInfo(int n,int k,int* localBuf,int* info){
	for(int i = 0; i < n; i++){
		info[localBuf[i]] = k;
	}
}


void nodeSplit(MPI_Comm comm,MPI_Comm* splited){
	MPI_Comm_split_type(comm,MPI_COMM_TYPE_SHARED,0,MPI_INFO_NULL,splited);
}

int gatherSplitInfoTo0(MPI_Comm comm,MPI_Comm splited,int* info){
	int globalRank,localRank;
	int globalSize,localSize;
	int* localBuf = NULL;
	MPI_Status status;

	MPI_Comm_rank(comm,&globalRank);
	MPI_Comm_size(comm,&globalSize);
	MPI_Comm_rank(splited,&localRank);
	MPI_Comm_size(splited,&localSize);

	info = (int*)malloc(globalSize*sizeof(int));
	localBuf = (int*)malloc(localSize*sizeof(int));

	if(info == NULL || localBuf == NULL){
		if(localBuf) free(localBuf);
		return -1;
	}
	
	MPI_Gather(&globalRank,1,MPI_INT,localBuf,1,0,splited);
	
	if(globalRank == 0){
		int numNode = globalSize / localSize;
		for(int i = 0; i < numNode; i++){
			if(i != 0){
				MPI_Recv(localBuf,localSize,MPI_ANY_SOURCE,0,comm,&status);
			}
			setInfo(localSize,i,localBuf,info);
		}
	}
	else if(localRank == 0){
		MPI_Send(localBuf,localSize,MPI_INT,0,0,comm);
	}
	
	if(localBuf) free(localBuf);
	return 0;
}
