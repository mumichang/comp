
// added by abpd

#ifndef HPCL_COMP_FPC_PL_PROC_H_
#define HPCL_COMP_FPC_PL_PROC_H_

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

#define DATA_SIZE 8

using namespace std;
extern abpd_comp_anal *abpd_anal_ptr;

extern hpcl_comp_anal* g_hpcl_comp_anal;
///
#define ABPD_DEBUG 0

template<class K>
class hpcl_comp_fpc_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;

  int m_pl_index;

public:
  hpcl_comp_fpc_pl_proc();
  ~hpcl_comp_fpc_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
      };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();
  unordered_set<string> lc_cmp;
  void init();

private:
  int m_type;
  void decompose_data(std::vector<unsigned char>& cache_data, string& packet);
  string int_to_hex( K i );
  string compress(string input,vector<pair<string,float> > tracker,float& bytes);
  string each_compress(string input,vector<pair<string,float> > tracker,float& bytes);

  string gen_zero(int n);
  string gen_f(int n);
  void compare_pattern(string input,string pattern,string& processed,float& bytes);
  bool check_rep(string input,string& rep);

  bool check_str(string input,int index,string& processed,float& bytes,int data_size);
  void convert_output(string input,vector<unsigned char>& data_to_send,vector<pair<string,float> > tracker,float& bytes_d);
  int get_pattern_len(string pattern,vector<pair<string,float> > tracker);

public:
  void run(vector<pair<string,float> > tracker);
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  bool fetch_data;
  bool execute_data;
  void set_pl_type(int type);
  int get_pl_type();
  enum 
  {
	  NONE=0,
	  DUMMY,
	  COMP,
	  GET_OUTPUT,
  };

};

template <class K>
hpcl_comp_fpc_pl_proc<K>::hpcl_comp_fpc_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_type=NONE;
  init();
}
template <class K>
void hpcl_comp_fpc_pl_proc<K>::init()
{
	lc_cmp.insert("8");
	lc_cmp.insert("9");
	lc_cmp.insert("A");
	lc_cmp.insert("B");
	lc_cmp.insert("C");
	lc_cmp.insert("D");
	lc_cmp.insert("E");
	lc_cmp.insert("F");

}
template <class K>
void hpcl_comp_fpc_pl_proc<K>::set_pl_type(int type) {
  m_type = type;
}
template <class K>
int hpcl_comp_fpc_pl_proc<K>::get_pl_type() {
  return m_type;
}

template <class K>
void hpcl_comp_fpc_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_fpc_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_fpc_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_fpc_pl_proc<K>::get_input() {
  return m_input;
}
template<class K>
string hpcl_comp_fpc_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
  return stream.str();
}
template <class K>
void hpcl_comp_fpc_pl_proc<K>::decompose_data(vector<unsigned char>& cache_data, string& packet)
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
mem_fetch* hpcl_comp_fpc_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_fpc_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}

template <class K>
void hpcl_comp_fpc_pl_proc<K>::reset_output() {
  m_output->clean();
}
// added by abpd
template<class K>
void hpcl_comp_fpc_pl_proc<K>::run(vector<pair<string,float> > tracker)
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

	float bytes_d=0;
	string output=compress(packet,tracker,bytes_d);

	cout<<"Input : 0x"<<packet<<endl;
	cout<<"Output : 0x"<<output<<endl;

	vector<unsigned char> data_to_send;

	convert_output(output,data_to_send,tracker,bytes_d);


	unsigned comp_byte_size = ceil((double)bytes_d);

	mf->set_comp_data_size(comp_byte_size);
	
	//mf->set_comp_data_size(mf->get_real_data_size());
	mf->init_size();
	


}
template<class K>
void hpcl_comp_fpc_pl_proc<K>::convert_output(string input,vector<unsigned char>& data_to_send,vector<pair<string,float> > tracker,float& bytes_d)
{


	int i=0;
	int len=0;
	//string output;
	while(i<input.size())
	{
		string pattern=input.substr(i,3);
		len=get_pattern_len(pattern,tracker);
		bytes_d+=(len/2);
		bytes_d+=(3/8);

		//output+=form_output(pattern,input.substr(i+3,len));
		i+=(len+3);	
	}

	//return output;					
}
template<class K>
string hpcl_comp_fpc_pl_proc<K>::compress(string input,vector<pair<string,float> > tracker,float& bytes)
{


	string output;
	for(int i=0;i<input.size();i+=DATA_SIZE)
	{
		string sub=input.substr(i,DATA_SIZE);
		output+=each_compress(sub,tracker,bytes);
	
	}

		
	return output;
}
template<class K>
string hpcl_comp_fpc_pl_proc<K>::each_compress(string input,vector<pair<string,float> > tracker,float& bytes)
{
	string processed;

	for(int i=0;i<tracker.size();i++)
	{
		pair<string,float> p = tracker[i];
		compare_pattern(input,p.first,processed,bytes);
		if(processed.empty()==false) return processed;
		
	}
	cout<<"Should not reach here..\n";
	return processed;	
}
template<class K>
string hpcl_comp_fpc_pl_proc<K>::gen_zero(int n)
{
	string res;

	for(int i=0;i<n;i++)
	res+="0";

	return res;
}
template<class K>
string hpcl_comp_fpc_pl_proc<K>::gen_f(int n)
{
	string res;

	for(int i=0;i<n;i++)
	res+="F";

	return res;
}
template<class K>
bool hpcl_comp_fpc_pl_proc<K>::check_str(string input,int index,string& processed,float& bytes,int data_size)
{

	string sub=input.substr(0,data_size-index);
	string cmp=input.substr(data_size-index,1);
	if(sub.compare(gen_zero(data_size-index))==0 && cmp[0]-'0'<=7)
	{
		if(cmp[0]-'0'>=0 && cmp[0]-'0'<=7)
		{
			processed+=input.substr(data_size-index);
		}
		return true;	
	}
	else if(sub.compare(gen_f(data_size-index))==0 && cmp[0]-'0'>7)
	{
		string ls_str=input.substr(data_size-index,1);
		if(lc_cmp.find(ls_str)!=lc_cmp.end())
		{
			processed+=input.substr(data_size-index);
		}
		return true;	
	}

	return false;
	
}
template<class K>
void hpcl_comp_fpc_pl_proc<K>::compare_pattern(string input,string pattern,string& processed,float& bytes)
{

	if(pattern.compare("000")==0)
	{	
		// case for zero run
		if(input.compare(gen_zero(DATA_SIZE))==0)
		processed.assign(pattern);
			
	}	
	else if(pattern.compare("001")==0)
	{
		// four bit sign extended

		if(check_str(input,1,processed,bytes,DATA_SIZE))
		processed=pattern+processed;	
	}	
	else if(pattern.compare("010")==0)
	{
		// 1 byte sign extended
			
		if(check_str(input,2,processed,bytes,DATA_SIZE))				
		processed=pattern+processed;	
		
	}	
	else if(pattern.compare("011")==0)
	{
		// 2 byte sign extended
		
		if(check_str(input,4,processed,bytes,DATA_SIZE))				
		processed=pattern+processed;	
	}
	else if(pattern.compare("100")==0)
	{
		// half word padded with zero half word

		string sub=input.substr(DATA_SIZE-4);
		
		if(sub.compare(gen_zero(DATA_SIZE-4))==0)
		{
			processed.assign(pattern);
			processed+=input.substr(0,DATA_SIZE-4);
		}	
	}
	else if(pattern.compare("101")==0)
	{

		// two half words each a byte sign extended

		string part_one=input.substr(0,DATA_SIZE-4);
		string part_two=input.substr(DATA_SIZE-4);

		string p_o,p_t;
		if(check_str(part_one,2,p_o,bytes,DATA_SIZE/2) && check_str(part_two,2,p_t,bytes,DATA_SIZE/2))
		{
			processed=pattern+p_o+p_t;	
		}
			
	}
	else if(pattern.compare("110")==0)
	{
		string rep;
		if(check_rep(input,rep))
		processed=pattern+rep;	
	}
	else if(pattern.compare("111")==0)
	{
		processed=pattern+input;		
	}
	return;
}

template<class K>
bool hpcl_comp_fpc_pl_proc<K>::check_rep(string input,string& rep)
{
	unordered_set<string> tmp;

	tmp.insert(input.substr(0,2));

	for(int i=2;i<input.size();i+=2)
	{
		string sub=input.substr(i,2);

		if(tmp.find(sub)==tmp.end())
		return false;
	}

	rep.assign(input.substr(0,2));
	return true;
}
template<class K>
int hpcl_comp_fpc_pl_proc<K>::get_pattern_len(string pattern,vector<pair<string,float> > tracker)
{
	for(int i=0;i<tracker.size();i++)
	{
		pair<string,float> p = tracker[i];
		if(p.first.compare(pattern)==0)
		return (int)p.second;
	}	
}



#endif /* HPCL_COMP_PL_PROC_H_ */
