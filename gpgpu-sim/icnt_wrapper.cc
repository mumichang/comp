// Copyright (c) 2009-2011, Tor M. Aamodt, Wilson W.L. Fung, Ali Bakhoda
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

#include "icnt_wrapper.h"
#include <assert.h>
#include "../intersim2/globals.hpp"
#include "../intersim2/interconnect_interface.hpp"

icnt_create_p                icnt_create;
icnt_init_p                  icnt_init;
icnt_has_buffer_p            icnt_has_buffer;
icnt_push_p                  icnt_push;
icnt_pop_p                   icnt_pop;
icnt_transfer_p              icnt_transfer;
icnt_busy_p                  icnt_busy;
icnt_display_stats_p         icnt_display_stats;
icnt_display_overall_stats_p icnt_display_overall_stats;
icnt_display_state_p         icnt_display_state;
icnt_get_flit_size_p         icnt_get_flit_size;


//added by kh(120815)
icnt_top_for_buffering_p     icnt_top_for_buffering;
//added by kh(042516)
icnt_get_buffer_occupancy_p  icnt_get_buffer_occupancy;

int   g_network_mode;
char* g_network_config_filename;

#include "../option_parser.h"

//added by kh(10/30/14)
//#include "mem_fetch.h"
#include "user-defined.h"
perf_icnt_push_p       perf_icnt_push;
perf_icnt_pop_p        perf_icnt_pop;
//#define PERF_ICNT
///

//added by kh(053015)
#include "gpu-cache.h"
extern l2_cache_config g_L2_config;
#include <sstream>

//added by kh(061215)
std::vector<mem_fetch_debug*> g_dbg_msg_vector;
std::map<unsigned,mem_fetch_debug*> g_dbg_msg_map;

// Wrapper to intersim2 to accompany old icnt_wrapper
// TODO: use delegate/boost/c++11<funtion> instead


//added by kh(030816)
extern struct mc_placement_config g_mc_placement_config;
#include "../gpgpu-sim/hpcl_comp.h"
#include "../gpgpu-sim/hpcl_comp_config.h"
extern hpcl_comp* g_hpcl_comp;
extern hpcl_comp_config g_hpcl_comp_config;
///

//added by kh(050216)
#include "../gpgpu-sim/hpcl_approx_anal.h"
extern hpcl_approx_anal* g_hpcl_approx_anal;
///



static void intersim2_create(unsigned int n_shader, unsigned int n_mem)
{
   g_icnt_interface->CreateInterconnect(n_shader, n_mem);
}

static void intersim2_init()
{
   g_icnt_interface->Init();
}

static bool intersim2_has_buffer(unsigned input, unsigned int size)
{
   return g_icnt_interface->HasBuffer(input, size);
}

static void intersim2_push(unsigned input, unsigned output, void* data, unsigned int size)
{
   g_icnt_interface->Push(input, output, data, size);
}

static void* intersim2_pop(unsigned output)
{
   return g_icnt_interface->Pop(output);
}

static void intersim2_transfer()
{
   g_icnt_interface->Advance();
}

static bool intersim2_busy()
{
   return g_icnt_interface->Busy();
}

static void intersim2_display_stats()
{
   g_icnt_interface->DisplayStats();
}

static void intersim2_display_overall_stats()
{
   g_icnt_interface->DisplayOverallStats();
}

static void intersim2_display_state(FILE *fp)
{
   g_icnt_interface->DisplayState(fp);
}

static unsigned intersim2_get_flit_size()
{
   return g_icnt_interface->GetFlitSize();
}


//added by kh(120815)
static void* intersim2_top_for_buffering(unsigned output)
{
   return g_icnt_interface->Top_for_buffering(output);
}
///

//added by kh(042515)
static unsigned intersim2_get_buffer_occupancy(unsigned input)
{
   return g_icnt_interface->GetOccupancy(input);
}
///



void icnt_reg_options( class OptionParser * opp )
{
   option_parser_register(opp, "-network_mode", OPT_INT32, &g_network_mode, "Interconnection network mode", "1");
   option_parser_register(opp, "-inter_config_file", OPT_CSTR, &g_network_config_filename, "Interconnection network config file", "mesh");
}

/*
//added by kh(10/30/14)
// data structure for global buffer
struct PERF_DATA {

	PERF_DATA(unsigned _input_id, unsigned _output_id, void* _data, unsigned int _size)
	{
		input_id = _input_id;
		output_id = _output_id;
		data = _data;
		size = _size;
	}

	unsigned input_id;
	unsigned output_id;
	void* data;
	unsigned int size;
};

vector<PERF_DATA> g_perf_icnt_buffer;
*/

static void perf_push(unsigned input, unsigned output, void* data, unsigned int size, perf_packet_type type, unsigned long long cycle, unsigned long long cycle2)
{
  unsigned packetID = ((mem_fetch*)data)->get_request_uid();
  new_addr_type addr = ((mem_fetch*)data)->get_addr();
  new_addr_type partition_addr = ((mem_fetch*)data)->get_partition_addr();

  //added by kh(030816)
  //if(g_hpcl_comp_config.hpcl_comp_en == 1) {
  //added by kh(042116)
  if(g_hpcl_comp_config.hpcl_comp_en == 1 && g_hpcl_comp_config.hpcl_comp_algo == hpcl_comp_config::GLOBAL_PRIVATE) {
    //save data into mem_fetch object
    mem_fetch* mf = (mem_fetch*)data;
    unsigned node_id = g_icnt_interface->get_node_id(input);
    if(g_mc_placement_config.is_mc_node(node_id) == true) {
      unsigned char* real_data = mf->config_real_data(mf->get_data_size());
      g_hpcl_comp->get_cache_data(mf, real_data);
//      printf("Data = ");
//      for(int i = mf->get_data_size()-1; i >= 0 ; i--) {
//      	printf("%02x", mf->get_real_data(i));
//      }
//      printf("\n");
//
//      printf("read_data_size = %u\n", mf->get_real_data_size());
    }
  }

  #ifdef ICNT_DEBUG
  std::cout << cycle << " | " << " PUSH | " << " input " << input << " output " << output << " mf " << packetID;
  std::cout << " subpartition " << ((mem_fetch*)data)->get_sub_partition_id();
  std::cout << std::endl;

  mem_fetch* tmp = (mem_fetch*)data;
  if(tmp->has_merged_mem_fetch()) {
    std::vector<mem_fetch*>& merged_mfs = tmp->get_merged_mfs();
    for(unsigned i = 0; i < merged_mfs.size(); i++) {
	std::cout << cycle << " | " << " PUSH2 | " << " input " << input << " output " << output << " mf " << merged_mfs[i]->get_request_uid();
	std::cout << std::endl;
    }
  }
  #endif



  ::icnt_push(input,output,data,size);

  ((mem_fetch*)data)->m_time_stamp.push_back(cycle2);
  ((mem_fetch*)data)->m_data_size_hist.push_back(size);

  if(((mem_fetch*)data)->m_time_stamp.size() == 1) {
	  ((mem_fetch*)data)->m_MC_id = output;
	  ((mem_fetch*)data)->m_blk_addr = g_L2_config.block_addr(((mem_fetch*)data)->get_addr());
  }
}

static void* perf_pop(unsigned output, perf_packet_type type, unsigned long long cycle, unsigned long long cycle2)
{
  void* ret = ::icnt_pop(output);
  if(ret != NULL)
  {
    unsigned packetID = ((mem_fetch*)ret)->get_request_uid();
    //unsigned long long cycle = gpu_sim_cycle;
    //std::cout << "[Intersim] Type: " << type << " Time: " << cycle << " Time2: " << cycle2 << " PacketID: " << packetID << std::endl;

    #ifdef ICNT_DEBUG
    std::cout << cycle << " | " << " POP | " << " output " << output << " mf " << ((mem_fetch*)ret)->get_request_uid();
    std::cout << " subpartition " << ((mem_fetch*)ret)->get_sub_partition_id();
    std::cout << std::endl;
    mem_fetch* tmp = (mem_fetch*)ret;
    if(tmp->has_merged_mem_fetch()) {
      std::vector<mem_fetch*>& merged_mfs = tmp->get_merged_mfs();
      for(unsigned i = 0; i < merged_mfs.size(); i++) {
	std::cout << cycle << " | " << " POP2 | " << " output " << output << " mf " << merged_mfs[i]->get_request_uid();
	std::cout << std::endl;
      }
    }
    ///
    #endif


    mem_fetch* data = (mem_fetch*)ret;
    data->m_time_stamp.push_back(cycle2);

    //Sort the debug msg in the order of arriving in MC
    if(data->m_time_stamp.size() == 2) {

      mem_fetch_debug* mfd = mem_fetch_debug::create(data);
      g_dbg_msg_vector.push_back(mfd);
      g_dbg_msg_map.insert(std::pair<unsigned,mem_fetch_debug*>(mfd->m_request_uid,mfd));

    }

    //print a debug msg in the sorted order
    if(data->m_time_stamp.size() == 4) {

      //update size and time_stamp for mem_fetch_debug
      std::map<unsigned,mem_fetch_debug*>::iterator it;
      mem_fetch_debug* mfd = NULL;
      it = g_dbg_msg_map.find(data->get_request_uid());
      if (it != g_dbg_msg_map.end())	{
	it->second->m_time_stamp = data->m_time_stamp;
	it->second->m_data_size_hist = data->m_data_size_hist;
	mfd = it->second;
      } else {
	assert(1==0);
      }
      ///

      int last_index = -1;
      for (int i = 0; i < g_dbg_msg_vector.size(); i++)	{

	if (g_dbg_msg_vector[i]->m_time_stamp.size() == 4)
	{
	  mem_fetch_debug::print_debug_msg(g_dbg_msg_vector[i]);
	  last_index = i;
	  g_dbg_msg_map.erase(g_dbg_msg_vector[i]->m_request_uid);
	} else {
	  break;
	}
      }

      if(last_index >= 0) {
	//std::cout << "last_index: " << last_index << std::endl;
	//std::cout << "before: g_dbg_msg_vector.size(): " << g_dbg_msg_vector.size() << std::endl;
	g_dbg_msg_vector.erase(g_dbg_msg_vector.begin(), g_dbg_msg_vector.begin()+last_index+1);
	//std::cout << "after: g_dbg_msg_vector.size(): " << g_dbg_msg_vector.size() << std::endl;
      }

    }
    ///


    //added by kh(050216)
    if(g_hpcl_approx_anal) {

      mem_fetch* mf = (mem_fetch*)ret;
      if(mf && mf->get_type() == READ_REQUEST) {
	new_addr_type cache_blk_addr = g_hpcl_comp->get_cache_block_addr(mf->get_addr());
	g_hpcl_approx_anal->add_sample(hpcl_approx_anal::L1_CACHE_MISS, cache_blk_addr, mf->get_pc(), mf->get_access_type());
	printf("LD - PC: %u, Type: %d Mem_Addr: %08x CB_Addr: %08x SM : %u\n", mf->get_pc(), mf->get_access_type(), mf->get_addr(), cache_blk_addr, mf->get_tpc());
      }

    }










  }
  return ret;
}
///


void icnt_wrapper_init()
{
   switch (g_network_mode) {
      case INTERSIM:
         //FIXME: delete the object: may add icnt_done wrapper
         g_icnt_interface = InterconnectInterface::New(g_network_config_filename);
         icnt_create     = intersim2_create;
         icnt_init       = intersim2_init;
         icnt_has_buffer = intersim2_has_buffer;
         icnt_push       = intersim2_push;
         icnt_pop        = intersim2_pop;
         icnt_transfer   = intersim2_transfer;
         icnt_busy       = intersim2_busy;
         icnt_display_stats = intersim2_display_stats;
         icnt_display_overall_stats = intersim2_display_overall_stats;
         icnt_display_state = intersim2_display_state;
         icnt_get_flit_size = intersim2_get_flit_size;

         //added by kh(10/30/14)
         perf_icnt_push = perf_push;
         perf_icnt_pop = perf_pop;
         ///

         //added by kh(120815)
         icnt_top_for_buffering = intersim2_top_for_buffering;
         ///

         //added by kh(042516)
         icnt_get_buffer_occupancy = intersim2_get_buffer_occupancy;

         break;
      default:
         assert(0);
         break;
   }
}
