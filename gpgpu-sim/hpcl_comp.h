/*
 * hpcl_comp.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_H_
#define HPCL_COMP_H_

#include "gpu-sim.h"
#include "../cuda-sim/memory.h"

#include "hpcl_stat.h"

class hpcl_comp
{
public:
  hpcl_comp ();
  virtual
  ~hpcl_comp ();

private:
  const struct memory_config *m_memory_config;
  class memory_space *m_global_mem;

public:
  void create(const struct memory_config * memory_config, class memory_space * global_mem);
  void get_cache_data(void* mf, void* data, int index=-1);
  void print_cache_data(mem_fetch* mf);

  //added by kh(031716)
  unsigned get_mem_no() {
    return m_memory_config->m_n_mem;
  }
  unsigned get_sub_partition_no_per_memory_channel() {
    return m_memory_config->m_n_sub_partition_per_memory_channel;
  }
  ///

  //added by kh(050216)
  new_addr_type get_cache_block_addr(new_addr_type access_addr) {
    return m_memory_config->m_L2_config.block_addr(access_addr);
  }
  ///
};

#endif /* HPCL_COMP_H_ */
