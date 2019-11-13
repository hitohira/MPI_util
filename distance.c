/*
 * calculate distance between MPI proc using MPI function
 */

#include "mpi_util.h"

/*
 * init buffer
 */
static double** initBuf(int numprocs,int dataSize,int* flag){
	double** buf = (double**)malloc(numprocs * sizeof(double*));
	if(buf == NULL){
		*flag = -1;
		return NULL;
	}
	for(int i = 0; i < numprocs; i++){
		buf[i] = NULL;
		buf[i] = (double*)mallc(dataSize * sizeof(double));
		if(buf[i] == NULL){
			*flag = -1;
			return NULL;
		}
		for(int j = 0; j < dataSize; j++){
			buf[i][j] = j;
		}
	}
	*flag = 0;
	return buf;
}
/*
 * free buffer
 */
static void freeBuf(int numprocs, double** buf){
	if(buf != NULL){
		for(int i = 0; i < numprocs; i++){
			if(buf[i] != NULL){
				free(buf[i]);
			}
		}
		free(buf);
	}
}

/*
 * commumicate and measure time
 */
static double* doCommunicate(MPI_Comm comm,int numprocs,int myid,int dataSize,double** sendbuf,double** recvbuf,double* timearr){
	MPI_Request *sendreq,*recvreq;
	sendreq = (MPI_Request*)malloc(numprocs*sizeof(MPI_Request));
	if(sendreq == NULL){
		return NULL;
	}
	recvreq = (MPI_Request*)malloc(numprocs*sizeof(MPI_Request));
	if(recvreq == NULL){
		free(sendreq);
		return NULL;
	}

	// prepost irecv
	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			MPI_Irecv(recvbuf[i],dataSize,MPI_DOUBLE,i,0,comm,&recvreq[i]);
		}
	}
	MPI_Barrier(comm);
	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			double t1 = timer();
			MPI_Isend(sendbuf[i],dataSize,MPI_DOUBLE,i,0,comm,&sendreq[i]);
			MPI_Wait(&sendreq[i],NULL);		
			double t2 = timer();
			timearr[i] = getSpan(t1,t2);
		}
	}

	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			MPI_Wait(&recvreq[i],NULL);
		}
	}
	free(sendreq);
	free(recvreq);
}

/*
 * calculate distance
 * if return value is -1, error occured
 */
int calcDistance(MPI_Comm comm,int numprocs,int myid,int dataSize,int repTimes,double* timearr){
	int flag = 0;
	double* timearr = NULL;
	double* timearr_sub = NULL;
	int end_stat = 0;
	double** sendbuf = NULL;
	double** recvbuf = NULL;

	MPI_Initialized(&flag);
	if(!flag){
		return -1;
	}
	
	sendbuf = initBuf(numprocs,dataSize,&flag);
	if(flag == -1) {
		end_stat = -1;
		goto fine;
	}
	recvbuf = initBuf(numprocs,dataSize,&flag);
	if(flag == -1){
		end_stat = -1;
		goto fine;
	}
	timearr_sub = (double*)malloc(numprocs * sizeof(double));
	if(timearr_sub == NULL){
		end_stat == -1;
		goto fine;
	}

	for(int i = 0; i < numprocs; i++){
		timearr[i] = 0.0;
	}
	
	for(int i = 0; i < repTimes; i++){
		doCommunicate(comm,numprocs,myid,dataSize,sendbuf,recvbuf,timearr_sub);
		for(int j = 0; j < numprocs; j++){
			timearr[j] += timearr_sub[j];
		}
	}
	for(int i = 0; i < numprocs; i++){
		timearr[j] /= repTimes;
	}

fine:
	if(timearr_sub != NULL) free(timearr_sub);
	freeBuf(numprocs,recvbuf);
	freeBuf(numprocs,sendbuf);
	return end_stat;
}


