/*
 * network_anal.cpp
 *
 *  Created on: Dec 28, 2015
 *      Author: mumichang
 */

#include "hpcl_network_anal.h"
#include "assert.h"
#include <iostream>
#include <iomanip>

hpcl_network_anal::hpcl_network_anal() {
  // TODO Auto-generated constructor stub

}

hpcl_network_anal::~hpcl_network_anal() {
  // TODO Auto-generated destructor stub
}

void hpcl_network_anal::create_input_buffer_util_stat(unsigned subnet_no, unsigned node_no, unsigned vc_no)
{
  //The # of input port is always 4 under the assumption of mesh network
  unsigned in_port_no = 4;

  for(unsigned m = 0; m < subnet_no; m++){
      std::vector<std::vector<hpcl_stat*> > tmp1;
      for(unsigned i = 0; i < node_no; i++){
	  std::vector<hpcl_stat*> tmp2;
	  for(unsigned j = 0; j < vc_no*in_port_no; j++){
	      tmp2.push_back(new hpcl_stat);
	  }
	  tmp1.push_back(tmp2);
      }
      m_input_buffer_util[m] = tmp1;
  }

  for(unsigned m = 0; m < subnet_no; m++){
      std::vector<std::vector<hpcl_stat*> > tmp1;
      for(unsigned i = 0; i < node_no; i++){
  	std::vector<hpcl_stat*> tmp2;
  	for(unsigned j = 0; j < vc_no*in_port_no; j++){
  	    tmp2.push_back(new hpcl_stat);
  	}
  	tmp1.push_back(tmp2);
      }
      m_ovr_input_buffer_util[m] = tmp1;
  }



}

void hpcl_network_anal::create_link_util_stat(unsigned subnet_no, unsigned channel_no)
{
  for(unsigned i = 0; i < subnet_no; i++){
      std::vector<hpcl_stat*> tmp;
      for(unsigned j = 0; j < channel_no; j++)	tmp.push_back(new hpcl_stat);
      m_link_util.push_back(tmp);
  }

  m_ovr_req_link_util = new hpcl_stat;
  if(m_link_util.size() > 1) {
      m_ovr_rep_link_util = new hpcl_stat;
  }
}

void hpcl_network_anal::create(unsigned subnet_no, unsigned channel_no, unsigned node_no, unsigned vc_no)
{
  m_port_no = 4;
  m_vc_no = vc_no;

  //create data structures for network latency stat.
  m_req_queuing_lat_in_sm = new hpcl_stat;
  m_req_net_lat = new hpcl_stat;
  m_req_mc_lat = new hpcl_stat;
  m_rep_queuing_lat_in_mc = new hpcl_stat;
  m_rep_net_lat = new hpcl_stat;
  m_req_zeroload_lat = new hpcl_stat;
  m_req_contention_lat = new hpcl_stat;
  m_rep_zeroload_lat = new hpcl_stat;
  m_rep_contention_lat = new hpcl_stat;

  m_ovr_req_queuing_lat_in_sm = new hpcl_stat;
  m_ovr_req_net_lat = new hpcl_stat;
  m_ovr_req_mc_lat = new hpcl_stat;
  m_ovr_rep_queuing_lat_in_mc = new hpcl_stat;
  m_ovr_rep_net_lat = new hpcl_stat;
  m_ovr_req_zeroload_lat = new hpcl_stat;
  m_ovr_req_contention_lat = new hpcl_stat;
  m_ovr_rep_zeroload_lat = new hpcl_stat;
  m_ovr_rep_contention_lat = new hpcl_stat;

  //create data structures for link utilization stat.
  create_link_util_stat(subnet_no, channel_no);

  //create data structures for link usage stat.
  m_ovr_req_link_usage = new hpcl_stat;
  m_ovr_rep_link_usage = new hpcl_stat;

  //crate data structures for input buffer utilization stat
  create_input_buffer_util_stat(subnet_no, node_no, vc_no);

  //added by kh(030816)
  m_norm_flit_no = new hpcl_stat;
  m_ctrl_flit_no = new hpcl_stat;
  m_read_reply_comp_flit_no = new hpcl_stat;
  m_read_reply_uncomp_flit_no = new hpcl_stat;
  m_ovr_avg_ctrl_flit_ratio = new hpcl_stat;
  m_ovr_avg_read_rep_comp_flit_ratio = new hpcl_stat;
  ///
}

void hpcl_network_anal::add_sample(enum sample_type type, double val, int id1, int id2, int id3, int id4)
{
    if(type == REQ_QUEUING_LAT_IN_SM)       	m_req_queuing_lat_in_sm->add_sample(val);
    else if(type == REQ_NET_LAT)      		m_req_net_lat->add_sample(val);
    else if(type == REQ_MC_LAT)             	m_req_mc_lat->add_sample(val);
    else if(type == REP_QUEUING_LAT_IN_MC)  	m_rep_queuing_lat_in_mc->add_sample(val);
    else if(type == REP_NET_LAT)      		m_rep_net_lat->add_sample(val);
    else if(type == REQ_ZEROLOAD_LAT)      	m_req_zeroload_lat->add_sample(val);
    else if(type == REQ_CONTENTION_LAT)      	m_req_contention_lat->add_sample(val);
    else if(type == REP_ZEROLOAD_LAT)      	m_rep_zeroload_lat->add_sample(val);
    else if(type == REP_CONTENTION_LAT)      	m_rep_contention_lat->add_sample(val);
    else if(type == LINK_UTIL){
	//id1: subnetID, id2: channel
	m_link_util[id1][id2]->add_sample(val);
    }
    else if(type == INPUT_BUFFER_UTIL) {
      //id1: subnetID, id2: node, id3: input_port(0~3), id4: vc_index(0~3)
      unsigned index = id3*m_vc_no+id4;
      m_input_buffer_util[id1][id2][index]->add_sample(val);
    }
    //added by kh(030816)
    else if(type == NORMAL_FLIT_NO) {
      m_norm_flit_no->add_sample(val);
    }
    else if(type == CTRL_FLIT_NO) {
      m_ctrl_flit_no->add_sample(val);
    }
    else if(type == READ_REPLY_COMP_FLIT_NO) {
      m_read_reply_comp_flit_no->add_sample(val);
    }
    else if(type == READ_REPLY_UNCOMP_FLIT_NO) {
      m_read_reply_uncomp_flit_no->add_sample(val);
    }
    ///
}
/*
void hpcl_network_anal::add_sample(enum sample_type type, double val, int id1, int id2, int id3, int id4)
{
  if(type == INPUT_BUFFER_UTIL) {
    //id1: subnetID, id2: node, id3: input_port(0~3), id4: vc_index(0~3)
    unsigned index = id3*m_vc_no+id4;
    m_input_buffer_util[id1][id2][index]->add_sample(val);
  }
}
*/


void hpcl_network_anal::update_overall_stat(double sim_time)
{
  //network latency update
  m_ovr_req_queuing_lat_in_sm->add_sample(m_req_queuing_lat_in_sm->avg());
  m_ovr_req_net_lat->add_sample(m_req_net_lat->avg());
  m_ovr_req_mc_lat->add_sample(m_req_mc_lat->avg());
  m_ovr_rep_queuing_lat_in_mc->add_sample(m_rep_queuing_lat_in_mc->avg());
  m_ovr_rep_net_lat->add_sample(m_rep_net_lat->avg());

  m_ovr_req_zeroload_lat->add_sample(m_req_zeroload_lat->avg());
  m_ovr_req_contention_lat->add_sample(m_req_contention_lat->avg());
  m_ovr_rep_zeroload_lat->add_sample(m_rep_zeroload_lat->avg());
  m_ovr_rep_contention_lat->add_sample(m_rep_contention_lat->avg());

  /*
  std::cout << "m_req_zeroload_lat->avg() : " << m_req_zeroload_lat->avg() << std::endl;
  std::cout << "m_req_contention_lat->avg() : " << m_req_contention_lat->avg() << std::endl;
  std::cout << "m_rep_zeroload_lat->avg() : " << m_rep_zeroload_lat->avg() << std::endl;
  std::cout << "m_rep_contention_lat->avg() : " << m_rep_contention_lat->avg() << std::endl;
  */

  //network link utilization update
  double ovr_req_link_util_rate = 0;
  for(unsigned i = 0; i < m_link_util[0].size(); i++){
      //std::cout << "net_0_ch_" << i << " = " << m_link_util[0][i]->sum() << " link_util " << (m_link_util[0][i]->sum()/(double)sim_time)*100 << std::endl;
      ovr_req_link_util_rate += (m_link_util[0][i]->sum()/(double)sim_time)*100;
  }
  //std::cout << "m_link_util[0].size() : " << m_link_util[0].size() << std::endl;
  ovr_req_link_util_rate /= m_link_util[0].size();
  m_ovr_req_link_util->add_sample(ovr_req_link_util_rate);
  //std::cout << "gpu_ovr_req_link_util = " << ovr_req_link_util_rate << std::endl;
  if(m_link_util.size() > 1) {
      double ovr_rep_link_util_rate = 0;
      for(unsigned i = 0; i < m_link_util[1].size(); i++){
	  //std::cout << "net_1_ch_" << i << " = " << m_link_util[1][i]->sum() << " link_util " << (m_link_util[1][i]->sum()/(double)sim_time)*100 << std::endl;
	  ovr_rep_link_util_rate += (m_link_util[1][i]->sum()/(double)sim_time)*100;
      }
      //std::cout << "m_link_util[1].size() : " << m_link_util[1].size() << std::endl;
      ovr_rep_link_util_rate /= m_link_util[1].size();
      m_ovr_rep_link_util->add_sample(ovr_rep_link_util_rate);
      //std::cout << "gpu_ovr_rep_link_util = " << ovr_rep_link_util_rate << std::endl;
  }
  ///

  //network link usage update
  double ovr_req_link_usage = 0;
  for(unsigned i = 0; i < m_link_util[0].size(); i++){
      //std::cout << "net_0_ch_" << i << " = " << m_link_util[0][i]->sum() << " link_util " << (m_link_util[0][i]->sum()/(double)sim_time)*100 << std::endl;
      ovr_req_link_usage += m_link_util[0][i]->sum();
  }
  ovr_req_link_usage /= m_link_util[0].size();
  m_ovr_req_link_usage->add_sample(ovr_req_link_usage);

  if(m_link_util.size() > 1) {
      double ovr_rep_link_usage = 0;
      for(unsigned i = 0; i < m_link_util[1].size(); i++){
	  ovr_rep_link_usage += m_link_util[1][i]->sum();
      }
      ovr_rep_link_usage /= m_link_util[1].size();
      m_ovr_rep_link_usage->add_sample(ovr_rep_link_usage);
  }
  ///

  //input buffer utilization
  for(unsigned i = 0; i < m_input_buffer_util[0].size(); i++){
      for(unsigned j = 0; j < m_input_buffer_util[0][i].size(); j++){
	  m_ovr_input_buffer_util[0][i][j]->add_sample(m_input_buffer_util[0][i][j]->sum()/(double)sim_time*100);
      }
  }
  if(m_link_util.size() > 1) {
      for(unsigned i = 0; i < m_input_buffer_util[1].size(); i++){
	  for(unsigned j = 0; j < m_input_buffer_util[1][i].size(); j++){
	      m_ovr_input_buffer_util[1][i][j]->add_sample(m_input_buffer_util[1][i][j]->sum()/(double)sim_time*100);
          }
      }
  }
  ///

  //added by kh(030816)
  m_ovr_avg_ctrl_flit_ratio->add_sample(m_ctrl_flit_no->sum()/(m_ctrl_flit_no->sum()+m_norm_flit_no->sum())*100);
  m_ovr_avg_read_rep_comp_flit_ratio->add_sample(m_read_reply_comp_flit_no->sum()/(m_read_reply_comp_flit_no->sum()+m_read_reply_uncomp_flit_no->sum())*100);
  ///

}

void hpcl_network_anal::display(std::ostream & os) const
{
  os << "====== hpcl_network_anal ======" << std::endl;

  os << "gpu_tot_ovr_req_qlat = " << m_ovr_req_queuing_lat_in_sm->avg() << std::endl;
  os << "gpu_tot_ovr_req_nlat = " << m_ovr_req_net_lat->avg() << std::endl;
  os << "gpu_tot_ovr_req_plat = " << (m_ovr_req_queuing_lat_in_sm->avg()+m_ovr_req_net_lat->avg()) << std::endl;
  os << "gpu_tot_ovr_mc_lat = " << m_ovr_req_mc_lat->avg() << std::endl;
  os << "gpu_tot_ovr_rep_qlat = " << m_ovr_rep_queuing_lat_in_mc->avg() << std::endl;
  os << "gpu_tot_ovr_rep_nlat = " << m_ovr_rep_net_lat->avg() << std::endl;
  os << "gpu_tot_ovr_rep_plat = " << (m_ovr_rep_queuing_lat_in_mc->avg()+m_ovr_rep_net_lat->avg()) << std::endl;

  os << "gpu_tot_ovr_req_zeroload_lat = " << m_ovr_req_zeroload_lat->avg() << std::endl;
  os << "gpu_tot_ovr_req_contention_lat = " << m_ovr_req_contention_lat->avg() << std::endl;
  os << "gpu_tot_ovr_rep_zeroload_lat = " << m_ovr_rep_zeroload_lat->avg() << std::endl;
  os << "gpu_tot_ovr_rep_contention_lat = " << m_ovr_rep_contention_lat->avg() << std::endl;

  if(m_link_util.size() > 1) {
      os << "gpu_tot_ovr_req_link_util = " << m_ovr_req_link_util->avg() << " \%" << std::endl;
      os << "gpu_tot_ovr_rep_link_util = " << m_ovr_rep_link_util->avg() << " \%" << std::endl;
      os << "gpu_tot_ovr_link_util = " << (m_ovr_req_link_util->avg()+m_ovr_rep_link_util->avg())/2 << " \%" << std::endl;
      os << "gpu_tot_ovr_avg_req_link_usage = " << m_ovr_req_link_usage->avg() << std::endl;
      os << "gpu_tot_ovr_avg_rep_link_usage = " << m_ovr_rep_link_usage->avg() << std::endl;
      os << "gpu_tot_ovr_avg_link_usage = " << (m_ovr_req_link_usage->avg()+m_ovr_rep_link_usage->avg())/2 << std::endl;
  } else {
      os << "gpu_tot_ovr_avg_link_util = " << m_ovr_req_link_util->avg() << " \%" << std::endl;
      os << "gpu_tot_ovr_avg_link_usage = " << m_ovr_req_link_usage->avg() << std::endl;
  }

  os << "----- Input Buffer Utilization (Subnet 0) -----" << std::endl;
  //unsigned node = 0;
  for(unsigned i = 0; i < m_ovr_input_buffer_util[0].size(); i++){
      os << "subnet_0_node " << i << "\n";
      unsigned input = 0;
      for(unsigned j = 0; j < m_ovr_input_buffer_util[0][i].size(); j=j+m_vc_no){
	  //if((j % (m_vc_no*m_port_no)) == 0){
	  //    os << "\tinput " << input++ << "\n";
	  //}
	  os << std::fixed << std::setprecision(2);
	  os << "\tinput" << input++ << ":(";
	  for(unsigned k = 0; k < m_vc_no; k++){
	      os << m_ovr_input_buffer_util[0][i][j+k]->avg();
	      if(k < (m_vc_no-1)) os << ",";
	  }
	  os << ")\n";
      }
      //os << " ]" << std::endl;
  }

  if(m_link_util.size() > 1) {
      //node = 0;
      os << "----- Input Buffer Utilization (Subnet 1) -----" << std::endl;
      for(unsigned i = 0; i < m_ovr_input_buffer_util[1].size(); i++){
	  os << "subnet_1_node " << i << "\n";
	  unsigned input = 0;
	  for(unsigned j = 0; j < m_ovr_input_buffer_util[1][i].size(); j=j+m_vc_no){
	      //if((j % (m_vc_no*m_port_no)) == 0){
	      //	  os << "\tinput " << input++ << "\n";
	      //}
	      os << std::fixed << std::setprecision(2);
	      os << "\tinput" << input++ << ":(";
	      for(unsigned k = 0; k < m_vc_no; k++){
		  os << m_ovr_input_buffer_util[1][i][j+k]->avg();
		  if(k < (m_vc_no-1)) os << ",";
	      }
	      os << ")\n";
	  }
	  //os << " ]" << std::endl;
      }
  }

  //added by kh(030816)
  os << "gpu_tot_ovr_avg_ctrl_flit_ratio = " << m_ovr_avg_ctrl_flit_ratio->avg() << " \%" << std::endl;
  os << "gpu_tot_ovr_avg_read_rep_comp_flit_ratio = " << m_ovr_avg_read_rep_comp_flit_ratio->avg() << " \%" << std::endl;
  ///

  os << "===============================" << std::endl;
}

void hpcl_network_anal::clear()
{
//  std::cout << "hpcl_network_anal::clear start" << std::endl;
  m_req_queuing_lat_in_sm->clear();
  m_req_net_lat->clear();
  m_req_mc_lat->clear();
  m_rep_queuing_lat_in_mc->clear();
  m_rep_net_lat->clear();

  m_req_zeroload_lat->clear();
  m_req_contention_lat->clear();
  m_rep_zeroload_lat->clear();
  m_rep_contention_lat->clear();

  clear_link_util_stat();
  clear_input_buffer_util_stat();

  //added by kh(030816)
  m_norm_flit_no->clear();
  m_ctrl_flit_no->clear();
  m_read_reply_comp_flit_no->clear();
  m_read_reply_uncomp_flit_no->clear();
  ///
//  std::cout << "hpcl_network_anal::clear end" << std::endl;
}



void hpcl_network_anal::clear_link_util_stat()
{
  for(unsigned i = 0; i < m_link_util.size(); i++){
      for(unsigned j = 0; j < m_link_util[i].size(); j++){
	  m_link_util[i][j]->clear();
      }
  }
}

void hpcl_network_anal::clear_input_buffer_util_stat()
{
  for(unsigned m = 0; m < 2; m++){
      for(unsigned i = 0; i < m_input_buffer_util[m].size(); i++){
	  for(unsigned j = 0; j < m_input_buffer_util[m][i].size(); j++){
	      m_input_buffer_util[m][i][j]->clear();
	  }
      }
  }
}



