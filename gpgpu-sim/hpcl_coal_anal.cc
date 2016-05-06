/*
 * hpcl_coal_anal.cc
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#include "hpcl_coal_anal.h"
#include <cassert>

hpcl_coal_anal::hpcl_coal_anal ()
{
  // TODO Auto-generated constructor stub

}

hpcl_coal_anal::~hpcl_coal_anal ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_coal_anal::create(unsigned sm_no)
{
  //added by kh(042516)
  m_single_req_to_same_mem_rate.resize(sm_no, NULL);
  m_multi_req_to_same_mem_rate.resize(sm_no, NULL);
  m_no_req_to_same_mem_rate.resize(sm_no, NULL);
  for(unsigned i = 0; i < sm_no; i++) {
    m_single_req_to_same_mem_rate[i] = new hpcl_stat;
    m_multi_req_to_same_mem_rate[i] = new hpcl_stat;
    m_no_req_to_same_mem_rate[i] = new hpcl_stat;
  }
  m_ovr_avg_single_req_to_same_mem_rate = new hpcl_stat;
  m_ovr_avg_multi_req_to_same_mem_rate = new hpcl_stat;
  m_ovr_avg_no_req_to_same_mem_rate = new hpcl_stat;

  m_multi_req_to_same_mem_no = new hpcl_stat;
  m_ovr_avg_multi_req_to_same_mem_no = new hpcl_stat;
  ///

}

void hpcl_coal_anal::add_sample(enum sample_type type, double val, int id)
{
  //added by kh(042516)
  if(type == SINGLE_REQ_TO_SAME_MEM_RATE)  	m_single_req_to_same_mem_rate[id]->add_sample(val);
  else if(type == MULTI_REQ_TO_SAME_MEM_RATE)  	m_multi_req_to_same_mem_rate[id]->add_sample(val);
  else if(type == NO_REQ_TO_SAME_MEM_RATE) 	m_no_req_to_same_mem_rate[id]->add_sample(val);
  else if(type == MULTI_REQ_TO_SAME_MEM_NO)	m_multi_req_to_same_mem_no->add_sample(val);
  else	assert(0);
}

void hpcl_coal_anal::display(std::ostream & os) const
{
  os << "====== hpcl_request_coalescing_anal ======" << std::endl;
  //added by kh(042516)
  os << "gpu_tot_ovr_avg_single_req_to_same_mem_rate = " << m_ovr_avg_single_req_to_same_mem_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_multi_req_to_same_mem_rate = " << m_ovr_avg_multi_req_to_same_mem_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_no_req_to_same_mem_rate = " << m_ovr_avg_no_req_to_same_mem_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_multi_req_to_same_mem_no = " << m_ovr_avg_multi_req_to_same_mem_no->avg() << std::endl;
  ///
  os << "===============================" << std::endl;
}

void hpcl_coal_anal::update_overall_stat()
{
  //added by kh(042516)
  double ovr_avg_single_req_to_same_mem_rate = 0;
  double ovr_avg_multi_req_to_same_mem_rate = 0;
  double ovr_avg_no_req_to_same_mem_rate = 0;
  double cnt = 0;
  for(unsigned i = 0; i < m_single_req_to_same_mem_rate.size(); i++) {
    if(m_single_req_to_same_mem_rate[i]->get_sample_no() > 0) {	//to avoid nan
	ovr_avg_single_req_to_same_mem_rate += m_single_req_to_same_mem_rate[i]->avg();
	ovr_avg_multi_req_to_same_mem_rate += m_multi_req_to_same_mem_rate[i]->avg();
	ovr_avg_no_req_to_same_mem_rate += m_no_req_to_same_mem_rate[i]->avg();
	cnt++;
    }
  }
  ovr_avg_single_req_to_same_mem_rate = ovr_avg_single_req_to_same_mem_rate / cnt;
  ovr_avg_multi_req_to_same_mem_rate = ovr_avg_multi_req_to_same_mem_rate / cnt;
  ovr_avg_no_req_to_same_mem_rate = ovr_avg_no_req_to_same_mem_rate / cnt;
  m_ovr_avg_single_req_to_same_mem_rate->add_sample(ovr_avg_single_req_to_same_mem_rate);
  m_ovr_avg_multi_req_to_same_mem_rate->add_sample(ovr_avg_multi_req_to_same_mem_rate);
  m_ovr_avg_no_req_to_same_mem_rate->add_sample(ovr_avg_no_req_to_same_mem_rate);
  //std::cout << "ovr_avg_single_rep_rate " << ovr_avg_single_rep_rate << std::endl;
  //std::cout << "ovr_avg_multi_rep_rate " << ovr_avg_multi_rep_rate << std::endl;
  //std::cout << "ovr_avg_no_rep_rate " << ovr_avg_no_rep_rate << std::endl;
  m_ovr_avg_multi_req_to_same_mem_no->add_sample(m_multi_req_to_same_mem_no->avg());
  ///
}

void hpcl_coal_anal::clear()
{
  for(unsigned i = 0; i < m_single_req_to_same_mem_rate.size(); i++) {
    m_single_req_to_same_mem_rate[i]->clear();
    m_multi_req_to_same_mem_rate[i]->clear();
    m_no_req_to_same_mem_rate[i]->clear();
  }
  m_multi_req_to_same_mem_no->clear();
}
