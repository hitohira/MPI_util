#!/bin/bash

#PBS -q u-debug
#PBS -l select=2:ncpus=32:mpiprocs=8:ompthreads=0
#PBS -W group_list=gi16
#PBS -l walltime=00:30:00
#PBS -o result.txt
#PBS -e err.txt


cd $PBS_O_WORKDIR
mpiexec -np 16 ./main
