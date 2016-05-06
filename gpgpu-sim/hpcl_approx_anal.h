/*
 * hpcl_comp_anal.h
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_APPROX_ANAL_H_
#define GPGPU_SIM_HPCL_APPROX_ANAL_H_

#include "hpcl_stat.h"
#include <iostream>
#include <vector>
#include <map>

class hpcl_approx_anal
{
public:
  hpcl_approx_anal ();
  virtual
  ~hpcl_approx_anal ();

public:
  enum sample_type {
    CUDA_ALLOC_SPACE_START_ADDR = 0,
    CUDA_ALLOC_SPACE_SIZE,
    CUDA_COPY_SPACE_START_ADDR,
    L1_CACHE_MISS,
  };

  void create();
  //void add_sample(enum sample_type type, unsigned long long val, int id=-1);
  void add_sample(enum sample_type type, double val1, double val2=-1, int id=-1);
  void display(std::ostream & os = std::cout) const;
  void update_overall_stat();
  void clear();
  ///

//added by kh(050216)
private:
  std::vector<unsigned> m_cuda_alloc_space_start_addr;
  std::vector<unsigned> m_cuda_alloc_space_size;
  std::vector<unsigned> m_cuda_copy_space_start_addr;

  class mem_space_stat {
  private:
    unsigned m_start_addr;
    unsigned m_end_addr;
    std::map<unsigned, unsigned long long> m_L1_misses_per_LD;
    std::map<unsigned, int> m_access_type_per_LD;
    unsigned long long m_all_L1_miss_no;
  public:
    mem_space_stat() {
      m_start_addr = 0;	m_end_addr = 0; m_all_L1_miss_no = 0;
    }
    mem_space_stat(unsigned start_addr, unsigned end_addr) {
      m_start_addr = start_addr; m_end_addr = end_addr;  m_all_L1_miss_no = 0;
    }
    ~mem_space_stat() {}

    bool is_in_mem_space (unsigned addr)
    {
      if(addr >= m_start_addr && addr <= m_end_addr)	return true;
      else						return false;
    }

    void add_L1_miss_cnt (unsigned ld_pc)
    {
      std::map<unsigned, unsigned long long>::iterator it;
      it = m_L1_misses_per_LD.find(ld_pc);
      if(it != m_L1_misses_per_LD.end()) {
	it->second++;
      } else {
	m_L1_misses_per_LD.insert(std::pair<unsigned, unsigned long long>(ld_pc, 1));
      }
    }

    void add_LD_access_type (unsigned ld_pc, int access_type)
    {
      std::map<unsigned, int>::iterator it;
      it = m_access_type_per_LD.find(ld_pc);
      if(it == m_access_type_per_LD.end()) {
	m_access_type_per_LD.insert(std::pair<unsigned, int>(ld_pc, access_type));
      }
    }


    void print()
    {
      printf("     MEM_SPACE = (0x%08x, 0x%08x)\n", m_start_addr, m_end_addr);
      std::map<unsigned, unsigned long long>::iterator it;
      for(it = m_L1_misses_per_LD.begin(); it != m_L1_misses_per_LD.end(); ++it) {
	std::map<unsigned, int>::iterator it2 = m_access_type_per_LD.find(it->first);
	printf("       LD_PC = 0x%08x, ACC_TYPE = %d, Misses = %llu, Rate = %3.2f \%\n", it->first, it2->second, it->second, (double)it->second/m_all_L1_miss_no*100);
      }
    }

    unsigned long long get_L1_miss_no() {
      unsigned long long ret = 0;
      std::map<unsigned, unsigned long long>::iterator it;
      for(it = m_L1_misses_per_LD.begin(); it != m_L1_misses_per_LD.end(); ++it) {
	ret += it->second;
      }
      return ret;
    }

    void set_all_L1_miss_no(unsigned long long all_L1_miss_no) {
      m_all_L1_miss_no = all_L1_miss_no;
    }

  };

  std::vector<class mem_space_stat> m_mem_space_stat;


public:
  void add_mem_space(unsigned start_addr, unsigned end_addr);	//based on memcpy_to_gpu



};

#endif /* GPGPU_SIM_HPCL_COMP_ANAL_H_ */
