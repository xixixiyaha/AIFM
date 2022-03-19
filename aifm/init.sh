
AIFM_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SHENANGO_PATH=$AIFM_PATH/../shenango
MEM_SERVER_DPDK_IP_1=128.110.218.117
MEM_SERVER_DPDK_IP_2=128.110.218.92
MEM_SERVER_PORT=8000
MEM_SERVER_STACK_KB=65536
function run_program {    
    sudo stdbuf -o0 sh -c "$1 $AIFM_PATH/configs/client.config \
                           $MEM_SERVER_DPDK_IP_1:$MEM_SERVER_PORT \
                           $MEM_SERVER_DPDK_IP_2:$MEM_SERVER_PORT"
}

cd ../shenango
sudo ./scripts/setup_machine.sh
sudo ./scripts/setup_machine.sh
cd -
sudo $SHENANGO_PATH/iokerneld simple > /dev/null 2>&1 &

# run_program ./bin/test_array_add_rw_api
# run_program ./bin/test_tcp_array_add

ulimit -s $MEM_SERVER_STACK_KB; 
sudo $AIFM_PATH/bin/tcp_device_server $AIFM_PATH/configs/server.config $MEM_SERVER_PORT