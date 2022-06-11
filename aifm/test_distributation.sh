#!/bin/bash
# [ Check list ]
# Local:  shared_distributation.sh configs/ssh_distributation configs/client.config 
# Server: configs/server.config 
# Make sure that the absolute paths of the local and remote AIFM directorys are the same

source shared_distributation.sh

all_passed=1

function run_single_test {
    echo "Running test $1..."
    rerun_local_iokerneld
    if [[ $1 == *"tcp"* ]]; then
        # rerun_local_iokerneld
    	rerun_mem_server
    fi
    if run_program ./bin/$1 2>/dev/null | grep -q "Passed"; then
        say_passed
    else
        say_failed
    	all_passed=0
    fi

}

function run_all_tests {
    TESTS=`ls bin | grep test_tcp_`
    for test in $TESTS
    do
        run_single_test $test
    done
}

function cleanup {
    kill_local_iokerneld
    kill_mem_server_1
    kill_mem_server_2
}

run_all_tests
cleanup

if [[ $all_passed -eq 1 ]]; then
    exit 0
else
    exit -1
fi
