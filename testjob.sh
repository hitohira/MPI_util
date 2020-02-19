#!/bin/bash

#PBS -q u-debug
#PBS -l select=2:ncpus=36:mpiprocs=16:ompthreads=0
#PBS -W group_list=TODO
#PBS -l walltime=00:30:00
#PBS -o result2.txt
#PBS -e err.txt


cd $PBS_O_WORKDIR
mpiexec -np 32 ./main > result.txt
