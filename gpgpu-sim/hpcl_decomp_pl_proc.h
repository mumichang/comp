/*
 * hpcl_decomp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DECOMP_PL_PROC_H_
#define HPCL_DECOMP_PL_PROC_H_

#include "hpcl_dict.h"
#include "hpcl_comp_pl_data.h"
//#include "hpcl_virt_flit.h"
#include <cassert>
#include <iostream>


//added by kh(022716)
extern struct mc_placement_config g_mc_placement_config;
extern gpgpu_sim *g_the_gpu;


template <class K>
class hpcl_decomp_pl_proc
{
/*
public:
  hpcl_decomp_pl_proc ();
  virtual
  ~hpcl_decomp_pl_proc ();
*/
public:
  enum decomp_pl_type {
    INVALID = 0,
    DECOMP,
    EJECT_DECOMP_FLIT
  };

private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output[2];
  int m_pl_index;
  unsigned m_par_id;
  enum decomp_pl_type m_type;

public:
  hpcl_decomp_pl_proc(unsigned dict_size, enum hpcl_dict_rep_policy policy, unsigned par_id);
  ~hpcl_decomp_pl_proc() {}

  void set_output(hpcl_comp_pl_data* output, int index=0);
  hpcl_comp_pl_data* get_output(int index=0);
  hpcl_comp_pl_data* get_input();
  void reset_output(int index=0);
  void set_pl_index(int pl_index);
  void set_pl_type(enum decomp_pl_type type);
  void run();

//global decompression table
private:
  vector<hpcl_dict<K>* > m_hpcl_global_dict;		//decoding table

public:
  void create_dict(unsigned dict_size, enum hpcl_dict_rep_policy policy);
  void update_dict(K word, unsigned long long time, int node_index, int word_index=-1);
  void print_dict(int node_index);

  int get_free_entry_dict(int node_index);
  K search_victim_word(int node_index);



};


template <class K>
hpcl_decomp_pl_proc<K>::hpcl_decomp_pl_proc(unsigned dict_size, enum hpcl_dict_rep_policy policy, unsigned par_id) {
  m_input = new hpcl_comp_pl_data;
  m_output[0] = NULL;
  m_output[1] = NULL;
  m_pl_index = -1;
  create_dict(dict_size, policy);

  m_par_id = par_id;
}

template <class K>
void hpcl_decomp_pl_proc<K>::set_output(hpcl_comp_pl_data* output, int index) {
  m_output[index] = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_decomp_pl_proc<K>::get_output(int index) {
  return m_output[index];
}

template <class K>
hpcl_comp_pl_data* hpcl_decomp_pl_proc<K>::get_input() {
  return m_input;
}

template <class K>
void hpcl_decomp_pl_proc<K>::reset_output(int index) {
  m_output[index]->clean();
}

template <class K>
void hpcl_decomp_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_decomp_pl_proc<K>::set_pl_type(enum decomp_pl_type type) {
  m_type = type;
}

template <class K>
void hpcl_decomp_pl_proc<K>::run()
{
  //std::cout << "decomp_pipeline (" << m_pl_index << ")" << std::endl;

  //select dictionary index
  int src_node_index = -1;
  Flit* flit = m_input->get_flit_ptr();
  if(flit) {
    //mem_fetch* mf = (mem_fetch*) flit->data;
    if(flit->type == Flit::READ_REPLY) {
      //int mem_channel_idx = mf->get_sub_partition_id()/g_the_gpu->getMemoryConfig()->m_n_sub_partition_per_memory_channel;
      //global_dict_index = g_mc_placement_config.get_mc_node(mem_channel_idx);
      src_node_index = flit->src;
    }
  }

  if(m_type == DECOMP) {

    if(!flit)	{
      //std::cout << "\tdecompress no flit " << std::endl;
      return;
    }

    /*
    if(m_pl_index == 0) {
	if(flit->m_enc_status == 0)	{	//if a flit is uncompressed
	    m_output[1]->copy(m_input);
	    m_input->clean();
	    //std::cout << "\tdetect uncompressed flit " << flit->id << std::endl;

	    //added by kh(022516)
	    if(flit->tail == true) {
		m_output[1]->set_comp_done_flag();
	    }
	    ///
	    return;
	}
    }
    */

    //assert(flit->m_enc_status == 1);
    if(flit->type == Flit::READ_REPLY) {
      assert(src_node_index>=0);
      if(flit->m_enc_status == 1) {
	std::vector<K> word_list;
	for(unsigned i = 0; i < ((hpcl_comp_data*)flit->comp_data[m_pl_index])->data_index.size(); i++) {
	  unsigned index = ((hpcl_comp_data*)flit->comp_data[m_pl_index])->data_index[i];

	  if(m_hpcl_global_dict[src_node_index]->get_valid_flag(index) != true) {
	      //std::cout << "wrong word " << m_hpcl_global_dict[src_node_index]->get_word(index) << " node_index " << src_node_index << std::endl;
	      std::cout << "decomp_id " << m_par_id;
	      std::cout << " node_index " << src_node_index;
	      std::cout << " word_index " << index;
	      std::cout << " err_word " << m_hpcl_global_dict[src_node_index]->get_word(index) << std::endl;
	  }

	  assert(m_hpcl_global_dict[src_node_index]->get_valid_flag(index) == true);
	  word_list.push_back(m_hpcl_global_dict[src_node_index]->get_word(index));
	}
	//transform words to bytes
	for(unsigned i = 0; i < word_list.size(); i++) {
	    K word_candi = word_list[i];
	    for(unsigned j = 0; j < sizeof(K); j++) {
	      if(j != 0) word_candi = (word_candi >> 8);
	      unsigned char byte_data = (unsigned char)(word_candi & 0xff);
	      flit->decomp_data.push_back(byte_data);
	    }
	}
      }
    } else if(flit->type == Flit::WRITE_REPLY) {
      //do nothing
    }

    m_output[0]->copy(m_input);

    //std::cout << "\tdo decompression flit " << flit->id << std::endl;

  } else if(m_type == EJECT_DECOMP_FLIT) {

    //Flit* flit = m_input->get_flit_ptr();
    if(flit) {
	//std::cout << "\teject decompressed flit " << flit->id << std::endl;

	//added by kh(022516)
	if(flit->tail == true) {
	    m_output[1]->set_comp_done_flag();
	}
	///

    } else {
	//std::cout << "\teject no flit!" << std::endl;
    }

    m_output[1]->copy(m_input);

  }

  //reset the current input.
  m_input->clean();
}

template <class K>
void hpcl_decomp_pl_proc<K>::create_dict(unsigned dict_size, enum hpcl_dict_rep_policy policy)
{
  //Assumption: 8 MCs are used
  m_hpcl_global_dict.resize(64);
  for(int i = 0; i < 64; i++) {
      m_hpcl_global_dict[i] = new hpcl_dict<K>(dict_size, policy);
  }
}

template <class K>
void hpcl_decomp_pl_proc<K>::update_dict(K word, unsigned long long time, int node_index, int word_index) {
  //std::cout << "node index " << node_index << std::endl;
  assert(m_hpcl_global_dict[node_index]);
  m_hpcl_global_dict[node_index]->update_dict(word, time, word_index);

  //added by kh(030816)
  m_hpcl_global_dict[node_index]->validate_word(word_index, word);
  ///
}

template <class K>
void hpcl_decomp_pl_proc<K>::print_dict(int node_index) {
  m_hpcl_global_dict[node_index]->print();
}

template <class K>
int hpcl_decomp_pl_proc<K>::get_free_entry_dict(int node_index) {
  return m_hpcl_global_dict[node_index]->get_free_entry();
}
template <class K>
K hpcl_decomp_pl_proc<K>::search_victim_word(int node_index) {
  return m_hpcl_global_dict[node_index]->search_victim_by_LFU();
}



#endif /* HPCL_DECOMP_PL_PROC_H_ */
