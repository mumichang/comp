/*
 * hpcl_comp_pl.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_PL_H_
#define HPCL_COMP_PL_H_

#include "hpcl_comp_pl_proc.h"
#include "hpcl_comp_pl_data.h"
#include "hpcl_dict_ctrl_msg.h"
#include <vector>

template<class K>
class hpcl_comp_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  std::vector<hpcl_comp_pl_proc<K>*> m_comp_pl_proc;
  hpcl_comp_pl_data* m_output_data;

public:
  hpcl_comp_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_pl() {}

  void create(unsigned dict_size, enum hpcl_dict_rep_policy policy);
  void set_input_data(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_output_data();
  void reset_output_data();
  void run(unsigned long long time);
  void update_dict(K word, unsigned long long time);
  void print(unsigned pl_index=0);

//added by kh(022416)
public:
  enum COMP_PL_STATE {
    HPCL_COMP_IDLE = 0,
    HPCL_COMP_RUN,
  };

private:
  enum COMP_PL_STATE m_state;

public:
  void set_state(enum COMP_PL_STATE state)	{	m_state = state;	}
  enum COMP_PL_STATE get_state()		{	return m_state;		}
///


//added by kh(022716)
//prepare word update
private:
  vector<K> m_word_queue;
  vector<int> m_node_queue;
public:
  void decompose(vector<unsigned char>& raw_data, vector<K>& word_list);
  void push_word_to_queue(K word, int node_index);
///

/*
//added by kh(022416)
//VLB table
private:
  vector<hpcl_dict<K>* > m_hpcl_vlb;
public:
  void create_vlb(unsigned dict_size, enum hpcl_dict_rep_policy policy);
  void manage_vlb(unsigned long long time);
  void update_vlb(K word, unsigned long long time, int node_index);
  bool need_dict_update(K word, int node_index);
  unsigned get_freq_vlb(K word, int node_index);
  void print_vlb(unsigned long long time, int node_index);
///
*/

//added by kh(022816)
//ctrl msg
private:
  vector<hpcl_dict_ctrl_msg<K>*> m_in_dict_ctrl_msg_list;
  vector<hpcl_dict_ctrl_msg<K>*> m_out_dict_ctrl_msg_list;
public:
  hpcl_dict_ctrl_msg<K>* front_out_dict_ctrl_msg();
  bool has_out_dict_ctrl_msg();
  void pop_out_dict_ctrl_msg();
///

//added by kh(022816)
private:
  vector<pair<pair<int,int>,K> > m_missing_word;	//first: (src,dest), second: word


public:
  void manage_out_dict_ctrl_msg(unsigned long long cycle, int node_index);


//added by kh(030216)
private:
  bool m_comp_done;
public:
  bool get_comp_done()		{	return m_comp_done;	}
  void reset_comp_done()	{	m_comp_done = false;	}
  void set_comp_done()		{	m_comp_done = true;	}



//added by kh(030716)
public:
  bool active() {
    if(m_out_dict_ctrl_msg_list.size() > 0)	return true;
    if(m_missing_word.size() > 0)		return true;
    if(m_state == HPCL_COMP_RUN)		return true;

    return false;
  }


  void reset_ctrl_msg() {
    for(int i = 0; i < m_out_dict_ctrl_msg_list.size(); i++) {
      if(m_out_dict_ctrl_msg_list[i]) {
	std::cout << "ooops!" << std::endl;
	K word = m_out_dict_ctrl_msg_list[i]->get_new_word();
	int node_index = m_out_dict_ctrl_msg_list[i]->get_dest_node();

	for(unsigned j = 0; j < m_pipeline_no; j++) {
	  m_comp_pl_proc[j]->erase_dict(word, node_index);
	}

	delete m_out_dict_ctrl_msg_list[i];
      }
    }

    m_out_dict_ctrl_msg_list.clear();
    m_missing_word.clear();
  }
///



//added by kh(030816)
public:
  void validate_word(int node_index, int word_index, K word);
  void hold_word(int node_index, int word_index, K word);
  void release_word(int node_index, int word_index, K word);
///

};

template<class K>
hpcl_comp_pl<K>::hpcl_comp_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_output_data = new hpcl_comp_pl_data;

  //added by kh(022416)
  m_state = HPCL_COMP_IDLE;

  //added by kh(030216)
  m_comp_done = false;

  m_id = id;
}

template<class K>
void hpcl_comp_pl<K>::create(unsigned dict_size, enum hpcl_dict_rep_policy policy) {
  for(unsigned i = 0; i < m_pipeline_no; i++) {
	m_comp_pl_proc.push_back(new hpcl_comp_pl_proc<K>(dict_size, policy, m_id));
	m_comp_pl_proc[i]->set_pl_index(i);
  }
  for(unsigned i = 0; i < (m_pipeline_no-1); i++) {
	m_comp_pl_proc[i]->set_output(m_comp_pl_proc[i+1]->get_input());
  }
  for(unsigned i = 0; i < m_pipeline_no-2; i++)	m_comp_pl_proc[i]->set_pl_type(hpcl_comp_pl_proc<K>::COMP);
  m_comp_pl_proc[m_pipeline_no-2]->set_pl_type(hpcl_comp_pl_proc<K>::INJECT_UNCOMP_FLIT);
  m_comp_pl_proc[m_pipeline_no-2]->set_output(new hpcl_comp_pl_data, 1);
  m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_pl_proc<K>::INJECT_COMP_FLIT);
  m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data, 1);

  //022716
  //create_vlb(dict_size, policy);
}

template<class K>
void hpcl_comp_pl<K>::set_input_data(hpcl_comp_pl_data* input_data) {
  m_comp_pl_proc[0]->get_input()->copy(input_data);

  //added by kh(022416)
  if(input_data)	m_state = HPCL_COMP_RUN;
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_pl<K>::get_output_data() {
  return m_output_data;
}

template<class K>
void hpcl_comp_pl<K>::reset_output_data() {
  m_output_data->clean();
}

template<class K>
void hpcl_comp_pl<K>::run(unsigned long long time) {

  //Update dictionary
  if(m_state == HPCL_COMP_IDLE) {

    if(m_missing_word.size() > 0) {
      int src_node_index = m_missing_word[0].first.first;
      int dest_node_index = m_missing_word[0].first.second;

      K word = m_missing_word[0].second;
      m_missing_word.erase(m_missing_word.begin());
      //std::cout << "\terased!!" << std::endl;
      for(int i = (m_pipeline_no-1); i >= 0; i--) {
	m_comp_pl_proc[i]->update_dict(word, time, dest_node_index);
      }

      //std::cout << "update word !!" << word <<  " dest_node " << dest_node_index << " std::endl;

      //although a word is just updated, a dict may not have space such that the word cannot be found.
      int word_index = m_comp_pl_proc[0]->search_word_dict(dest_node_index, word);
      if(word_index >= 0) {
	int word_freq = m_comp_pl_proc[0]->get_word_freq(dest_node_index, word_index);
	K word_bak = m_comp_pl_proc[0]->get_word_dict(dest_node_index, word_index);
	assert(word==word_bak);

	int threshold = 2;
	if(word_freq == threshold) {	//use equal to sent a ctrl msg once.

	  //added by kh(030816)
	  for(int i = (m_pipeline_no-1); i >= 0; i--) {
	      m_comp_pl_proc[i]->hold_word(dest_node_index, word_index, word);
	  }
	  ///

	  hpcl_dict_ctrl_msg<K>* msg = new hpcl_dict_ctrl_msg<K>;
	  msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_PRIVATE_UPDATE);
	  msg->set_new_word(word);
	  msg->set_dest_node(dest_node_index);
	  msg->set_src_node(src_node_index);
	  msg->set_word_index(word_index);
	  m_out_dict_ctrl_msg_list.push_back(msg);

	  /*
	  std:cout << " Word " << word << " dest_node_index " << dest_node_index << " src_node_index " << src_node_index;
	  std::cout << " word_index " << word_index << std::endl;
	  */
	}
      }
    }
  } else {

    //Pipelined-Compression
    //m_comp_done = false;
    for(int i = (m_pipeline_no-1); i >= 0; i--) {
      m_comp_pl_proc[i]->run();
      hpcl_comp_pl_data* tmp = m_comp_pl_proc[i]->get_output(1);
      if(tmp && tmp->get_flit_ptr()) {
	m_output_data->copy(tmp);
	if(tmp->get_comp_done_flag() == true) {
	  //std::cout << "COMPLETE!!!!!!!!" << std::endl;
	  m_comp_done = true;
	  //m_state= HPCL_COMP_IDLE;
	  //comp_done = true;
	}

	Flit* flit = tmp->get_flit_ptr();
	if(flit->tail == true && tmp->get_comp_done_flag() == false)
	{
	  //other flits are compressed, and they are sent next cycle.
	  //reset an original tail flit as a body flit
	  flit->tail = false;
	} else if(flit->tail == true && tmp->get_comp_done_flag() == true) {
	  //do nothing
	} else if(flit->tail == false && tmp->get_comp_done_flag() == false) {
	  //do nothing
	} else if(flit->tail == false && tmp->get_comp_done_flag() == true) {
	  //a flit compressing all flits is sent.
	  //an original tail flit could be there.
	  //The firstly compressed flit is sent to a final destionations.
	  //Set its tail field as one.
	  flit->tail = true;
	  //copy other compressed flits to the first flit
	  for(int i = 1; i < tmp->get_flit_no(); i++) {
	    Flit* f = tmp->get_flit_ptr(i);
	    assert(f);
	    f->tail = false;
	    flit->compressed_other_flits.push_back(tmp->get_flit_ptr(i));
	  }
	  //remove all compressed flits when a new tail arrives in a final destinations.
	}

	/*
	//multiple flits could be compressed. The firstly compressed flit is sent here.
	//Thus, it is necessary to set tail field of the flit as one.
	Flit* compressed_flit = m_output[0]->get_flit_ptr(0);
	      compressed_flit->tail = true;
	      //free other flits
	      for(int i = 1; i < m_output[0]->get_flit_no(); i++) {
		Flit* f = m_output[0]->get_flit_ptr(i);
		free_flit(f);
	      }
	*/

	m_comp_pl_proc[i]->reset_output(1);

	/*
	for(int i = 0; i < tmp->get_flit_no(); i++) {
	  Flit* flit = tmp->get_flit_ptr(i);
	  std::cout << "\tflit " << flit->id << " is compressed!!!" << std::endl;
	}
	*/
      }
    }

    //added by kh(022816)
    //move missing word list from hpcl_comp_pl_proc to hpcl_comp_pl
    if(m_comp_done == true) {
      for(int i = (m_pipeline_no-1); i >= 0; i--) {
	int missing_word_size = m_comp_pl_proc[i]->get_missing_word_size();
	//std::cout << "\tmissing_word_size " << missing_word_size << std::endl;
	for(int j = 0; j < missing_word_size; j++) {
	    m_missing_word.push_back(m_comp_pl_proc[i]->get_missing_word(j));
	}
	m_comp_pl_proc[i]->clear_missing_word();
	//std::cout << "\tm_missing_word_size " << m_missing_word.size() << std::endl;
	///
      }
    }



  }

}

template<class K>
void hpcl_comp_pl<K>::update_dict(K word, unsigned long long time) {

  std::cout << "hpcl_comp_pl<K>::update_dict called " << std::endl;

  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_comp_pl_proc[i]->update_dict(word, time);
  }

}

template<class K>
void hpcl_comp_pl<K>::print(unsigned pl_index) {
  m_comp_pl_proc[pl_index]->print_dict();
}

template<class K>
void hpcl_comp_pl<K>::decompose(vector<unsigned char>& raw_data, vector<K>& word_list)
{
  for(unsigned i = 0; i < raw_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    for(int j = sizeof(K)-1; j >= 0; j--) {
    	K tmp = raw_data[i+j];
    	tmp = (tmp << (8*j));
    	word_candi += tmp;
    }
    /*
    for(int j = 0; j < sizeof(K); j++) {
    K tmp = data[i+j];
    tmp = (tmp << (8*(sizeof(K)-1-j)));
    word_candi += tmp;
    }
    */
    word_list.push_back(word_candi);
  }
}



template<class K>
void hpcl_comp_pl<K>::push_word_to_queue(K word, int node_index) {
  m_word_queue.push_back(word);
  m_node_queue.push_back(node_index);


  //std::cout << "\tword " << word << " node_index " << node_index << std::endl;

}


#ifdef old
//added by kh(022416)
template <class K>
void hpcl_comp_pl<K>::create_vlb(unsigned dict_size, enum hpcl_dict_rep_policy policy)
{
  //Assumption: 8 MCs are used
  m_hpcl_vlb.resize(64);
  for(int i = 0; i < 64; i++) {
      m_hpcl_vlb[i] = new hpcl_dict<K>(dict_size, policy);
  }
}

template<class K>
void hpcl_comp_pl<K>::manage_vlb(unsigned long long time) {

  if(m_word_queue.size() == 0)	return;

  K word = m_word_queue[0];
  m_word_queue.erase(m_word_queue.begin());

  int node_index = m_node_queue[0];
  m_node_queue.erase(m_node_queue.begin());

  //Step1: update VLB table
  update_vlb(word, time, node_index);

//  printf("\t");
//  hpcl_dict_elem<K>::print_word_data(word);
//  printf(" is updated!! time %llu, size %d\n", time, m_word_queue.size());

  //Step2: update dictionary update msg
  if(need_dict_update(word, node_index) == true) {
    hpcl_dict_ctrl_msg<K>* msg = new hpcl_dict_ctrl_msg<K>;
    int free_entry = m_comp_pl_proc[0]->get_free_entry_dict(node_index);
    msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_PRIVATE_UPDATE);
    msg->set_new_word(word);
    msg->set_dest_node(node_index);
    /*
    if(free_entry >= 0) {	//update case
	msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_PRIVATE_UPDATE);
	msg->set_new_word(word);
	msg->set_dest_node(node_index);
    } else {			//invalidate case
	msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_REQ);
	//msg->set_victim_word(m_comp_pl_proc[0]->search_victim_word(node_index));
	msg->set_new_word(word);
	msg->set_dest_node(node_index);
    }
    */
    m_out_dict_ctrl_msg_list.push(msg);
  }

  //testing.
  print_vlb(time, node_index);
}

template <class K>
void hpcl_comp_pl<K>::update_vlb(K word, unsigned long long time, int node_index) {
//  std::cout << "node_index " << node_index << std::endl;
  m_hpcl_vlb[node_index]->update_dict(word, time);
}

template <class K>
void hpcl_comp_pl<K>::print_vlb(unsigned long long time, int node_index) {
  std::cout << "cycle - " << time << " node " << node_index << std::endl;

  m_hpcl_vlb[node_index]->print();
}

template <class K>
unsigned hpcl_comp_pl<K>::get_freq_vlb(K word, int node_index) {
  int word_index = m_hpcl_vlb[node_index]->search_word(word);
  assert(word_index>=0);
  return m_hpcl_vlb[node_index]->get_freq(word_index);
}

template<class K>
bool hpcl_comp_pl<K>::need_dict_update(K word, int node_index) {
  unsigned word_freq = get_freq_vlb(word, node_index);
  unsigned threshold = 2;
  if(word_freq == threshold) {
      return true;
  } else {
      return false;
  }
}
#endif

template<class K>
bool hpcl_comp_pl<K>::has_out_dict_ctrl_msg() {
  return !m_out_dict_ctrl_msg_list.empty();
}

template<class K>
hpcl_dict_ctrl_msg<K>* hpcl_comp_pl<K>::front_out_dict_ctrl_msg() {
  return m_out_dict_ctrl_msg_list[0];
}

template<class K>
void hpcl_comp_pl<K>::pop_out_dict_ctrl_msg() {
  m_out_dict_ctrl_msg_list.erase(m_out_dict_ctrl_msg_list.begin());
}


template<class K>
void hpcl_comp_pl<K>::manage_out_dict_ctrl_msg(unsigned long long cycle, int node_index)
{
  std::cout << cycle << " | manage_out_dict_ctrl_msg @ node " << node_index << std::endl;

  for(int i = 0; i < m_out_dict_ctrl_msg_list.size(); i++)
  {
      m_out_dict_ctrl_msg_list[i]->print();
  }
}

template<class K>
void hpcl_comp_pl<K>::validate_word(int node_index, int word_index, K word) {
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_comp_pl_proc[i]->validate_word(node_index, word_index, word);
  }
}

template<class K>
void hpcl_comp_pl<K>::hold_word(int node_index, int word_index, K word) {
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_comp_pl_proc[i]->hold_word(node_index, word_index, word);
  }
}

template<class K>
void hpcl_comp_pl<K>::release_word(int node_index, int word_index, K word) {
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_comp_pl_proc[i]->release_word(node_index, word_index, word);
  }
}

#endif /* HPCL_COMP_PL_H_ */
