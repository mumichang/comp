
// added by abpd

#ifndef HPCL_COMP_CPACK_PL_H_
#define HPCL_COMP_CPACK_PL_H_

#include "hpcl_comp_cpack_pl_proc.h"
#include "hpcl_comp_pl_data.h"
#include "hpcl_cpack_dict.h"

#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

template<class K>
class hpcl_comp_cpack_pl {
private:
  unsigned m_id;
  unsigned m_pipeline_no;
  // used for number of cycles , will try with 1 : TODO for rest
  std::vector<hpcl_comp_cpack_pl_proc<K>*> m_comp_pl_proc;

  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  vector<mem_fetch*> m_comp_buffer;

  
public:

  hpcl_comp_cpack_pl(unsigned pipeline_no, unsigned id);
  ~hpcl_comp_cpack_pl() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    if(m_glb_dict)	delete m_glb_dict;

    for(int i = 0; i < m_comp_pl_proc.size(); i++) {
	delete m_comp_pl_proc[i];
    }
  }

  void create(unsigned buffer_size);
  void set_input(hpcl_comp_pl_data* input_data);
  hpcl_comp_pl_data* get_input();
  mem_fetch* run();
  void print(unsigned pl_index=0);
  hpcl_cpack_dict<K>* m_glb_dict;
  vector<string> update_list;
  
  mem_fetch* top_compressed_mem_fetch();
  void pop_compressed_mem_fetch();
  
private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();
  unordered_map<string,code_table_glb*> pattern_finder;
  unordered_map<string,code_table_glb*> code_finder;
  void initialize(unordered_map<string,code_table_glb*>& code_finder,unordered_map<string,code_table_glb*>& pattern_finder);
	vector<string> get_update_list();

};
template<class K>
hpcl_comp_cpack_pl<K>::hpcl_comp_cpack_pl(unsigned pipeline_no, unsigned id) {
  m_pipeline_no = pipeline_no;
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_id = id;
  m_comp_buffer_size = -1;
  m_glb_dict = new hpcl_cpack_dict<K>(32);
  initialize(code_finder,pattern_finder);
}
// get updated dict
template<class K>
vector<string> hpcl_comp_cpack_pl<K>::get_update_list()
{
	return update_list;
}
/*
* function to initialize code table
*/
template<class K>
void hpcl_comp_cpack_pl<K>::initialize(unordered_map<string,code_table_glb*>& code_finder,unordered_map<string,code_table_glb*>& pattern_finder)
{
	
	string code;
	string pattern;
	int length=0;
	float frequency=0;

	/* init 1*/
	
	// code init 00
	code.assign("00");

	// string init zzzz
	pattern.assign("zzzz");

	// len 2
	length=2;

	// frequency
	frequency=39.7;

	
	code_table_glb *ob1 = new code_table_glb(code,pattern,length,frequency);	

	pattern_finder.insert(make_pair(pattern,ob1));
	code_finder.insert(make_pair(code,ob1));

	/* init 2 */

	// code init 01
	
	code.assign("01");
	
	// string init xxxx
	pattern.assign("xxxx");

	
	// len=34;
	length=34;
	
	frequency=32.1;

	code_table_glb *ob2= new code_table_glb(code,pattern,length,frequency);

	
	pattern_finder.insert(make_pair(pattern,ob2));
	code_finder.insert(make_pair(code,ob2));
	
	
	/* init 3 */
	
	code.assign("10");
	

	// string init mmmm
	pattern.assign("mmmm");

	
	// len=6;
	length=6;
	
	frequency=7.6;

	code_table_glb *ob3= new code_table_glb(code,pattern,length,frequency);
	
	pattern_finder.insert(make_pair(pattern,ob3));
	code_finder.insert(make_pair(code,ob3));
	
	
	
	/* init 4 */
	
	code.assign("1100");

	// string init mmxx
	pattern.assign("mmxx");

	
	// len=24;
	length=24;
	
	frequency=6.1;

	code_table_glb *ob4= new code_table_glb(code,pattern,length,frequency);

	
	pattern_finder.insert(make_pair(pattern,ob4));
	code_finder.insert(make_pair(code,ob4));
	
	/* init 5 */
	
	code.assign("1101");
	
	// string init zzzx
	pattern.assign("zzzx");

	
	// len=12;
	length=12;
	
	frequency=7.3;

	code_table_glb *ob5= new code_table_glb(code,pattern,length,frequency);

		
	pattern_finder.insert(make_pair(pattern,ob5));
	code_finder.insert(make_pair(code,ob5));
	
	/* init 6 */
	
	code.assign("1110");

	// string init mmmx
	pattern.assign("mmmx");

	
	// len=16;
	length=16;
	
	frequency=7.2;

	code_table_glb *ob6= new code_table_glb(code,pattern,length,frequency);

	
	pattern_finder.insert(make_pair(pattern,ob6));
	code_finder.insert(make_pair(code,ob6));
	

}
template<class K>
void hpcl_comp_cpack_pl<K>::create(unsigned comp_buffer_size) 
{
	// create pipeline  -> 9 for cpack
	for(unsigned i = 0; i < m_pipeline_no; i++) 
  	{
		m_comp_pl_proc.push_back(new hpcl_comp_cpack_pl_proc<K>());
		m_comp_pl_proc[i]->set_pl_index(i);
  	}
	m_comp_pl_proc[0]->set_pl_type(hpcl_comp_cpack_pl_proc<K>::GET_OUTPUT);
	for(unsigned i = 1; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_pl_type(hpcl_comp_cpack_pl_proc<K>::DUMMY);
	}
	m_comp_pl_proc[m_pipeline_no-1]->set_pl_type(hpcl_comp_cpack_pl_proc<K>::COMP);
	for(unsigned i = 0; i < m_pipeline_no-1; i++)
	{
		m_comp_pl_proc[i]->set_output(new hpcl_comp_pl_data);
	}


	m_input = m_comp_pl_proc[m_pipeline_no-1]->get_input();
  	m_comp_pl_proc[m_pipeline_no-1]->set_output(new hpcl_comp_pl_data);
  	m_comp_buffer_size = comp_buffer_size;


}

template<class K>
void hpcl_comp_cpack_pl<K>::set_input(hpcl_comp_pl_data* input_data) {
  m_input->copy(input_data);
}

template<class K>
hpcl_comp_pl_data* hpcl_comp_cpack_pl<K>::get_input() {
  return m_input;
}
template<class K>
mem_fetch* hpcl_comp_cpack_pl<K>::run()
{
  mem_fetch* ret = NULL;
	for(int i = 0; i < m_pipeline_no; i++) 
	{
		if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_cpack_pl_proc<K>::COMP)
		{
			m_comp_pl_proc[i]->run(pattern_finder,m_glb_dict->word_dict_glb);
			
			unordered_set<string> lcl = m_comp_pl_proc[i]->get_local_dict();
			m_comp_pl_proc[i-1]->set_local_dict(lcl);
			m_comp_pl_proc[i]->clear_dict();
		 	hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
			m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(tmp->get_mem_fetch());
			tmp->clean();
			
		}
		else if(m_comp_pl_proc[i]->get_pl_type()==hpcl_comp_cpack_pl_proc<K>::GET_OUTPUT)
		{
			hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
			assert(tmp);
			mem_fetch* mf = tmp->get_mem_fetch();
			
			unordered_set<string> lcl = m_comp_pl_proc[i]->get_local_dict();
			update_list.clear();
			update_list = m_glb_dict->check_new_added_word(lcl);
			m_glb_dict->update_dict(lcl);
			m_comp_pl_proc[i]->clear_dict();

			tmp->clean();

			ret = mf;
			
		}
		else
		{
			hpcl_comp_pl_data *tmp = m_comp_pl_proc[i]->get_output();
			assert(tmp);
			mem_fetch* mf = tmp->get_mem_fetch();
			if(mf)
			{
				m_comp_pl_proc[i-1]->get_output()->set_mem_fetch(mf);
			        unordered_set<string> lcl = m_comp_pl_proc[i]->get_local_dict();
				m_comp_pl_proc[i-1]->set_local_dict(lcl);
				m_comp_pl_proc[i]->clear_dict();

			}
			tmp->clean();
		}
		
	}		

	return ret;
		
}

template<class K>
void hpcl_comp_cpack_pl<K>::print(unsigned pl_index) {

}

template<class K>
mem_fetch* hpcl_comp_cpack_pl<K>::top_compressed_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

template<class K>
void hpcl_comp_cpack_pl<K>::pop_compressed_mem_fetch() {
  m_comp_buffer.erase(m_comp_buffer.begin());
}

template<class K>
bool hpcl_comp_cpack_pl<K>::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}


#endif /* HPCL_COMP_PL_H_ */
