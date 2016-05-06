/*
 * user-defined.h
 *
 *  Created on: Nov 19, 2014
 *      Author: mumichang
 */

#ifndef GPGPU_SIM_USER_DEFINED_H_
#define GPGPU_SIM_USER_DEFINED_H_

//added by kh(11/19/14)
//#define MULTICAST
#include "mem_fetch.h"

//added by kh(070715)
#define SM_NUM 56
#define MC_NUM 8

//#define PERF_ICNT				//Enable perfect network

//added by kh(061215)
//#include "gpu-cache.h"
#include <map>
#include <sstream>
#include <set>
#include <algorithm>

struct mem_fetch_debug {

   unsigned m_request_uid;
   unsigned m_sid;
   unsigned m_tpc;
   unsigned m_wid;
   unsigned m_mc;
   new_addr_type m_addr;
   new_addr_type m_blk_addr;
   address_type m_pc;
   unsigned m_read;		//1:read, 0:write
   //unsigned long long m_req_size;
   //unsigned long long m_rep_size;
   std::string m_byte_mask_str;
   std::vector<unsigned long long> m_time_stamp;
   std::vector<unsigned long long> m_data_size_hist;

   static mem_fetch_debug* create(mem_fetch* data)
   {
	   mem_fetch_debug* mfd = new mem_fetch_debug;

	   mfd->m_request_uid = data->get_request_uid();
	   mfd->m_sid = data->get_sid();
	   mfd->m_tpc = data->get_tpc();
	   mfd->m_wid = data->get_wid();
	   mfd->m_mc = data->m_MC_id;
	   mfd->m_addr = data->get_addr();
	   mfd->m_blk_addr = data->m_blk_addr;
	   mfd->m_pc = data->get_pc();
	   mfd->m_read = !data->is_write();
	   //mfd->m_time_stamp = data->m_time_stamp;
	   //mfd->m_req_size = data->m_data_size_hist[0];
	   //mfd->m_rep_size = data->m_data_size_hist[1];

	   std::ostringstream s;
	   s << data->get_access_byte_mask();
	   mfd->m_byte_mask_str = s.str();

	   return mfd;
   }


   static void print_debug_msg(mem_fetch_debug* mfd)
   {
	   return;

		std::cout << "[Intersim]";
		std::cout << " Time1: " << mfd->m_time_stamp[0];
		std::cout << " Time2: " << mfd->m_time_stamp[1];
		std::cout << " Time3: " << mfd->m_time_stamp[2];
		std::cout << " Time4: " << mfd->m_time_stamp[3];
		std::cout << " SM: " << mfd->m_tpc;
		std::cout << " MC: " << mfd->m_mc;
		std::cout << " PacketID: " << mfd->m_request_uid;
		std::cout << " Addr: " << mfd->m_addr;
		std::cout << " BlkAddr: " << mfd->m_blk_addr;
		std::cout << " PC: " << mfd->m_pc;
		std::cout << " Wid: " << mfd->m_wid;
		std::cout << " Read: " << mfd->m_read;
		std::cout << " ReqSize: " << mfd->m_data_size_hist[0];
		std::cout << " RepSize: " << mfd->m_data_size_hist[1];
		//std::cout << " ByteMask: " << mfd->m_byte_mask_str;
		std::cout << std::endl;
   }

};

//added by kh(070715)
extern std::set<unsigned> g_SMs_in_use;
extern std::set<unsigned> g_MCs_in_use;
///

/*
void g_print_msg();

void g_print_msg(mem_fetch* data)
{
	unsigned packetID = data->get_request_uid();
	new_addr_type addr = data->get_addr();

	std::cout << "[Intersim]";
	std::cout << " Time1: " << data->m_time_stamp[0];
	std::cout << " Time2: " << data->m_time_stamp[1];
	std::cout << " Time3: " << data->m_time_stamp[2];
	std::cout << " Time4: " << data->m_time_stamp[3];
	std::cout << " SM: " << data->get_tpc();
	std::cout << " MC: " << data->m_MC_id;
	std::cout << " PacketID: " << data->get_request_uid();
	std::cout << " Addr: " << data->get_addr();
	//added by kh(053015)
	std::cout << " BlkAddr: " << g_L2_config.block_addr(data->get_addr());
	std::cout << " PC: " << data->get_pc();
	std::cout << " Wid: " << data->get_wid();
	std::cout << " Read: " << !data->is_write();
	std::cout << " Size1: " << data->m_data_size_hist[0];
	std::cout << " Size2: " << data->m_data_size_hist[1];

	std::ostringstream s;
	s << data->get_access_byte_mask();
	std::string str = s.str();
	//printf(" WriteMask: %s\n", str.c_str());
	std::cout << " ByteMask: " << str << std::endl;
}

*/



//added by kh(121715)
struct mc_placement_config {

public:

	std::vector<unsigned> mc_node_list;
	std::vector<unsigned> sm_node_list;

	void print_mc_placement_config()
	{
	  std::cout << "SM_nodes = [ ";
	  for(unsigned i=0;i<sm_node_list.size();i++)
	  {
		  std::cout << sm_node_list[i] << " ";
	  }
	  std::cout << " ] " << std::endl;

	  std::cout << "MC_nodes = [ ";
	  for(unsigned i=0;i<mc_node_list.size();i++)
	  {
		  std::cout << mc_node_list[i] << " ";
	  }
	  std::cout << " ] " << std::endl;
	  ///
	}

	bool is_mc_node(unsigned node)
	{
		std::vector<unsigned>::iterator it;
		it = find (mc_node_list.begin(), mc_node_list.end(), node);
		if (it != mc_node_list.end())	return true;
		else							return false;
	}

	bool is_sm_node(unsigned node)
	{
		std::vector<unsigned>::iterator it;
		it = find (sm_node_list.begin(), sm_node_list.end(), node);
		if (it != sm_node_list.end())	return true;
		else							return false;
	}

	int get_mc_node_index(unsigned node)
	{
		int ret = -1;
		for(unsigned i=0; i<mc_node_list.size(); i++)
		{
			if(mc_node_list[i]==node)	ret = i;
		}
		assert(ret>=0);
		return ret;
	}

	int get_sm_node_index(unsigned node)
	{
		int ret = -1;
		for(unsigned i=0; i<sm_node_list.size(); i++)
		{
			if(sm_node_list[i]==node)	ret = i;
		}
		assert(ret>=0);
		return ret;
	}

	unsigned get_mc_node(unsigned index)
	{
		assert(mc_node_list.size() > index);
		return	mc_node_list[index];
	}



};

//#define	MULTICAST
//#ifdef MULTICAST
//#define MULTICAST_DEBUG
#define MULTICAST_SEL
//#define MULTICAST_SUBNETWORK
#ifdef MULTICAST
#define	MULTICAST_MECHANISM1
//#define MULTICAST_MECHANISM2
#define MULTICAST_PRT_DEBUG
#define INTERSIM_PRT_DEBUG
#endif


#ifdef INTERSIM_PRT_DEBUG
	#define INTERSIM_PRINT(x) std::cout << x;
#else
	#define INTERSIM_PRINT(x)
#endif

#ifdef INTERSIM_PRT_DEBUG
	#define INTERSIM_PRINTLN(x) std::cout << x << std::endl;
#else
	#define INTERSIM_PRINTLN(x)
#endif








#endif /* GPGPU_SIM_USER_DEFINED_H_ */
