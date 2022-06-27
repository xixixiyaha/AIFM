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

uint64_t raw_array_A[kNumEntries];
uint64_t raw_array_B[kNumEntries];
uint64_t raw_array_C[kNumEntries];

template <uint64_t N, typename T>
void copy_array(Array<T, N> *array, T *raw_array) {
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    (*array).at_mut(scope, i) = raw_array[i];
  }
}

template <typename T, uint64_t N>
void add_array(Array<T, N> *array_C, Array<T, N> *array_A,
               Array<T, N> *array_B) {
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    (*array_C).at_mut(scope, i) =
        (*array_A).at(scope, i) + (*array_B).at(scope, i);
  }
}

void gen_random_array(uint64_t num_entries, uint64_t *raw_array) {
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> distr;

  for (uint64_t i = 0; i < num_entries; i++) {
    raw_array[i] = distr(eng);
  }
}

void do_work(FarMemManager *manager) {
  
  auto array_A = manager->allocate_array<uint64_t, kNumEntries>();
  auto array_B = manager->allocate_array<uint64_t, kNumEntries>();
  auto array_C = manager->allocate_array<uint64_t, kNumEntries>();

  GenericUniquePtr* ptr_a = &array_A.ptrs_[0];
  GenericUniquePtr* ptr_b = &array_B.ptrs_[0];
  GenericUniquePtr* ptr_c = &array_C.ptrs_[0];
  
  cout<<"array_A `_device:"<<ptr_a->get_device()<<endl;
  cout<<"array_A `_device_index:"<<ptr_a->get_device_index()<<endl;
  cout<<"array_A `_object_address:"<<ptr_a->meta().get_object_data_addr()<<endl;
  cout<<"array_B `_device:"<<ptr_b->get_device()<<endl;
  cout<<"array_B `_device_index:"<<ptr_c->get_device_index()<<endl;
  cout<<"array_B `_object_address:"<<ptr_b->meta().get_object_data_addr()<<endl;
  cout<<"array_C `_device:"<<ptr_c->get_device()<<endl;
  cout<<"array_C `_device_index:"<<ptr_c->get_device_index()<<endl;
  cout<<"array_C `_object_address:"<<ptr_c->meta().get_object_data_addr()<<endl;

  gen_random_array(kNumEntries, raw_array_A);
  gen_random_array(kNumEntries, raw_array_B);
  for (uint64_t i = 0; i < 10; i++) {
    GenericUniquePtr* ptr_a = &array_A.ptrs_[i];
    cout<<"array_A `_device" <<i<<" : "<<ptr_a->get_device()<<endl;
    cout<<"array_A `_device_index" <<i<<" : "<<ptr_a->get_device_index()<<endl;
    cout<<"array_A `_object_address" <<i<<" : "<<ptr_a->meta().get_object_data_addr()<<endl;
    cout<<"array_A["<<i<<"]:"<<raw_array_A[i]<<endl;
  }


  copy_array(&array_A, raw_array_A);
  copy_array(&array_B, raw_array_B);
  add_array(&array_C, &array_A, &array_B);

  for (uint64_t i = 0; i < kNumEntries; i++) {
    DerefScope scope;
    if (array_C.at(scope, i) != raw_array_A[i] + raw_array_B[i]) {
      goto fail;
    }
  }

  cout << "Passed" << endl;
  return;

fail:
  cout << "Failed" << endl;
}

int argc;
void _main(void *arg) {
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
