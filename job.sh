#!/bin/bash

#PBS -q u-debug
#PBS -l select=4:ncpus=36:mpiprocs=12:ompthreads=0
#PBS -W group_list=TODO
#PBS -l walltime=00:10:00
#PBS -o result.txt
#PBS -e err.txt


cd $PBS_O_WORKDIR
mpiexec -np 48 ./main
