/*
 * hpcl_comp.cc
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#include "hpcl_comp.h"
#include "../abstract_hardware_model.h"
#include <cassert>

hpcl_comp::hpcl_comp ()
{
  // TODO Auto-generated constructor stub

}

hpcl_comp::~hpcl_comp ()
{
  // TODO Auto-generated destructor stub
}


void hpcl_comp::create(const struct memory_config * memory_config, class memory_space * global_mem)
{
  m_memory_config = memory_config;
  m_global_mem = global_mem;
}

void hpcl_comp::get_cache_data(void* _mf, void* data, int index)
{
  mem_fetch* mf = (mem_fetch*)_mf;
  new_addr_type access_addr = mf->get_addr();
  new_addr_type blk_addr = m_memory_config->m_L2_config.block_addr(access_addr);
  //blk_addr = m_memory_config->m_L2_config.block_addr(access_addr);

  assert(data != 0);

  //deleted by kh(032016)
  //if(mf->get_access_type() == GLOBAL_ACC_R) {

  //added by kh(032016)
  if(mf->get_access_type() == GLOBAL_ACC_R
  || mf->get_access_type() == LOCAL_ACC_R
  || mf->get_access_type() == CONST_ACC_R
  || mf->get_access_type() == TEXTURE_ACC_R)
  //|| mf->get_access_type() == INST_ACC_R)
  {
//    if(mf->get_access_type() == INST_ACC_R) {
//	std::cout << "size " << mf->get_data_size() << std::endl;
//	assert(0);
//    }

    if(index == -1) {
      for(unsigned i = 0; i < mf->get_data_size(); i++) m_global_mem->read((mem_addr_t)blk_addr+i, 1, data+i);
    } else {
      assert(index*32+32 <= mf->get_data_size());
      int start_addr = (mem_addr_t)blk_addr + index*32;
      for(unsigned i = 0; i < 32; i++)	m_global_mem->read(start_addr+i, 1, data+i);
    }

    //printf("LD - PC: %u, Type: %d, Mem_Addr: %016llx, Data[0]: %02x\n", mf->get_pc(), mf->get_access_type(), blk_addr, *((unsigned char*)(data)));
  } else {
    //printf("LD - PC: %u, Type: %d, Mem_Addr: %016llx\n", mf->get_pc(), mf->get_access_type(), blk_addr);
  }





}

void hpcl_comp::print_cache_data(mem_fetch* mf)
{
  return;
/*
  new_addr_type access_addr = mf->get_addr();
  new_addr_type blk_addr = m_memory_config->m_L2_config.block_addr(access_addr);
  unsigned char* cache_data = NULL;
  if(mf->get_access_type() == GLOBAL_ACC_R) {
      cache_data = (unsigned char*)malloc(128*sizeof(unsigned char));
      for(unsigned i = 0; i < 128; i++)	m_global_mem->read((mem_addr_t)blk_addr+i, 1, cache_data+i);

      //printf("gm_addr = 0x%0x\n", blk_addr);
      printf("gm_addr = %0x\n", blk_addr);
      //printf("gm_data = 0x");
      printf("gm_data = ");
      for(int i = 127; i >= 0; i--)	printf("%02x", *(cache_data+i));
      printf("\n");
  }

  if(cache_data) free(cache_data);
*/
}

