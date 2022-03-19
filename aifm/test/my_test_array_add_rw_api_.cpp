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
    (16ULL << 22); // So the array size is larger than the local cache size.

uint64_t raw_array_A[kNumEntries];
uint64_t raw_array_B[kNumEntries];
uint64_t raw_array_C[kNumEntries];

template <uint64_t N, typename T>
void copy_array(Array<T, N> *array, T *raw_array) {
  for (uint64_t i = 0; i < N; i++) {
    array->write(raw_array[i], i);
  }
}

template <typename T, uint64_t N>
void add_array(Array<T, N> *array_C, Array<T, N> *array_A,
               Array<T, N> *array_B) {
  for (uint64_t i = 0; i < N; i++) {
    array_C->write(array_A->read(i) + array_B->read(i), i);
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
  cout << "Running " << __FILE__ "..." << endl;

  // UniquePtr<uint64_t> ptr = manager->allocate_unique_ptr<uint64_t>();
  // UniquePtr<uint64_t> ptr1 = manager->allocate_unique_ptr<uint64_t>();
  // ptr.set_device_index(0);
  // ptr1.set_device_index(1);
  // ptr.set_device(manager->get_device_by_index(0));
  // ptr1.set_device(manager->get_device_by_index(1));

  // ptr.swap_out();
  // ptr1.swap_out();

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr;
  //   uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   *raw_const_ptr=123;
  // }
  // ptr.swap_out();

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr1;
  //   uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   *raw_const_ptr=666;
  // }
  // ptr1.swap_out();

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr;
  //   const uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   cout << *raw_const_ptr << endl;
  // } 

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr1;
  //   const uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   cout << *raw_const_ptr << endl;
  // } 
  // ptr.swap_out();
  // ptr1.swap_out();

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr;
  //   const uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   cout << *raw_const_ptr << endl;
  // } 

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   auto *far_mem_ptr = &ptr1;
  //   const uint64_t *raw_const_ptr = far_mem_ptr->deref_mut(scope);
  //   ACCESS_ONCE(*raw_const_ptr);
  //   cout << *raw_const_ptr << endl;
  // } 
  // return;

  auto array_A = manager->allocate_array<uint64_t, kNumEntries>();


  GenericUniquePtr* ptr_00 = &array_A.ptrs_[0];
  GenericUniquePtr* ptr_i0 = &array_A.ptrs_[32768];
  cout << "array_A[0].device=" << ptr_00->get_device() << endl;
  cout << "array_A[" << 32768 << "].device=" << ptr_i0->get_device() << endl;
  cout << "array_A[0].data_address=" << ptr_00->meta().get_object_data_addr() << endl;
  cout << "array_A[" << 32768 << "].data_address=" << ptr_i0->meta().get_object_data_addr() << endl;


  cout << "array_A" <<endl;
  auto array_B = manager->allocate_array<uint64_t, kNumEntries>();
  cout << "array_B" <<endl;
  auto array_C = manager->allocate_array<uint64_t, kNumEntries>();
  cout << "array_C" <<endl;



  cout << "array_A[0].device=" << ptr_00->get_device() << endl;
  cout << "array_A[" << 32768 << "].device=" << ptr_i0->get_device() << endl;
  cout << "array_A[0].data_address=" << ptr_00->meta().get_object_data_addr() << endl;
  cout << "array_A[" << 32768 << "].data_address=" << ptr_i0->meta().get_object_data_addr() << endl;

  gen_random_array(kNumEntries, raw_array_A);
  cout << "array_D" <<endl;
  gen_random_array(kNumEntries, raw_array_B);
  cout << "array_E" <<endl;
  copy_array(&array_A, raw_array_A);
  cout << "array_F" <<endl;

  cout << "array_A[0].device=" << ptr_00->get_device() << endl;
  cout << "array_A[" << 32768 << "].device=" << ptr_i0->get_device() << endl;
  cout << "array_A[0].data_address=" << ptr_00->meta().get_object_data_addr() << endl;
  cout << "array_A[" << 32768 << "].data_address=" << ptr_i0->meta().get_object_data_addr() << endl;


  copy_array(&array_B, raw_array_B);
  cout << "array_G" <<endl;
  // add_array(&array_C, &array_A, &array_B);
  // cout << "array_H" <<endl;
  
  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   cout << "array_I" <<endl;
  //   cout <<  "|" << &array_A.at(scope, i) << "|" << &array_B.at(scope, i) << endl;
  //   cout <<  "|" << array_A.at(scope, i) << "|" << array_B.at(scope, i) << endl;
  //   cout <<  "|" << raw_array_A[i] << "|" << raw_array_B[i] << endl;
  // }

  // for (uint64_t i = 0; i < 1; i++) {
  //   DerefScope scope;
  //   cout << "array_I" <<endl;
  //   cout << &array_C.at(scope, i) << "|" << &array_A.at(scope, i) << "|" << &array_B.at(scope, i) << endl;
  //   cout << array_C.at(scope, i) << "|" << array_A.at(scope, i) << "|" << array_B.at(scope, i) << endl;
  //   cout << array_C.at(scope, i) << "|" << raw_array_A[i] << "|" << raw_array_B[i] << endl;
  //   if (array_C.at(scope, i) != raw_array_A[i] + raw_array_B[i]) {
  //     cout <<"no equal" <<endl;
  //     // goto fail;
  //   }else{
  //     cout <<"equal" <<endl;
  //   }
  // }


  ptr_00 = &array_A.ptrs_[0];
  ptr_i0 = &array_A.ptrs_[32768];
  cout << "array_A[0].device=" << ptr_00->get_device() << endl;
  cout << "array_A[" << 32768 << "].device=" << ptr_i0->get_device() << endl;
  cout << "array_A[0].data_address=" << ptr_00->meta().get_object_data_addr() << endl;
  cout << "array_A[" << 32768 << "].data_address=" << ptr_i0->meta().get_object_data_addr() << endl;

  DerefScope scope;
  for (uint64_t i = 0; i < kNumEntries; i++) {
    if(array_A.at(scope, 0) == raw_array_A[i]){
      cout << "array_A[0]=raw_array_A[" <<  i <<"]" <<endl;
      cout << "array_A[" << i << "]=" << array_A.at(scope, i) << endl;
      GenericUniquePtr* ptr_0 = &array_A.ptrs_[0];
      GenericUniquePtr* ptr_i = &array_A.ptrs_[i];
      cout << "array_A[0].device=" << ptr_0->get_device() << endl;
      cout << "array_A[" << i << "].device=" << ptr_i->get_device() << endl;
      cout << "array_A[0].data_address=" << ptr_0->meta().get_object_data_addr() << endl;
      cout << "array_A[" << i << "].data_address=" << ptr_i->meta().get_object_data_addr() << endl;

    }
    if(array_A.at(scope, 0) == raw_array_B[i]){
      cout << "array_A[0]=raw_array_B[" <<  i <<"]" <<endl;
      cout << "array_B[" << i << "]=" << array_B.at(scope, i) << endl;
      GenericUniquePtr* ptr_0 = &array_A.ptrs_[0];
      GenericUniquePtr* ptr_i = &array_B.ptrs_[i];
      cout << "array_A[0].device=" << ptr_0->get_device() << endl;
      cout << "array_B[" << i << "].device=" << ptr_i->get_device() << endl;
      cout << "array_A[0].data_address=" << ptr_0->meta().get_object_data_addr() << endl;
      cout << "array_B[" << i << "].data_address=" << ptr_i->meta().get_object_data_addr() << endl;      
    }
    if(array_B.at(scope, 0) == raw_array_A[i]){
      cout << "array_B[0]=raw_array_A[" <<  i <<"]" <<endl;
      cout << "array_A[" << i << "]=" << array_A.at(scope, i) << endl;
      GenericUniquePtr* ptr_0 = &array_B.ptrs_[0];
      GenericUniquePtr* ptr_i = &array_A.ptrs_[i];
      cout << "array_B[0].device=" << ptr_0->get_device() << endl;
      cout << "array_A[" << i << "].device=" << ptr_i->get_device() << endl;
      cout << "array_B[0].data_address=" << ptr_0->meta().get_object_data_addr() << endl;
      cout << "array_A[" << i << "].data_address=" << ptr_i->meta().get_object_data_addr() << endl;      
    }
    if(array_B.at(scope, 0) == raw_array_B[i]){
      cout << "array_B[0]=raw_array_B[" <<  i <<"]" <<endl;
      cout << "array_B[" << i << "]=" << array_B.at(scope, i) << endl;
      GenericUniquePtr* ptr_0 = &array_B.ptrs_[0];
      GenericUniquePtr* ptr_i = &array_B.ptrs_[i];
      cout << "array_B[0].device=" << ptr_0->get_device() << endl;
      cout << "array_B[" << i << "].device=" << ptr_i->get_device() << endl;
      cout << "array_B[0].data_address=" << ptr_0->meta().get_object_data_addr() << endl;
      cout << "array_B[" << i << "].data_address=" << ptr_i->meta().get_object_data_addr() << endl;   
    }
  }

  cout << "Passed" << endl;
  return;

fail:
  cout << "Failed" << endl;
  return;
}

void _main(void *arg) {
  std::vector<FarMemDevice*> *devices = new std::vector<FarMemDevice*>();
  devices->push_back(new FakeDevice(kFarMemSize));
  devices->push_back(new FakeDevice(kFarMemSize));
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads, devices));
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
