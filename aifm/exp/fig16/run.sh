#!/bin/bash

source ../../shared_distributation.sh

sudo pkill -9 main
make clean
make -j CXXFLAGS="-DMONITOR_FREE_MEM_RATIO"

rerun_local_iokerneld
rerun_mem_server
echo > log.average
run_program ./main 1>log.average 2>&1

sudo pkill -9 main
kill_local_iokerneld
kill_mem_server1
kill_mem_server2
make clean
