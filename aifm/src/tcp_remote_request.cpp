extern "C" {
#include <runtime/runtime.h>
#include <runtime/tcp.h>
#include <net/ip.h>
#include <runtime/storage.h>
}


#include "helpers.hpp"

#include "array.hpp"
#include "device.hpp"

#include "manager.hpp"
#include "server.hpp"
#include "shared_pool.hpp"
#include "stats.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using namespace far_memory;
using namespace std;
 // Request format:
  //     |OpCode (1B)|Data (optional)|
  // All possible OpCode:
  //     0. init
  //     1. shutdown
  //     2. read_object
  //     3. write_object
  //     4. remove_object
  //     5. construct
  //     6. destruct
  //     7. compute
constexpr static uint32_t kOpcodeSize = 1;
constexpr static uint32_t kPortSize = 2;
constexpr static uint32_t kLargeDataSize = 512;
constexpr static uint32_t kMaxComputeDataLen = 65535;

constexpr static uint8_t kOpInit = 0;
constexpr static uint8_t kOpShutdown = 1;
constexpr static uint8_t kOpReadObject = 2;
constexpr static uint8_t kOpWriteObject = 3;
constexpr static uint8_t kOpRemoveObject = 4;
constexpr static uint8_t kOpConstruct = 5;
constexpr static uint8_t kOpDeconstruct = 6;
constexpr static uint8_t kOpCompute = 7;



constexpr static uint64_t kCacheSize = (128ULL << 20);
constexpr static uint64_t kFarMemSize = (4ULL << 30);
constexpr static uint32_t kNumGCThreads = 12;
constexpr static uint32_t kNumEntries =
    (8ULL << 20); // So the array size is larger than the local cache size.
constexpr static uint32_t kNumConnections = 300;

uint64_t array_C[5];
uint64_t obj_id[5]={274146542516,274146668138,274146530824,274146764463,274146749679};
uint8_t ds_id[5];
// ={0,0,0,0,0}

// array_C_ds_id[78009] : �
// array_C_data[78009] : 6725001401512693756
// array_C_object_id[78010] : 274146542516,274146668138,274146530824,274146764463,274146749679
// array_C_ds_id[78010] : �
// array_C_data[78010] : 3142845970594195479
// array_C_object_id[78011] : 274146668138,274146530824,274146764463,274146749679
// array_C_ds_id[78011] : �
// array_C_data[78011] : 7774978804262704663
// array_C_object_id[78012] : 274146530824,274146764463,274146749679
// array_C_ds_id[78012] : �
// array_C_data[78012] : 5378496769351390329
// array_C_object_id[78013] : 274146764463,274146749679
// array_C_ds_id[78013] : �
// array_C_data[78013] : 342761107379922716
// array_C_object_id[78014] : 274146749679

//  reinterpret_cast<uint8_t *>(&obj_id),
// auto obj_data_addr = reinterpret_cast<uint8_t *>(obj.get_data_addr());


tcpconn_t *remote_master_;
SharedPool<tcpconn_t *> shared_pool_ =  far_memory::SharedPool<tcpconn*>(kNumConnections);
/*
// get:obj_id, ds_id, obj_id_len, device_id

//首先根据device_id确认数据在哪里，然后根据device_index选择ip绑定连接
//参考test_tcp文件 main()function

//连接建立：process——init（），把原来的farmemsize这个配置去掉
// TCPDevice(netaddr raddr, uint32_t num_connections, uint64_t far_mem_size);
// -> TCPRequestDevice(netaddr raddr, uint32_t num_connections);

// Request:
// |Opcode = KOpReadObject(1B) | ds_id(1B) | obj_id_len(1B) | obj_id |
// |Opcode = 操作符 | ds_id(1B) | obj_id_len(1B) | obj_id |
（其余三个参数通过指针获得，只需要把meta的这三个参数传过去就好
// Response:
// |data_len(2B)|data_buf(data_len B)| 
//所以需要组织一下拿回来的数据
//还有一个判断，判断数据对于caller程序来说是否是local的（先用device_index来判断吧）：
    （1）如果数据在caller的local_node，要用address把数据拿到，直接把数据取出来，通过tcp传回去；
    （2）如果数据本身在caller的远端，就通过object_id来获取；
*/
// void TCPDevice::read_object(uint8_t ds_id, uint8_t obj_id_len,
//                             const uint8_t *obj_id, uint16_t *data_len,
//                             uint8_t *data_buf)



void connect_init(netaddr raddr, uint32_t num_connections, uint64_t far_mem_size){
  cout<<"connect_init"<<endl;
  // Initialize the master connection.
  netaddr laddr = {.ip = MAKE_IP_ADDR(0, 0, 0, 0), .port = 0};
  BUG_ON(tcp_dial(laddr, raddr, &remote_master_) != 0);
  char req[kOpcodeSize + sizeof(far_mem_size)];
  __builtin_memcpy(req, &kOpInit, kOpcodeSize);
  __builtin_memcpy(req + kOpcodeSize, &far_mem_size, sizeof(far_mem_size));
  cout<<"connect_init"<<endl;
  helpers::tcp_write_until(remote_master_, req, sizeof(req));
  uint8_t ack;
  helpers::tcp_read_until(remote_master_, &ack, sizeof(ack));

  // Initialize slave connections.
  tcpconn_t *remote_slave;
  for (uint32_t i = 0; i < num_connections; i++) {
    // BUG_ON(tcp_dial(laddr, raddr, &remote_slave) != 0);
    shared_pool_.push(remote_slave);
  }
  cout<<"connect_init_finish"<<endl;
}

void _read_object(tcpconn_t *remote_slave, uint8_t ds_id,
                             uint8_t obj_id_len, const uint8_t *obj_id,
                             uint16_t *data_len, uint8_t *data_buf) {
  cout<<"_read_object"<<endl;
  Stats::start_measure_read_object_cycles();
  cout<<"_read_object_1"<<endl;
  uint8_t req[kOpcodeSize + Object::kDSIDSize + Object::kIDLenSize +
              Object::kMaxObjectIDSize];
  cout<<"_read_object_2"<<endl;
  __builtin_memcpy(&req[0], &kOpReadObject, sizeof(kOpReadObject));
  __builtin_memcpy(&req[kOpcodeSize], &ds_id, Object::kDSIDSize);
  __builtin_memcpy(&req[kOpcodeSize + Object::kDSIDSize], &obj_id_len,
                   Object::kIDLenSize);
  memcpy(&req[kOpcodeSize + Object::kDSIDSize + Object::kIDLenSize], obj_id,
         obj_id_len);
         
  // cout<<"obj_id is: "<<*obj_id<<endl;
  // cout<<"obj_id_len: "<<obj_id_len<<endl;

  // auto start = kOpcodeSize + Object::kDSIDSize + Object::kIDLenSize;
  // auto end = start + obj_id_len;
  // cout<<"start_len: "<<start<<endl;
  // cout<<"stop_len: "<<end<<endl;
  // cout<<"the request length is "<<sizeof(req)<<endl;
  // cout<<"_read_object_3"<<endl;
  helpers::tcp_write_until(remote_slave, req, sizeof(req));
  cout<<"_read_object_4"<<endl;
  helpers::tcp_read_until(remote_slave, data_len, sizeof(*data_len));
  if (*data_len) {
    helpers::tcp_read_until(remote_slave, data_buf, *data_len);
  }
  cout<<"_read_object_5"<<endl;
  Stats::finish_measure_read_object_cycles();
}

void tcp_read_object(uint8_t ds_id, uint8_t obj_id_len,
                            const uint8_t *obj_id, uint16_t *data_len,
                            uint8_t *data_buf) {
  cout<<"tcp_read_object"<<endl;
  auto remote_slave = shared_pool_.pop();//从池化里面拿出一个connection
  cout<<"tcp_read_object_1"<<endl;
  _read_object(remote_slave, ds_id, obj_id_len, obj_id, data_len, data_buf);//读取object
  cout<<"tcp_read_object_2"<<endl;
  shared_pool_.push(remote_slave);
  cout<<"tcp_read_object_finish"<<endl;
}

void do_work(uint64_t obj_id[]){
  cout<<"do_work()"<<endl;
  uint16_t obj_data_len;
  uint8_t obj_id_len = sizeof(obj_id[0]);
  for(uint64_t i=0;i<5;i++){
    uint8_t *obj_id_req = reinterpret_cast<uint8_t *>(&obj_id[i]);
    // cout<<"test obj_id_req: "<<*obj_id_req<<endl;
    tcp_read_object(ds_id[i],obj_id_len,obj_id_req,&obj_data_len,reinterpret_cast<uint8_t *>(&array_C+i));
  }
  cout<<"do_work_finish"<<endl;
  
}

void run(netaddr raddr1,netaddr raddr2) {
  connect_init(raddr1,kNumConnections,kFarMemSize);
  // connect_init(raddr2,kNumConnections,kFarMemSize);
  // BUG_ON(madvise(all_gen_reqs, sizeof(Req) * kNumReqs, MADV_HUGEPAGE) != 0);
  // FarMemDevice* device = new TCPDevice(raddr, kNumConnections, kFarMemSize);
  do_work(obj_id);
  for(uint64_t i=0;i<5;i++){
    cout<<array_C[i]<<endl;
  }
  
}

int argc;
void _main(void *arg) {
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port1(argv[1]);//ip_addr_port1 is the node_0(compute node)
  auto raddr1 = helpers::str_to_netaddr(ip_addr_port1);
  std::string ip_addr_port2(argv[2]);//ip_addr_port2 is the node_2(another memory node)
  auto raddr2 = helpers::str_to_netaddr(ip_addr_port2);
  // cout<<"addr1 is "<<raddr1<<endl;
  // cout<<"addr2 is "<<raddr2<<endl;
  // run(raddr1);
  run(raddr1,raddr2);

}

int main(int _argc, char *argv[]) {
  int ret;

  if (_argc < 3) {
    std::cerr << "usage: [cfg_file] [ip_addr:port]..." << std::endl;
    return -EINVAL;
  }

  char conf_path[strlen(argv[1]) + 1];
  strcpy(conf_path, argv[1]);
  for (int i = 2; i < _argc; i++) {
    argv[i - 1] = argv[i];
  }
  argc = _argc - 1;

  ret = runtime_init(conf_path, _main, argv);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}
