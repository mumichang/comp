/*
 * hpcl_comp_anal.cc
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#include "hpcl_approx_anal.h"
#include <cassert>

hpcl_approx_anal::hpcl_approx_anal ()
{
  // TODO Auto-generated constructor stub
}

hpcl_approx_anal::~hpcl_approx_anal ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_approx_anal::create()
{
  //This is set for memory space that memcpy does not capture.
  class mem_space_stat tmp;
  m_mem_space_stat.push_back(tmp);

}

void hpcl_approx_anal::add_sample(enum sample_type type, double val1, double val2, int id)
{
  if(type == CUDA_ALLOC_SPACE_START_ADDR)	m_cuda_alloc_space_start_addr.push_back((unsigned)val1);
  else if(type == CUDA_ALLOC_SPACE_SIZE)	m_cuda_alloc_space_size.push_back((unsigned)val1);
  else if(type == CUDA_COPY_SPACE_START_ADDR)	m_cuda_copy_space_start_addr.push_back((unsigned)val1);
  else if(type == L1_CACHE_MISS) {

    //val1: memory address accessed by ld inst
    //val2: ld inst pc
    bool is_found = false;
    for(unsigned i = 1; i < m_mem_space_stat.size(); i++) {
      if(m_mem_space_stat[i].is_in_mem_space((unsigned)val1) == true) {
	m_mem_space_stat[i].add_L1_miss_cnt((unsigned)val2);
	m_mem_space_stat[i].add_LD_access_type ((unsigned)val2, id);
	is_found = true; break;
      }
    }

    if(is_found == false) {
      m_mem_space_stat[0].add_L1_miss_cnt((unsigned)val2);
      m_mem_space_stat[0].add_LD_access_type ((unsigned)val2, id);
    }

  }
  else assert(0);
}

void hpcl_approx_anal::add_mem_space(unsigned start_addr, unsigned end_addr)
{
  class mem_space_stat tmp(start_addr, end_addr);
  m_mem_space_stat.push_back(tmp);
}


/*
void hpcl_approx_anal::add_sample(enum sample_type type, unsigned long long val, int id)
{
  if(type == CUDA_ALLOC_SPACE_START_ADDR)	m_cuda_alloc_space_start_addr.push_back(val);
  else if(type == CUDA_ALLOC_SPACE_SIZE)	m_cuda_alloc_space_size.push_back(val);
  else if(type == CUDA_COPY_SPACE_START_ADDR)	m_cuda_copy_space_start_addr.push_back(val);
  else assert(0);
}
*/

void hpcl_approx_anal::display(std::ostream & os) const
{
  os << "====== hpcl_approx_anal ======" << std::endl;

  os << "gpu_tot_cuda_alloc_space_start_addr = [ ";
  for(unsigned i = 0; i < m_cuda_alloc_space_start_addr.size(); i++) {
    printf("0X%08llx ", m_cuda_alloc_space_start_addr[i]);
  }
  os << " ] " << std::endl;

  os << "gpu_tot_cuda_alloc_space_size = [ ";
  for(unsigned i = 0; i < m_cuda_alloc_space_size.size(); i++) {
    printf("%llu ", m_cuda_alloc_space_size[i]);
  }
  os << " ] " << std::endl;

  os << "gpu_tot_cuda_copy_space_start_addr = [ ";
  for(unsigned i = 0; i < m_cuda_copy_space_start_addr.size(); i++) {
    printf("0X%08llx ", m_cuda_copy_space_start_addr[i]);
  }
  os << " ] " << std::endl;

  os << "--------- gpu_tot_cuda_mem_space_analysis start ---------" << std::endl;
  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    ((class mem_space_stat)m_mem_space_stat[i]).print();
  }
  os << "--------- gpu_tot_cuda_mem_space_analysis end ---------" << std::endl;

  os << "===============================" << std::endl;
}

void hpcl_approx_anal::update_overall_stat()
{
  //get L1 miss no
  unsigned long long all_l1_miss_no = 0;
  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    all_l1_miss_no += m_mem_space_stat[i].get_L1_miss_no();
  }

  for(unsigned i = 0; i < m_mem_space_stat.size(); i++) {
    m_mem_space_stat[i].set_all_L1_miss_no(all_l1_miss_no);
  }
  ///

}

void hpcl_approx_anal::clear()
{

}
