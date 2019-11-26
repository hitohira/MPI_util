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
		buf[i] = (double*)malloc(dataSize * sizeof(double));
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

int cmpFunc(const void* a,const void* b){
	return *(double*)a <  *(double*) b ? -1 :
	       *(double*)a == *(double*) b ?  0 : 1;
}

double getMode(int idx,int numprocs,int repTimes,double* timearr_sub){
	double *tmp = (double*)malloc(repTimes*sizeof(double));
	if(tmp == NULL){
		return -1;
	}
	for(int i = 0; i < repTimes; i++){
		tmp[i] = timearr_sub[numprocs*i+idx];
	}
	qsort(tmp,repTimes,sizeof(double),cmpFunc);
	double res = tmp[repTimes/2];

	free(tmp);
	return res;
}

/*
 * commumicate and measure time
 */
static int doCommunicate(MPI_Comm comm,int numprocs,int myid,int dataSize,double** sendbuf,double** recvbuf,double* timearr){
	MPI_Request *sendreq,*recvreq;
	MPI_Status status;
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
			MPI_Wait(&sendreq[i2],&status);		
			double t2 = timer();
			timearr[i2] = getSpan(t1,t2);
		}
		else{
			timearr[i2] = 0;
		}
	}

	for(int i = 0; i < numprocs; i++){
		if(i != myid){
			MPI_Wait(&recvreq[i],&status);
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
int calcDistance(MPI_Comm comm,int dataSize,int repTimes,double** timearr){
	int numprocs,myid;
	int flag = 0;
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

	*timearr = (double*)malloc(5*numprocs*sizeof(double));
	if(*timearr == NULL){
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
	timearr_sub = (double*)malloc(repTimes*numprocs * sizeof(double));
	if(timearr_sub == NULL){
		end_stat = -1;
		goto fine;
	}

	for(int i = 0; i < numprocs; i++){
		(*timearr)[i] = 0.0;
		(*timearr)[numprocs+i] = 0.0;
		(*timearr)[numprocs*2+i] = 1000000000.0;
		(*timearr)[numprocs*3+i] = 0.0;
		(*timearr)[numprocs*4+i] = 0.0;

	}
	
	for(int i = 0; i < repTimes; i++){
		flag = doCommunicate(comm,numprocs,myid,dataSize,sendbuf,recvbuf,timearr_sub+i*numprocs);
		if(flag == -1){
			end_stat = -1;
			goto fine;
		}
		for(int j = 0; j < numprocs; j++){
			double val = timearr_sub[i*numprocs+j];
			(*timearr)[j] += val;
			(*timearr)[numprocs+j] += val * val;
			(*timearr)[numprocs*2+j] = fmin((*timearr)[numprocs*2+j],val);
			(*timearr)[numprocs*3+j] = fmax((*timearr)[numprocs*3+j],val);
		}
	}
	for(int i = 0; i < numprocs; i++){
		(*timearr)[i] /= repTimes;
		(*timearr)[numprocs+i] = (*timearr)[numprocs+i] / repTimes - ((*timearr)[i] * (*timearr)[i]);
		(*timearr)[numprocs+i] = sqrt((*timearr)[numprocs+i]);

		(*timearr)[numprocs*4+i] = getMode(i,numprocs,repTimes,timearr_sub);
		if((*timearr)[numprocs*4+i] < 0){
			end_stat = -1;
			goto fine;
		}
	}

fine:
	if(timearr_sub) free(timearr_sub);
	freeBuf(numprocs,recvbuf);
	freeBuf(numprocs,sendbuf);
	return end_stat;
}


