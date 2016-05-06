
// added by abpd

#ifndef HPCL_COMP_CPACK_PL_PROC_H_
#define HPCL_COMP_CPACK_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <unordered_set>


//added by abpd
#include "hpcl_comp_anal.h"
#include "hpcl_comp_analysis.h"
#include "hpcl_code_table.h"

#define BIT_WORD 2
#define PATTERN_SIZE 4
#define MAX_DICT_SIZE 32


using namespace std;
extern abpd_comp_anal *abpd_anal_ptr;

extern hpcl_comp_anal* g_hpcl_comp_anal;
///
#define ABPD_DEBUG 0

template<class K>
class hpcl_comp_cpack_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  int m_pl_index;

public:
  hpcl_comp_cpack_pl_proc();
  ~hpcl_comp_cpack_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
      };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  int m_type;
  unordered_set<string> word_dict_lcl;
  void decompose_data(std::vector<unsigned char>& cache_data, string& packet);
  vector<string> input_processor(vector<string> input_to_process,unordered_map<string,code_table_glb*> pattern_finder,vector<pair<string,string> > word_dict_glb,float& flit_count);
  string int_to_hex( K i );
 


public:
  void run(unordered_map<string,code_table_glb*> pattern_finder,vector<pair<string,string> > word_dict_glb);
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  bool fetch_data;
  bool execute_data;
  void set_pl_type(int type);
  int get_pl_type();
  string process_string_pattern(string pattern_str,string cur_str,int index,vector<pair<string,string> > word_dict_glb);
  string search_dict(string dict_index,vector<pair<string,string> > word_dict_glb);
  int compare_str(string dict_str,string cur_str,string& local_unmatched,string& local_processed_str);
  bool dict_compare(string cur_str,string& index_of_dict,string& processed_str,vector<pair<string,string> > word_dict_glb,string& unmatched,unordered_map<string,code_table_glb*> pattern_finder);
  string process_string_partial(string cur_str,string& unmatched);
  
  unordered_set<string> get_local_dict();
  void set_local_dict(unordered_set<string> lcl);
  void clear_dict();

  enum 
  {
	  NONE=0,
	  DUMMY,
	  COMP,
	  GET_OUTPUT,
  };

};

template <class K>
hpcl_comp_cpack_pl_proc<K>::hpcl_comp_cpack_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_type=NONE;
}

template <class K>
void hpcl_comp_cpack_pl_proc<K>::set_local_dict(unordered_set<string> lcl) 
{
	word_dict_lcl=lcl;
}
template <class K>
unordered_set<string> hpcl_comp_cpack_pl_proc<K>::get_local_dict() 
{
	return word_dict_lcl;
}
template <class K>
void hpcl_comp_cpack_pl_proc<K>::clear_dict()
{
	word_dict_lcl.clear();
}
template <class K>
void hpcl_comp_cpack_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_cpack_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_cpack_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_cpack_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_cpack_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_cpack_pl_proc<K>::get_input() {
  return m_input;
}
template<class K>
string hpcl_comp_cpack_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
  return stream.str();
}
template <class K>
void hpcl_comp_cpack_pl_proc<K>::decompose_data(vector<unsigned char>& cache_data, string& packet)
{
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
    for(int j = sizeof(K)-1; j >= 0; j--) {
	         K tmp = cache_data[i+j];
      tmp = (tmp << (8*j));

      word_candi += tmp;
    }
    packet+=int_to_hex(word_candi);
  }
}

// added by abpd
template <class K>
mem_fetch* hpcl_comp_cpack_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_cpack_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}

template <class K>
void hpcl_comp_cpack_pl_proc<K>::reset_output() {
  m_output->clean();
}
template <class K>
vector<string> hpcl_comp_cpack_pl_proc<K>::input_processor(vector<string> input_to_process,unordered_map<string,code_table_glb*> pattern_finder,vector<pair<string,string> > word_dict_glb,float& flit_count)
{
	vector<string> output_to_get;

	for(int i=0;i<input_to_process.size();i++)
	{
		string cur_str = input_to_process[i];
		string unmatched,output_string,processed_string;
		string index_of_dict;
		/* process_string_partial will convert input string to only zzzx || zzzz format
		* if none of these are found -> return empty string
		*/
		processed_string=process_string_partial(cur_str,unmatched);
		if((processed_string.compare("zzzz")==0)||(processed_string.compare("zzzx")==0))
		{
			/* patter found in code table , need not to look for dict
			*  code + unmatched_part  
			*/
			if(pattern_finder.find(processed_string)!=pattern_finder.end()) 
			output_string=pattern_finder[processed_string]->code+unmatched;


			// update the flit count
			if(unmatched.empty()==false)
			{
				flit_count+=1.5;
			}
			else
			{
				flit_count+=0.25;
			}			 
		}
		else
		{
			bool ret=dict_compare(cur_str,index_of_dict,processed_string,word_dict_glb,unmatched,pattern_finder);
			if(ret)
			{
				/* procesed string will contain all values of type mm xx zz *
				 code + index + unmached */
				
				if(pattern_finder.find(processed_string)!=pattern_finder.end()) 
				output_string=pattern_finder[processed_string]->code + index_of_dict + unmatched;


				// update flit cont
				if(unmatched.size()==2)
				{
					// mmmx
					flit_count+=(0.5+(log2(MAX_DICT_SIZE)/8)+1);
				}
				else if(unmatched.size()==4)
				{
					// mmxx
					
					flit_count+=(0.5+(log2(MAX_DICT_SIZE)/8)+2);
				}
				else
				{
					// mmmm
					
					flit_count+=(0.25+(log2(MAX_DICT_SIZE)/8));
				}	
				
			}	
			else
			{
				
				if(pattern_finder.find(processed_string)!=pattern_finder.end()) 
				output_string=pattern_finder[processed_string]->code + unmatched;
				/* update the same in dict */
				if(word_dict_lcl.find(cur_str)==word_dict_lcl.end())
				word_dict_lcl.insert(cur_str);

				// update flit count
				flit_count+=4.25; 
			}
		}
		output_to_get.push_back(output_string);
	}
		
	return output_to_get;	
}
void get_red_data(vector<string> output_to_get,unordered_map<string,code_table_glb*> pattern_finder)
{
	for(int i=0;i<output_to_get.size();i++)
	{
		string str = output_to_get[i];
		string key1 = str.substr(0,2);
		string key2 = str.substr(0,4);

		auto it = pattern_finder.begin();

		for(;it!=pattern_finder.end();it++)
		{
			pair<string,code_table_glb*> p=*it;
			if(p.second->code.compare(key1)==0)
			{
				if(abpd_anal_ptr->bdi_anal.find(key1)!=abpd_anal_ptr->bdi_anal.end())
				{
					if(abpd_anal_ptr->bdi_anal[key1].find(str.substr(2))!=abpd_anal_ptr->bdi_anal[key1].end())
					{
						abpd_anal_ptr->bdi_anal[key1][str.substr(2)]++;	
					}
					else
					{
						abpd_anal_ptr->bdi_anal[key1].insert(make_pair(str.substr(2),1));
					}

				}
				else
				{
					unordered_map<string,int> tmp;
					abpd_anal_ptr->bdi_anal.insert(make_pair(key1,tmp));
				}
			}
			else if(p.second->code.compare(key2)==0)
			{
				if(abpd_anal_ptr->bdi_anal.find(key2)!=abpd_anal_ptr->bdi_anal.end())
				{
					if(abpd_anal_ptr->bdi_anal[key2].find(str.substr(4))!=abpd_anal_ptr->bdi_anal[key2].end())
					{
						abpd_anal_ptr->bdi_anal[key2][str.substr(4)]++;	
					}
					else
					{
						abpd_anal_ptr->bdi_anal[key2].insert(make_pair(str.substr(4),1));
					}

				}
				else
				{
					unordered_map<string,int> tmp;
					abpd_anal_ptr->bdi_anal.insert(make_pair(key2,tmp));
				}

			}
			else
			{
				// DO NOTHING
			}


		}
	}
}
// added by abpd
template<class K>
void hpcl_comp_cpack_pl_proc<K>::run(unordered_map<string,code_table_glb*> pattern_finder,vector<pair<string,string> > word_dict_glb)
{

	mem_fetch* mf = m_input->get_mem_fetch();
  	m_output->set_mem_fetch(mf);

  	if(!mf) return;

  	//If mf is not type for compression,
  	if(mf->get_real_data_size() == 0)	return;

  	std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
	std::string packet;
	decompose_data(cache_data,packet);

	transform(packet.begin(),packet.end(),packet.begin(),::toupper);
	mf->compare_data.assign(packet);

	vector<string> input_to_process;
	vector<string> output_to_get;

	for(int i=0;i<packet.size();i+=8)
	{
		string sub = packet.substr(i,8);
		input_to_process.push_back(sub);
	}

	float bytes_count=0;
	output_to_get=input_processor(input_to_process,pattern_finder,word_dict_glb,bytes_count);

	/* get redundancy across and within words */
	get_red_data(output_to_get,pattern_finder);
	abpd_anal_ptr->write_to_file();
	//abpd_anal_ptr->display_red();
	abpd_anal_ptr->bdi_anal.clear();

	string output;
	/* TODO for getting actual data and sending it to network */
	for(int i=0;i<output_to_get.size();i++)
	{
		output= output + output_to_get[i];
	}

	//#ifdef ABPD_DEBUG
	//cout<<"Input : 0x"<<packet<<endl;
	//cout<<"Output : 0x"<<output<<endl;
	//#endif

	unsigned comp_byte_size = ceil((double)bytes_count);

	mf->set_comp_data_size(comp_byte_size);
	mf->init_size();
	


}
/* function to return string in code pattern format
*/

template<class K>
string hpcl_comp_cpack_pl_proc<K>::process_string_partial(string cur_str,string& unmatched)
{
	string processed_string;
	int str_sz=cur_str.size();
	for(int i=0;i<cur_str.size()-BIT_WORD;i+=BIT_WORD)
	{
		if(cur_str[i]=='0' && cur_str[i+1]=='0')
		processed_string+="z";
	}

	if(cur_str[str_sz-2]=='0' && cur_str[str_sz-1]=='0')
	processed_string+="z";
	else
	{
		processed_string+="x";	
		unmatched.assign(cur_str.substr(str_sz-2));
	}	
	return processed_string;
}

/* function to compare glb dictonary in condition word is not found in code table
*/

template<class K>
bool hpcl_comp_cpack_pl_proc<K>::dict_compare(string cur_str,string& index_of_dict,string& processed_str,vector<pair<string,string> > word_dict_glb,string& unmatched,unordered_map<string,code_table_glb*> pattern_finder)
{
	
	int val=0,max_val=0,max_index=-1;
	
	for(int i=0;i<word_dict_glb.size();i++)
	{
		
		string local_unmatched,local_processed_str;
		
		string dict_str=word_dict_glb[i].first;
		val=compare_str(dict_str,cur_str,local_unmatched,local_processed_str);
		if(val>max_val)
		{
			max_val=val;
			max_index=i;	
			unmatched.assign(local_unmatched);
			index_of_dict.assign(word_dict_glb[i].second);
			processed_str.assign(local_processed_str);
		}			
	}
	
	// now we need to check for match

	if(max_val==0 || max_val==1)
	{
		processed_str.assign("xxxx");
		unmatched.assign(cur_str);
		return false;
	}
	
	if(pattern_finder.find(processed_str)==pattern_finder.end())
	{
		 return false;	
	}
	else				
	return true;
}

/* function to compare two indiv strings
*/

template<class K>
int hpcl_comp_cpack_pl_proc<K>::compare_str(string dict_str,string cur_str,string& local_unmatched,string& local_processed_str)
{
	int match_count=0;

	bool is_first_unmatch=true;
	for(int i=0;i<=dict_str.size()-BIT_WORD;i+=BIT_WORD)
	{
		if(dict_str[i]==cur_str[i] && dict_str[i+1]== cur_str[i+1]&& is_first_unmatch==true)
		{
			local_processed_str+="m";
			match_count++;
		}
		else
		{
			local_processed_str+="x";
			is_first_unmatch=false;
			local_unmatched.assign(cur_str.substr(i));
			break;
		}		
	}

	while(local_processed_str.size()<PATTERN_SIZE)
	{
		local_processed_str+="x";		
	}

	return match_count;		
}
/* functio to search word in dict */
template<class K>
string hpcl_comp_cpack_pl_proc<K>::search_dict(string dict_index,vector<pair<string,string> > word_dict_glb)
{
	for(int i;i<word_dict_glb.size();i++)
	//for(pair<string,string> p : word_dict_glb)
	{
		pair<string,string> p=word_dict_glb[i];
		if(dict_index.compare(p.second)==0) return p.first;
	}

//	cout<<" Debug  : Not correctly decoded 3\n";
	return " ";
}
/* function to decode each */

template<class K>
string hpcl_comp_cpack_pl_proc<K>::process_string_pattern(string pattern_str,string cur_str,int index,vector<pair<string,string> > word_dict_glb)
{
	string dict_index;
	if(pattern_str.compare("zzzz")==0)
	{
		return "00000000";
	}
	else if (pattern_str.compare("xxxx")==0)
	{
		return cur_str.substr(index);

	}
	else if (pattern_str.compare("mmmm")==0)
	{

		dict_index= cur_str.substr(index);
		return search_dict(dict_index,word_dict_glb);	
	}
	else if ( pattern_str.compare("mmxx")==0) 
	{
		string rem_str=cur_str.substr(index);
		dict_index=rem_str.substr(0,log2(MAX_DICT_SIZE));
		
		return search_dict(dict_index,word_dict_glb).substr(0,4)+rem_str.substr(log2(MAX_DICT_SIZE));
	}
	else if ( pattern_str.compare("zzzx")==0) 
	{
		return "000000"+cur_str.substr(index);
	}
	else if(pattern_str.compare("mmmx")==0)
	{
		string rem_str=cur_str.substr(index);
		dict_index=rem_str.substr(0,log2(MAX_DICT_SIZE));
		
		return search_dict(dict_index,word_dict_glb).substr(0,6)+rem_str.substr(log2(MAX_DICT_SIZE));
	
	}
	else
	{
//		cout<<"Debug : Not correctly decoded 2\n";
		return " ";	
	}
			
		
}








#endif /* HPCL_COMP_PL_PROC_H_ */
