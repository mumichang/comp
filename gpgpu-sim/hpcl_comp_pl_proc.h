/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_PL_PROC_H_
#define HPCL_COMP_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_dict.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>

//added by kh(030816)
#include "hpcl_comp_anal.h"
extern hpcl_comp_anal* g_hpcl_comp_anal;
///

template<class K>
class hpcl_comp_pl_proc {
public:
  enum comp_pl_type {
    INVALID = 0,
    COMP,
    INJECT_UNCOMP_FLIT,
    INJECT_COMP_FLIT
  };
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output[2];
  int m_pl_index;
  enum comp_pl_type m_type;
  unsigned m_comp_parl_size;		//bytes that a compression pipeline can perform concurrently.
  unsigned m_par_id;

public:
  hpcl_comp_pl_proc(unsigned dict_size, enum hpcl_dict_rep_policy policy, unsigned m_par_id);
  ~hpcl_comp_pl_proc() {};
  void set_pl_index(int pl_index);
  void set_pl_type(enum comp_pl_type type);
  void set_output(hpcl_comp_pl_data* output, int index=0);
  hpcl_comp_pl_data* get_output(int index=0);
  hpcl_comp_pl_data* get_input();
  void reset_output(int index=0);
  void decompose_data(std::vector<unsigned char>& flit_data, std::vector<K>& word_list);
  void run();

//global compression table for 64 nodes
private:
  vector<hpcl_dict<K>* > m_hpcl_global_dict;	//encoding table

public:
  void create_dict(unsigned dict_size, enum hpcl_dict_rep_policy policy);
  void update_dict(K word, unsigned long long time, int node_index);
  void print_dict(int node_index);

  //added by kh(030716)
  void erase_dict(K word, int node_index);


  int get_free_entry_dict(int node_index);
  K search_victim_word(int node_index);


//added by kh(022816)
private:
  //missing words
  vector<pair<pair<int,int>, K> > m_missing_word;	//first: <sm,mc>, second: word

public:
  pair<pair<int,int>,K> get_missing_word(int index);
  int get_missing_word_size();
  void clear_missing_word();

  int search_word_dict(int node_index, K word);
  K get_word_dict(int node_index, unsigned index);
  int get_word_freq(int node_index, unsigned index);
  ///

  //added by kh(030816)
  void validate_word(int node_index, int word_index, K word);
  bool get_word_valid_flag(int node_index, int word_index);
  void hold_word(int node_index, int word_index, K word);
  void release_word(int node_index, int word_index, K word);
  ///


};

template <class K>
hpcl_comp_pl_proc<K>::hpcl_comp_pl_proc(unsigned dict_size, enum hpcl_dict_rep_policy policy, unsigned par_id) {
  m_input = new hpcl_comp_pl_data;
  m_output[0] = NULL;
  m_output[1] = NULL;
  m_pl_index = -1;
  m_type = INVALID;
  create_dict(dict_size, policy);
  m_comp_parl_size = 8;
  m_par_id = par_id;
}

template <class K>
void hpcl_comp_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_pl_proc<K>::set_pl_type(enum comp_pl_type type) {
  m_type = type;
}

template <class K>
void hpcl_comp_pl_proc<K>::set_output(hpcl_comp_pl_data* output, int index) {
  m_output[index] = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_pl_proc<K>::get_output(int index) {
  return m_output[index];
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_pl_proc<K>::get_input() {
  return m_input;
}

template <class K>
void hpcl_comp_pl_proc<K>::reset_output(int index) {
  m_output[index]->clean();
}

template <class K>
void hpcl_comp_pl_proc<K>::decompose_data(std::vector<unsigned char>& flit_data, std::vector<K>& word_list) {
  //copy a segment of flit data for this compression pipeline
  unsigned char* comp_target_data = new unsigned char[m_comp_parl_size];
  for(unsigned i = 0; i < m_comp_parl_size; i++) {
    unsigned offset=m_pl_index*m_comp_parl_size;
    comp_target_data[i] = flit_data[offset+i];
  }

  for(unsigned i = 0; i < m_comp_parl_size; i=i+sizeof(K)) {
    K word_candi = 0;
    for(int j = sizeof(K)-1; j >= 0; j--) {
      K tmp = comp_target_data[i+j];
      tmp = (tmp << (8*j));
      word_candi += tmp;
    }
    word_list.push_back(word_candi);
//    printf("\tword -- ");
//    hpcl_dict_elem<K>::print_word_data(word_candi);
//    printf("\n");
  }
  assert(word_list.size() == m_comp_parl_size/sizeof(K));
}

template <class K>
void hpcl_comp_pl_proc<K>::run() {
//  std::cout << "comp_pipeline (" << m_pl_index << ")" << std::endl;

  Flit* flit_ptr = m_input->get_flit_ptr();
  std::vector<unsigned char> flit_data;

  //select dictionary index
  //int global_dict_index = -1;
  int dest_node_index = -1;
  int src_node_index = -1;
  if(flit_ptr) {
    flit_data = flit_ptr->raw_data;
    if(flit_ptr->type == Flit::READ_REPLY) {
      dest_node_index = flit_ptr->const_dest;	//SM node index
      src_node_index = flit_ptr->src;		//MC node index
    }
  }

  if(m_type == COMP) {

    //double comp_rsl = m_input->get_comp_result();
    if(flit_data.size() > 0) {

      assert(dest_node_index>=0);

      //word-matching compression
      //step1: transform raw data to a set of words
      std::vector<K> word_list;
      decompose_data(flit_data, word_list);

      //step2: search every word in a dictionary and compute compression ratio.
      double comp_ratio = 0;
      hpcl_comp_data* comp_data = new hpcl_comp_data;
      for(unsigned i = 0; i < word_list.size(); i++) {
	  int data_ptr = m_hpcl_global_dict[dest_node_index]->search_word(word_list[i]);

	  if(data_ptr >= 0 && m_hpcl_global_dict[dest_node_index]->get_valid_flag(data_ptr)) {

//	    std::cout << "decomp " << m_par_id << " dest_node_index " << dest_node_index;
//	    std::cout << " src_node_index " << src_node_index << " word " << word_list[i];
//	    std::cout <<  " flag " << m_hpcl_global_dict[dest_node_index]->get_valid_flag(data_ptr) << std::endl;
	    comp_ratio++;
	    comp_data->data_index.push_back(data_ptr);
	    //for debugging
	    comp_data->raw_data.push_back(word_list[i]);

	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::GLOBAL_TABLE_HIT_NO, 1);

	  } else {	//if word is not found (022816)
	    m_missing_word.push_back(std::make_pair(std::make_pair(src_node_index,dest_node_index), word_list[i]));

	      //std::cout << "\tword " << word_list[i] << " src " << src_node_index << " dest " << dest_node_index << std::endl;
	      //std::cout << "\tword " << word_list[i] << " is missing!! at pl " << m_pl_index;
	      //std::cout << " m_missing_word_size " << m_missing_word.size();
	      //std::cout << std::endl;

	    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::GLOBAL_TABLE_MISS_NO, 1);
	  }
      }
      flit_ptr->comp_data.push_back(comp_data);
      comp_ratio = comp_ratio/word_list.size()*100;

      //step3: cumulatively average comp ratio and update comp_result
      double tot_comp_ratio = (m_input->get_comp_result()*m_pl_index+comp_ratio)/(m_pl_index+1);
      m_input->set_comp_result(tot_comp_ratio);

//	    std::cout << "\tcompressing flit " << flit_ptr->id << " ... ";
//	    std::cout << ", comp_ratio " << comp_ratio << "%";
//	    std::cout << ", tot_comp_ratio " << tot_comp_ratio << "%" << std::endl;

      //copy the current compression result to next pipeline
      m_output[0]->copy(m_input);
    } else {
//	    std::cout << "\tcompressing no flit" << std::endl;
    }
    //reset the current input.
    m_input->clean();

  } else if(m_type == INJECT_UNCOMP_FLIT) {

    m_output[1]->clear_comp_done_flag();

    if(flit_data.size() > 0) {

	assert(dest_node_index>=0);

	Flit* flit_ptr = m_input->get_flit_ptr();
	//double comp_rsl = m_input->get_comp_result();
	//if(fabs(comp_rsl - 100) < std::numeric_limits<double>::epsilon()) {
	if(m_input->is_compressible() == true) {
	    //std::cout << "\tcomplete compression for flit " << flit_ptr->id << std::endl;
	    flit_ptr->m_enc_status = 1;
	    m_output[0]->add_comp_pl_data(flit_ptr);
	    m_output[0]->set_comp_result(100);

	    if(flit_ptr->tail == true) {
	      //compute the number of bits after compression. This helps to decide the number of flits where
	      //compressed index are stored.
	      unsigned flit_no = m_output[0]->get_flit_no();
	      int tot_word_no_per_flit = 32/sizeof(K);
	      double tot_comp_size = 1+(3+tot_word_no_per_flit*m_hpcl_global_dict[dest_node_index]->get_index_bit_size())*flit_no;

//		  std::cout << "\tsend a compressed flit for " << flit_no << " flits";
//		  std::cout << " size: " << tot_comp_size << "bits (" << ceil(tot_comp_size/8) << " B)" << std::endl;

//		  for(unsigned i = 0; i < m_output[1]->get_flit_no(); i++) {
//		      Flit* flit = m_output[1]->get_flit_ptr(i);
//		      std::cout << "\tflit " << flit->id << std::endl;
//		      for(unsigned j = 0; j < flit->comp_data.size(); j++) {
//			  std::cout << "\tcomp_chunk_no " << flit->comp_data[i]->data_index.size() << std::endl;
//		      }
//		  }

	      //std::cout << "m_output[0]->m_comp_rsl : " << m_output[0]->get_comp_rsl() << std::endl;
	      m_output[1]->copy(m_output[0]);
	      m_output[1]->set_comp_done_flag();
	      m_output[0]->clean();
	    }

	} else {
//		std::cout << "\tincomplete compression, send flit!" << std::endl;
	    flit_ptr->m_enc_status = 0;
	    m_output[1]->copy(m_input);

	    if(flit_ptr->tail == true) {
//		    std::cout << "\t\t" << m_output[0]->get_flit_no() << " compressible flits!" << std::endl;
		if(m_output[0]->get_flit_no() > 0)	{
		  m_output[0]->set_tail_done_flag();
		  /*
		  //added by kh (030116)
		  //if other flits except tail flit are compressed, then the current tail flit shouldn't be tail.
		  flit_ptr->tail = false;
		  ///
		  */
		} else if(m_output[0]->get_flit_no() == 0)	{
		  m_output[1]->set_comp_done_flag();
		}
	    }

	}

    } else {
//	    std::cout << "\tdo nothing!" << std::endl;
    }
    //reset the current input.
    m_input->clean();

  } else if(m_type == INJECT_COMP_FLIT) {

	m_output[1]->clear_comp_done_flag();

	if(m_input->get_tail_done_flag() == true) {
	    unsigned flit_no = m_input->get_flit_no();
	    m_input->set_comp_result(100);	//all flits here are compressible
	    m_output[1]->copy(m_input);

	    //compute the number of bits after compression. This helps to decide the number of flits where
	    //compressed index are stored.
	    int tot_word_no_per_flit = 32/sizeof(K);
	    double tot_comp_size = 1+(3+tot_word_no_per_flit*m_hpcl_global_dict[dest_node_index]->get_index_bit_size())*flit_no;

//	    std::cout << "\tsend a compressed flit for " << flit_no << " flits";
//	    std::cout << " size: " << tot_comp_size << "bits (" << ceil(tot_comp_size/8) << " B)" << std::endl;
//	    for(unsigned i = 0; i < m_output[1]->get_flit_no(); i++) {
//		Flit* flit = m_output[1]->get_flit_ptr(i);
//		std::cout << "\tflit " << flit->id << std::endl;
//		for(unsigned j = 0; j < flit->comp_data.size(); j++) {
//		    std::cout << "\tcomp_chunk_no " << flit->comp_data[i]->data_index.size() << std::endl;
//		}
//	    }

	    m_output[1]->set_comp_done_flag();

	    //reset the current input.
	    m_input->clean();
	} else {
//	    std::cout << "\tsend no compressed flit, tail flit is not processed yet" << std::endl;
	}

  } else {
	assert(0);
  }

  //return ret;
}

template <class K>
void hpcl_comp_pl_proc<K>::create_dict(unsigned dict_size, enum hpcl_dict_rep_policy policy)
{
  //Assumption: we use 64 nodes
  m_hpcl_global_dict.resize(64);
  for(int i = 0; i < 64; i++) {
    m_hpcl_global_dict[i] = new hpcl_dict<K>(dict_size, policy);
  }
}

template <class K>
void hpcl_comp_pl_proc<K>::erase_dict(K word, int node_index)
{
  m_hpcl_global_dict[node_index]->erase_dict(word);
}


template <class K>
void hpcl_comp_pl_proc<K>::update_dict(K word, unsigned long long time, int node_index) {
  m_hpcl_global_dict[node_index]->update_dict(word, time);
}

template <class K>
void hpcl_comp_pl_proc<K>::print_dict(int node_index) {
  m_hpcl_global_dict[node_index]->print();
}

template <class K>
int hpcl_comp_pl_proc<K>::get_free_entry_dict(int node_index) {
  return m_hpcl_global_dict[node_index]->get_free_entry();
}

template <class K>
K hpcl_comp_pl_proc<K>::search_victim_word(int node_index) {
  return m_hpcl_global_dict[node_index]->search_victim_by_LFU();
}


template <class K>
pair<pair<int,int>,K> hpcl_comp_pl_proc<K>::get_missing_word(int index) {
  assert(index>=0);
  return m_missing_word[index];
}

template <class K>
int hpcl_comp_pl_proc<K>::get_missing_word_size() {
  //std::cout << "get_missing_word_size() - " << m_missing_word.size() << std::endl;
  return m_missing_word.size();
}

template <class K>
void hpcl_comp_pl_proc<K>::clear_missing_word() {
  //std::cout << "clear_missing_word() - " << std::endl;
  m_missing_word.clear();
}

template <class K>
int hpcl_comp_pl_proc<K>::search_word_dict(int node_index, K word) {
  return m_hpcl_global_dict[node_index]->search_word(word);
}

template <class K>
K hpcl_comp_pl_proc<K>::get_word_dict(int node_index, unsigned word_index) {
  return m_hpcl_global_dict[node_index]->get_word(word_index);
}
template <class K>
bool hpcl_comp_pl_proc<K>::get_word_valid_flag(int node_index, int word_index) {
  return m_hpcl_global_dict[node_index]->get_valid_flag(word_index);
}

template <class K>
int hpcl_comp_pl_proc<K>::get_word_freq(int node_index, unsigned word_index) {
  return m_hpcl_global_dict[node_index]->get_freq(word_index);
}

//added by kh(030816)
template <class K>
void hpcl_comp_pl_proc<K>::validate_word(int node_index, int word_index, K word) {
  m_hpcl_global_dict[node_index]->validate_word(word_index, word);
}

template <class K>
void hpcl_comp_pl_proc<K>::hold_word(int node_index, int word_index, K word) {
  m_hpcl_global_dict[node_index]->hold_word(word_index, word);
}

template <class K>
void hpcl_comp_pl_proc<K>::release_word(int node_index, int word_index, K word) {
  m_hpcl_global_dict[node_index]->release_word(word_index, word);
}

///


#endif /* HPCL_COMP_PL_PROC_H_ */
