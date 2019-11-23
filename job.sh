#!/bin/bash

#PBS -q TODO
#PBS -l select=4:ncpus=4:mpiprocs=4:ompthreads=0
#PBS -W group_list=TODO
#PBS -l walltime=00:30:00
#PBS -o result.txt
#PBS -e err.txt


cd $PBS_O_WORKDIR
mpiexec -np 16 ./main
