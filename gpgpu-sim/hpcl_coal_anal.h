/*
 * hpcl_comp_anal.h
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_COAL_ANAL_H_
#define GPGPU_SIM_HPCL_COAL_ANAL_H_

#include "hpcl_stat.h"
#include <iostream>
#include <vector>

class hpcl_coal_anal
{
public:
  hpcl_coal_anal ();
  virtual ~hpcl_coal_anal ();

//added by kh(030816)
public:
  enum sample_type {
    SINGLE_REQ_TO_SAME_MEM_RATE = 0,
    MULTI_REQ_TO_SAME_MEM_RATE,
    NO_REQ_TO_SAME_MEM_RATE,
    MULTI_REQ_TO_SAME_MEM_NO,
  };

  void create(unsigned sm_no);
  void add_sample(enum sample_type type, double val, int id=-1);

  void display(std::ostream & os = std::cout) const;
  void update_overall_stat();
  void clear();
  ///

//added by kh(042516)
private:
  //distribution of the number of requests in the coalescing request buffer
  std::vector<hpcl_stat*> m_single_req_to_same_mem_rate;
  std::vector<hpcl_stat*> m_multi_req_to_same_mem_rate;
  std::vector<hpcl_stat*> m_no_req_to_same_mem_rate;
  hpcl_stat* m_ovr_avg_single_req_to_same_mem_rate;
  hpcl_stat* m_ovr_avg_multi_req_to_same_mem_rate;
  hpcl_stat* m_ovr_avg_no_req_to_same_mem_rate;

  hpcl_stat* m_multi_req_to_same_mem_no;
  hpcl_stat* m_ovr_avg_multi_req_to_same_mem_no;
///


};

#endif /* GPGPU_SIM_HPCL_COAL_ANAL_H_ */
