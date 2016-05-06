/*
 * hpcl_comp_anal.h
 *
 *  Created on: Mar 8, 2016
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_HPCL_COMP_ANAL_H_
#define GPGPU_SIM_HPCL_COMP_ANAL_H_

#include "hpcl_stat.h"
#include <iostream>
#include <vector>

class hpcl_comp_anal
{
public:
  hpcl_comp_anal ();
  virtual
  ~hpcl_comp_anal ();

//added by kh(030816)
public:
  enum sample_type {
    GLOBAL_TABLE_HIT_NO = 0,
    GLOBAL_TABLE_MISS_NO,
    READ_REPLY_COMP_RATIO,
    FRAGMENT_RATE_PER_FLIT,		//fragmentation rate in the last flit
    FRAGMENT_RATE_PER_PACKET,		//fragmentation rate over the packet
    COMP_OVERHEAD_NO,
    ORG_FLIT_NO,
    COMP_FLIT_NO,
    COMP_2B_NO,
    COMP_4B_NO,
    COMP_8B_NO,
    //added by kh(042116)
    WORD2B_ZEROS_REP_RATE,
    WORD2B_NONZEROS_REP_RATE,
    WORD2B_UNI_VAL_RATE,
    WORD4B_ZEROS_REP_RATE,
    WORD4B_NONZEROS_REP_RATE,
    WORD4B_UNI_VAL_RATE,
    WORD8B_ZEROS_REP_RATE,
    WORD8B_NONZEROS_REP_RATE,
    WORD8B_UNI_VAL_RATE,
    SINGLE_REP_TO_SAME_SM_RATE,
    MULTI_REP_TO_SAME_SM_RATE,
    NO_REP_TO_SAME_SM_RATE,
    MULTI_REP_TO_SAME_SM_NO,
    //added by kh(042216)
    WORD1B_ZEROS_REP_RATE_IN_UNCOMP_DATA,
    WORD1B_NONZEROS_REP_RATE_IN_UNCOMP_DATA,
    WORD1B_UNI_VAL_RATE_IN_UNCOMP_DATA,
    WORD1B_NONZEROS_REP_NO_IN_UNCOMP_DATA,
    //added by kh(042316)
    WORD1B_REDUND_RATE_INTER_READ_REPLIES,
  };

  void create(unsigned sub_partition_no);
  void add_sample(enum sample_type type, double val, int id=-1);
  hpcl_stat* m_comp_table_hit_no;
  hpcl_stat* m_comp_table_miss_no;
  hpcl_stat* m_ovr_avg_comp_table_hit_rate;

  void display(std::ostream & os = std::cout) const;
  void update_overall_stat();
  void clear();
  ///



//added by kh(032016)
private:
  hpcl_stat* m_read_reply_comp_ratio;		//(flit no for original_packet - flit no for compressed packet)/flit no for original_packet*100
  hpcl_stat* m_ovr_avg_read_reply_comp_ratio;
  //hpcl_stat* m_read_reply_comp_flit_no;
  //hpcl_stat* m_ovr_read_reply_comp_flit_no;


//added by kh(041616)
private:
  hpcl_stat* m_fragment_rate_per_flit;			//unused space/flit size*100
  hpcl_stat* m_fragment_rate_per_packet;			//unused space/packet size*100
  hpcl_stat* m_ovr_avg_fragment_rate_per_flit;
  hpcl_stat* m_ovr_avg_fragment_rate_per_packet;
  hpcl_stat* m_comp_overhead_no;		//The # of the case where compressed Data > original Data
  hpcl_stat* m_ovr_avg_comp_overhead_rate;
///


//added by kh(041816)
private:
  hpcl_stat* m_org_flit_no;
  hpcl_stat* m_ovr_avg_org_flit_no;
  hpcl_stat* m_comp_flit_no;
  hpcl_stat* m_ovr_avg_comp_flit_no;


  hpcl_stat* m_comp_2B_no;
  hpcl_stat* m_comp_4B_no;
  hpcl_stat* m_comp_8B_no;
  hpcl_stat* m_ovr_avg_comp_2B_rate;
  hpcl_stat* m_ovr_avg_comp_4B_rate;
  hpcl_stat* m_ovr_avg_comp_8B_rate;

///

//added by kh(042116)
private:
  hpcl_stat* m_word2B_zeros_rep_rate;
  hpcl_stat* m_word2B_nonzeros_rep_rate;
  hpcl_stat* m_word2B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word2B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word2B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word2B_uni_val_rate;

  hpcl_stat* m_word4B_zeros_rep_rate;
  hpcl_stat* m_word4B_nonzeros_rep_rate;
  hpcl_stat* m_word4B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word4B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word4B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word4B_uni_val_rate;

  hpcl_stat* m_word8B_zeros_rep_rate;
  hpcl_stat* m_word8B_nonzeros_rep_rate;
  hpcl_stat* m_word8B_uni_val_rate;
  hpcl_stat* m_ovr_avg_word8B_zeros_rep_rate;
  hpcl_stat* m_ovr_avg_word8B_nonzeros_rep_rate;
  hpcl_stat* m_ovr_avg_word8B_uni_val_rate;

  //distribution of replies in the compression buffer
  std::vector<hpcl_stat*> m_single_rep_to_same_sm_rate;
  std::vector<hpcl_stat*> m_multi_rep_to_same_sm_rate;
  std::vector<hpcl_stat*> m_no_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_single_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_multi_rep_to_same_sm_rate;
  hpcl_stat* m_ovr_avg_no_rep_to_same_sm_rate;
  hpcl_stat* m_multi_rep_to_same_sm_no;
  hpcl_stat* m_ovr_avg_multi_rep_to_same_sm_no;

  /*
  std::vector<hpcl_stat*> m_ovr_avg_single_rep_rate;
  std::vector<hpcl_stat*> m_ovr_avg_multi_rep_rate;
  std::vector<hpcl_stat*> m_ovr_avg_no_rep_rate;
  hpcl_stat* m_ovr_avg_single_rep_rate_for_all;
  hpcl_stat* m_ovr_avg_multi_rep_rate_for_all;
  hpcl_stat* m_ovr_avg_no_rep_rate_for_all;
  */
///


//added by kh(042216)
private:
  hpcl_stat* m_word1B_zeros_rep_rate_in_uncomp_data;
  hpcl_stat* m_word1B_nonzeros_rep_rate_in_uncomp_data;
  hpcl_stat* m_word1B_uni_val_rate_in_uncomp_data;
  hpcl_stat* m_word1B_nonzeros_rep_no_in_uncomp_data;
  hpcl_stat* m_ovr_avg_word1B_zeros_rep_rate_in_uncomp_data;
  hpcl_stat* m_ovr_avg_word1B_nonzeros_rep_rate_in_uncomp_data;
  hpcl_stat* m_ovr_avg_word1B_uni_val_rate_in_uncomp_data;
  hpcl_stat* m_ovr_avg_word1B_nonzeros_rep_no_in_uncomp_data;
///


//added by kh(042316)
private:
  hpcl_stat* m_word1B_word1B_redund_rate_inter_read_replies;
  hpcl_stat* m_ovr_avg_word1B_word1B_redund_rate_inter_read_replies;
///







};

#endif /* GPGPU_SIM_HPCL_COMP_ANAL_H_ */
