/*
 * calculate distance between MPI proc using MPI function
 */

#include "mpi_util.h"
#include "MT.h"
#include <time.h>

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

static void shuffleRank(int n,int* a){
	init_genrand((unsigned)time(NULL));
	for(int i = n-1; i > 0; i--){
		int j = genrand_int32() % (i+1);
		int t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
}

/*
 * commumicate and measure time
 */
static int doCommunicate(MPI_Comm comm,int numprocs,int myid,int dataSize,double** sendbuf,double** recvbuf,double* timearr){
	MPI_Request *sendreq,*recvreq;
	sendreq = (MPI_Request*)malloc(numprocs*sizeof(MPI_Request));
	if(sendreq == NULL){
		return -1;
	}
	recvreq = (MPI_Request*)malloc(numprocs*sizeof(MPI_Request));
	if(recvreq == NULL){
		free(sendreq);
		return -1;
	}
	int* rank = (int*)malloc(numprocs*sizeof(int));
	if(rank == NULL){
		free(recvreq);
		free(sendreq);
		return -1;
	}
	for(int i = 0; i < numprocs; i++){
		rank[i] = i;
	}
	shuffleRank(numprocs,rank);

	// prepost irecv
	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			MPI_Irecv(recvbuf[i],dataSize,MPI_DOUBLE,i,0,comm,&recvreq[i]);
		}
	}
	MPI_Barrier(comm);
	for(int i = 0; i < numprocs; i++){
		int i2 = rank[i];
		if(i2 != myid){
			double t1 = timer();
			MPI_Isend(sendbuf[i2],dataSize,MPI_DOUBLE,i2,0,comm,&sendreq[i2]);
			MPI_Wait(&sendreq[i2],NULL);		
			double t2 = timer();
			timearr[i2] = getSpan(t1,t2);
		}
		else{
			timearr[i2] = 0;
		}
	}

	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			MPI_Wait(&recvreq[i],NULL);
		}
	}
	free(rank);
	free(sendreq);
	free(recvreq);
	return 0;
}

/*
 * calculate distance
 * if return value is -1, error occured
 * timearr[2*numproc] [average[numproc],variance[numproc]]
 */
int calcDistance(MPI_Comm comm,int dataSize,int repTimes,double* timearr){
	int numprocs,myid;
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

	MPI_Comm_rank(comm,&myid);
	MPI_Comm_size(comm,&numprocs);

	timearr = (double*)malloc(2*numprocs*sizeof(double));
	if(timearr == NULL){
		end_stat = -1;
		goto fine;
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
		end_stat = -1;
		goto fine;
	}

	for(int i = 0; i < 2*numprocs; i++){
		timearr[i] = 0.0;
	}
	
	for(int i = 0; i < repTimes; i++){
		flag = doCommunicate(comm,numprocs,myid,dataSize,sendbuf,recvbuf,timearr_sub);
		if(flag == -1){
			end_stat = -1;
			goto fine;
		}
		for(int j = 0; j < numprocs; j++){
			timearr[j] += timearr_sub[j];
			timearr[numprocs+j] += timearr_sub[j] * timearr_sub[j];
		}
	}
	for(int i = 0; i < numprocs; i++){
		timearr[j] /= repTimes;
		timearr[numprocs+j] = timearr[numprocs+j] / repTimes - timearr[j];
	}

fine:
	if(timearr_sub) free(timearr_sub);
	freeBuf(numprocs,recvbuf);
	freeBuf(numprocs,sendbuf);
	return end_stat;
}


