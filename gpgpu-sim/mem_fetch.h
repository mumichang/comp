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

#ifndef MEM_FETCH_H
#define MEM_FETCH_H

#include "addrdec.h"
#include "../abstract_hardware_model.h"
#include <bitset>

enum mf_type {
   READ_REQUEST = 0,
   WRITE_REQUEST,
   READ_REPLY, // send to shader
   WRITE_ACK,
   // added by abpd (050516)
   CTRL_MSG,
};

#define MF_TUP_BEGIN(X) enum X {
#define MF_TUP(X) X
#define MF_TUP_END(X) };
#include "mem_fetch_status.tup"
#undef MF_TUP_BEGIN
#undef MF_TUP
#undef MF_TUP_END

class mem_fetch {
public:
    mem_fetch( const mem_access_t &access, 
               const warp_inst_t *inst,
               unsigned ctrl_size, 
               unsigned wid,
               unsigned sid, 
               unsigned tpc, 
               const class memory_config *config );
    // added by abpd (050516)
    mem_fetch(enum mf_type type,
	      unsigned tpc,
	      unsigned size,
              unsigned wid,
    	      unsigned sid);

    ~mem_fetch();



   void set_status( enum mem_fetch_status status, unsigned long long cycle );
   void set_reply() 
   { 
       assert( m_access.get_type() != L1_WRBK_ACC && m_access.get_type() != L2_WRBK_ACC );
       if( m_type==READ_REQUEST ) {
           assert( !get_is_write() );
           m_type = READ_REPLY;
       } else if( m_type == WRITE_REQUEST ) {
           assert( get_is_write() );
           m_type = WRITE_ACK;
       }
   }
   void do_atomic();

   void print( FILE *fp, bool print_inst = true ) const;

   const addrdec_t &get_tlx_addr() const { return m_raw_addr; }
   unsigned get_data_size() const { return m_data_size; }
   void     set_data_size( unsigned size ) { m_data_size=size; }
   unsigned get_ctrl_size() const { return m_ctrl_size; }
   unsigned size() const { return m_data_size+m_ctrl_size; }
   bool is_write() {return m_access.is_write();}
   void set_addr(new_addr_type addr) { m_access.set_addr(addr); }
   new_addr_type get_addr() const { return m_access.get_addr(); }
   new_addr_type get_partition_addr() const { return m_partition_addr; }
   unsigned get_sub_partition_id() const { return m_raw_addr.sub_partition; }
   bool     get_is_write() const { return m_access.is_write(); }
   unsigned get_request_uid() const { return m_request_uid; }
   unsigned get_sid() const { return m_sid; }
   unsigned get_tpc() const { return m_tpc; }
   unsigned get_wid() const { return m_wid; }
   bool istexture() const;
   bool isconst() const;
   enum mf_type get_type() const { return m_type; }
   bool isatomic() const;

   void set_return_timestamp( unsigned t ) { m_timestamp2=t; }
   void set_icnt_receive_time( unsigned t ) { m_icnt_receive_time=t; }
   unsigned get_timestamp() const { return m_timestamp; }
   unsigned get_return_timestamp() const { return m_timestamp2; }
   unsigned get_icnt_receive_time() const { return m_icnt_receive_time; }

   enum mem_access_type get_access_type() const { return m_access.get_type(); }
   const active_mask_t& get_access_warp_mask() const { return m_access.get_warp_mask(); }
   mem_access_byte_mask_t get_access_byte_mask() const { return m_access.get_byte_mask(); }

   address_type get_pc() const { return m_inst.empty()?-1:m_inst.pc; }
   const warp_inst_t &get_inst() { return m_inst; }
   enum mem_fetch_status get_status() const { return m_status; }

   const memory_config *get_mem_config(){return m_mem_config;}

   unsigned get_num_flits(bool simt_to_mem);
private:
   // request source information
   unsigned m_request_uid;
   unsigned m_sid;
   unsigned m_tpc;
   unsigned m_wid;

   // where is this request now?
   enum mem_fetch_status m_status;
   unsigned long long m_status_change;

   // request type, address, size, mask
   mem_access_t m_access;
   unsigned m_data_size; // how much data is being written
   unsigned m_ctrl_size; // how big would all this meta data be in hardware (does not necessarily match actual size of mem_fetch)
   new_addr_type m_partition_addr; // linear physical address *within* dram partition (partition bank select bits squeezed out)
   addrdec_t m_raw_addr; // raw physical address (i.e., decoded DRAM chip-row-bank-column address)
   enum mf_type m_type;

   // statistics
   unsigned m_timestamp;  // set to gpu_sim_cycle+gpu_tot_sim_cycle at struct creation
   unsigned m_timestamp2; // set to gpu_sim_cycle+gpu_tot_sim_cycle when pushed onto icnt to shader; only used for reads
   unsigned m_icnt_receive_time; // set to gpu_sim_cycle + interconnect_latency when fixed icnt latency mode is enabled

   // requesting instruction (put last so mem_fetch prints nicer in gdb)
   warp_inst_t m_inst;

   static unsigned sm_next_mf_request_uid;

   const class memory_config *m_mem_config;
   unsigned icnt_flit_size;

public:
   std::vector<unsigned long long> m_time_stamp;
   unsigned m_MC_id;
   std::vector<unsigned long long> m_data_size_hist;

   new_addr_type m_blk_addr;

//added by kh(070715)
   public:
   std::vector<unsigned long long> m_status_timestamp;
   ///

  //added by kh(070715)
public:
  void set_timestamp(enum mem_fetch_status status, unsigned long long cycle);
  unsigned long long get_timestamp(enum mem_fetch_status status);
  ///


  //added by kh(030816)
private:
  std::vector<unsigned char> m_real_data;

public:
  unsigned char* config_real_data(unsigned data_size);
  unsigned char get_real_data(unsigned data_index);
  void set_real_data(unsigned data_index, unsigned char data);
  void get_real_data_stream(unsigned flit_seq, std::vector<unsigned char>& dest, int stream_size=32);
  unsigned get_real_data_size();
  ///

  //added by kh(031716)
private:
  unsigned m_comp_data_size;

public:
  std::vector<unsigned char>& get_real_data_ptr();
  void set_comp_data_size(unsigned comp_data_size) 	{	m_comp_data_size = comp_data_size;	}
  unsigned get_comp_data_size()		 		{	return m_comp_data_size;		}
  //unsigned size_after_comp() const 			{ 	return m_comp_data_size+m_ctrl_size; 	}

  //added by kh(041816)
private:
  unsigned m_comp_res;
public:
  void set_comp_res(unsigned comp_res)			{	m_comp_res = comp_res;			}
  unsigned get_comp_res()				{	return m_comp_res;			}
  ///

  //added by kh(042216)
public:
  enum comp_status_type {
    UNCOMPRESSED = 0,
    COMPRESSED,
    INTRA_COMPRESSED,
    INTER_COMPRESSED,
  };

  class comp_word {
    public:
      comp_word()	{};
      ~comp_word()	{};
    public:
      unsigned char word1B;
      unsigned short word2B;
      unsigned int word4B;
      unsigned long long word8B;
      unsigned res;
      enum comp_status_type comp_status;

      void print(bool linefeed=true)
      {
	if(linefeed == true)	printf("\t");

	if(res == 1)		printf("word %02x, comp %d", word1B, comp_status);
	else if(res == 2)	printf("word %04x, comp %d", word2B, comp_status);
	else if(res == 4)	printf("word %08x, comp %d", word4B, comp_status);
	else if(res == 8)	printf("word %016llx, comp %d", word8B, comp_status);
	else assert(0);

	if(linefeed == true)	printf("\n");
      }



  };

  std::vector<class comp_word> m_compword2B_list;
  std::vector<class comp_word> m_compword4B_list;
  std::vector<class comp_word> m_compword8B_list;

  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res2;	//row: word, col: one byte in the word.
  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res4;	//row: word, col: one byte in the word.
  std::vector<std::vector<class comp_word> > m_uncompword1B_list_res8;	//row: word, col: one byte in the word.

  void push_compword(unsigned res, unsigned long long comp_word, enum comp_status_type comp_status)
  {
    class comp_word tmp;
    if(res == 2) {
	tmp.res = 2;
	tmp.word2B = (unsigned short) comp_word;
	tmp.comp_status = comp_status;
	m_compword2B_list.push_back(tmp);
    } else if(res == 4) {
	tmp.res = 4;
	tmp.word4B = (unsigned int) comp_word;
	tmp.comp_status = comp_status;
	m_compword4B_list.push_back(tmp);
    } else if(res == 8) {
	tmp.res = 8;
	tmp.word8B = (unsigned long long) comp_word;
	tmp.comp_status = comp_status;
	m_compword8B_list.push_back(tmp);
    }
  }

  void push_uncompword(unsigned res, unsigned row, unsigned char uncomp_word, enum comp_status_type comp_status)
  {
    //printf("push_uncompword starts rest %u\n", res);
    class comp_word tmp;
    if(res == 2) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res2.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res2.push_back(tmp);
      }
      m_uncompword1B_list_res2[row].push_back(tmp);
    } else if(res == 4) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res4.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res4.push_back(tmp);
      }
      m_uncompword1B_list_res4[row].push_back(tmp);
    } else if(res == 8) {
      tmp.res = 1;
      tmp.word1B = uncomp_word;
      tmp.comp_status = comp_status;
      if(m_uncompword1B_list_res8.size() == row) {
	std::vector<class comp_word> tmp;
	m_uncompword1B_list_res8.push_back(tmp);
      }
      m_uncompword1B_list_res8[row].push_back(tmp);
    }
    //printf("push_uncompword ends\n");
  }

  void print_compword()
  {
    if(m_comp_res == 2) {
      for(unsigned i = 0; i < m_compword2B_list.size(); i++)	m_compword2B_list[i].print();
    } else if(m_comp_res == 4) {
      for(unsigned i = 0; i < m_compword4B_list.size(); i++)	m_compword4B_list[i].print();
    } else if(m_comp_res == 8) {
      for(unsigned i = 0; i < m_compword8B_list.size(); i++)	m_compword8B_list[i].print();
    }
  }

  void print_uncompword()
  {
    printf("print_uncompword starts, comp_res %u\n", m_comp_res);
    if(m_comp_res == 2) {
      for(unsigned i = 0; i < m_uncompword1B_list_res2.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res2[i].size(); j++) {
	    m_uncompword1B_list_res2[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    } else if(m_comp_res == 4) {
      for(unsigned i = 0; i < m_uncompword1B_list_res4.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res4[i].size(); j++) {
	    m_uncompword1B_list_res4[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    } else if(m_comp_res == 8) {
      for(unsigned i = 0; i < m_uncompword1B_list_res8.size(); i++) {
	for(unsigned j = 0; j < m_uncompword1B_list_res8[i].size(); j++) {
	    m_uncompword1B_list_res8[i][j].print(false);
	    printf(" ");
	}
	printf("\n");
      }
    }
    printf("print_uncompword ends\n");
  }
  ///

  //added by kh(042316)
  unsigned get_uncompword_row_no() {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2.size();
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4.size();
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8.size();

    assert(0);
    return 0;
  }

  unsigned get_uncompword_col_no(unsigned index) {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2[index].size();
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4[index].size();
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8[index].size();

    assert(0);
    return 0;
  }

  class comp_word get_uncompword(unsigned row, unsigned col) {
    if(m_comp_res == 2) 	return	m_uncompword1B_list_res2[row][col];
    else if(m_comp_res == 4) 	return	m_uncompword1B_list_res4[row][col];
    else if(m_comp_res == 8) 	return	m_uncompword1B_list_res8[row][col];

    assert(0);
    class comp_word dummy;
    return dummy;
  }
  ///

  //added by kh(042316)
  //inter-replies merging to reduce the fragmentation
private:
  std::vector<mem_fetch*> m_merged_mfs;
public:
  void add_merged_mfs(std::vector<mem_fetch*>& mfs)
  {
    for(unsigned i = 0; i < mfs.size(); i++)	m_merged_mfs.push_back(mfs[i]);
  }
  void del_merged_mfs()	{	m_merged_mfs.clear();	}

  std::vector<mem_fetch*>& get_merged_mfs()		{	return m_merged_mfs;	}
  bool has_merged_mem_fetch()				{	return (m_merged_mfs.size() > 0)? true: false;	}
  unsigned get_comp_size_merged_mem_fetch() {
    unsigned ret = 0;
    //printf("mf %u\n", get_request_uid());
    //printf("\tctrl0 %u, comp_size0 %u\n", get_ctrl_size(), get_comp_data_size());
    for(unsigned i = 0; i < m_merged_mfs.size(); i++) {
      ret = ret + (m_merged_mfs[i]->get_ctrl_size()+m_merged_mfs[i]->get_comp_data_size());
      //printf("\tctrl %u, comp_size %u\n", m_merged_mfs[i]->get_ctrl_size(),m_merged_mfs[i]->get_comp_data_size());
    }
    //printf("\tret %u\n", ret);

    return ret;
  }
  ///

  //added by kh(042516)
private:
  std::vector<mem_fetch*> m_merged_req_mfs;
public:
  void add_merged_req_mfs(std::vector<mem_fetch*>& mfs)
  {
    for(unsigned i = 0; i < mfs.size(); i++)	m_merged_req_mfs.push_back(mfs[i]);
  }
  void del_merged_req_mfs()				{	m_merged_req_mfs.clear();	}

  std::vector<mem_fetch*>& get_merged_req_mfs()		{	return m_merged_req_mfs;	}
  bool has_merged_req_mfs()				{	return (m_merged_req_mfs.size() > 0)? true: false;}
  unsigned get_size_merged_req_mfs() {
    unsigned ret = 0;
    for(unsigned i = 0; i < m_merged_req_mfs.size(); i++) {
      ret = ret + (m_merged_req_mfs[i]->get_ctrl_size());
    }
    return ret;
  }


  //added by abpd(042816)
public:
  std::string compare_data;
private:
  std::vector<unsigned char> m_compress_data;
public:
  void set_comp_data(unsigned data_index, unsigned char data)
  {
      m_compress_data[data_index] = data;
  }
  void init_size()
  {
    m_compress_data.resize(m_comp_data_size);
  }
  ///
};

#endif
