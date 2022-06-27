extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "manager.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <random>

using namespace far_memory;
using namespace std;

constexpr uint64_t kCacheSize = (128ULL << 20);
constexpr uint64_t kFarMemSize = (4ULL << 30);
constexpr uint32_t kNumGCThreads = 12;
constexpr uint32_t kNumEntries =
    (16ULL << 20); // So the array size is larger than the local cache size.

uint64_t raw_array_A[kNumEntries];
uint64_t raw_array_B[kNumEntries];
uint64_t raw_array_C[kNumEntries];

template <uint64_t N, typename T>
void copy_array(Array<T, N> *array, T *raw_array) {
  cout<<".............copy_array_length: "<<N;
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    (*array).at_mut(scope, i) = raw_array[i];
  }
}

template <typename T, uint64_t N>
void add_array(Array<T, N> *array_C, Array<T, N> *array_A,
               Array<T, N> *array_B) {
  cout<<".............add_array";
  for (uint64_t i = 0; i < N; i++) {
    DerefScope scope;
    (*array_C).at_mut(scope, i) =
        (*array_A).at(scope, i) + (*array_B).at(scope, i);
  }
}

void gen_random_array(uint64_t num_entries, uint64_t *raw_array) {
  cout<<".............gen_random_array";
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<uint64_t> distr;

  for (uint64_t i = 0; i < num_entries; i++) {
    raw_array[i] = distr(eng);
  }
}

void do_work(FarMemManager *manager) {
  cout << "Running " << __FILE__ "..." << endl;

  auto array_A = manager->allocate_array<uint64_t, kNumEntries>();
  auto array_B = manager->allocate_array<uint64_t, kNumEntries>();
  auto array_C = manager->allocate_array<uint64_t, kNumEntries>();

  gen_random_array(kNumEntries, raw_array_A);
  gen_random_array(kNumEntries, raw_array_B);
  cout<<"The size of raw_array_A: "<<std::size(raw_array_A)<<endl;
  copy_array(&array_A, raw_array_A);
  copy_array(&array_B, raw_array_B);
  add_array(&array_C, &array_A, &array_B);

  for (uint64_t i = 0; i < kNumEntries; i++) {
    DerefScope scope;
    if (array_C.at(scope, i) != raw_array_A[i] + raw_array_B[i]) {
      goto fail;
    }
  }
  
  // cout << "Passed1" << endl;
  cout << "Passed" << endl;
  // for (uint64_t i = 0; i < kNumEntries; i++) {
  //   // raw_array[i] = distr(eng);
  //   cout<<"...context_array..."<<raw_array_A[i]<<endl;
  //   cout<<"...run_to_here..."<<endl;
  // }
  // cout << "Passed2" << endl;
  return;

fail:
  cout << "Failed" << endl;
  return;
}

void _main(void *arg) {
  //由于本地内存不够，所以创建了一个远程device来进行内存卸载
  std::vector<FarMemDevice*> *devices = new std::vector<FarMemDevice*>();
  devices->push_back(new FakeDevice(kFarMemSize));
  // devices->push_back(new FakeDevice(kFarMemSize));
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads, devices));
          //FarMemManagerFactory创建FarMemManager，创建FarMemManagerFactory类型的唯一指针
          // std::unique_ptr<int> up1(new int(11));unique指针用法
  do_work(manager.get());
}

int main(int argc, char *argv[]) {
  int ret;

  if (argc < 2) {
    std::cerr << "usage: [cfg_file]" << std::endl;
    return -EINVAL;
  }

  ret = runtime_init(argv[1], _main, NULL);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}
