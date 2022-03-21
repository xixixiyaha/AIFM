#!/bin/bash

source ../../../../shared_distributation.sh

sudo pkill -9 main
make clean
make -j
rerun_local_iokerneld
rerun_mem_server
run_program ./main
kill_local_iokerneld
