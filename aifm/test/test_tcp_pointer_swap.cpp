extern "C" {
#include <runtime/runtime.h>
}

#include "deref_scope.hpp"
#include "device.hpp"
#include "manager.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

using namespace far_memory;
using namespace std;

constexpr static uint64_t kCacheSize = 256 * Region::kSize;
constexpr static uint64_t kFarMemSize = (1ULL << 32); // 4 GB.
constexpr static uint64_t kWorkSetSize = 1 << 30;
constexpr static uint64_t kNumGCThreads = 12;
constexpr static uint64_t kNumConnections = 300;

struct Data4096 {
  char data[4096];
};

using Data_t = struct Data4096;

constexpr static uint64_t kNumEntries = kWorkSetSize / sizeof(Data_t);

void do_work(FarMemManager *manager) {
  std::vector<UniquePtr<Data_t>> vec;

  for (uint64_t i = 0; i < kNumEntries; i++) {
    auto far_mem_ptr = manager->allocate_unique_ptr<Data_t>();//每次循环都会创建一个指针，在内存地址写入数据，然后通过move把数据放到vec中
    {
      DerefScope scope;
      auto raw_mut_ptr = far_mem_ptr.deref_mut(scope);//解引用的指针
      memset(raw_mut_ptr->data, static_cast<char>(i), sizeof(Data_t));
      // cout<<"data address: "<<raw_mut_ptr->data<<", data value: "<<static_cast<char>(i)<<endl;
    }
    vec.emplace_back(std::move(far_mem_ptr));//数据最后都在vector里了
  }

  for (uint64_t i = 0; i < kNumEntries; i++) {
    {
      DerefScope scope;
      const auto raw_const_ptr = vec[i].deref(scope);
      cout<<"The length of data: "<<sizeof(Data_t)<<endl;
      cout<<"static_cast<char>(i) = "<<static_cast<char>(i)<<endl;
      for (uint32_t j = 0; j < sizeof(Data_t); j++) {
        // cout<<"data value of raw_const_ptr: "<<raw_const_ptr->data[j]<<endl;
        if (raw_const_ptr->data[j] != static_cast<char>(i)) {
          goto fail;
        }
      }
    }
  }

  cout << "Passed" << endl;
  return;

fail:
  cout << "Failed" << endl;
}

int argc;
void _main(void *arg) {
  cout << "Running " << __FILE__ "..." << endl;
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port1(argv[1]);
  auto raddr1 = helpers::str_to_netaddr(ip_addr_port1);
  std::string ip_addr_port2(argv[2]);
  auto raddr2 = helpers::str_to_netaddr(ip_addr_port2);

  std::vector<FarMemDevice*> *devices = new std::vector<FarMemDevice*>();
  devices->push_back(new FakeDevice(kFarMemSize));
  devices->push_back(new TCPDevice(raddr1, kNumConnections, kFarMemSize));
  devices->push_back(new TCPDevice(raddr2, kNumConnections, kFarMemSize));
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads, devices));
  do_work(manager.get());

  delete devices->at(0);
  delete devices->at(1);
  delete devices->at(2);
}

int main(int _argc, char *argv[]) {
  int ret;

  if (_argc < 3) {
    std::cerr << "usage: [cfg_file] [ip_addr:port]" << std::endl;
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
