/*
 * hpcl_decomp_pl.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_DECOMP_PL_H_
#define HPCL_DECOMP_PL_H_

#include "hpcl_decomp_pl_proc.h"

template <class K>
class hpcl_decomp_pl
{
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  std::vector<hpcl_decomp_pl_proc<K>*> m_decomp_pl_proc;
  hpcl_comp_pl_data* m_output_data;

public:
  hpcl_decomp_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_decomp_pl () {}

  void create(unsigned dict_size, enum hpcl_dict_rep_policy policy);
  void set_input_data(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_output_data();
  void reset_output_data();
  void run(unsigned long long time);
  void update_dict(K word, unsigned long long time, int nodex_index, int word_index=-1);
  void print(unsigned pl_index=0);

//added by kh(022416)
public:
  enum DECOMP_PL_STATE {
    HPCL_DECOMP_IDLE = 0,
    HPCL_DECOMP_RUN,
  };

private:
  enum DECOMP_PL_STATE m_state;

public:
  void set_state(enum DECOMP_PL_STATE state)	{	m_state = state;	}
  enum DECOMP_PL_STATE get_state()		{	return m_state;		}
///

////added by kh(022416)
//private:
//  void update_vlb(K word, unsigned long long time);
//  bool need_dict_update(K word);
//  bool get_free_entry_dict();
//  K search_victim_word();

public:

  void decompose(std::vector<unsigned char>& raw_data, std::vector<K>& word_list);

private:
  //vector<hpcl_dict_ctrl_msg<K>*> m_out_dict_ctrl_msg_list;	//msg from myself to other nodes
  //std::vector<hpcl_dict_ctrl_msg<K>*> m_in_dict_ctrl_msg_list;	//msg from other nodes to myself
  std::vector<hpcl_comp_pl_data*> m_in_dict_ctrl_msg_list;	//msg from other nodes to myself

public:
  //void print_out_hpcl_dict_ctrl_msg();
  /*
  hpcl_dict_ctrl_msg<K>* manage_out_dict_ctrl_msg();
  void manage_in_dict_ctrl_msg(hpcl_dict_ctrl_msg<K>* msg);
  */

//added by kh(022916)
//control in_msgs
public:
  void push_in_dict_ctrl_msg(hpcl_comp_pl_data* msg);
  hpcl_comp_pl_data* front_in_dict_ctrl_msg();
  void pop_in_dict_ctrl_msg();
  bool has_in_dict_ctrl_msg();

///

private:
  std::vector<hpcl_comp_pl_data*> m_input_to_decomp_proc_queue;

public:
  unsigned get_data_in_pl_size() {
    return m_input_to_decomp_proc_queue.size();
  }

  unsigned get_ctrl_msg_in_pl_size() {
    return m_in_dict_ctrl_msg_list.size();
  }

//added by kh(030316)
  std::map<unsigned, Flit*> _flits_sent_to_decomp;

//added by kh(030716)
public:
  bool active() {
    if(m_input_to_decomp_proc_queue.size() > 0)	return true;
    if(_flits_sent_to_decomp.size() > 0) return true;

    return false;
  }







///

//for shared table
/*

private:
  vector<K> m_word_queue;
  vector<int> m_node_queue;

public:
  //When a flit is popped, this function is called.
  void push_word_to_queue(K word, int mc_dest);


//VLB table
//added by kh(022416)
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

};

template <class K>
hpcl_decomp_pl<K>::hpcl_decomp_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_output_data = new hpcl_comp_pl_data;

  //added by kh(022516)
  m_state= HPCL_DECOMP_IDLE;
  m_id = id;
}

template <class K>
void hpcl_decomp_pl<K>::create(unsigned dict_size, enum hpcl_dict_rep_policy policy)
{
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_decomp_pl_proc.push_back(new hpcl_decomp_pl_proc<K>(dict_size, policy, m_id));
    m_decomp_pl_proc[i]->set_pl_index(i);
  }
  for(unsigned i = 0; i < (m_pipeline_no-1); i++) {
    m_decomp_pl_proc[i]->set_output(m_decomp_pl_proc[i+1]->get_input());
    m_decomp_pl_proc[i]->set_pl_type(hpcl_decomp_pl_proc<K>::DECOMP);
  }

  //set output to external system (gpgpu-sim)
  m_decomp_pl_proc[0]->set_output(new hpcl_comp_pl_data, 1);
  m_decomp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data, 1);
  m_decomp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_decomp_pl_proc<K>::EJECT_DECOMP_FLIT);

  //022716
  //create_vlb(dict_size, policy);
}

template <class K>
void hpcl_decomp_pl<K>::set_input_data(hpcl_comp_pl_data* input_data) {
  //deleted by kh(030216)
  //m_decomp_pl_proc[0]->get_input()->copy(input_data);

  //added by kh(030216)
  //std::cout << "set_input_data starts" << std::endl;

//  m_input_to_decomp_proc_queue.push_back(input_data);
//  Flit* f = input_data->get_flit_ptr();
//  for(int i = 0; i < f->compressed_other_flits.size(); i++)
//  {
//    Flit* of = f->compressed_other_flits[i];
//    hpcl_comp_pl_data* pl_data = new hpcl_comp_pl_data;
//    pl_data->add_comp_pl_data(of);
//    m_input_to_decomp_proc_queue.push_back(pl_data);
//  }

  //added by kh(030216)
  //change the order of flits. Push a tail flit last due to latency computation and pakcet ejection
  Flit* f = input_data->get_flit_ptr();
  if(f->type == Flit::CTRL_MSG) {

    //hpcl_dict_ctrl_msg<unsigned short>* ctrl_msg = (hpcl_dict_ctrl_msg<unsigned short>*)f->data;
    //ctrl_msg->print();
    //assert(g_hpcl_global_decomp_pl_2B[n]);
    //push_in_dict_ctrl_msg(input_data);
    //delete input_data;

    //std::cout << "Flit " << f->id << " type " << f->type << " is inserted to decomp" << std::endl;
    m_input_to_decomp_proc_queue.push_back(input_data);

  } else {

    if(f->tail == true && f->compressed_other_flits.size() > 0) {
      for(int i = 0; i < f->compressed_other_flits.size(); i++)
      {
	Flit* of = f->compressed_other_flits[i];
	assert(of);
	assert(f->dest == of->dest);
	hpcl_comp_pl_data* pl_data = new hpcl_comp_pl_data;
	pl_data->add_comp_pl_data(of);
	m_input_to_decomp_proc_queue.push_back(pl_data);
	//std::cout << "set_input_data: flit " << of->id << " type " << of->type << " tail " << of->tail << std::endl;
      }
      m_input_to_decomp_proc_queue.push_back(input_data);
      //std::cout << "set_input_data: flit " << input_data->get_flit_ptr()->id << " type " << input_data->get_flit_ptr()->type << " tail " << input_data->get_flit_ptr()->tail << std::endl;
    } else {
      m_input_to_decomp_proc_queue.push_back(input_data);
    }
    ///

    //added by kh(022516)
    //m_state= HPCL_DECOMP_RUN;
    //if(has_in_dict_ctrl_msg() == true)	m_state = HPCL_DECOMP_IDLE;
    //else				m_state	= HPCL_DECOMP_RUN;


  }


  //std::cout << "set_input_data ends" << std::endl;
}

template <class K>
hpcl_comp_pl_data* hpcl_decomp_pl<K>::get_output_data() {
  return m_output_data;
}

template <class K>
void hpcl_decomp_pl<K>::reset_output_data() {
  m_output_data->clean();
}

template <class K>
void hpcl_decomp_pl<K>::run(unsigned long long time)
{

 /*
  if(m_state == HPCL_DECOMP_IDLE) {

    if(has_in_dict_ctrl_msg() == true) {

      std::cout << "hpcl_decomp_pl - Flit " << front_in_dict_ctrl_msg()->get_flit_ptr()->id << " out!!!" << std::endl;

      m_output_data->copy(front_in_dict_ctrl_msg());
      hpcl_dict_ctrl_msg<unsigned short>* ctrl_msg = (hpcl_dict_ctrl_msg<unsigned short>*)front_in_dict_ctrl_msg()->get_flit_ptr()->data;
      std::cout << " has in_dict_ctrl_msg " << " src " << ctrl_msg->get_src_node() << std::endl;
      assert(ctrl_msg->get_word_index() >= 0);
      update_dict(ctrl_msg->get_new_word(), time, ctrl_msg->get_src_node(), ctrl_msg->get_word_index());
      delete ctrl_msg;
      pop_in_dict_ctrl_msg();
    }

//    if(has_in_dict_ctrl_msg() == true)			m_state = HPCL_DECOMP_IDLE;
//    else if(!m_input_to_decomp_proc_queue.empty())	m_state = HPCL_DECOMP_RUN;
//    else						m_state = HPCL_DECOMP_IDLE;

  } else {
*/
    //std::cout << "hpcl_decomp_pl::run starts" << std::endl;
    //added by kh (030216)
    //move hpcl_comp_data from queue to the input of the first decomp proc
    hpcl_comp_pl_data* new_input = NULL;
    if(!m_input_to_decomp_proc_queue.empty()) {
      new_input = m_input_to_decomp_proc_queue[0];
      if(new_input->get_flit_ptr()->type == Flit::CTRL_MSG) {
	if(_flits_sent_to_decomp.size() == 0) {
	  m_output_data->copy(new_input);

	  hpcl_dict_ctrl_msg<unsigned short>* ctrl_msg = (hpcl_dict_ctrl_msg<unsigned short>*)new_input->get_flit_ptr()->data;
	  assert(ctrl_msg);

	  //std::cout << " has in_dict_ctrl_msg " << " src " << ctrl_msg->get_src_node() << std::endl;
	  assert(ctrl_msg->get_word_index() >= 0);

	  //std::cout << " oops2~ " << std::endl;
	  update_dict(ctrl_msg->get_new_word(), time, ctrl_msg->get_src_node(), ctrl_msg->get_word_index());
	  //delete ctrl_msg;
	  //new_input->get_flit_ptr()->data = NULL;

	  /*
	  std::cout << "decomp_id " << m_id;
	  std::cout << " | ctrl id " << ctrl_msg->get_id();
	  std::cout << " | node_index " << ctrl_msg->get_src_node();
	  std::cout << " | dest_node " << ctrl_msg->get_dest_node();
	  std::cout << " | word_index " << ctrl_msg->get_word_index();
	  std::cout << " | word " << ctrl_msg->get_new_word() << std::endl;
	  */

	  m_input_to_decomp_proc_queue.erase(m_input_to_decomp_proc_queue.begin());
	  delete new_input;
	}
      } else {
	m_decomp_pl_proc[0]->get_input()->copy(new_input);
	//std::cout << "hpcl_decomp_pl<K>::run - flit " << new_input->get_flit_ptr()->id << std::endl;
	_flits_sent_to_decomp.insert(make_pair(new_input->get_flit_ptr()->id, new_input->get_flit_ptr()));
	m_input_to_decomp_proc_queue.erase(m_input_to_decomp_proc_queue.begin());
	delete new_input;
      }
    }
    ///

    //std::cout << "hpcl_decomp_pl::run 1" << std::endl;

    for(int i = (m_pipeline_no-1); i >= 0; i--) {
      m_decomp_pl_proc[i]->run();
      //if(pl_ret)	m_output_data = pl_ret;
      hpcl_comp_pl_data* tmp = m_decomp_pl_proc[i]->get_output(1);
      if(tmp && tmp->get_flit_ptr()) {
	m_output_data->copy(tmp);

	//std::cout << "hpcl_decomp_pl_run : flit " << tmp->get_flit_ptr()->id << " tail " << tmp->get_flit_ptr()->tail << " vc " << tmp->get_flit_ptr()->vc << std::endl;

	//added by kh(022516)
	/*
	if(tmp->get_comp_done_flag() == true) {
	    std::cout << "DECOMP COMPLETE!!!!!!!!" << std::endl;
	    m_state= HPCL_DECOMP_IDLE;
	}
	*/

	//erase flit from flit history
	Flit* f = tmp->get_flit_ptr();
	std::map<unsigned, Flit*>::iterator it = _flits_sent_to_decomp.find(f->id);
	if(it == _flits_sent_to_decomp.end()) {
	  std::cout << time << " | Error: Flit " << f->id << " type " << f->type << std::endl;
	}

	assert(it != _flits_sent_to_decomp.end());
	_flits_sent_to_decomp.erase(it);
	///

	m_decomp_pl_proc[i]->reset_output(1);
      }
    }



/*
    //added by kh(030316)
    if(m_input_to_decomp_proc_queue.empty())	m_state = HPCL_DECOMP_IDLE;
    else 					m_state = HPCL_DECOMP_RUN;
    ///

  }
*/




}

template <class K>
void hpcl_decomp_pl<K>::update_dict(K word, unsigned long long time, int node_index, int word_index) {
  for(unsigned i = 0; i < m_pipeline_no; i++) {
    m_decomp_pl_proc[i]->update_dict(word, time, node_index, word_index);
  }
}

template <class K>
void hpcl_decomp_pl<K>::print(unsigned pl_index) {
  m_decomp_pl_proc[pl_index]->print_dict();
}

/*
template<class K>
void hpcl_decomp_pl<K>::push_word_to_queue(K word, int node_index) {
  m_word_queue.push_back(word);
  m_node_queue.push_back(node_index);
}

template<class K>
hpcl_dict_ctrl_msg<K>* hpcl_decomp_pl<K>::manage_out_dict_ctrl_msg() {

  if(m_out_dict_ctrl_msg_list.size() == 0)	return NULL;

  hpcl_dict_ctrl_msg<K>* msg = m_out_dict_ctrl_msg_list[0];
  m_out_dict_ctrl_msg_list.erase(m_out_dict_ctrl_msg_list.begin());

//  if(in_msg) {
//      assert(in_msg == msg);
//      assert(in_msg->get_type() != hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_REQ);
//  }

  if(msg->get_type() == hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_REQ) {
    //Do nothing
  } else if(msg->get_type() == hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_ACK) {
    msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_REPLACE);
  } else if(msg->get_type() == hpcl_dict_ctrl_msg<K>::HPCL_UPDATE) {
    //Do nothing
  } else {
    assert(0);
  }

  return msg;

}

template<class K>
void hpcl_decomp_pl<K>::manage_in_dict_ctrl_msg(hpcl_dict_ctrl_msg<K>* msg) {
  m_out_dict_ctrl_msg_list.push_back(msg);
}
*/

template<class K>
void hpcl_decomp_pl<K>::decompose(vector<unsigned char>& raw_data, vector<K>& word_list)
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
    /*
    printf("\t");
    hpcl_dict_elem<K>::print_word_data(word_candi);
    printf("\n");
    */
    word_list.push_back(word_candi);
  }
}


template<class K>
void hpcl_decomp_pl<K>::push_in_dict_ctrl_msg(hpcl_comp_pl_data* msg) {
  m_in_dict_ctrl_msg_list.push_back(msg);
}

template<class K>
hpcl_comp_pl_data* hpcl_decomp_pl<K>::front_in_dict_ctrl_msg() {
  return m_in_dict_ctrl_msg_list[0];
}

template<class K>
void hpcl_decomp_pl<K>::pop_in_dict_ctrl_msg() {
  m_in_dict_ctrl_msg_list.erase(m_in_dict_ctrl_msg_list.begin());
}

template<class K>
bool hpcl_decomp_pl<K>::has_in_dict_ctrl_msg() {
  return !m_in_dict_ctrl_msg_list.empty();
}

/*
template<class K>
void hpcl_decomp_pl<K>::print_out_hpcl_dict_ctrl_msg()
{
  for(int i = 0; i < m_out_dict_ctrl_msg_list.size(); i++)
  {
    m_out_dict_ctrl_msg_list[i]->print();
  }
}
*/

/*
//added by kh(022416)
template <class K>
void hpcl_decomp_pl<K>::create_vlb(unsigned dict_size, enum hpcl_dict_rep_policy policy)
{
  //Assumption: 8 MCs are used
  m_hpcl_vlb.resize(64);
  for(int i = 0; i < 64; i++) {
      m_hpcl_vlb[i] = new hpcl_dict<K>(dict_size, policy);
  }

}

template <class K>
void hpcl_decomp_pl<K>::update_vlb(K word, unsigned long long time, int node_index) {
  m_hpcl_vlb[node_index]->update_dict(word, time);
}

template <class K>
void hpcl_decomp_pl<K>::print_vlb(unsigned long long time, int node_index) {
  std::cout << "cycle - " << time << " node " << node_index << std::endl;

  m_hpcl_vlb[node_index]->print();
}

template <class K>
unsigned hpcl_decomp_pl<K>::get_freq_vlb(K word, int node_index) {
  int word_index = m_hpcl_vlb[node_index]->search_word(word);
  assert(word_index>=0);
  return m_hpcl_vlb[node_index]->get_freq(word_index);
}

template<class K>
bool hpcl_decomp_pl<K>::need_dict_update(K word, int node_index) {
  unsigned word_freq = get_freq_vlb(word, node_index);
  unsigned threshold = 2;
  if(word_freq == threshold) {
      return true;
  } else {
      return false;
  }
}

template<class K>
void hpcl_decomp_pl<K>::manage_vlb(unsigned long long time) {

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
    int free_entry = m_decomp_pl_proc[0]->get_free_entry_dict(node_index);
    if(free_entry >= 0) {	//update case
	msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_UPDATE);
	msg->set_new_word(word);
	msg->set_dest_node(node_index);
    } else {			//invalidate case
	msg->set_type(hpcl_dict_ctrl_msg<K>::HPCL_INVALIDATE_REQ);
	msg->set_victim_word(m_decomp_pl_proc[0]->search_victim_word(node_index));
	msg->set_new_word(word);
	msg->set_dest_node(node_index);
    }
    m_out_dict_ctrl_msg_list.push_back(msg);
  }

  //testing.
  print_vlb(time, node_index);
}

*/


#endif /* HPCL_DECOMP_PL_H_ */
