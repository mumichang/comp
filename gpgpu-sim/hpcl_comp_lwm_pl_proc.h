/*
 * hpcl_comp_pl_proc.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_LWM_PL_PROC_H_
#define HPCL_COMP_LWM_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_dict.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>


//#define DUMMY_COMP_TEST 0

//#define DEBUG_LWM 1

//added by kh(030816)
#include "hpcl_comp_anal.h"
extern hpcl_comp_anal* g_hpcl_comp_anal;
///

template<class K>
class hpcl_comp_lwm_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  int m_pl_index;

public:
  hpcl_comp_lwm_pl_proc();
  ~hpcl_comp_lwm_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    if(m_loc_dict)	delete m_loc_dict;
  };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  hpcl_dict<K>* m_loc_dict;		//1B word
  unsigned simul_local_comp(hpcl_dict<K>& loc_dict, vector<unsigned char>& cache_data);
public:
  void run();

//added by kh(042216)
public:
  static void decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list);
  static void decompose_data(K word, std::set<unsigned char>& word_list);
  static void decompose_data(K word, std::vector<unsigned char>& word1B_list);
  static void print_word_list(std::vector<K>& word_list);

private:
  hpcl_dict<unsigned char>* m_loc_dict_for_uncomp_data;

  double m_word1B_zeros_rate_in_uncomp_data;
  double m_word1B_nonzeros_rate_in_uncomp_data;
  double m_word1B_uni_val_rate_in_uncomp_data;
  double m_word1B_nonzeros_no;

public:
  void get_word1B_redund_stat_in_uncomp_data(double& word1B_zeros_rate, double& word1B_nonzeros_rate, double& word1B_uni_val_rate, double& word1B_nonzeros_no)
  {
    word1B_zeros_rate = m_word1B_zeros_rate_in_uncomp_data;
    word1B_nonzeros_rate = m_word1B_nonzeros_rate_in_uncomp_data;
    word1B_uni_val_rate = m_word1B_uni_val_rate_in_uncomp_data;
    word1B_nonzeros_no = m_word1B_nonzeros_no;
  }



  ///



};

template <class K>
hpcl_comp_lwm_pl_proc<K>::hpcl_comp_lwm_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_loc_dict = new hpcl_dict<K>(128, HPCL_LFU);
  m_loc_dict->clear();

  //added by kh(042216)
  m_loc_dict_for_uncomp_data = new hpcl_dict<unsigned char>(128, HPCL_LFU);
  m_loc_dict_for_uncomp_data->clear();
  ///
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_lwm_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_lwm_pl_proc<K>::get_input() {
  return m_input;
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::reset_output() {
  m_output->clean();
}

//#define DEBUG_LWM 1

template<class K>
void hpcl_comp_lwm_pl_proc<K>::run()
{
  #ifdef DUMMY_COMP_TEST
  m_output->set_mem_fetch(m_input->get_mem_fetch());
  #else

  mem_fetch* mf = m_input->get_mem_fetch();
  m_output->set_mem_fetch(mf);

  if(!mf) return;

  //If mf is not type for compression,
  if(mf->get_real_data_size() == 0)	return;

  std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
  std::vector<K> word_list;
  decompose_data(cache_data, word_list);

  //LWM simulation
  //encodeing format: encoding flag(1 bit), encoded word(index_size or word_size)
  unsigned comp_bit_size = 0;
  unsigned data_ptr_bits = (unsigned) ceil(log2(cache_data.size()/sizeof(K)));

  #ifdef DEBUG_LWM
  printf("org_data = 0x");
  for(unsigned i = 0; i < cache_data.size(); i++) {
    printf("%02x", cache_data[i]);
  }
  printf("\n");
  #endif

  //added by kh(041216)
  //intra-compression
  //hpcl_dict<unsigned char>* loc_dict;
  //loc_dict = new hpcl_dict<unsigned char>(128, HPCL_LFU);
  //loc_dict->clear();
  ///

  std::vector<std::vector<unsigned char> > uncompdata;

  for(unsigned i = 0; i < word_list.size(); i++) {
      comp_bit_size++;		//flag(1 bit)
      K word = word_list[i];
      int word_index = m_loc_dict->search_word(word);
      if(word_index >= 0) {
	comp_bit_size += data_ptr_bits;

	#ifdef DEBUG_LWM
	std::cout << "\t";
	m_loc_dict->print_word(word);
	std::cout << " is found!" << std::endl;
	#endif

	//added by kh(042116)
	m_loc_dict->update_dict(word, 0);
	///

	//added by kh(042216)
	mf->push_compword(sizeof(K), word, mem_fetch::COMPRESSED);
	///

      } else {
	comp_bit_size += (sizeof(K)*8);
	m_loc_dict->update_dict(word, 0);

	#ifdef DEBUG_LWM
	std::cout << "\t";
	m_loc_dict->print_word(word);
	std::cout << " is not found!" << std::endl;
	#endif

	//added by kh(042216)
	mf->push_compword(sizeof(K), word, mem_fetch::UNCOMPRESSED);
	///

	std::set<unsigned char> word1B_list;
	hpcl_comp_lwm_pl_proc<K>::decompose_data(word, word1B_list);

	std::set<unsigned char>::iterator it;
	for(it = word1B_list.begin(); it != word1B_list.end(); ++it) {
	  m_loc_dict_for_uncomp_data->update_dict(*it, 0);
	}

	std::vector<unsigned char> tmp;
	hpcl_comp_lwm_pl_proc<K>::decompose_data(word, tmp);
	uncompdata.push_back(tmp);
      }
  }

  unsigned comp_byte_size = ceil((double)comp_bit_size/8);
  mf->set_comp_data_size(comp_byte_size);

  #ifdef DEBUG_LWM
  if(comp_byte_size < mf->get_real_data_size()) {
    printf("access_type %u org_data = 0x", mf->get_access_type());
    for(unsigned i = 0; i < cache_data.size(); i++) {
      printf("%02x", cache_data[i]);
    }
    printf("\n");
    printf("org_data_size = %d, comp_data_size = %d\n", mf->get_real_data_size(), comp_byte_size);
    m_loc_dict->print();
  }
  #endif

  //added by kh(042116)
  //m_loc_dict->print();
  double zeros_rate = 0;
  double nonzeros_rate = 0;
  double uni_val_rate = 0;
  m_loc_dict->get_word_redun_type_dist(zeros_rate, nonzeros_rate, uni_val_rate);
  if(sizeof(K) == 2) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD2B_UNI_VAL_RATE, uni_val_rate);
  } else if(sizeof(K) == 4) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD4B_UNI_VAL_RATE, uni_val_rate);
  } else if(sizeof(K) == 8) {
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_ZEROS_REP_RATE, zeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_NONZEROS_REP_RATE, nonzeros_rate);
    g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD8B_UNI_VAL_RATE, uni_val_rate);
  }
  ///

  //delete a local dictionary
  m_loc_dict->clear();

  //added by kh(042216)
  m_word1B_zeros_rate_in_uncomp_data = 0;
  m_word1B_nonzeros_rate_in_uncomp_data = 0;
  m_word1B_uni_val_rate_in_uncomp_data = 0;

  //loc_dict->print();
  m_loc_dict_for_uncomp_data->get_word_redun_type_dist(m_word1B_zeros_rate_in_uncomp_data, m_word1B_nonzeros_rate_in_uncomp_data, m_word1B_uni_val_rate_in_uncomp_data);
  //printf("word1B_rate_mf_%u = %f, %f, %f\n", mf->get_request_uid(), m_word1B_zeros_rate_in_uncomp_data, m_word1B_nonzeros_rate_in_uncomp_data, m_word1B_uni_val_rate_in_uncomp_data);


  for(unsigned i = 0; i < uncompdata.size(); i++)
  {
    for(unsigned j = 0; j < uncompdata[i].size(); j++)
    {
      int index = m_loc_dict_for_uncomp_data->search_word(uncompdata[i][j]);
      if(m_loc_dict_for_uncomp_data->get_freq(index) > 1) {
	mf->push_uncompword(sizeof(K), i, uncompdata[i][j], mem_fetch::INTRA_COMPRESSED);
      } else if(m_loc_dict_for_uncomp_data->get_freq(index) == 1) {
	mf->push_uncompword(sizeof(K), i, uncompdata[i][j], mem_fetch::UNCOMPRESSED);
      } else assert(0);
    }
  }

  m_word1B_nonzeros_no = m_loc_dict_for_uncomp_data->get_word_no_with_multi_freq();


  m_loc_dict_for_uncomp_data->clear();
  ///



  #endif
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::decompose_data(std::vector<unsigned char>& cache_data, std::vector<K>& word_list)
{
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    //printf("decomposed %u\n", i);
    for(int j = sizeof(K)-1; j >= 0; j--) {
      K tmp = cache_data[i+j];
      tmp = (tmp << (8*j));
      word_candi += tmp;
      //printf("\t%x %x %x\n", cache_data[i+j], tmp, word_candi);
    }
    word_list.push_back(word_candi);
    //printf("\tword -- ");
    //hpcl_dict_elem<K>::print_word_data(word_candi);
    //printf("\n");
  }

  //printf("word_list_size %u, test %u\n",word_list.size(), cache_data.size()/sizeof(K));

  assert(word_list.size() == cache_data.size()/sizeof(K));
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::decompose_data(K word, std::set<unsigned char>& word1B_list)
{
  //std::cout << "decompose_data ----" << std::endl;
  //hpcl_dict_elem<K>::print_word_data(word);
  for(int j = 0; j < sizeof(K); j++) {
    unsigned char word1B = (word >> (8*j)) & 0xff;
    word1B_list.insert(word1B);
    //printf("\tword1B = %02x\n", word1B);
  }
}

template <class K>
void hpcl_comp_lwm_pl_proc<K>::decompose_data(K word, std::vector<unsigned char>& word1B_list)
{
  //std::cout << "decompose_data ----" << std::endl;
  //hpcl_dict_elem<K>::print_word_data(word);
  for(int j = 0; j < sizeof(K); j++) {
    unsigned char word1B = (word >> (8*j)) & 0xff;
    word1B_list.push_back(word1B);
    //printf("\tword1B = %02x\n", word1B);
  }
}


template <class K>
void hpcl_comp_lwm_pl_proc<K>::print_word_list(std::vector<K>& word_list)
{
  for(unsigned i = 0; i < word_list.size(); i++) {
    printf("\tword%u -- ", i);
    hpcl_dict_elem<K>::print_word_data(word_list[i]);
    printf("\n");
  }
}

#endif /* HPCL_COMP_PL_PROC_H_ */
