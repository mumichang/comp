/*
 * hpcl_comp_pl.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_LWM_PL_H_
#define HPCL_COMP_LWM_PL_H_

#include "hpcl_comp_lwm_pl_proc.h"
#include "hpcl_comp_pl_data.h"
//#include "hpcl_dict_ctrl_msg.h"
#include <vector>

template<class K>
class hpcl_comp_lwm_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  std::vector<hpcl_comp_lwm_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;

public:
  hpcl_comp_lwm_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_lwm_pl() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }

  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run();
  void print(unsigned pl_index=0);

  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();

  //added by kh(031916)
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();
  ///

  //added by kh(042216)
private:
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

template<class K>
hpcl_comp_lwm_pl<K>::hpcl_comp_lwm_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_comp_buffer_size = -1;
}

template<class K>
void hpcl_comp_lwm_pl<K>::create(unsigned comp_buffer_size) {
  //hpcl_comp_lwm_pl_proc<K>* tmp = new hpcl_comp_lwm_pl_proc();
  m_comp_pl_proc.push_back(new hpcl_comp_lwm_pl_proc<K>());
  m_comp_pl_proc[0]->set_pl_index(0);
  m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
  m_input = m_comp_pl_proc[0]->get_input();
  m_comp_buffer_size = comp_buffer_size;
}

template<class K>
void hpcl_comp_lwm_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_lwm_pl<K>::get_input() {
  return m_input;
}

template<class K>
mem_fetch* hpcl_comp_lwm_pl<K>::run()
{
  m_comp_pl_proc[0]->run();
  m_comp_pl_proc[0]->get_word1B_redund_stat_in_uncomp_data(m_word1B_zeros_rate_in_uncomp_data, m_word1B_nonzeros_rate_in_uncomp_data, m_word1B_uni_val_rate_in_uncomp_data, m_word1B_nonzeros_no);

  hpcl_comp_pl_data* tmp = m_comp_pl_proc[0]->get_output();
  assert(tmp);
  mem_fetch* mf = tmp->get_mem_fetch();
  if(mf) {
    //deleted by kh(041816)
    /*
    m_comp_buffer.push_back(tmp->get_mem_fetch());
    //std::cout << "hpcl_comp_lwm_pl | " << m_id << " " << m_comp_buffer.size() << std::endl;
    assert(m_comp_buffer.size() <= m_comp_buffer_size);
    */
  }
  tmp->clean();

  //added by kh(041816)
  return mf;
}

template<class K>
void hpcl_comp_lwm_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_lwm_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_lwm_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_lwm_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
