extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "helpers.hpp"
#include "manager.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using namespace far_memory;
using namespace std;

constexpr static uint64_t kCacheSize = (128ULL << 20);
constexpr static uint64_t kFarMemSize = (4ULL << 30);
constexpr static uint32_t kNumGCThreads = 12;
constexpr static uint32_t kNumEntries =
    (8ULL << 20); // So the array size is larger than the local cache size.
constexpr static uint32_t kNumConnections = 300;

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
void do_work(FarMemDevice *device){
  device->read_object()

}

void run(netaddr raddr) {
  BUG_ON(madvise(all_gen_reqs, sizeof(Req) * kNumReqs, MADV_HUGEPAGE) != 0);
  FarMemDevice* device = new TCPDevice(raddr, kNumConnections, kFarMemSize);
  do_work(device);
}

int argc;
void _main(void *arg) {
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port1(argv[1]);//ip_addr_port1 is the node_0(compute node)
  auto raddr1 = helpers::str_to_netaddr(ip_addr_port1);
  std::string ip_addr_port2(argv[2]);//ip_addr_port2 is the node_2(another memory node)
  auto raddr2 = helpers::str_to_netaddr(ip_addr_port2);
  // run(raddr1);
  run(raddr2);

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
