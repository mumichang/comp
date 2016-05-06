// Copyright (c) 2009-2011, Tor M. Aamodt
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

#include "mem_fetch.h"
#include "mem_latency_stat.h"
#include "shader.h"
#include "visualizer.h"
#include "gpu-sim.h"

unsigned mem_fetch::sm_next_mf_request_uid=1;

mem_fetch::mem_fetch( const mem_access_t &access, 
                      const warp_inst_t *inst,
                      unsigned ctrl_size, 
                      unsigned wid,
                      unsigned sid, 
                      unsigned tpc, 
                      const class memory_config *config )
{
   m_request_uid = sm_next_mf_request_uid++;
   m_access = access;
   if( inst ) { 
       m_inst = *inst;
       assert( wid == m_inst.warp_id() );
   }
   m_data_size = access.get_size();
   m_ctrl_size = ctrl_size;
   m_sid = sid;
   m_tpc = tpc;
   m_wid = wid;
   config->m_address_mapping.addrdec_tlx(access.get_addr(),&m_raw_addr);
   m_partition_addr = config->m_address_mapping.partition_address(access.get_addr());
   m_type = m_access.is_write()?WRITE_REQUEST:READ_REQUEST;
   m_timestamp = gpu_sim_cycle + gpu_tot_sim_cycle;
   m_timestamp2 = 0;
   m_status = MEM_FETCH_INITIALIZED;
   m_status_change = gpu_sim_cycle + gpu_tot_sim_cycle;
   m_mem_config = config;
   icnt_flit_size = config->icnt_flit_size;

	//added by kh(070715)
	m_status_timestamp.resize(END_STATUS+1,0);
	///
}

//added by abpd (050516)
mem_fetch::mem_fetch(enum mf_type type,
		     unsigned tpc,
		     unsigned size,
		     unsigned wid,
		     unsigned sid)
{
   m_type = type;
   m_tpc = tpc;
   m_sid= sid;
   m_wid=wid;
   m_timestamp = gpu_sim_cycle + gpu_tot_sim_cycle;
   //m_ctrl_size = size;
   m_status_timestamp.resize(END_STATUS+1,0);
   //m_compress_data.resize(4*size);
   m_comp_data_size = (4*size);

   m_data_size = 0;
   m_ctrl_size = 8;
}
///

mem_fetch::~mem_fetch()
{
    m_status = MEM_FETCH_DELETED;
}

#define MF_TUP_BEGIN(X) static const char* Status_str[] = {
#define MF_TUP(X) #X
#define MF_TUP_END(X) };
#include "mem_fetch_status.tup"
#undef MF_TUP_BEGIN
#undef MF_TUP
#undef MF_TUP_END

void mem_fetch::print( FILE *fp, bool print_inst ) const
{
    if( this == NULL ) {
        fprintf(fp," <NULL mem_fetch pointer>\n");
        return;
    }
    fprintf(fp,"  mf: uid=%6u, sid%02u:w%02u, part=%u, ", m_request_uid, m_sid, m_wid, m_raw_addr.chip );
    m_access.print(fp);
    if( (unsigned)m_status < NUM_MEM_REQ_STAT ) 
       fprintf(fp," status = %s (%llu), ", Status_str[m_status], m_status_change );
    else
       fprintf(fp," status = %u??? (%llu), ", m_status, m_status_change );
    if( !m_inst.empty() && print_inst ) m_inst.print(fp);
    else fprintf(fp,"\n");
}

void mem_fetch::set_status( enum mem_fetch_status status, unsigned long long cycle ) 
{
    m_status = status;
    m_status_change = cycle;

    //added by kh(070715)
    assert(m_status_timestamp.size() > status);
    m_status_timestamp[status] = cycle;
    ///
}

bool mem_fetch::isatomic() const
{
   if( m_inst.empty() ) return false;
   return m_inst.isatomic();
}

void mem_fetch::do_atomic()
{
    m_inst.do_atomic( m_access.get_warp_mask() );
}

bool mem_fetch::istexture() const
{
    if( m_inst.empty() ) return false;
    return m_inst.space.get_type() == tex_space;
}

bool mem_fetch::isconst() const
{ 
    if( m_inst.empty() ) return false;
    return (m_inst.space.get_type() == const_space) || (m_inst.space.get_type() == param_space_kernel);
}

/// Returns number of flits traversing interconnect. simt_to_mem specifies the direction
unsigned mem_fetch::get_num_flits(bool simt_to_mem){
	unsigned sz=0;
	// If atomic, write going to memory, or read coming back from memory, size = ctrl + data. Else, only ctrl
	if( isatomic() || (simt_to_mem && get_is_write()) || !(simt_to_mem || get_is_write()) )
		sz = size();
	else
		sz = get_ctrl_size();

	return (sz/icnt_flit_size) + ( (sz % icnt_flit_size)? 1:0);
}


//added by kh(070715)

void mem_fetch::set_timestamp(enum mem_fetch_status status, unsigned long long cycle)
{
    assert(m_status_timestamp.size() > status);
    m_status_timestamp[status] = cycle;
}

unsigned long long mem_fetch::get_timestamp(enum mem_fetch_status status)
{
    assert(m_status_timestamp.size() > status);
    return m_status_timestamp[status];
}
///


//added by kh(030816)
unsigned char* mem_fetch::config_real_data(unsigned data_size)
{
  m_real_data.clear();
  m_real_data.resize(data_size, 0);
  return &m_real_data[0];
}

unsigned char mem_fetch::get_real_data(unsigned data_index)
{
  assert(m_real_data.size() > data_index);
  return m_real_data[data_index];
}

void mem_fetch::set_real_data(unsigned data_index, unsigned char data)
{
  m_real_data[data_index] = data;
}

void mem_fetch::get_real_data_stream(unsigned flit_seq, std::vector<unsigned char>& dest, int stream_size)
{
  assert(m_real_data.size() >= (flit_seq+1)*stream_size);
  for(int i = 0; i < stream_size; i++) {
      dest[i] = m_real_data[flit_seq*stream_size+i];
  }
}

unsigned mem_fetch::get_real_data_size()
{
  return m_real_data.size();
}
///


//added by kh(031716)
std::vector<unsigned char>& mem_fetch::get_real_data_ptr()
{
  return m_real_data;
}
///










