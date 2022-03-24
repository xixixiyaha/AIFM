#include "stats.hpp"
#include "helpers.hpp"
#include "manager.hpp"

#include <algorithm>
#include <iostream>
int wklwklwklw=0;

namespace far_memory {
bool Stats::enable_swap_;
#ifdef MONITOR_FREE_MEM_RATIO
std::vector<std::pair<uint64_t, Stats::ratio_point>>
    Stats::free_mem_ratio_records_[helpers::kNumCPUs];
// std::vector<std::pair<uint64_t, Stats::ratio_point>>
//     Stats::free_far_mem_ratio_records1_[helpers::kNumCPUs];
// std::vector<std::pair<uint64_t, Stats::ratio_point>>
//     Stats::free_far_mem_ratio_records2_[helpers::kNumCPUs];

#endif

#ifdef MONITOR_READ_OBJECT_CYCLES
unsigned Stats::read_object_cycles_high_start_;
unsigned Stats::read_object_cycles_low_start_;
unsigned Stats::read_object_cycles_high_end_;
unsigned Stats::read_object_cycles_low_end_;
#endif

#ifdef MONITOR_WRITE_OBJECT_CYCLES
unsigned Stats::write_object_cycles_high_start_;
unsigned Stats::write_object_cycles_low_start_;
unsigned Stats::write_object_cycles_high_end_;
unsigned Stats::write_object_cycles_low_end_;
#endif

void Stats::_add_free_mem_ratio_record(int operation,int device_id) {
#ifdef MONITOR_FREE_MEM_RATIO
  preempt_disable();

  uint64_t t_us =
      helpers::chrono_to_timestamp(std::chrono::steady_clock::now());
  double free_mem_ratio = FarMemManagerFactory::get()->get_free_mem_ratio();
  double free_far_mem_ratio1_ = FarMemManagerFactory::get()->get_free_far_mem_ratio(0);
  double free_far_mem_ratio2_ = FarMemManagerFactory::get()->get_free_far_mem_ratio(1);

  Stats::ratio_point rp = {free_mem_ratio,free_far_mem_ratio2_,free_far_mem_ratio1_,operation,device_id};
  free_mem_ratio_records_[get_core_num()].push_back(
      std::make_pair(t_us, rp));
  wklwklwklw++;
  
  // Stats::ratio_point rp1 = {free_far_mem_ratio1_,operation,device_id};
  // free_far_mem_ratio_records1_[get_core_num()].push_back(
  //     std::make_pair(t_us, rp1));

  
  // Stats::ratio_point rp2 = {free_far_mem_ratio2_,operation,device_id};
  // free_far_mem_ratio_records2_[get_core_num()].push_back(
  //     std::make_pair(t_us, rp2));
  // std::cout << t_us << " " << get_core_num() << " "<<operation << " " << device_id << " " <<  free_mem_ratio <<" "<<free_far_mem_ratio1_<<" "<<free_far_mem_ratio2_<<std::endl;

  preempt_enable();
#endif
}

void Stats::print_free_mem_ratio_records() {
#ifdef MONITOR_FREE_MEM_RATIO
  std::cout << "start" << std::endl;
  std::vector<std::pair<uint64_t, Stats::ratio_point>> all_records;
  // std::vector<std::pair<uint64_t, Stats::ratio_point>> all_records_1;
  // std::vector<std::pair<uint64_t, Stats::ratio_point>> all_records_2;
  std::cout << "count"<<wklwklwklw << std::endl;
  FOR_ALL_SOCKET0_CORES(core_id) {
    std::cout << free_mem_ratio_records_[core_id].size() << std::endl;
  }


  FOR_ALL_SOCKET0_CORES(core_id) {
    all_records.insert(all_records.end(),
                       free_mem_ratio_records_[core_id].begin(),
                       free_mem_ratio_records_[core_id].end());
  }
  // FOR_ALL_SOCKET0_CORES(core_id) {
  //   all_records_1.insert(all_records_1.end(),
  //                      free_far_mem_ratio_records1_[core_id].begin(),
  //                      free_far_mem_ratio_records1_[core_id].end());
  // }
  // FOR_ALL_SOCKET0_CORES(core_id) {
  //   all_records_2.insert(all_records_2.end(),
  //                      free_far_mem_ratio_records2_[core_id].begin(),
  //                      free_far_mem_ratio_records2_[core_id].end());
  // }
  // sort(all_records.begin(), all_records.end());
  // sort(all_records_1.begin(), all_records_1.end());
  // sort(all_records_2.begin(), all_records_2.end());
  std::cout << "cache_free_mem_ratio" << std::endl;
  for (auto [t_us, rp] : all_records) {
    std::cout << t_us << " " <<rp.operation << " " << rp.device_id << " " << rp.cache_ratio << " " << rp.far1_ratio << " " <<rp.far2_ratio  << " " << std::endl;
  }
  // std::cout << "cache_free_far_mem_ratio1" << std::endl;
  // for (auto [t_us, rp] : all_records_1) {
  //   std::cout << t_us << " " << rp.ratio << " " << rp.operation << " " << rp.device_id << " " << std::endl;
  // }
  // std::cout << "cache_free_far_mem_ratio2" << std::endl;
  // for (auto [t_us, rp] : all_records_2) {
  //   std::cout << t_us << " " << rp.ratio << " " << rp.operation << " " << rp.device_id << " " << std::endl;
  // }
#endif
}

void Stats::clear_free_mem_ratio_records() {
#ifdef MONITOR_FREE_MEM_RATIO
  // FOR_ALL_SOCKET0_CORES(core_id) { free_mem_ratio_records_[core_id].clear(); }
  // FOR_ALL_SOCKET0_CORES(core_id) { free_far_mem_ratio_records1_[core_id].clear(); }
  // FOR_ALL_SOCKET0_CORES(core_id) { free_far_mem_ratio_records2_[core_id].clear(); }
#endif
}

} // namespace far_memory
