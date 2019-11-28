#include "mpi_util.c"
#include <math.h>

static int cmp(const void* a,const void* b){
	return *(double*)a > *(double*)b ?  1 :
	       *(double*)a < *(double*)b ? -1 : 0;

static double measeure_comm(MPI_Comm,int myid,int numprocs,int dataSize,double* sendbuf,double* recvbuf){
	MPI_Status status;
	MPI_Barrier(comm);
	double t1 = timer();
	if(myid < width){
		MPI_Send(sendbuf,dataSize,MPI_DOUBLE,myid+width,0,comm);
	}
	else if(myid < width*2){
		MPI_Recv(recvbuf,dataSize,MPI_DOUBLE,myid-width,0,comm,&status);
	}
	MPI_Barrier(comm);
	double t2 = timer();
	return getSpan(t1,t2);
}

void ping_pong(MPI_Comm comm,int dataSize,int repTimes,int width){
	double* sendbuf = NULL;
	double* recvbuf = NULL;
	double* timearr = NULL;
	int numprocs,myid;

	MPI_Comm_size(comm,&numprocs);
	MPI_Comm_rank(comm,&myid);

	if(width > numprocs / 2){
		printf("too large width\n");
		return;
	}

	sendbuf = (double*)malloc(dataSize*sizeof(double));
	recvbuf = (double*)malloc(dataSize*sizeof(double));
	timearr = (double*)malloc(repTimes*sizeof(double));
	if(sendbuf == NULL || recvbuf == NULL){
		printf("malloc failed\n");
		goto fine;
	}
	
	double ave = 0;
	double sd = 0;
	double mn = 0;
	double mx = 0;
	double md = 0;
	measeure_comm(comm,myid,numprocs,dataSize,sendbuf,recvbuf);
	for(int i = 0; i < repTimes; i++){
		timearr[i] = measeure_comm(comm,myid,numprocs,dataSize,sendbuf,recvbuf);
		ave += timearr[i];
		sd += timearr[i] * timearr[i];
	}
	ave /= numprocs;
	sd = sqrt(sd / numprocs - (ave*ave));
	qsort(timearr,repTimes,sizeof(double),cmp);
	mn = timearr[0];
	mx = timearr[repTimes-1];
	md = timearr[repTimes/2];
	
	if(myid == 0){
		printf("data size = %d B\n# proc = %d\nwidth = %d",dataSize*sizeof(double),numprocs,width);
		printf("\tave\tSD\tmin\tmax\tmode\n");
		printf("\t\.5f\t\.5f\t\.5f\t\.5f\t\.5f\n",ave,sd,mn,mx,md);
	}

fine:
	if(sendbuf) free(sendbuf);
	if(recvbuf) free(recvbuf);
}
