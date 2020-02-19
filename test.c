#include <stdio.h>
#include "mpi_util.h"

int main(){
	int n = 16;
	int ndims = 2;
	int global[3] = {12,12,4};
	int local[3];

	get_inner_dims(n,ndims,global,local);
	for(int i = 0; i < 3; i++){
		printf("dim %d = %d\n",i,local[i]);
	}
	return 0;

}
