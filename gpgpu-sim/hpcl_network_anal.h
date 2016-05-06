/*
 * network_anal.hpp
 *
 *  Created on: Dec 28, 2015
 *      Author: mumichang
 */

#ifndef INTERSIM2_NETWORK_ANAL_HPP_
#define INTERSIM2_NETWORK_ANAL_HPP_

#include "hpcl_stat.h"
#include <iostream>
#include <sstream>
#include <vector>

/*
  < Definition of latencies >
  req_queuing_lat_in_sm = {x | x = Time@HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM - Time@HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM};
  req_net_lat = {x | x = Time@TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM - Time@HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM};
  req_mc_lat = {x | x = Time@HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM - Time@TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM};
  rep_queuing_lat_in_mc = {x | x = Time@HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM - Time@HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM};
  rep_net_lat = {x | x = Time@TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_SM - Time@HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM}
*/

class hpcl_network_anal {
public:
  hpcl_network_anal();
  virtual ~hpcl_network_anal();

//general functions
public:
  void update_overall_stat(double sim_time);
  void display(std::ostream & os = std::cout) const;
  void clear();
  void create(unsigned subnet_no, unsigned channel_no, unsigned node_no, unsigned vc_no);

//network latency analysis
private:
  hpcl_stat* m_req_queuing_lat_in_sm;
  hpcl_stat* m_req_net_lat;
  hpcl_stat* m_req_mc_lat;
  hpcl_stat* m_rep_queuing_lat_in_mc;
  hpcl_stat* m_rep_net_lat;

  hpcl_stat* m_req_zeroload_lat;
  hpcl_stat* m_req_contention_lat;
  hpcl_stat* m_rep_zeroload_lat;
  hpcl_stat* m_rep_contention_lat;

  hpcl_stat* m_ovr_req_queuing_lat_in_sm;
  hpcl_stat* m_ovr_req_net_lat;
  hpcl_stat* m_ovr_req_mc_lat;
  hpcl_stat* m_ovr_rep_queuing_lat_in_mc;
  hpcl_stat* m_ovr_rep_net_lat;

  hpcl_stat* m_ovr_req_zeroload_lat;
  hpcl_stat* m_ovr_req_contention_lat;
  hpcl_stat* m_ovr_rep_zeroload_lat;
  hpcl_stat* m_ovr_rep_contention_lat;

public:

  enum sample_type {
    REQ_QUEUING_LAT_IN_SM = 0,
    REQ_NET_LAT,
    REQ_MC_LAT,
    REP_QUEUING_LAT_IN_MC,
    REP_NET_LAT,
    REQ_ZEROLOAD_LAT,
    REQ_CONTENTION_LAT,
    REP_ZEROLOAD_LAT,
    REP_CONTENTION_LAT,
    LINK_UTIL,
    INPUT_BUFFER_UTIL,

    //added by kh(030816)
    NORMAL_FLIT_NO,
    CTRL_FLIT_NO,
    READ_REPLY_COMP_FLIT_NO,		//among read reply flits
    READ_REPLY_UNCOMP_FLIT_NO,		//among read reply flits
  };

  //void add_sample(enum sample_type type, double val, int id1=0, int id2=0);
  void add_sample(enum sample_type type, double val, int id1=0, int id2=0, int id3=0, int id4=0);

//network link utilization/usage
private:
  std::vector<std::vector<hpcl_stat*> > m_link_util;
  hpcl_stat* m_ovr_req_link_util;
  hpcl_stat* m_ovr_rep_link_util;

  void clear_link_util_stat();
  void create_link_util_stat(unsigned subnet_no, unsigned channel_no);

  hpcl_stat* m_ovr_req_link_usage;
  hpcl_stat* m_ovr_rep_link_usage;

 //input buffer utilization
private:
  std::vector<std::vector<hpcl_stat*> > m_input_buffer_util[2];
  std::vector<std::vector<hpcl_stat*> > m_ovr_input_buffer_util[2];
  void create_input_buffer_util_stat(unsigned subnet_no, unsigned node_no, unsigned vc_no);
  void clear_input_buffer_util_stat();
  unsigned m_port_no;
  unsigned m_vc_no;

  //added by kh(030816)
  //compression performance based on accepted flits
  hpcl_stat* m_norm_flit_no;
  hpcl_stat* m_ctrl_flit_no;
  hpcl_stat* m_read_reply_comp_flit_no;
  hpcl_stat* m_read_reply_uncomp_flit_no;
  hpcl_stat* m_ovr_avg_ctrl_flit_ratio;			//ctrl_flit_no/(ctrl_flit_no+norm_flit_no)*100
  hpcl_stat* m_ovr_avg_read_rep_comp_flit_ratio;	//comp_flit_no/(comp_flit_no+uncomp_flit_no)*100



};

#endif /* INTERSIM2_NETWORK_ANAL_HPP_ */






