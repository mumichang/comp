
// added by abpd

#ifndef HPCL_COMP_BDI_PL_H_
#define HPCL_COMP_BDI_PL_H_

#include "hpcl_comp_bdi_pl_proc.h"
#include "hpcl_comp_pl_data.h"
#include <vector>

template<class K>
class hpcl_comp_bdi_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_bdi_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;
  vector<pair<string,Blocker*> >tracker;

  
public:

  hpcl_comp_bdi_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_bdi_pl() {
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
  void init_blocks();
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();

private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();

};
template<class K>
void hpcl_comp_bdi_pl<K>::init_blocks()
{
	tracker.clear();

	Blocker *ob1 = new Blocker("0010","base8d1",8,1,12);
	tracker.push_back(make_pair("base8d1",ob1));
	Blocker *ob2 = new Blocker("0011","base8d4",8,2,24);
	tracker.push_back(make_pair("base8d4",ob2));
	Blocker *ob3 = new Blocker("0100","base8d2",8,4,16);
	tracker.push_back(make_pair("base8d2",ob3));


	Blocker *ob4 = new Blocker("0101","base4d2",4,1,20);
	tracker.push_back(make_pair("base4d2",ob4));

	Blocker *ob5 = new Blocker("0110","base4d1",4,2,12);
	tracker.push_back(make_pair("base4d1",ob5));

	Blocker *ob6 = new Blocker("0111","base2d1",2,1,18);
	tracker.push_back(make_pair("base2d1",ob6));

}
template<class K>
hpcl_comp_bdi_pl<K>::hpcl_comp_bdi_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_comp_buffer_size = -1;
}

template<class K>
void hpcl_comp_bdi_pl<K>::create(unsigned comp_buffer_size) {
  m_comp_pl_proc.push_back(new hpcl_comp_bdi_pl_proc<K>());
  m_comp_pl_proc.push_back(new hpcl_comp_bdi_pl_proc<K>());
  /* can be used cycle */
  m_comp_pl_proc[0]->set_pl_index(0);
  m_comp_pl_proc[1]->set_pl_index(1);
  m_input = m_comp_pl_proc[0]->get_input();
  m_comp_pl_proc[0]->set_output(new hpcl_comp_pl_data);
  m_comp_pl_proc[1]->set_output(new hpcl_comp_pl_data);
  m_comp_buffer_size = comp_buffer_size;
  m_comp_pl_proc[0]->fetch_data=true;

}

template<class K>
void hpcl_comp_bdi_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_bdi_pl<K>::get_input() {
  return m_input;
}
template<class K>
mem_fetch* hpcl_comp_bdi_pl<K>::run() {
  mem_fetch* ret = NULL;
	
	if(m_comp_pl_proc[1]->execute_data)
	{
		m_comp_pl_proc[1]->run(tracker);
		hpcl_comp_pl_data* tmp = m_comp_pl_proc[1]->get_output();
  		assert(tmp);
  		mem_fetch* mf = tmp->get_mem_fetch();
		
		m_comp_pl_proc[1]->execute_data = false;
		m_comp_pl_proc[0]->fetch_data = true;
		//if(mf) {

	  	// push data to memory buffer
    		//m_comp_buffer.push_back(tmp->get_mem_fetch());
    		//assert(m_comp_buffer.size() <= m_comp_buffer_size);
  		//}
  		tmp->clean();
  		ret = mf;
	}
  	if(m_comp_pl_proc[0]->fetch_data)
  	{
		 mem_fetch* mf = m_comp_pl_proc[0]->get_mf_input();
		 if(mf) {
		 	m_comp_pl_proc[1]->execute_data = true;
		 	m_comp_pl_proc[0]->fetch_data=false;
		 	//m_comp_pl_proc[1]->set_output(new hpcl_comp_pl_data);
		 	m_comp_pl_proc[1]->set_mf_input(mf);
		}


  	}

  	return ret;
	
}

template<class K>
void hpcl_comp_bdi_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_bdi_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_bdi_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_bdi_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
