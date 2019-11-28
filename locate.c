#include "mpi_util.h"

// return value is the size of res
static int factorize(int n,int** res){
	int sz = 16;
	*res = (int*)malloc(sz*sizeof(int));

	for(int t = 0;;t++){
		if(n <= 1){
			return t;
		}

		for(int i = 2; i <= n;i++){
			if(n % i == 0){
				(*res)[t] = i;
				n /= i;
				break;
			}
		}

		if(t == sz - 1){
			sz *= 2;
			int* tmp = (int*)realloc(*res,sz);
			*res = tmp;
		}
	}
}

void swap(int* a,int* b){
	int tmp = *a;
	*a = *b;
	*b = tmp;
}
void rev(int* b,int* e){
	if(b == e) return;
	e--;
	while(b < e){
		swap(b,e);
		b++;
		e--;
	}
}

// a should be sorted at first call.
int next_permutation(int* begin,int* end){
	if(begin == end) return 0;
	int *i = begin;
	++i;
	if(i == end) return 0;
	i = end;
	i--;
	while(1){
		int* j = i;
		--i;
		if(*i < *j){
			int* k = end;
			while(!(*i < *--k)) ;
			swap(i,k);
			rev(j,end);
			return 1;
		}
		if(i == begin){
			rev(begin,end);
			return 0;
		}
	}
}

int calcS(int n,int dim){
	int sm = 0;
	for(int i = 0; i < n; i++){
		int t = 1;
		for(int j = 0; j < n; j++){
			if(i != j){
				t *= dim[j];
			}
		}
		sm += t;
	}
	return t;
}

static void get_inner_dims(int n,int ndims,const int global[],int local[]){
	int* primes = NULL;
	int len = factorize(n,&primes);
	int workSize = len + ndims - 1;
	int* work = (int*)realloc(primes,workSize*sizeof(int));
	int* dim = (int*)malloc(ndims*sizeof(int));

	const int sep = 1e9;
	for(int i = len; i < workSize; i++){
		work[i] = sep;
	}

	int minS = sep;

	do{
		for(int i = 0; i < ndims; i++){
			dim[i] = 1;
		}
		int k = 0;
		for(int i = 0; i < workSize; i++){
			if(work[i] == sep){
				k++;
			}
			else{
				dim[k] *= work[i];
			}
		}
		
		int ok = 1;
		for(int i = 0; i < ndims; i++){
			if(global[i] % dim[i] != 0){
				ok = 0;
			}
		}
		if(!ok) continue;

		int s = calcS(ndims,dim);
		if(s < minS){
			minS = s;
			for(int i = 0; i < ndims; i++){
				local[i] = dim[i];
			}
		}
	}while(next_permutation(work,work+workSize));

	free(work);
	free(dim);
}

int cart_create(MPI_Comm comm_old,int ndims,const int dims[],MPI_Comm* comm_cart){
	int end_stat = 0;
	int gSize,gId,lSize,lId;
	MPI_Comm splited;
	int* info = NULL;
	int* local = NULL;

	nodeSplit(comm_old,&splited);
	end_stat = gatherSplitInfoTo0(comm_old,splited,&info);
	if(end_stat == -1){
		goto fine;
	}
	
	MPI_Comm_size(comm_old,&gSize);
	MPI_Comm_rank(comm_old,&gId);
	MPI_Comm_size(splited,&lSize);
	MPI_Comm_rank(splited.&lId);

	local = (int*)malloc(ndims*sizeof(int));
	if(local == NULL){
		end_stat = -1;
		goto fine;
	}

	get_inner_dims(lSize,ndims,dims,local);

	// TODO reorder

fine:
	if(info) free(info);
	if(local) free(local);
	return end_stat;
}
