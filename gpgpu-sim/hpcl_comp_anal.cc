/*
 * hpcl_comp_anal.cc
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#include "hpcl_comp_anal.h"
#include <cassert>

hpcl_comp_anal::hpcl_comp_anal ()
{
  // TODO Auto-generated constructor stub

}

hpcl_comp_anal::~hpcl_comp_anal ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_comp_anal::create(unsigned sub_partition_no)
{
  m_comp_table_hit_no = new hpcl_stat;
  m_comp_table_miss_no = new hpcl_stat;
  m_read_reply_comp_ratio = new hpcl_stat;	//added by kh(041616)
  m_ovr_avg_read_reply_comp_ratio = new hpcl_stat;	//added by kh(041616)
  m_ovr_avg_comp_table_hit_rate = new hpcl_stat;

  //added by kh(041616)
  m_fragment_rate_per_flit = new hpcl_stat;
  m_ovr_avg_fragment_rate_per_flit = new hpcl_stat;
  m_fragment_rate_per_packet = new hpcl_stat;
  m_ovr_avg_fragment_rate_per_packet = new hpcl_stat;
  m_comp_overhead_no = new hpcl_stat;
  m_ovr_avg_comp_overhead_rate = new hpcl_stat;
  ///

  //added by kh(041816)
  m_org_flit_no = new hpcl_stat;
  m_ovr_avg_org_flit_no = new hpcl_stat;
  m_comp_flit_no = new hpcl_stat;
  m_ovr_avg_comp_flit_no = new hpcl_stat;
  ///

  //added by kh(041816)
  m_comp_2B_no = new hpcl_stat;
  m_comp_4B_no = new hpcl_stat;
  m_comp_8B_no = new hpcl_stat;
  m_ovr_avg_comp_2B_rate = new hpcl_stat;
  m_ovr_avg_comp_4B_rate = new hpcl_stat;
  m_ovr_avg_comp_8B_rate = new hpcl_stat;
  ///

  //added by kh(042116)
  m_word2B_zeros_rep_rate = new hpcl_stat;
  m_word2B_nonzeros_rep_rate = new hpcl_stat;
  m_word2B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word2B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word2B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word2B_uni_val_rate = new hpcl_stat;

  m_word4B_zeros_rep_rate = new hpcl_stat;
  m_word4B_nonzeros_rep_rate = new hpcl_stat;
  m_word4B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word4B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word4B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word4B_uni_val_rate = new hpcl_stat;

  m_word8B_zeros_rep_rate = new hpcl_stat;
  m_word8B_nonzeros_rep_rate = new hpcl_stat;
  m_word8B_uni_val_rate = new hpcl_stat;
  m_ovr_avg_word8B_zeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word8B_nonzeros_rep_rate = new hpcl_stat;
  m_ovr_avg_word8B_uni_val_rate = new hpcl_stat;
  ///

  //added by kh(042116)
  m_single_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  m_multi_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  m_no_rep_to_same_sm_rate.resize(sub_partition_no, NULL);
  for(unsigned i = 0; i < sub_partition_no; i++) {
    m_single_rep_to_same_sm_rate[i] = new hpcl_stat;
    m_multi_rep_to_same_sm_rate[i] = new hpcl_stat;
    m_no_rep_to_same_sm_rate[i] = new hpcl_stat;
  }
  m_ovr_avg_single_rep_to_same_sm_rate = new hpcl_stat;
  m_ovr_avg_multi_rep_to_same_sm_rate = new hpcl_stat;
  m_ovr_avg_no_rep_to_same_sm_rate = new hpcl_stat;

  m_multi_rep_to_same_sm_no = new hpcl_stat;
  m_ovr_avg_multi_rep_to_same_sm_no = new hpcl_stat;
  ///

  //added by kh(042216)
  m_word1B_zeros_rep_rate_in_uncomp_data = new hpcl_stat;
  m_word1B_nonzeros_rep_rate_in_uncomp_data = new hpcl_stat;
  m_word1B_uni_val_rate_in_uncomp_data = new hpcl_stat;
  m_word1B_nonzeros_rep_no_in_uncomp_data = new hpcl_stat;
  m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data = new hpcl_stat;
  m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data = new hpcl_stat;;
  m_ovr_avg_word1B_uni_val_rate_in_uncomp_data = new hpcl_stat;
  m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data = new hpcl_stat;
  ///

  //added by kh(042316)
  m_word1B_word1B_redund_rate_inter_read_replies = new hpcl_stat;
  m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies = new hpcl_stat;
  ///
}

void hpcl_comp_anal::add_sample(enum sample_type type, double val, int id)
{
  if(type == GLOBAL_TABLE_HIT_NO)  	 m_comp_table_hit_no->add_sample(val);
  else if(type == GLOBAL_TABLE_MISS_NO)  m_comp_table_miss_no->add_sample(val);
  else if(type == READ_REPLY_COMP_RATIO) {
      m_read_reply_comp_ratio->add_sample(val);

      //std::cout << "my_comp_ratio " << val << std::endl;

  }
  //added by kh(041616)
  else if(type == FRAGMENT_RATE_PER_FLIT)	m_fragment_rate_per_flit->add_sample(val);
  else if(type == FRAGMENT_RATE_PER_PACKET)	m_fragment_rate_per_packet->add_sample(val);
  else if(type == COMP_OVERHEAD_NO)	 	m_comp_overhead_no->add_sample(val);
  //added by kh(041816)
  else if(type == ORG_FLIT_NO)		 	m_org_flit_no->add_sample(val);
  else if(type == COMP_FLIT_NO)			m_comp_flit_no->add_sample(val);
  else if(type == COMP_2B_NO)		 	m_comp_2B_no->add_sample(val);
  else if(type == COMP_4B_NO)			m_comp_4B_no->add_sample(val);
  else if(type == COMP_8B_NO)		 	m_comp_8B_no->add_sample(val);
  //added by kh(042116)
  else if(type == WORD2B_ZEROS_REP_RATE)	m_word2B_zeros_rep_rate->add_sample(val);
  else if(type == WORD2B_NONZEROS_REP_RATE)	m_word2B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD2B_UNI_VAL_RATE)		m_word2B_uni_val_rate->add_sample(val);
  else if(type == WORD4B_ZEROS_REP_RATE)	m_word4B_zeros_rep_rate->add_sample(val);
  else if(type == WORD4B_NONZEROS_REP_RATE)	m_word4B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD4B_UNI_VAL_RATE)		m_word4B_uni_val_rate->add_sample(val);
  else if(type == WORD8B_ZEROS_REP_RATE)	m_word8B_zeros_rep_rate->add_sample(val);
  else if(type == WORD8B_NONZEROS_REP_RATE)	m_word8B_nonzeros_rep_rate->add_sample(val);
  else if(type == WORD8B_UNI_VAL_RATE)		m_word8B_uni_val_rate->add_sample(val);
  else if(type == SINGLE_REP_TO_SAME_SM_RATE)	m_single_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == MULTI_REP_TO_SAME_SM_RATE)	m_multi_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == NO_REP_TO_SAME_SM_RATE)	m_no_rep_to_same_sm_rate[id]->add_sample(val);
  else if(type == MULTI_REP_TO_SAME_SM_NO)	m_multi_rep_to_same_sm_no->add_sample(val);
  //added by kh(042216)
  else if(type == WORD1B_ZEROS_REP_RATE_IN_UNCOMP_DATA)		m_word1B_zeros_rep_rate_in_uncomp_data->add_sample(val);
  else if(type == WORD1B_NONZEROS_REP_RATE_IN_UNCOMP_DATA)	m_word1B_nonzeros_rep_rate_in_uncomp_data->add_sample(val);
  else if(type == WORD1B_UNI_VAL_RATE_IN_UNCOMP_DATA)		m_word1B_uni_val_rate_in_uncomp_data->add_sample(val);
  else if(type == WORD1B_NONZEROS_REP_NO_IN_UNCOMP_DATA)	m_word1B_nonzeros_rep_no_in_uncomp_data->add_sample(val);
  //added by kh(042316)
  else if(type == WORD1B_REDUND_RATE_INTER_READ_REPLIES)	m_word1B_word1B_redund_rate_inter_read_replies->add_sample(val);
  else	assert(0);
}

void hpcl_comp_anal::display(std::ostream & os) const
{
  os << "====== hpcl_comp_anal ======" << std::endl;
  os << "gpu_tot_ovr_avg_comp_table_hit_rate = " << m_ovr_avg_comp_table_hit_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_read_reply_comp_ratio = " << m_ovr_avg_read_reply_comp_ratio->avg() << std::endl;
  os << "gpu_tot_ovr_avg_fragment_rate_per_flit = " << m_ovr_avg_fragment_rate_per_flit->avg() << std::endl;
  os << "gpu_tot_ovr_avg_fragment_rate_per_packet = " << m_ovr_avg_fragment_rate_per_packet->avg() << std::endl;
  os << "gpu_tot_ovr_avg_comp_overhead_rate = " << m_ovr_avg_comp_overhead_rate->avg() << std::endl;

  //added by kh(041816)
  os << "gpu_tot_ovr_avg_org_flit_no = " << m_ovr_avg_org_flit_no->avg() << std::endl;		//added by kh(041816)
  os << "gpu_tot_ovr_avg_comp_flit_no = " << m_ovr_avg_comp_flit_no->avg() << std::endl;	//added by kh(041816)
  os << "gpu_tot_ovr_avg_comp_2B_rate = " << m_ovr_avg_comp_2B_rate->avg() << std::endl;	//added by kh(041816)
  os << "gpu_tot_ovr_avg_comp_4B_rate = " << m_ovr_avg_comp_4B_rate->avg() << std::endl;	//added by kh(041816)
  os << "gpu_tot_ovr_avg_comp_8B_rate = " << m_ovr_avg_comp_8B_rate->avg() << std::endl;	//added by kh(041816)
  ///

  //added by kh(042116)
  os << "gpu_tot_ovr_avg_word2B_zeros_rep_rate = " << m_ovr_avg_word2B_zeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word2B_nonzeros_rep_rate = " << m_ovr_avg_word2B_nonzeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word2B_uni_val_rate = " << m_ovr_avg_word2B_uni_val_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word4B_zeros_rep_rate = " << m_ovr_avg_word4B_zeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word4B_nonzeros_rep_rate = " << m_ovr_avg_word4B_nonzeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word4B_uni_val_rate = " << m_ovr_avg_word4B_uni_val_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word8B_zeros_rep_rate = " << m_ovr_avg_word8B_zeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word8B_nonzeros_rep_rate = " << m_ovr_avg_word8B_nonzeros_rep_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word8B_uni_val_rate = " << m_ovr_avg_word8B_uni_val_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_single_rep_to_same_sm_rate = " << m_ovr_avg_single_rep_to_same_sm_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_multi_rep_to_same_sm_rate = " << m_ovr_avg_multi_rep_to_same_sm_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_no_rep_to_same_sm_rate = " << m_ovr_avg_no_rep_to_same_sm_rate->avg() << std::endl;
  os << "gpu_tot_ovr_avg_multi_rep_to_same_sm_no = " << m_ovr_avg_multi_rep_to_same_sm_no->avg() << std::endl;
  ///

  //added by kh(042216)
  os << "gpu_tot_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data = " << m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data = " << m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word1B_uni_val_rate_in_uncomp_data = " << m_ovr_avg_word1B_uni_val_rate_in_uncomp_data->avg() << std::endl;
  os << "gpu_tot_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data = " << m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data->avg() << std::endl;
  ///

  //added by kh(042316)
  os << "gpu_tot_ovr_avg_word1B_redund_rate_inter_read_replies = " << m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies->avg() << std::endl;
  ///

  os << "===============================" << std::endl;
}

void hpcl_comp_anal::update_overall_stat()
{
  m_ovr_avg_comp_table_hit_rate->add_sample(m_comp_table_hit_no->sum()/(m_comp_table_hit_no->sum()+m_comp_table_miss_no->sum())*100);

  //added by kh(050516)
  if(m_read_reply_comp_ratio->get_sample_no() == 0)
    m_ovr_avg_read_reply_comp_ratio->add_sample(0);
  else
    m_ovr_avg_read_reply_comp_ratio->add_sample(m_read_reply_comp_ratio->avg());
  ///
  //std::cout << "##### m_read_reply_comp_ratio->avg() " << m_read_reply_comp_ratio->avg() << std::endl;

  //added by kh(041616)
  m_ovr_avg_fragment_rate_per_flit->add_sample(m_fragment_rate_per_flit->avg());
  m_ovr_avg_fragment_rate_per_packet->add_sample(m_fragment_rate_per_packet->avg());
  m_ovr_avg_comp_overhead_rate->add_sample(m_comp_overhead_no->sum()/m_read_reply_comp_ratio->get_sample_no()*100);
  ///

  //added by kh(041816)
  m_ovr_avg_org_flit_no->add_sample(m_org_flit_no->avg());
  m_ovr_avg_comp_flit_no->add_sample(m_comp_flit_no->avg());

  if((m_comp_2B_no->sum()+m_comp_4B_no->sum()+m_comp_8B_no->sum()) == 0) {
//    m_ovr_avg_comp_2B_rate->add_sample(0);
//    m_ovr_avg_comp_4B_rate->add_sample(0);
//    m_ovr_avg_comp_8B_rate->add_sample(0);
  } else {
    m_ovr_avg_comp_2B_rate->add_sample(m_comp_2B_no->sum()/(m_comp_2B_no->sum()+m_comp_4B_no->sum()+m_comp_8B_no->sum())*100);
    m_ovr_avg_comp_4B_rate->add_sample(m_comp_4B_no->sum()/(m_comp_2B_no->sum()+m_comp_4B_no->sum()+m_comp_8B_no->sum())*100);
    m_ovr_avg_comp_8B_rate->add_sample(m_comp_8B_no->sum()/(m_comp_2B_no->sum()+m_comp_4B_no->sum()+m_comp_8B_no->sum())*100);
  }
  ///

  //added by kh(042116)
  m_ovr_avg_word2B_zeros_rep_rate->add_sample(m_word2B_zeros_rep_rate->avg());
  m_ovr_avg_word2B_nonzeros_rep_rate->add_sample(m_word2B_nonzeros_rep_rate->avg());
  m_ovr_avg_word2B_uni_val_rate->add_sample(m_word2B_uni_val_rate->avg());
  m_ovr_avg_word4B_zeros_rep_rate->add_sample(m_word4B_zeros_rep_rate->avg());
  m_ovr_avg_word4B_nonzeros_rep_rate->add_sample(m_word4B_nonzeros_rep_rate->avg());
  m_ovr_avg_word4B_uni_val_rate->add_sample(m_word4B_uni_val_rate->avg());
  m_ovr_avg_word8B_zeros_rep_rate->add_sample(m_word8B_zeros_rep_rate->avg());
  m_ovr_avg_word8B_nonzeros_rep_rate->add_sample(m_word8B_nonzeros_rep_rate->avg());
  m_ovr_avg_word8B_uni_val_rate->add_sample(m_word8B_uni_val_rate->avg());

  double ovr_avg_single_rep_rate = 0;
  double ovr_avg_multi_rep_rate = 0;
  double ovr_avg_no_rep_rate = 0;
  double cnt = 0;
  for(unsigned i = 0; i < m_single_rep_to_same_sm_rate.size(); i++) {
    if(m_single_rep_to_same_sm_rate[i]->get_sample_no() > 0) {	//to avoid nan
	ovr_avg_single_rep_rate += m_single_rep_to_same_sm_rate[i]->avg();
	ovr_avg_multi_rep_rate += m_multi_rep_to_same_sm_rate[i]->avg();
	ovr_avg_no_rep_rate += m_no_rep_to_same_sm_rate[i]->avg();
	cnt++;
    }
  }
  ovr_avg_single_rep_rate = ovr_avg_single_rep_rate / cnt;
  ovr_avg_multi_rep_rate = ovr_avg_multi_rep_rate / cnt;
  ovr_avg_no_rep_rate = ovr_avg_no_rep_rate / cnt;
  m_ovr_avg_single_rep_to_same_sm_rate->add_sample(ovr_avg_single_rep_rate);
  m_ovr_avg_multi_rep_to_same_sm_rate->add_sample(ovr_avg_multi_rep_rate);
  m_ovr_avg_no_rep_to_same_sm_rate->add_sample(ovr_avg_no_rep_rate);
  m_ovr_avg_multi_rep_to_same_sm_no->add_sample(m_multi_rep_to_same_sm_no->avg());

  //std::cout << "ovr_avg_single_rep_rate " << ovr_avg_single_rep_rate << std::endl;
  //std::cout << "ovr_avg_multi_rep_rate " << ovr_avg_multi_rep_rate << std::endl;
  //std::cout << "ovr_avg_no_rep_rate " << ovr_avg_no_rep_rate << std::endl;
  ///

  //added by kh(042216)
  m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data->add_sample(m_word1B_zeros_rep_rate_in_uncomp_data->avg());
  m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data->add_sample(m_word1B_nonzeros_rep_rate_in_uncomp_data->avg());
  m_ovr_avg_word1B_uni_val_rate_in_uncomp_data->add_sample(m_word1B_uni_val_rate_in_uncomp_data->avg());
  m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data->add_sample(m_word1B_nonzeros_rep_no_in_uncomp_data->avg());
  ///

  //added by kh(042316)
  m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies->add_sample(m_word1B_word1B_redund_rate_inter_read_replies->avg());
  ///

}

void hpcl_comp_anal::clear()
{
  m_comp_table_hit_no->clear();
  m_comp_table_miss_no->clear();
  m_read_reply_comp_ratio->clear();

  //added by kh(041616)
  m_fragment_rate_per_flit->clear();
  m_fragment_rate_per_packet->clear();
  m_comp_overhead_no->clear();
  ///

  //added by kh(041816)
  m_org_flit_no->clear();
  m_comp_flit_no->clear();
  m_comp_2B_no->clear();
  m_comp_4B_no->clear();
  m_comp_8B_no->clear();
  ///

  //added by kh(042116)
  m_word2B_zeros_rep_rate->clear();
  m_word2B_nonzeros_rep_rate->clear();
  m_word2B_uni_val_rate->clear();
  m_word4B_zeros_rep_rate->clear();
  m_word4B_nonzeros_rep_rate->clear();
  m_word4B_uni_val_rate->clear();
  m_word8B_zeros_rep_rate->clear();
  m_word8B_nonzeros_rep_rate->clear();
  m_word8B_uni_val_rate->clear();

  for(unsigned i = 0; i < m_single_rep_to_same_sm_rate.size(); i++) {
    m_single_rep_to_same_sm_rate[i]->clear();
    m_multi_rep_to_same_sm_rate[i]->clear();
    m_no_rep_to_same_sm_rate[i]->clear();
  }
  m_multi_rep_to_same_sm_no->clear();
  ///




  //added by kh(042216)
  m_word1B_zeros_rep_rate_in_uncomp_data->clear();
  m_word1B_nonzeros_rep_rate_in_uncomp_data->clear();
  m_word1B_uni_val_rate_in_uncomp_data->clear();
  m_word1B_nonzeros_rep_no_in_uncomp_data->clear();
  ///

  //added by kh(042316)
  m_word1B_word1B_redund_rate_inter_read_replies->clear();
  ///

}
