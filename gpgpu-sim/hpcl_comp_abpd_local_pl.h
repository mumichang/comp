
// added by abpd

#ifndef HPCL_COMP_ABPD_LOCAL_PL_H_
#define HPCL_COMP_ABPD_LOCAL_PL_H_

#include "hpcl_comp_abpd_local_pl_proc.h"
#include "hpcl_comp_pl_data.h"
#include <vector>

template<class K>
class hpcl_comp_abpd_local_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_abpd_local_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;

  
public:

  hpcl_comp_abpd_local_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_abpd_local_pl() {
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
  //void print(unsigned pl_index=0);
 // void init_blocks();
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();

  //added by kh(031916)
private:
  unsigned m_comp_buffer_size;
  bool pipe_flag;	//PD
public:
  bool has_comp_buffer_space();

};
template<class K>
hpcl_comp_abpd_local_pl<K>::hpcl_comp_abpd_local_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_comp_buffer_size = -1;
}

template<class K>
void hpcl_comp_abpd_local_pl<K>::create(unsigned comp_buffer_size) {
	
  m_comp_pl_proc.push_back(new hpcl_comp_abpd_local_pl_proc<K>());
 /* can be used cycle */
  m_comp_pl_proc[0]->set_pl_index(0);
  m_input = m_comp_pl_proc[0]->get_input();
  m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
  m_comp_buffer_size = comp_buffer_size;

}

template<class K>
void hpcl_comp_abpd_local_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_abpd_local_pl<K>::get_input() {
  return m_input;
}

template<class K>
mem_fetch* hpcl_comp_abpd_local_pl<K>::run() {

	// we have one function run that simulates one pipeline
	// we can break run into many and can simulate for multiple pipeline
	// proc[0]->get_some_data
	// proc[1]->perform_some_action

	// m_comp_pl_proc[0]->run1(tracker);
	//hpcl_comp_pl_data* uu=  m_comp_pl_proc[0]->get_output();
	// m_comp_pl_proc[1]->set_input(uu);
	// m_comp_pl_proc[1]->run2(tracker);
	// hpcl_cpm_pl_data *tmp = m_comp_pl_proc[1]->get_output();

//if(m_comp_pl_proc)

	/*if(m_comp_pl_proc[1]->execute_data)
	{*/

  mem_fetch* ret = NULL;

		//cout<<"Execute some data \n";
		m_comp_pl_proc[0]->run();
		hpcl_comp_pl_data* tmp = m_comp_pl_proc[0]->get_output();
  		assert(tmp);
  		mem_fetch* mf = tmp->get_mem_fetch();
		
		//m_comp_pl_proc[1]->execute_data = false;
		//m_comp_pl_proc[0]->fetch_data = true;
		 if(mf) {

	  	// push data to memory buffer
    		m_comp_buffer.push_back(tmp->get_mem_fetch());
    		assert(m_comp_buffer.size() <= m_comp_buffer_size);
  		}
  		tmp->clean();

  		ret = mf;

  		return ret;
	//}
	/*
  	if(m_comp_pl_proc[0]->fetch_data)
  	{
		 //cout<<"Fetch some data \n";
		 mem_fetch* mf = m_comp_pl_proc[0]->get_mf_input();
		 if(mf) {
			 cout<<"Fetch some data \n";
			//cout<<"Inside\n";
		 	m_comp_pl_proc[1]->execute_data = true;
		 	m_comp_pl_proc[0]->fetch_data=false;
		 	m_comp_pl_proc[1]->set_output(new hpcl_comp_pl_data);
		 	m_comp_pl_proc[1]->set_mf_input(mf);
		}


  	}*/
 // m_comp_pl_proc[0]->run(tracker);
  //hpcl_comp_pl_data* tmp = m_comp_pl_proc[0]->get_output();
  //assert(tmp);
  //mem_fetch* mf = tmp->get_mem_fetch();
	/*
  if(mf) {

	  // push data to memory buffer
    m_comp_buffer.push_back(tmp->get_mem_fetch());
    assert(m_comp_buffer.size() <= m_comp_buffer_size);
  }
  tmp->clean();
  */

}

//template<class K>
//void hpcl_comp_abpd_local_pl<K>::print(unsigned pl_index) {

//}

template<class K>
mem_fetch* hpcl_comp_abpd_local_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_abpd_local_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_abpd_local_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
