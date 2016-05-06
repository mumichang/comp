// Copyright (c) 2009-2013, Tor M. Aamodt, Dongdong Li, Ali Bakhoda
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <sstream>
#include <fstream>
#include <limits> 

#include "gputrafficmanager.hpp"
#include "interconnect_interface.hpp"
#include "globals.hpp"

//added by kh(070715)
#include "../gpgpu-sim/user-defined.h"
#include "../gpgpu-sim/mem_fetch.h"
extern unsigned long long  gpu_sim_cycle;
extern unsigned long long  gpu_tot_sim_cycle;
///

//added by kh(121715)
extern struct mc_placement_config g_mc_placement_config;


//added by kh(122815)
#include "../gpgpu-sim/hpcl_network_anal.h"
extern hpcl_network_anal* g_hpcl_network_anal;
///

//added by kh(030816)
#include <cstdlib>
#include "../gpgpu-sim/hpcl_comp_anal.h"
#include "../gpgpu-sim/hpcl_comp_pl.h"
#include "../gpgpu-sim/hpcl_decomp_pl.h"
#include "../gpgpu-sim/hpcl_dict_ctrl_msg.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern std::vector<hpcl_comp_pl<unsigned short>* > g_hpcl_global_comp_pl_2B;
extern std::vector<hpcl_decomp_pl<unsigned short>* > g_hpcl_global_decomp_pl_2B;
extern hpcl_comp_config g_hpcl_comp_config;
extern hpcl_comp_anal* g_hpcl_comp_anal;
///

GPUTrafficManager::GPUTrafficManager( const Configuration &config, const vector<Network *> &net)
:TrafficManager(config, net)
{
  // The total simulations equal to number of kernels
  _total_sims = 0;
  
  _input_queue.resize(_subnets);
  for ( int subnet = 0; subnet < _subnets; ++subnet) {
    _input_queue[subnet].resize(_nodes);
    for ( int node = 0; node < _nodes; ++node ) {
      _input_queue[subnet][node].resize(_classes);
    }
  }

  //added by kh(122815)
  assert(g_hpcl_network_anal);
  g_hpcl_network_anal->create(_subnets, _net[0]->GetChannels().size(), _nodes, _vcs);
  ///


}

GPUTrafficManager::~GPUTrafficManager()
{
}

void GPUTrafficManager::Init()
{
  //added by kh(061715)
  _tot_sim_time = _tot_sim_time + _time;

  _time = 0;
  _sim_state = running;
  _ClearStats( );
  
  //added by kh(122815)
  g_hpcl_network_anal->clear();


  //added by kh(030816)
  if(g_hpcl_comp_anal)	g_hpcl_comp_anal->clear();
  ///

}

void GPUTrafficManager::_RetireFlit( Flit *f, int dest )
{
  _deadlock_timer = 0;
  
  assert(_total_in_flight_flits[f->cl].count(f->id) > 0);
  _total_in_flight_flits[f->cl].erase(f->id);
  
  if(f->record) {
    assert(_measured_in_flight_flits[f->cl].count(f->id) > 0);
    _measured_in_flight_flits[f->cl].erase(f->id);
  }
  
  if ( f->watch ) {
  //if(1) {
    *gWatchOut << GetSimTime() << " | "
    << "node" << dest << " | "
    << "Retiring flit " << f->id
    << " (packet " << f->pid
    << ", src = " << f->src
    << ", dest = " << f->dest
	//added by kh(062515)
	<< ", const_dest = " << f->const_dest
	<< ", hops = " << f->hops
    << ", flat = " << f->atime - f->itime
    << ", subnetwork = " << f->subnetwork
    << ", class = " << f->cl
    << ", priority = " << f->pri
	<< ")." << endl;
  }
  

  if ( f->head && ( f->dest != dest ) ) {
    ostringstream err;
    err << "Flit " << f->id << " arrived at incorrect output " << dest;
    Error( err.str( ) );
  }
  
  if((_slowest_flit[f->cl] < 0) ||
     (_flat_stats[f->cl]->Max() < (f->atime - f->itime)))
    _slowest_flit[f->cl] = f->id;
  
  _flat_stats[f->cl]->AddSample( f->atime - f->itime);
  if(_pair_stats){
    _pair_flat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - f->itime );
  }
  
  if ( f->tail ) {
    Flit * head;
    if(f->head) {
      head = f;
    } else {
      map<int, Flit *>::iterator iter = _retired_packets[f->cl].find(f->pid);
      assert(iter != _retired_packets[f->cl].end());
      head = iter->second;
      _retired_packets[f->cl].erase(iter);
      assert(head->head);
      assert(f->pid == head->pid);
    }
    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | "
      << "node" << dest << " | "
      << "Retiring packet " << f->pid
      << " (plat = " << f->atime - head->ctime
      << ", nlat = " << f->atime - head->itime
      << ", frag = " << (f->atime - head->atime) - (f->id - head->id) // NB: In the spirit of solving problems using ugly hacks, we compute the packet length by taking advantage of the fact that the IDs of flits within a packet are contiguous.
      << ", src = " << head->src
      << ", dest = " << head->dest
      << ")." << endl;
    }
   
// GPGPUSim: Memory will handle reply, do not need this
#if 0
    //code the source of request, look carefully, its tricky ;)
    if (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST) {
      PacketReplyInfo* rinfo = PacketReplyInfo::New();
      rinfo->source = f->src;
      rinfo->time = f->atime;
      rinfo->record = f->record;
      rinfo->type = f->type;
      _repliesPending[dest].push_back(rinfo);
    } else {
      if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY  ){
        _requestsOutstanding[dest]--;
      } else if(f->type == Flit::ANY_TYPE) {
        _requestsOutstanding[f->src]--;
      }
      
    }
#endif

    if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY  ){
      _requestsOutstanding[dest]--;
    } else if(f->type == Flit::ANY_TYPE) {
      ostringstream err;
      err << "Flit " << f->id << " cannot be ANY_TYPE" ;
      Error( err.str( ) );
    }
    
    // Only record statistics once per packet (at tail)
    // and based on the simulation state
    if ( ( _sim_state == warming_up ) || f->record ) {
      
      _hop_stats[f->cl]->AddSample( f->hops );
      
      if((_slowest_packet[f->cl] < 0) ||
         (_plat_stats[f->cl]->Max() < (f->atime - head->itime)))
        _slowest_packet[f->cl] = f->pid;
      _plat_stats[f->cl]->AddSample( f->atime - head->ctime);
      _nlat_stats[f->cl]->AddSample( f->atime - head->itime);
      _frag_stats[f->cl]->AddSample( (f->atime - head->atime) - (f->id - head->id) );
      
      if(_pair_stats){
        _pair_plat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - head->ctime );
        _pair_nlat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - head->itime );
      }
    }
    
    if(f != head) {
      head->Free();
    }
    
    /*
    //added by kh(070215)
    double zeroload_lat = ((f->hops-1)*2 + (f->hops-1) + 1 + f->flit_no);
    _p_zeroload_lat[f->subnetwork][f->const_dest] += zeroload_lat;

    //deleted by kh(092415)
    //_p_dynamic_lat[f->subnetwork][f->const_dest] += ((f->atime-head->itime) - zeroload_lat);

    //added by kh(092415)
    _p_dynamic_lat[f->subnetwork][f->const_dest] += ((f->tatime-head->itime) - zeroload_lat);
    _p_ejection_lat[f->subnetwork][f->const_dest] += (f->atime-f->tatime);
    ///

    //std::cout << "fid: " << f->id << ", dest: " << f->const_dest << ", (head->itime-head->ctime) = " << (head->itime-head->ctime) << std::endl;
    _p_queing_lat[f->subnetwork][f->const_dest] += (head->itime-head->ctime);
    _p_packet_lat[f->subnetwork][f->const_dest] += (f->atime - head->ctime);
    _p_hop_no[f->subnetwork][f->const_dest] += f->hops;
    _p_no[f->subnetwork][f->const_dest]++;
    ///
    */

    //added by kh(122915)
    double zeroload_lat = 1 + (f->hops-1)*3 + (f->hops-1) + f->flit_no;
    double contention_lat = (f->atime-head->itime) - zeroload_lat;
    if(f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST) {
	//unsigned long long head_flit_inj_time = ((mem_fetch*)f->data)->m_status_timestamp[HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM];
	//unsigned long long tail_flit_arr_time = ((mem_fetch*)f->data)->m_status_timestamp[TAIL_FLIT_ARRIVING_IN_VC_IN_MEM];
	//std::cout << "req_zeroload_lat = " << zeroload_lat << " f->flit_no = " << f->flit_no << std::endl;

	//std::cout << "add_sample(REQ_ZEROLOAD_LAT) calls" << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_ZEROLOAD_LAT, zeroload_lat);

	//std::cout << "add_sample(REQ_CONTENTION_LAT) calls" << std::endl;
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, (tail_flit_arr_time-head_flit_inj_time) - zeroload_lat);
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, (f->tatime-head->itime) - zeroload_lat);
	//std::cout << "req_contention_lat = " << contention_lat << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_CONTENTION_LAT, contention_lat);

	//((mem_fetch*)f->data)->req_zeroload_lat = (unsigned long long)zeroload_lat;
	//((mem_fetch*)f->data)->req_dynamic_lat = (unsigned long long)((tail_flit_arr_time-head_flit_inj_time) - zeroload_lat);
    } else if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY) {

	//std::cout << "rep_zeroload_lat = " << zeroload_lat << " f->flit_no = " << f->flit_no << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_ZEROLOAD_LAT, zeroload_lat);
	//g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_CONTENTION_LAT, (f->tatime-head->itime) - zeroload_lat);
	//std::cout << "rep_contention_lat = " << contention_lat << std::endl;
	g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_CONTENTION_LAT, contention_lat);

	//((mem_fetch*)f->data)->rep_zeroload_lat = (unsigned long long)zeroload_lat;
	//((mem_fetch*)f->data)->rep_dynamic_lat = (unsigned long long)((f->tatime-head->itime) - zeroload_lat);
    }
    ///







  }
  
  if(f->head && !f->tail) {
    _retired_packets[f->cl].insert(make_pair(f->pid, f));
  } else {
    f->Free();
  }
}
int  GPUTrafficManager::_IssuePacket( int source, int cl )
{
  return 0;
}

//TODO: Remove stype?
void GPUTrafficManager::_GeneratePacket(int source, int stype, int cl, int time, int subnet, int packet_size, const Flit::FlitType& packet_type, void* const data, int dest)
{
  assert(stype!=0);
  
  //  Flit::FlitType packet_type = Flit::ANY_TYPE;
  int size = packet_size; //input size
  int pid = _cur_pid++;
  assert(_cur_pid);
  int packet_destination = dest;
  bool record = false;
  bool watch = gWatchOut && (_packets_to_watch.count(pid) > 0);
  
  // In GPGPUSim, the core specified the packet_type and size
  
#if 0
  if(_use_read_write[cl]){
    if(stype > 0) {
      if (stype == 1) {
        packet_type = Flit::READ_REQUEST;
        size = _read_request_size[cl];
      } else if (stype == 2) {
        packet_type = Flit::WRITE_REQUEST;
        size = _write_request_size[cl];
      } else {
        ostringstream err;
        err << "Invalid packet type: " << packet_type;
        Error( err.str( ) );
      }
    } else {
      PacketReplyInfo* rinfo = _repliesPending[source].front();
      if (rinfo->type == Flit::READ_REQUEST) {//read reply
        size = _read_reply_size[cl];
        packet_type = Flit::READ_REPLY;
      } else if(rinfo->type == Flit::WRITE_REQUEST) {  //write reply
        size = _write_reply_size[cl];
        packet_type = Flit::WRITE_REPLY;
      } else {
        ostringstream err;
        err << "Invalid packet type: " << rinfo->type;
        Error( err.str( ) );
      }
      packet_destination = rinfo->source;
      time = rinfo->time;
      record = rinfo->record;
      _repliesPending[source].pop_front();
      rinfo->Free();
    }
  }
#endif
  
  if ((packet_destination <0) || (packet_destination >= _nodes)) {
    ostringstream err;
    err << "Incorrect packet destination " << packet_destination
    << " for stype " << packet_type;
    Error( err.str( ) );
  }
  
  if ( ( _sim_state == running ) ||
      ( ( _sim_state == draining ) && ( time < _drain_time ) ) ) {
    record = _measure_stats[cl];
  }
  
  int subnetwork = subnet;
  //                ((packet_type == Flit::ANY_TYPE) ?
  //                    RandomInt(_subnets-1) :
  //                    _subnet[packet_type]);
  
  if ( watch ) {
    *gWatchOut << GetSimTime() << " | "
    << "node" << source << " | "
    << "Enqueuing packet " << pid
    << " at time " << time
    << "." << endl;
  }
  
  for ( int i = 0; i < size; ++i ) {
    Flit * f  = Flit::New();
    f->id     = _cur_id++;
    assert(_cur_id);
    f->pid    = pid;
    f->watch  = watch | (gWatchOut && (_flits_to_watch.count(f->id) > 0));
    f->subnetwork = subnetwork;
    f->src    = source;
    f->ctime  = time;
    f->record = record;
    f->cl     = cl;
    f->data = data;
    
    //added by kh (030816) for pipelined compression
    if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
      if(g_mc_placement_config.is_mc_node(source) && i != 0 && packet_type < Flit::ANY_TYPE) {
	  mem_fetch* mf = (mem_fetch*) f->data;
	  if(mf->get_real_data_size() > 0) {
	    f->raw_data.clear();
	    f->raw_data.resize(32,0);
	    mf->get_real_data_stream(i-1, f->raw_data);	//copy data from mf to flit
	    //printf("Raw_Data = ");
	    //for(int k = f->raw_data.size()-1; k >=0; k--) {
	    //  printf("%02x", f->raw_data[k]);
	    //}
	    //printf("\n");
	  }
      }
    }


    _total_in_flight_flits[f->cl].insert(make_pair(f->id, f));
    if(record) {
      _measured_in_flight_flits[f->cl].insert(make_pair(f->id, f));
    }
    
    if(gTrace){
      cout<<"New Flit "<<f->src<<endl;
    }
    f->type = packet_type;
    
    if ( i == 0 ) { // Head flit
      f->head = true;
      //packets are only generated to nodes smaller or equal to limit
      f->dest = packet_destination;

      //added by kh(062515)
      f->const_dest = packet_destination;
      ///

      //added by kh (030816)
      //dict_ctrl_msg should be considered in a different way
      if(packet_type < Flit::ANY_TYPE) {


	//added by kh(121715)
	if(g_mc_placement_config.is_sm_node(source)) {
	  ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM, _time);

	  //added by kh(042615)
	  mem_fetch* mf = (mem_fetch*)f->data;
	  if(mf->has_merged_req_mfs() == true) {
	    std::vector<mem_fetch*>& merged_mfs = mf->get_merged_req_mfs();
	    for(unsigned j = 0; j < merged_mfs.size(); j++) {
	      merged_mfs[j]->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM, _time);
	    }
	  }
	  ///

	} else if(g_mc_placement_config.is_mc_node(source)) {
	    ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM, _time);
	    //added by kh(122815)
	    //double req_mc_lat = _time - ((mem_fetch*)f->data)->get_timestamp(PACKET_EJECTED_FROM_NI_OUTPUT_BUF_IN_MEM);
	    double req_mc_lat = _time - ((mem_fetch*)f->data)->get_timestamp(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM);
	    if(req_mc_lat < 0 || req_mc_lat > 5000)
	    {
	      std::cout << "_time : " << _time << std::endl;
	      std::cout << "PACKET_EJECTED_FROM_NI_OUTPUT_BUF_IN_MEM : " << ((mem_fetch*)f->data)->get_timestamp(PACKET_EJECTED_FROM_NI_OUTPUT_BUF_IN_MEM) << std::endl;
	    }

	    assert(req_mc_lat>=0 && req_mc_lat <= 5000);
	    g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_MC_LAT, req_mc_lat);
	    ///
	}
	///

	//added by kh(092315)
	//xy-yx hybrid routing
	//if(source >= 0 && source < SM_NUM && (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST)) {
	//added by kh(121715)
	/*
	if(g_mc_placement_config.is_sm_node(source) && (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST)) {
	    if(source%2 == 0) 	f->route_type = Flit::XY_ROUTE;
	    else		f->route_type = Flit::YX_ROUTE;
	}
	*/
	///
      }

    } else {
      f->head = false;
      f->dest = -1;

      //added by kh(062515)
      f->const_dest = packet_destination;
      ///
    }
    switch( _pri_type ) {
      case class_based:
        f->pri = _class_priority[cl];
        assert(f->pri >= 0);
        break;
      case age_based:
        f->pri = numeric_limits<int>::max() - time;
        assert(f->pri >= 0);
        break;
      case sequence_based:
        f->pri = numeric_limits<int>::max() - _packet_seq_no[source];
        assert(f->pri >= 0);
        break;
      default:
        f->pri = 0;
    }
    if ( i == ( size - 1 ) ) { // Tail flit
      f->tail = true;

      //added by kh(070715)
      f->flit_no = packet_size;
      ///

    } else {
      f->tail = false;
    }
    
    f->vc  = -1;
    
    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | "
      << "node" << source << " | "
      << "Enqueuing flit " << f->id
      << " (packet " << f->pid
      << ") at time " << time
      << "." << endl;
    }
    
    _input_queue[subnet][source][cl].push_back( f );
  }
}

void GPUTrafficManager::_Step()
{
  bool flits_in_flight = false;
  for(int c = 0; c < _classes; ++c) {
    flits_in_flight |= !_total_in_flight_flits[c].empty();
  }
  if(flits_in_flight && (_deadlock_timer++ >= _deadlock_warn_timeout)){
    _deadlock_timer = 0;
    cout << "WARNING: Possible network deadlock.\n";
  }
  
  vector<map<int, Flit *> > flits(_subnets);
  
  for ( int subnet = 0; subnet < _subnets; ++subnet ) {
    for ( int n = 0; n < _nodes; ++n ) {
      Flit * const f = _net[subnet]->ReadFlit( n );
      if ( f ) {
        if(f->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Ejecting flit " << f->id
          << " (packet " << f->pid << ")"
          << " from VC " << f->vc
          << "." << endl;
        }

        if(g_hpcl_comp_config.hpcl_comp_en == 0) {
          g_icnt_interface->WriteOutBuffer(subnet, n, f);
          //added by kh(030816)
          g_hpcl_network_anal->add_sample(hpcl_network_anal::NORMAL_FLIT_NO, 1);
          ///

        } else {

	  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	    //added by kh (030816)
	    //(2.1.a) all reply flits (even write reply) are injected to decompressor for maintaining the order of flits.
	    //coming from the same vc
	    //Decompression: WRITE_REPLY should go through decomp, although no data is compressed.
	    //READ_REPLY = {f1,f2,f3,f4,f5}, WRITE_REPLY = {f6}
	    //Problematic case queue: f1,f2,f6,f3,f4,f5.
	    if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY) {	//only sm receives

	      assert(g_hpcl_global_decomp_pl_2B[n]);
	      //inject a flit to decompressor
	      hpcl_comp_pl_data* pl_data = new hpcl_comp_pl_data;
	      pl_data->add_comp_pl_data(f);
	      g_hpcl_global_decomp_pl_2B[n]->set_input_data(pl_data);

	      //added by kh (030316)
	      //set vc info to compressed flits.
	      for(int i = 0; i < f->compressed_other_flits.size(); i++) {
		Flit* cf = f->compressed_other_flits[i];
		assert(cf);
		cf->vc = f->vc;
	      }
	      ///
	      //delete pl_data;

	      //added by kh (030816)
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::NORMAL_FLIT_NO, 1);

	      if(f->compressed_other_flits.size() > 0) {
		g_hpcl_network_anal->add_sample(hpcl_network_anal::READ_REPLY_COMP_FLIT_NO, f->compressed_other_flits.size()+1);
	      } else {
		g_hpcl_network_anal->add_sample(hpcl_network_anal::READ_REPLY_UNCOMP_FLIT_NO, 1);
	      }
	      ///

	    } else if(f->type == Flit::CTRL_MSG) {	//(2.2.a) all ctrl flits are injected to decompressor

	      //std::cout << "f->id " << f->id << " f->src " << f->src << " f->dest " << f->dest << " node " << n << std::endl;
	      hpcl_comp_pl_data* pl_data = new hpcl_comp_pl_data;
	      pl_data->add_comp_pl_data(f);
	      g_hpcl_global_decomp_pl_2B[n]->set_input_data(pl_data);

	      //added by kh (030816)
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::CTRL_FLIT_NO,1);
	      ///

	    } else {

	      g_icnt_interface->WriteOutBuffer(subnet, n, f);

	      //added by kh (030816)
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::NORMAL_FLIT_NO, 1);
	      ///

	    }
	    ///
	  } else {

	    g_icnt_interface->WriteOutBuffer(subnet, n, f);
	    //added by kh(030816)
	    g_hpcl_network_anal->add_sample(hpcl_network_anal::NORMAL_FLIT_NO, 1);
	    ///
	  }
        }
      }
      

      g_icnt_interface->Transfer2BoundaryBuffer(subnet, n);
      Flit* const ejected_flit = g_icnt_interface->GetEjectedFlit(subnet, n);
      if (ejected_flit) {
        if(ejected_flit->head)
          assert(ejected_flit->dest == n);
        if(ejected_flit->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Ejected flit " << ejected_flit->id
          << " (packet " << ejected_flit->pid
          << " VC " << ejected_flit->vc << ")"
          << "from ejection buffer." << endl;
        }
        flits[subnet].insert(make_pair(n, ejected_flit));
        if((_sim_state == warming_up) || (_sim_state == running)) {
          ++_accepted_flits[ejected_flit->cl][n];
          if(ejected_flit->tail) {
            ++_accepted_packets[ejected_flit->cl][n];
          }
        }
      }
    
      // Processing the credit From the network
      Credit * const c = _net[subnet]->ReadCredit( n );
      if ( c ) {
#ifdef TRACK_FLOWS
        for(set<int>::const_iterator iter = c->vc.begin(); iter != c->vc.end(); ++iter) {
          int const vc = *iter;
          assert(!_outstanding_classes[n][subnet][vc].empty());
          int cl = _outstanding_classes[n][subnet][vc].front();
          _outstanding_classes[n][subnet][vc].pop();
          assert(_outstanding_credits[cl][subnet][n] > 0);
          --_outstanding_credits[cl][subnet][n];
        }
#endif
        _buf_states[n][subnet]->ProcessCredit(c);
        c->Free();
      }
    }
    _net[subnet]->ReadInputs( );
  }


  if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE)
  {
    //added by kh (030816)
    //(2.3) Decompressor
    for(int i = 0; i < g_hpcl_global_decomp_pl_2B.size(); i++)
    {
      if(!g_hpcl_global_decomp_pl_2B[i])	continue;

      g_hpcl_global_decomp_pl_2B[i]->run(getTime()+getTotalTime());
      hpcl_comp_pl_data* comp_pl_data = g_hpcl_global_decomp_pl_2B[i]->get_output_data();
      Flit* flit = comp_pl_data->get_flit_ptr();
      if(flit) {
	int subnet = 0;
	assert(flit->const_dest == i);
	//std::cout << "Flit " << flit->id << " type " << flit->type << " is out from decomp" << std::endl;
	/*
	std::cout << GetSimTime() << "| MYEJECT | node " << i << " Flit " << flit->id << " type " << flit->type << " tail " << flit->tail << " vc " << flit->vc;
	if(flit->type == Flit::ANY_TYPE)	std::cout << " ctrl " << std::endl;
	else				std::cout << " mf " << ((mem_fetch*)flit->data)->get_request_uid() << std::endl;
	*/

	//std::cout << "Flit " << flit->id << " enc_status " << flit->m_enc_status;
	#ifdef CORRECTNESS_CHECK
	if(flit->m_enc_status == 1) {
	  std::cout << "dec_status " << flit->check_correctness() << std::endl;
	  if(flit->check_correctness() == false) {

	    printf("flit %d has incorrect decoding!\n", flit->id);

	    printf("\traw_data = ");
	    for(int k = flit->raw_data.size()-1; k >=0; k--) {
		    printf("%02x", flit->raw_data[k]);
	    }
	    printf("\n");

	    printf("\tdec_data = ");
	    for(int k = flit->decomp_data.size()-1; k >=0; k--) {
		    printf("%02x", flit->decomp_data[k]);
	    }
	    printf("\n");
	  }
	}
	#endif

	g_icnt_interface->WriteOutBuffer(subnet, i, flit);
      }

      //reset output data
      g_hpcl_global_decomp_pl_2B[i]->reset_output_data();
    }
    ///


    //added by kh (030816)
    //(1.2.c) generate/inject ctrl flit is injected to NI injection buffer
    for(int i = 0; i < g_hpcl_global_comp_pl_2B.size(); i++)
    {
      if(!g_hpcl_global_comp_pl_2B[i])	continue;
      if(g_hpcl_global_comp_pl_2B[i]->has_out_dict_ctrl_msg() == false)	continue;
      if(g_hpcl_global_comp_pl_2B[i]->get_state() == hpcl_comp_pl<unsigned short>::HPCL_COMP_RUN)	continue;

      hpcl_dict_ctrl_msg<unsigned short>* ctrl_msg = g_hpcl_global_comp_pl_2B[i]->front_out_dict_ctrl_msg();
      int packet_size = 1;	//1 flit
      Flit::FlitType packet_type = Flit::CTRL_MSG;
      void* data = NULL;
      assert(ctrl_msg->get_dest_node()>=0);
      int subnet = 0;
      int response_size = 32; 	//32bytes

      //send ctrl from MC
      int sender_device_id = -1;
      if(g_mc_placement_config.is_mc_node(i)) {
	sender_device_id = g_mc_placement_config.get_mc_node_index(i) + 56;	//56 SMs and 8 MCs
      }
      assert(sender_device_id >= 0);

      int sender_node_id = i;
      int receiver_node_id = ctrl_msg->get_dest_node();
      int receiver_device_id = g_mc_placement_config.get_sm_node_index(receiver_node_id);

      assert(sender_node_id>=0);
      assert(receiver_node_id>=0);
      if(g_icnt_interface->HasBuffer(sender_device_id, response_size)) {
	//std::cout << getTime()+getTotalTime() << " | " << " PUSH2 | " << " input " << sender_device_id << " output " << receiver_device_id << " ctrl_msg " << ctrl_msg->get_id();
	//std::cout << std::endl;
	_GeneratePacket(sender_node_id, -1, 0, getTime(), subnet, packet_size, packet_type, ctrl_msg, receiver_node_id);
	g_hpcl_global_comp_pl_2B[i]->pop_out_dict_ctrl_msg();

  //      std::cout << "ctrl_msg | id " << ctrl_msg->get_id();
  //      std::cout << " | src_node " << ctrl_msg->get_src_node();
  //      std::cout << " | dest_node " << ctrl_msg->get_dest_node();
  //      std::cout << " | word_index " << ctrl_msg->get_word_index();
  //      std::cout << " | word " << ctrl_msg->get_new_word() << std::endl;;
  //
  //      g_hpcl_global_comp_pl_2B[i]->validate_word(ctrl_msg->get_dest_node(), ctrl_msg->get_word_index(), ctrl_msg->get_new_word());
      }
    }
    ///

  }

// GPGPUSim will generate/inject packets from interconnection interface
#if 0
  if ( !_empty_network ) {
    _Inject();
  }
#endif
  
  for(int subnet = 0; subnet < _subnets; ++subnet) {
    
    for(int n = 0; n < _nodes; ++n) {
      
      Flit * f = NULL;
      
      BufferState * const dest_buf = _buf_states[n][subnet];
      
      int const last_class = _last_class[n][subnet];
      
      int class_limit = _classes;
      
      if(_hold_switch_for_packet) {
        list<Flit *> const & pp = _input_queue[subnet][n][last_class];
        if(!pp.empty() && !pp.front()->head &&
           !dest_buf->IsFullFor(pp.front()->vc)) {
          f = pp.front();
          assert(f->vc == _last_vc[n][subnet][last_class]);
          
          // if we're holding the connection, we don't need to check that class
          // again in the for loop
          --class_limit;
        }
      }
      
      for(int i = 1; i <= class_limit; ++i) {
        
        int const c = (last_class + i) % _classes;
        
        list<Flit *> const & pp = _input_queue[subnet][n][c];
        
        if(pp.empty()) {
          continue;
        }
        
        Flit * const cf = pp.front();
        assert(cf);
        assert(cf->cl == c);
        assert(cf->subnetwork == subnet);

        if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	  //added by kh (030816)
	  //If compressor is busy with compressing flits of a packet,
	  //don't inject a head flit of a new packet to the busy compressor
	  //compressor is only for read reply, but write reply goes through the compressor to avoid intermixed flits from different packets
	  //Thus, the following filtering is not restricted to read reply, thus we have no flit type checking to read reply.
	  if(g_mc_placement_config.is_mc_node(n) == true && cf->head == true) {
	    if(g_hpcl_global_comp_pl_2B[n] && g_hpcl_global_comp_pl_2B[n]->get_state() == hpcl_comp_pl<unsigned short>::HPCL_COMP_RUN) {
	      f = NULL;
	      break;
	    }
	  }
	  ///
        }

        if(f && (f->pri >= cf->pri)) {
          continue;
        }
        
        //KH: routing output port is decided here.
        if(cf->head && cf->vc == -1) { // Find first available VC
          
          //std::cout << "_Step is called!!!!" << std::endl;
          //assert(1 == 0);

          OutputSet route_set;
          _rf(NULL, cf, -1, &route_set, true);
          set<OutputSet::sSetElement> const & os = route_set.GetSet();
          assert(os.size() == 1);
          OutputSet::sSetElement const & se = *os.begin();
          assert(se.output_port == -1);
          int vc_start = se.vc_start;
          int vc_end = se.vc_end;
          int vc_count = vc_end - vc_start + 1;
          if(_noq) {
            assert(_lookahead_routing);
            const FlitChannel * inject = _net[subnet]->GetInject(n);
            const Router * router = inject->GetSink();
            assert(router);
            int in_channel = inject->GetSinkPort();
            

            // NOTE: Because the lookahead is not for injection, but for the
            // first hop, we have to temporarily set cf's VC to be non-negative
            // in order to avoid seting of an assertion in the routing function.
            cf->vc = vc_start;
            _rf(router, cf, in_channel, &cf->la_route_set, false);
            cf->vc = -1;
            
            if(cf->watch) {
              *gWatchOut << GetSimTime() << " | "
              << "node" << n << " | "
              << "Generating lookahead routing info for flit " << cf->id
              << " (NOQ)." << endl;
            }
            set<OutputSet::sSetElement> const sl = cf->la_route_set.GetSet();
            assert(sl.size() == 1);
            int next_output = sl.begin()->output_port;
            vc_count /= router->NumOutputs();
            vc_start += next_output * vc_count;
            vc_end = vc_start + vc_count - 1;
            assert(vc_start >= se.vc_start && vc_start <= se.vc_end);
            assert(vc_end >= se.vc_start && vc_end <= se.vc_end);
            assert(vc_start <= vc_end);
          }
          if(cf->watch) {
            *gWatchOut << GetSimTime() << " | " << FullName() << " | "
            << "Finding output VC for flit " << cf->id
            << ":" << endl;
          }
          for(int i = 1; i <= vc_count; ++i) {
            int const lvc = _last_vc[n][subnet][c];
            int const vc =
            (lvc < vc_start || lvc > vc_end) ?
            vc_start :
            (vc_start + (lvc - vc_start + i) % vc_count);
            assert((vc >= vc_start) && (vc <= vc_end));
            if(!dest_buf->IsAvailableFor(vc)) {
              if(cf->watch) {
                *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                << "  Output VC " << vc << " is busy." << endl;
              }
            } else {
              if(dest_buf->IsFullFor(vc)) {
                if(cf->watch) {
                  *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                  << "  Output VC " << vc << " is full." << endl;
                }
              } else {
                if(cf->watch) {
                  *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                  << "  Selected output VC " << vc << "." << endl;
                }
                cf->vc = vc;
                break;
              }
            }
          }
        }
        
        if(cf->vc == -1) {
          if(cf->watch) {
            *gWatchOut << GetSimTime() << " | " << FullName() << " | "
            << "No output VC found for flit " << cf->id
            << "." << endl;
          }
        } else {
          if(dest_buf->IsFullFor(cf->vc)) {
            if(cf->watch) {
              *gWatchOut << GetSimTime() << " | " << FullName() << " | "
              << "Selected output VC " << cf->vc
              << " is full for flit " << cf->id
              << "." << endl;
            }
          } else {
            f = cf;
          }
        }
      }
      
      if(f) {
        
        assert(f->subnetwork == subnet);
        
        int const c = f->cl;
        
        if(f->head) {
          
          if (_lookahead_routing) {
            if(!_noq) {
              const FlitChannel * inject = _net[subnet]->GetInject(n);
              const Router * router = inject->GetSink();
              assert(router);
              int in_channel = inject->GetSinkPort();
              _rf(router, f, in_channel, &f->la_route_set, false);
              if(f->watch) {
                *gWatchOut << GetSimTime() << " | "
                << "node" << n << " | "
                << "Generating lookahead routing info for flit " << f->id
                << "." << endl;
              }
            } else if(f->watch) {
              *gWatchOut << GetSimTime() << " | "
              << "node" << n << " | "
              << "Already generated lookahead routing info for flit " << f->id
              << " (NOQ)." << endl;
            }
          } else {
            f->la_route_set.Clear();
          }
          
          dest_buf->TakeBuffer(f->vc);
          _last_vc[n][subnet][c] = f->vc;
        }
        
        _last_class[n][subnet] = c;
        
        _input_queue[subnet][n][c].pop_front();
        
#ifdef TRACK_FLOWS
        ++_outstanding_credits[c][subnet][n];
        _outstanding_classes[n][subnet][f->vc].push(c);
#endif
        

        if(g_hpcl_comp_config.hpcl_comp_en == 0) {
	  dest_buf->SendingFlit(f);
        } else {

	  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	    //added by kh(030816)
	    //(1.1.a) send a head flit first: update buffer state
	    if(g_mc_placement_config.is_mc_node(n) == true && f->type == Flit::READ_REPLY) {
		if(f->head)	dest_buf->SendingFlit(f);
		//skip for other flits
	    } else {
		dest_buf->SendingFlit(f);
	    }
	    ///
	  } else {
	    dest_buf->SendingFlit(f);
	  }

        }

        if(_pri_type == network_age_based) {
          f->pri = numeric_limits<int>::max() - _time;
          assert(f->pri >= 0);
        }
        
        if(f->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Injecting flit " << f->id
          << " into subnet " << subnet
          << " at time " << _time
          << " with priority " << f->pri
          << "." << endl;
        }
        f->itime = _time;
        
        // Pass VC "back"
        if(!_input_queue[subnet][n][c].empty() && !f->tail) {
          Flit * const nf = _input_queue[subnet][n][c].front();
          nf->vc = f->vc;
        }
        
        if((_sim_state == warming_up) || (_sim_state == running)) {
          ++_sent_flits[c][n];

          //added by kh(070215)
          ++_sent_flits_net[subnet][n];


          if(f->head) {
            ++_sent_packets[c][n];

            //added by kh(030816)
            if(f->type < Flit::ANY_TYPE) {
	      //added by kh(121715)
	      if(g_mc_placement_config.is_sm_node(n)) {
		  ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM, _time);

		  //added by kh(122815)
		  double req_queuing_lat_in_sm = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_SM);
		  g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_QUEUING_LAT_IN_SM, req_queuing_lat_in_sm);
		  ///

		  //added by kh(042615)
		  mem_fetch* mf = (mem_fetch*)f->data;
		  if(mf->has_merged_req_mfs() == true) {
		    std::vector<mem_fetch*>& merged_mfs = mf->get_merged_req_mfs();
		    for(unsigned j = 0; j < merged_mfs.size(); j++) {
		      merged_mfs[j]->set_status(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM, _time);
		    }
		  }
		  ///

	      } else if(g_mc_placement_config.is_mc_node(n)) {
		  ((mem_fetch*)f->data)->set_status(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM, _time);
		  //added by kh(122815)
		  double rep_queuing_lat_in_mc = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_NI_INPUT_BUF_IN_MEM);
		  g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_QUEUING_LAT_IN_MC, rep_queuing_lat_in_mc);
		  ///
	      }
	      ///
            }
          }
        }
        
#ifdef TRACK_FLOWS
        ++_injected_flits[c][n];
#endif
        
        if(g_hpcl_comp_config.hpcl_comp_en == 0) {

	  _net[subnet]->WriteFlit(f, n);

        } else {

	  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	    //added by kh (021416)
	    //Compression
	    //(1.1.b) flits but a head flit are injected to the compressor
	    if(g_mc_placement_config.is_mc_node(n) == true && f->type == Flit::READ_REPLY) {

	       hpcl_comp_pl_data* pl_data = NULL;

	       //A head flit is found. It means other flits are in _input_queue now.
	       //flits move from _input_queue to compressor by force.
	       //Those flits are sent to network in different paths such that we set info for
	       //flit navigation here.
	       if(f->head == true) {
		  assert(!_input_queue[subnet][n][c].empty());
		  Flit * body_flit = _input_queue[subnet][n][c].front();
		  _input_queue[subnet][n][c].pop_front();

		  if(_pri_type == network_age_based) {
		    body_flit->pri = numeric_limits<int>::max() - _time;
		    assert(body_flit->pri >= 0);
		  }
		  body_flit->itime = _time;

		  // Pass VC "back" for next flit
		  if(!_input_queue[subnet][n][c].empty() && !body_flit->tail) {
		    Flit * const nf = _input_queue[subnet][n][c].front();
		    nf->vc = body_flit->vc;
		  }

		  ++_sent_flits[c][n];
		  ++_sent_flits_net[subnet][n];

		  pl_data = new hpcl_comp_pl_data;
		  pl_data->add_comp_pl_data(body_flit);
		  g_hpcl_global_comp_pl_2B[n]->set_input_data(pl_data);
		  //delete pl_data;

		  //(1.1.a)	send a head flit first
		  _net[subnet]->WriteFlit(f, n);

	      } else {

		  pl_data = new hpcl_comp_pl_data;
		  pl_data->add_comp_pl_data(f);
		  g_hpcl_global_comp_pl_2B[n]->set_input_data(pl_data);
		  //delete pl_data;
	      }


	      //(1.2.a) words in a flit are pushed to a queue in a compressor
	      if(pl_data) {

		  Flit* flit = pl_data->get_flit_ptr();
		  assert(flit);

		  //Use receiver's id as global dict index
		  int global_dict_index = flit->const_dest;

		  //Push words to be updated to word_queue (this is done at zero-time)
		  vector<unsigned short> word_list;
		  g_hpcl_global_comp_pl_2B[n]->decompose(flit->raw_data, word_list);
		  for(int i = 0; i < word_list.size(); i++)
		    g_hpcl_global_comp_pl_2B[n]->push_word_to_queue(word_list[i], global_dict_index);

		  delete pl_data;
	      }

	    } else if (g_mc_placement_config.is_mc_node(n) == true && f->type == Flit::CTRL_MSG) {


		hpcl_dict_ctrl_msg<unsigned short>* ctrl_msg = (hpcl_dict_ctrl_msg<unsigned short>*)f->data;

		/*
		std::cout << "ctrl_msg | id " << ctrl_msg->get_id();
		std::cout << " | src_node " << ctrl_msg->get_src_node();
		std::cout << " | dest_node " << ctrl_msg->get_dest_node();
		std::cout << " | word_index " << ctrl_msg->get_word_index();
		std::cout << " | word " << ctrl_msg->get_new_word();
		std::cout << " | n " << n;
		std::cout << std::endl;;
		*/

		g_hpcl_global_comp_pl_2B[n]->release_word(ctrl_msg->get_dest_node(), ctrl_msg->get_word_index(), ctrl_msg->get_new_word());
		g_hpcl_global_comp_pl_2B[n]->validate_word(ctrl_msg->get_dest_node(), ctrl_msg->get_word_index(), ctrl_msg->get_new_word());

		_net[subnet]->WriteFlit(f, n);

	    } else {

		_net[subnet]->WriteFlit(f, n);
	    }
	    ///

	  } else {

	    _net[subnet]->WriteFlit(f, n);

	  }
        }
      }
    }
  }

  if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {

    //added by kh(030816)
    //(1.3) Compression: Pipelined Compression
    for(int i = 0; i < g_hpcl_global_comp_pl_2B.size(); i++)
    {
      if(!g_hpcl_global_comp_pl_2B[i])	continue;

      //(1.2.b) words in the queue are updated to dictionary, when idle state
      //(1.2.d) ctrl msgs for words with frequency (>= threshold) are generated in compressor's run function, when idle state
      g_hpcl_global_comp_pl_2B[i]->run(getTime()+getTotalTime());
      hpcl_comp_pl_data* comp_pl_rsl = g_hpcl_global_comp_pl_2B[i]->get_output_data();
      Flit* flit = comp_pl_rsl->get_flit_ptr();
      if(flit) {
	//std::cout << "node " << i << " flit->id " << flit->id << std::endl;

	//(1.1.c) flits are injected from the compressor to an input buffer
	if(HasBufferSpace(i, flit)) {
	  SendFlitToBuffer(i, flit);		//for credit management.
	  SendFlitToNetwork(i, flit);
	  //std::cout << "\t" << " send flit " << flit->id << std::endl;

	  if(g_hpcl_global_comp_pl_2B[i]->get_comp_done() == true)
	  {
	      assert(g_hpcl_global_comp_pl_2B[i]->get_state() == hpcl_comp_pl<unsigned short>::HPCL_COMP_RUN);
	      g_hpcl_global_comp_pl_2B[i]->set_state(hpcl_comp_pl<unsigned short>::HPCL_COMP_IDLE);
	      g_hpcl_global_comp_pl_2B[i]->reset_comp_done();
	  }

	  //reset output data
	  g_hpcl_global_comp_pl_2B[i]->reset_output_data();

	} else {
	  //std::cout << "\t" << " cannot send flit " << flit->id << " due to no space!!!!!!!!!!" << std::endl;
	}
      }
    }
    ///

  }

  //Send the credit To the network
  for(int subnet = 0; subnet < _subnets; ++subnet) {
    for(int n = 0; n < _nodes; ++n) {
      map<int, Flit *>::const_iterator iter = flits[subnet].find(n);
      if(iter != flits[subnet].end()) {
        Flit * const f = iter->second;

        f->atime = _time;

        //added by kh(070715)
        if(f->tail) {

          //added by kh(030816)
	  if(f->type < Flit::ANY_TYPE) {
	    //added by kh(121715)
	    if(g_mc_placement_config.is_sm_node(n)) {
	      //std::cout << "mf's id: " << ((mem_fetch*)f->data)->get_request_uid() << ", time: " << _time << std::endl;
	      ((mem_fetch*)f->data)->set_status(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_SM, _time);
	      //added by kh(122815)
	      double rep_net_lat = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_MEM);
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::REP_NET_LAT, rep_net_lat);
	      ///

	    } else if(g_mc_placement_config.is_mc_node(n)) {
	      ((mem_fetch*)f->data)->set_status(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM, _time);

	      //added by kh(122815)
	      double req_net_lat = _time - ((mem_fetch*)f->data)->get_timestamp(HEAD_FLIT_INJECTED_TO_INJECT_BUF_IN_SM);
	      g_hpcl_network_anal->add_sample(hpcl_network_anal::REQ_NET_LAT, req_net_lat);
	      ///

	      //added by kh(042615)
	      mem_fetch* mf = (mem_fetch*)f->data;
	      if(mf->has_merged_req_mfs() == true) {
		std::vector<mem_fetch*>& merged_mfs = mf->get_merged_req_mfs();
		for(unsigned j = 0; j < merged_mfs.size(); j++) {
		  merged_mfs[j]->set_status(TAIL_FLIT_EJECTED_FROM_EJECT_BUF_IN_MEM, _time);
		}
	      }
	      ///

	    }
          }
        }
        ///

        if(f->watch) {
          *gWatchOut << GetSimTime() << " | "
          << "node" << n << " | "
          << "Injecting credit for VC " << f->vc
          << " into subnet " << subnet
          << "." << endl;
        }
        

        if(g_hpcl_comp_config.hpcl_comp_en == 0) {

          Credit * const c = Credit::New();
	  c->vc.insert(f->vc);
	  _net[subnet]->WriteCredit(c, n);

        } else {

	  if(g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
	    //added by kh(030816)
	    if(f->type == Flit::READ_REPLY) {
		if(f->tail == false && f->m_enc_status == 1) {

		} else {
		  Credit * const c = Credit::New();
		  c->vc.insert(f->vc);
		  _net[subnet]->WriteCredit(c, n);
		  //std::cout << "\tf->vc " << f->vc << " f->id " << f->id << std::endl;
		}

	    } else {
	      Credit * const c = Credit::New();
	      c->vc.insert(f->vc);
	      _net[subnet]->WriteCredit(c, n);
	    }
	    ///
	  } else {

	    Credit * const c = Credit::New();
	    c->vc.insert(f->vc);
	    _net[subnet]->WriteCredit(c, n);

	  }
        }





#ifdef TRACK_FLOWS
        ++_ejected_flits[f->cl][n];
#endif
        
        _RetireFlit(f, n);
      }
    }
    flits[subnet].clear();
    // _InteralStep here
    _net[subnet]->Evaluate( );					//run router's pipeline
    _net[subnet]->WriteOutputs( );				//send flits..
  }
  
  ++_time;
  assert(_time);
  if(gTrace){
    cout<<"TIME "<<_time<<endl;
  }
  
}


//added by kh(030816)
void GPUTrafficManager::SendFlitToNetwork(int node_id, Flit* f)
{
//  std::cout << GetSimTime() << " Node " << node_id << ", Flit " << f->id << " head " << f->head << " tail " << f->tail;
//  std::cout << " Write!!!" << std::endl;

  _net[f->subnetwork]->WriteFlit(f, node_id);
}

bool GPUTrafficManager::HasBufferSpace(int node_id, Flit* f)
{
  BufferState * const dest_buf = _buf_states[node_id][f->subnetwork];
  if(dest_buf->IsFullFor(f->vc)) {
      return false;
  } else {
      return true;
  }
}

void GPUTrafficManager::SendFlitToBuffer(int node_id, Flit* f)
{
  BufferState * const dest_buf = _buf_states[node_id][f->subnetwork];
  dest_buf->SendingFlit(f);
}
///
