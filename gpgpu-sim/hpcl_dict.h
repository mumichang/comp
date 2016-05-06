/*
 * hpcl_dict.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DICT_H_
#define HPCL_DICT_H_

#include <vector>
#include <map>
#include "hpcl_dict_elem.h"
#include "hpcl_comp_pl_data.h"
#include <cassert>
#include <cmath>

template<class K>
class hpcl_dict {
/*
public:
	hpcl_dict();
	virtual ~hpcl_dict();
*/
public:
  hpcl_dict(unsigned int dict_size, enum hpcl_dict_rep_policy policy);
  ~hpcl_dict() {};

//compression
private:
  unsigned int m_MAX_DICT_SIZE;			//default: 256
  //unsigned int m_dict_word_size;		//default: 2B
  enum hpcl_dict_rep_policy m_rep_policy;
  std::vector<hpcl_dict_elem<K>*> m_dict;
  //std::map<K, hpcl_dict_elem<K>*> m_dict;

public:
  void init(unsigned int dict_size, enum hpcl_dict_rep_policy policy);
  int search_word(K word);
  K get_word(unsigned index);
  bool get_valid_flag(unsigned index);
  void update_dict(K word, unsigned long long use_time, int index=-1);
  void update_elem_by_LRU(K word, unsigned long long time);
  void update_elem_by_LFU(K word, unsigned long long time);
  void print();
  int get_index_bit_size();

//added by kh(022416)
  int get_freq(unsigned index);
  K search_victim_by_LFU();
  int get_free_entry();
///

  //added by kh(030716)
  void erase_dict(K word);

  //added by kh(030816)
  void validate_word(int index, K word);
  void hold_word(int index, K word);
  void release_word(int index, K word);

  //added by kh(031716)
public:
  void clear();
  ///

  //added by kh(031916)
  void print_word(K word);


  //added by kh(042116)
  void get_word_redun_type_dist(double& zeros_rate, double& nonzeros_rate, double& uni_val_rate);
  //double get_zeros_rep_rate();
  //double get_nonzeros_rep_rate();
  //double get_uni_val_rate();
  ///

  unsigned get_word_no_with_multi_freq();

};

template<class K>
hpcl_dict<K>::hpcl_dict(unsigned int dict_size, enum hpcl_dict_rep_policy policy)
{
  init(dict_size, policy);
}

template<class K>
void hpcl_dict<K>::init(unsigned int dict_size, enum hpcl_dict_rep_policy policy)
{
  m_MAX_DICT_SIZE = dict_size;
  m_rep_policy = policy;

  //added by kh(030216)
  for(int i = 0; i < m_MAX_DICT_SIZE; i++) {
    m_dict.push_back(new hpcl_dict_elem<K>(0, 0));
  }
  ///

}

template<class K>
int hpcl_dict<K>::search_word(K word)
{
  int ret = -1;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    //if(m_dict[i]->get_valid() == true && m_dict[i]->search(word) == true) {
    if(m_dict[i]->search(word) == true) {
      ret = (int) i; break;
    }
  }
  return ret;
}
template<class K>
bool hpcl_dict<K>::get_valid_flag(unsigned index) {
  return m_dict[index]->get_valid();
}

template<class K>
K hpcl_dict<K>::get_word(unsigned index)
{
  assert(m_dict.size() > index);
  //assert(m_dict[index]->get_valid() == true);
  return m_dict[index]->get_word();
}

template<class K>
void hpcl_dict<K>::erase_dict(K word)
{
  int index = search_word(word);
  assert(index>=0);
  m_dict[index]->clear_valid();
}

template<class K>
void hpcl_dict<K>::validate_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->set_valid();
}

template<class K>
void hpcl_dict<K>::hold_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->set_hold_flag();
}

template<class K>
void hpcl_dict<K>::release_word(int index, K word)
{
  assert(m_dict[index]->get_word() == word);
  m_dict[index]->clear_hold_flag();
}

template<class K>
void hpcl_dict<K>::update_dict(K word, unsigned long long use_time, int index)
{
  if(index >= 0) {
    //store word in the specified index
    //std::cout << "update_dict word " << word << " word_index " << index << " m_dict.size() " << m_dict.size() << std::endl;
    m_dict[index]->init(word, use_time);
    //m_dict[index]->set_valid();

  } else {
    bool found = false;
    for(unsigned i = 0; i < m_dict.size(); i++) {
      //if(m_dict[i]->get_valid() == true && m_dict[i]->search(word) == true) {
      if(m_dict[i]->search(word) == true) {
	m_dict[i]->update_last_use_time(use_time);
	m_dict[i]->update_freq();
	found = true;
	break;
      }
    }

    if(found == false) {
      if(m_dict.size() < m_MAX_DICT_SIZE) {
	hpcl_dict_elem<K> * new_elem = new hpcl_dict_elem<K>(word, use_time);
	m_dict.push_back(new_elem);
	//assert(0);
      } else {
	if(m_rep_policy == HPCL_LRU)	      	update_elem_by_LRU(word, use_time);
	else if(m_rep_policy == HPCL_LFU)  	update_elem_by_LFU(word, use_time);
	else					assert(0);
      }
    }
  }



}

template<class K>
void hpcl_dict<K>::update_elem_by_LRU(K word, unsigned long long time)
{
  unsigned long long last_use_time = m_dict[0]->get_last_use_time();
  unsigned oldest_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_last_use_time() < last_use_time) {
      oldest_index = i;
    }
  }

  /*
  printf("index %u, ", oldest_index);
  printf("old_word ");
  m_dict[oldest_index]->print_word();
  */
  m_dict[oldest_index]->init(word, time);
  //m_dict[oldest_index]->set_valid();
  /*
  printf(" --> new_word ");
  m_dict[oldest_index]->print_word();
  printf("\n");
  */
}

template<class K>
void hpcl_dict<K>::update_elem_by_LFU(K word, unsigned long long time)
{
  unsigned long long freq = m_dict[0]->get_freq();
  unsigned least_used_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() < freq && m_dict[i]->get_hold_flag() == false) {
      least_used_index = i;
    }
  }
  /*
  printf("index %u, ", least_used_index);
  printf("old_word ");
  m_dict[least_used_index]->print_word();
  */
  m_dict[least_used_index]->init(word, time);
  //m_dict[least_used_index]->set_valid();
  /*
  printf(" --> new_word ");
  m_dict[least_used_index]->print_word();
  printf("\n");
  */
}

template<class K>
void hpcl_dict<K>::print()
{
  for(int i = 0; i < m_dict.size(); i++) {
    printf("index = %d ", i);
    m_dict[i]->print();
    printf("\n");
  }
}

template<class K>
int hpcl_dict<K>::get_index_bit_size() {
  return (int)log2(m_MAX_DICT_SIZE);
}

template<class K>
int hpcl_dict<K>::get_freq(unsigned index) {
  return m_dict[index]->get_freq();
}

template<class K>
K hpcl_dict<K>::search_victim_by_LFU()
{
  unsigned long long freq = m_dict[0]->get_freq();
  K victim_word = m_dict[0]->get_word();
  unsigned least_used_index = 0;
  for(unsigned i = 1; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() < freq) {
      least_used_index = i;
      victim_word = m_dict[i]->get_word();
    }
  }
  return victim_word;
}

template<class K>
int hpcl_dict<K>::get_free_entry()
{
  int ret = -1;
  if(m_dict.size() < m_MAX_DICT_SIZE) {
    ret = m_dict.size();
  }
  return ret;
}

//added by kh(031716)
template<class K>
void hpcl_dict<K>::clear()
{
  for(int i = 0; i < m_dict.size(); i++) {
    delete m_dict[i];
  }
  m_dict.clear();
}
///

template<class K>
void hpcl_dict<K>::print_word(K word)
{
  if(sizeof(K) == sizeof(unsigned char)) {
    printf("%02x", word);
  } else if(sizeof(K) == sizeof(unsigned short)) {
    printf("%04x", word);
  } else if(sizeof(K) == sizeof(unsigned int)) {
    printf("%08x", word);
  } else if(sizeof(K) == sizeof(unsigned long long)) {
    printf("%016llx", word);
  }
}

template<class K>
void hpcl_dict<K>::get_word_redun_type_dist(double& zeros_rate, double& nonzeros_rate, double& uni_val_rate) {
  unsigned all_freq = 0;
  zeros_rate = 0;
  nonzeros_rate = 0;
  uni_val_rate = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
      all_freq += m_dict[i]->get_freq();
      if(m_dict[i]->get_word() == 0 && m_dict[i]->get_freq() > 1) {
	  zeros_rate += m_dict[i]->get_freq();
      } else if(m_dict[i]->get_word() != 0 && m_dict[i]->get_freq() > 1) {
	  nonzeros_rate += m_dict[i]->get_freq();
      } else if(m_dict[i]->get_freq() == 1) {
	  uni_val_rate += m_dict[i]->get_freq();
      } else	assert(0);
  }

  if(all_freq > 0) {	//to avoid -nan
    zeros_rate = zeros_rate/all_freq;
    nonzeros_rate = nonzeros_rate/all_freq;
    uni_val_rate = uni_val_rate/all_freq;
  }
}

template<class K>
unsigned hpcl_dict<K>::get_word_no_with_multi_freq()
{
  unsigned ret = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
    if(m_dict[i]->get_freq() > 1) ret++;
  }
  return ret;
}



/*
template<class K>
double hpcl_dict<K>::get_zeros_rep_rate() {
  unsigned all_freq = 0;
  unsigned zero_freq = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
      all_freq += m_dict[i]->get_freq();
      if(m_dict[i]->get_word() == 0 && m_dict[i]->get_freq() > 1) {
	zero_freq += m_dict[i]->get_freq();
      }
  }
  return (double)zero_freq/all_freq;
}

template<class K>
double hpcl_dict<K>::get_nonzeros_rep_rate() {
  unsigned all_freq = 0;
  unsigned nonzero_freq = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
      all_freq += m_dict[i]->get_freq();
      if(m_dict[i]->get_word() != 0 && m_dict[i]->get_freq() > 1) {
	  nonzero_freq += m_dict[i]->get_freq();
      }
  }
  return (double)nonzero_freq/all_freq;
}

template<class K>
double hpcl_dict<K>::get_uni_val_rate() {
  unsigned all_freq = 0;
  unsigned uni_val_freq = 0;
  for(unsigned i = 0; i < m_dict.size(); i++) {
      all_freq += m_dict[i]->get_freq();
      if(m_dict[i]->get_freq() == 1) {
	  uni_val_freq += m_dict[i]->get_freq();
      }
  }
  return (double)uni_val_freq/all_freq;
}
*/

#endif /* HPCL_DICT_H_ */
