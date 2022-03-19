#!/bin/bash

AIFM_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SHENANGO_PATH=$AIFM_PATH/../shenango

MEM_SERVER_DPDK_IP_1=128.110.218.117
MEM_SERVER_DPDK_IP_2=128.110.218.92
MEM_SERVER_PORT=8000
MEM_SERVER_STACK_KB=65536

source $AIFM_PATH/configs/ssh_distributation

function say_failed() {
    echo -e "----\e[31mFailed\e[0m"
}

function say_passed() {
    echo -e "----\e[32mPassed\e[0m"
}

function assert_success {
    if [[ $? -ne 0 ]]; then
        say_failed
        exit -1
    fi
}

function kill_process {
    pid=`pgrep $1`
    if [ -n "$pid" ]; then
	{ sudo kill $pid && sudo wait $pid; } 2>/dev/null
    fi
}

function ssh_kill_process_1 {
    pid=`ssh_execute_1 "pgrep $1"`
    if [ -n "$pid" ]; then
	ssh_execute_1 "{ sudo kill $pid && sudo wait $pid; } 2>/dev/null"
    fi
}

function ssh_kill_process_2 {
    pid=`ssh_execute_2 "pgrep $1"`
    if [ -n "$pid" ]; then
	ssh_execute_2 "{ sudo kill $pid && sudo wait $pid; } 2>/dev/null"
    fi
}

function kill_local_iokerneld {
    kill_process iokerneld
}

function run_local_iokerneld {
    kill_local_iokerneld
    sudo $SHENANGO_PATH/iokerneld $@ > /dev/null 2>&1 &
    disown -r
    assert_success
    sleep 3
}

function rerun_local_iokerneld {
    kill_local_iokerneld
    run_local_iokerneld simple
}

function rerun_local_iokerneld_noht {
    kill_local_iokerneld
    run_local_iokerneld simple noht
}

function rerun_local_iokerneld_args {
    kill_local_iokerneld
    run_local_iokerneld $@
}

function kill_mem_server_1 {
    ssh_kill_process_1 iokerneld
    ssh_kill_process_1 tcp_device_serv
}

function kill_mem_server_2 {
    ssh_kill_process_2 iokerneld
    ssh_kill_process_2 tcp_device_serv
}


function run_mem_server_1 {
    ssh_execute_1 "sudo $SHENANGO_PATH/iokerneld simple" > /dev/null 2>&1 &
    sleep 3
    ssh_execute_tty_1 "sudo sh -c 'ulimit -s $MEM_SERVER_STACK_KB; \
                     $AIFM_PATH/bin/tcp_device_server $AIFM_PATH/configs/server.config \
                     $MEM_SERVER_PORT'" > /dev/null 2>&1 &
    sleep 3
}

function run_mem_server_2 {
    ssh_execute_2 "sudo $SHENANGO_PATH/iokerneld simple" > /dev/null 2>&1 &
    sleep 3
    ssh_execute_tty_2 "sudo sh -c 'ulimit -s $MEM_SERVER_STACK_KB; \
                     $AIFM_PATH/bin/tcp_device_server $AIFM_PATH/configs/server.config \
                     $MEM_SERVER_PORT'" > /dev/null 2>&1 &
    sleep 3
}

function rerun_mem_server {
    kill_mem_server_1
    run_mem_server_1
    kill_mem_server_2
    run_mem_server_2
}

function run_program {    
    sudo stdbuf -o0 sh -c "$1 $AIFM_PATH/configs/client.config \
                           $MEM_SERVER_DPDK_IP_1:$MEM_SERVER_PORT \
                           $MEM_SERVER_DPDK_IP_2:$MEM_SERVER_PORT"
}

function run_program_noht {
    sudo stdbuf -o0 sh -c "$1 $AIFM_PATH/configs/client_noht.config \
                           $MEM_SERVER_DPDK_IP_1:$MEM_SERVER_PORT \
                           $MEM_SERVER_DPDK_IP_2:$MEM_SERVER_PORT"
}
