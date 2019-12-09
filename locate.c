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

static void swap(int* a,int* b){
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

int calcS(int n,int* dims){
	int sm = 0;
	for(int i = 0; i < n; i++){
		int t = 1;
		for(int j = 0; j < n; j++){
			if(i != j){
				t *= dims[j];
			}
		}
		sm += t;
	}
	return sm;
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

static void get_global_coord(int ndims,int nodeId,int localId,int* global,int* local,int** coord){
	*coord = (int*)malloc(ndims*sizeof(int));
	if(*coord == NULL){
		return;
	}
	if(ndims == 2){
		int width = global[1] / local[1];
		(*coord)[0] = (nodeId / width) * local[0] + localId / local[1];
		(*coord)[1] = (nodeId % width) * local[1] + localId % local[1];
	}
	else if(ndims == 3){
		if(localId == 0 && nodeId == 0){
			printf("come\n");
		}
		int width = global[1] / local[1];
		int height = global[0] / local[0];
		int area = width * height;
		(*coord)[2] = nodeId / area * local[2] + localId / (local[0] * local[1]);
		if(localId == 0 && nodeId == 0){
			printf("come\n");
		}
		int snodeId = nodeId % area;
		int slocalId = localId % (local[0] * local[1]);
		if(localId == 0 && nodeId == 0){
			printf("come\n");
		}
		(*coord)[0] = snodeId / width * local[0] + slocalId / local[1];
		(*coord)[1] = snodeId % width * local[1] + slocalId % local[1];
		if(localId == 0 && nodeId == 0){
			printf("come\n");
		}
	}
}

static int get_global_rank(int ndims,int* global,int* coord){
	int rank = 0;
	for(int i = ndims-1; i >= 0; i--){
		int area = 1;
		for(int j = 0; j < i; j++){
			area *= global[j];
		}
		rank += area * coord[i];
	}
	return rank;
}

int MPIMY_Cart_create(MPI_Comm comm_old,int ndims,int dims[],int periods[],MPI_Comm* comm_cart){
	int end_stat = 0;
	int gSize,gId,lSize,lId,nodeId;
	MPI_Comm splited;
	int* info = NULL;
	int* local = NULL;
	int* coord = NULL;

	nodeSplit(comm_old,&splited);
	end_stat = gatherSplitInfo(comm_old,splited,&info);
	if(end_stat == -1){
		goto fine;
	}
	
	MPI_Comm_size(comm_old,&gSize);
	MPI_Comm_rank(comm_old,&gId);
	MPI_Comm_size(splited,&lSize);
	MPI_Comm_rank(splited,&lId);
	
	int numP = 1;
	for(int i = 0; i < ndims; i++){
		numP *= dims[i];
	}
	if(gSize != numP){
		end_stat = -1;
		printf("wrong num\n");
		goto fine;
	}

	nodeId = info[gId];

	local = (int*)malloc(ndims*sizeof(int));
	if(local == NULL){
		end_stat = -1;
		goto fine;
	}

	get_inner_dims(lSize,ndims,dims,local);
	if(local == NULL){
		end_stat = -1;
		goto fine;
	}
	for(int i = 0; i < ndims; i++){
		if(gId == 0){
			printf("dim %d %d\n",i,local[i]);
		}
	}
	get_global_coord(ndims,nodeId,lId,dims,local,&coord);
	if(coord == NULL){
		end_stat = -1;
		goto fine;
	}
	int new_rank = get_global_rank(ndims,dims,coord);
	if(gId == 0){
		printf("new rank 0 = %d\n",new_rank);
	}

	MPI_Comm comm_new;
	MPI_Comm_split(comm_old,0,new_rank,&comm_new);
	end_stat = MPI_Cart_create(comm_new,ndims,dims,periods,0,comm_cart);

fine:
	if(info) free(info);
	if(local) free(local);
	if(coord) free(coord);
	return end_stat;
}
