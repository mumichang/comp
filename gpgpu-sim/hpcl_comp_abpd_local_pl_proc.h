
// added by abpd

#ifndef HPCL_COMP_ABPD_LOCAL_PL_PROC_H_
#define HPCL_COMP_ABPD_LOCAL_PL_PROC_H_

#include "hpcl_comp_pl_data.h"
#include "hpcl_dict.h"
#include "hpcl_comp_data.h"
#include <vector>
#include <cassert>
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include<unordered_map>
#include<unordered_set>
#include<algorithm>

#define BIT_WORD 2
#define PATTERN_SIZE 4
#define MAX_DICT_SIZE 32
#define WORD_SIZE 4

//added by abpd
#include "hpcl_comp_anal.h"
//#include "blocker.h"

using namespace std;

extern hpcl_comp_anal* g_hpcl_comp_anal;
///

template<class K>
class hpcl_comp_abpd_local_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  unordered_map<char,int> lc_cmp;
  unordered_map<int,char> lc_dcp;
  int m_pl_index;

public:
  hpcl_comp_abpd_local_pl_proc();
  ~hpcl_comp_abpd_local_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
  };
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  void reset_output();

public:
  void run();
  vector<string> input_processor(vector<string> input_to_process,float& flit_count);

  void prepare_decode_dict(vector<string> input,unordered_map<string,string>& lcl_dict);
  string get_binary(int index);
  unordered_map<string,int> prepare_dict(vector<string> input_to_process,unordered_map<string,string>& lcl_dict);

  string encode_data(string input,unordered_map<string,string> lcl_dict,float& flit_count,unordered_map<string,int>& occ_check);
  vector<string> decode_string(vector<string> input_string);
  void decompose_data(std::vector<unsigned char>& cache_data, string& packet);
  string int_to_hex( K i );
  void lookup_hexa();
  long long get_int(vector<char> binary,bool is_neg);
  void  convert_output(vector<string> output_to_get,vector<unsigned char>& data_to_send,long long& comp_bit_size); 
  vector<char> get_binary_hexa(string str); 
  vector<char> get_binary_new(long long num,int limit);

};


template <class K>
hpcl_comp_pl_data* hpcl_comp_abpd_local_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_abpd_local_pl_proc<K>::get_input() {
  return m_input;
}

// added by abpd
/*
template <class K>
mem_fetch* hpcl_comp_abpd_local_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}
*/



template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::run() {
	mem_fetch* mf = m_input->get_mem_fetch();
  	m_output->set_mem_fetch(mf);

  	if(!mf) return;

  	//If mf is not type for compression,
  	if(mf->get_real_data_size() == 0)	return;

  	std::vector<unsigned char>& cache_data = mf->get_real_data_ptr();
  	std::vector<K> word_list;
	std::string packet;
	decompose_data(cache_data,packet);

	transform(packet.begin(),packet.end(),packet.begin(),::toupper);
	mf->compare_data.assign(packet);
	
	/* convert into input vector word size 4 def -> need to make it config*/

	int word_size = WORD_SIZE*2;
	vector<string> input_to_process;
	vector<string> output_to_get;
	float flit_count=0;

	for(int i=0;i<packet.size();i+=word_size)
	{
		string sub = packet.substr(i,word_size);
		input_to_process.push_back(sub);
	}

	output_to_get=input_processor(input_to_process,flit_count);
	int i;
	for(i=0;i<output_to_get.size();i++)
	{
		cout<<output_to_get[i]<<"\t";
	}
	cout<<"\n";

	vector<unsigned char> data_to_send;
	long long comp_bit_size=0;
        convert_output(output_to_get,data_to_send,comp_bit_size);
//	cout<<"Abpd Output String "<<output<<" Output Size : "<<output.size()<<"\n";

	cout<<"Abdp data to send \n";

	for(int i=0;i<data_to_send.size();i++)
	{
		printf("%d",data_to_send[i]);
	}
	printf("\n");
	
	 unsigned comp_byte_size = ceil((double)comp_bit_size/8);
	 cout<<"Set byte \tsize=\t"<<comp_byte_size<<endl;
	 cout<<"Acutal byte size=\t"<<data_to_send.size()<<endl;
	 assert(comp_byte_size==data_to_send.size());
  	mf->set_comp_data_size(comp_byte_size);
	mf->init_size();
	for(int i=0;i<data_to_send.size();i++)
	{
		mf->set_comp_data(i,data_to_send[i]);
	}


}

template<class K>
void  hpcl_comp_abpd_local_pl_proc<K>::convert_output(vector<string> output_to_get,vector<unsigned char>& data_to_send,long long& comp_bit_size)
{
	int i;
	string out_binary;
	
	for(i=0;i<output_to_get.size();i++)
	{
		string temp;
		string rem_str;
		temp=output_to_get[i].substr(0,2);
		rem_str=output_to_get[i].substr(2);
		out_binary=out_binary+temp;
		cout<<"PD::Debug::"<<temp<<"\t"<<rem_str<<endl;
		if(temp.compare("00")|temp.compare("01"))
		{
			//out_binary=out_binary+temp;
			vector<char> bi=get_binary_hexa(rem_str);
			for(int j=0;j<bi.size();j++)
			{
				out_binary=out_binary+bi[j];	
			}
		}
		else if (temp.compare("10"))
		{
			out_binary=out_binary+output_to_get[i];

		}

	}
	comp_bit_size=out_binary.size();
	cout<<"PD::debug"<<out_binary<<endl;
	i=0;
	//convert binary to char;
	while(i+8<=out_binary.size())
		{
			vector<char> to_send;
			for(int j=i;j<i+8;j++)
			{
				to_send.push_back(out_binary[j]);
			}
			data_to_send.push_back((unsigned char)get_int(to_send,false));
			i+=8;

		}

	string last = out_binary.substr(i);
	if(last.size()!=0)
	{
		while(last.size()%8!=0)
		{
			last = "0" + last;
		}
		vector<char> to_send;
		for(int i=0;i<8;i++)
		{
			to_send.push_back(last[i]);
		}
		data_to_send.push_back((unsigned char)get_int(to_send,false));

	}
}

template <class K>
vector<char> hpcl_comp_abpd_local_pl_proc<K>::get_binary_hexa(string str)
{
	vector<char> res;

	for(int i=0;i<str.size();i++)
	{
		char c= str[i];
		int val=0;
		if(lc_cmp.find(c)!=lc_cmp.end())
		{
			val=lc_cmp[c];
		}
		else
		{
			val=c-'0';
		}
		//cout<<val<<"\t";
		vector<char> temp= get_binary_new(val,4);
		for(int j=0;j<temp.size();j++)
		{
			char p = temp[j];
			res.push_back(p);
		}
	}

	return res;
}
template <class K>
vector<char> hpcl_comp_abpd_local_pl_proc<K>::get_binary_new(long long num,int limit)
{
	vector<char> res;

	while(limit>0)
	{
		res.insert(res.begin(),num%2+'0');
		num=num/2;
		limit--;
	}
	return res;
}

template <class K>
long long hpcl_comp_abpd_local_pl_proc<K>::get_int(vector<char> binary,bool is_neg)
{

	int count=binary.size()-1;
	
	long long val=0;

	if(is_neg==true)
	{
		//cout<<"neg\n";
		val=-1*pow(2,count);
	}
	else
	{
		//cout<<"pos\n";
		val=(binary[0]-'0')*pow(2,count);
	}
	count--;


	//cout<<val<<"\n";
	for(int i=1;i<binary.size();i++)
	{
		val+=(binary[i]-'0')*pow(2,count);
		count--;
	}
	return val;
}




template <class K>
hpcl_comp_abpd_local_pl_proc<K>::hpcl_comp_abpd_local_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  //m_loc_dict = new hpcl_dict<K>(128, HPCL_LFU);
  //m_loc_dict->clear();
}


template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::reset_output() {
  m_output->clean();
}

template <class K>
vector<string> hpcl_comp_abpd_local_pl_proc<K>::input_processor(vector<string> input_to_process,float& flit_count)
{

	vector<string> output_to_get;
	
	// local dict for the input
	unordered_map<string,string> lcl_dict;
	unordered_map<string,int> occ_check;

	occ_check=prepare_dict(input_to_process,lcl_dict);

//	display_dict(lcl_dict);
	int i;
	for(i=0;i<input_to_process.size();i++)
	//for(string lcl_input : input_to_process)
	{
		output_to_get.push_back(encode_data(input_to_process[i],lcl_dict,flit_count,occ_check));
		//output_to_get.push_back(encode_data(lcl_input,lcl_dict,flit_count,occ_check));
	}		
			
	return output_to_get;	
}
/* function to encode data */
template <class K>
string hpcl_comp_abpd_local_pl_proc<K>::encode_data(string input,unordered_map<string,string> lcl_dict,float& flit_count,unordered_map<string,int>& occ_check)
{

	string output;
	if(lcl_dict.find(input)!=lcl_dict.end())
	{
		// word found in dict
		// can be of category 01 -> raw data used , 10 -> raw data pointed to

		if(occ_check[input]==1)
		{
			// first occurance of dict word
			output.assign("01"+input);
			occ_check[input]++;
			flit_count+=(0.25+4);	
		}
		else
		{
			// repeated occurance
			output.assign("10"+lcl_dict[input]);
			
			flit_count+=(0.25+(log2(MAX_DICT_SIZE)/8));	
		}	
	}
	else
	{
		// category -> raw data unused
		output.assign("00"+input);
		flit_count+=(0.25+4);	
	}

	return output;	
}

/* function to prepare dict 1 cycle */

template <class K>
unordered_map<string,int> hpcl_comp_abpd_local_pl_proc<K>::prepare_dict(vector<string> input_to_process,unordered_map<string,string>& lcl_dict)
{

	unordered_map<string,int> occ_check;
	vector<pair<string,int> > order_check;
	
	unordered_map<string,int> tracker;
	string s;
	int i;
	for(i=0;i<input_to_process.size();i++)
	//for(string s : input_to_process)
	{
		s=input_to_process[i];
		if(occ_check.find(s)!=occ_check.end())
		{
			tracker[s]++;		
		}
		else
		{
			occ_check.insert(make_pair(s,1));
			order_check.push_back(make_pair(s,1));
			tracker.insert(make_pair(s,1));
		}
	}
	/* update dict */
	
	for(i=0;i<input_to_process.size();i++)
	//for(string s : input_to_process)
	{
		s=input_to_process[i];
		if(tracker.find(s)!=tracker.end() && tracker[s]>1)
		{
			int index=lcl_dict.size();
			lcl_dict.insert(make_pair(s,get_binary(index)));
		}
	}
	
	return occ_check;
}

/* function to convert index to binary */
template <class K>
string hpcl_comp_abpd_local_pl_proc<K>::get_binary(int index)
{
	string output;
	int max_size=log2(MAX_DICT_SIZE);
	int a=index;
	char temp;
	while(max_size>0)
	{
		//output=to_string(index%2)+output;
		temp=index%2+'0';
		output=temp+output;
		index=index/2;
		max_size--;
	}
	return output;
}

/* function to prepare decode dict */

template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::prepare_decode_dict(vector<string> input,unordered_map<string,string>& lcl_dict)
{	int i;
	string s;
	for(i=0;i<input.size();i++)
	//for(string s : input)
	{
		s=input[i];
		if(s.substr(0,2).compare("01")==0)
		{
			int index=lcl_dict.size();
			lcl_dict.insert(make_pair(get_binary(index),s.substr(2)));
		}
	}	
}

/* function to decode data */

template <class K>
vector<string> hpcl_comp_abpd_local_pl_proc<K>::decode_string(vector<string> input_string)
{
	vector<string> output_string;
	unordered_map<string,string> lcl_dict;

	prepare_decode_dict(input_string,lcl_dict);

	
//	display_dict(lcl_dict);
	int i;
	string s;
	for(i=0;i<input_string.size();i++)
	//for(string s : input_string)
	{
		s=input_string[i];
		if(s.substr(0,2).compare("00")==0 || s.substr(0,2).compare("01")==0)
		{
			// raw data unused
			output_string.push_back(s.substr(2));
		}
		else if(s.substr(0,2).compare("10")==0)
		{
			output_string.push_back(lcl_dict[s.substr(2)]);
		}
		else
		{
			/* Do Nothing */
		}
	}

	return output_string;
}

template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::decompose_data(std::vector<unsigned char>& cache_data, string& packet)
{
	//cout<<" abpd  cache size"<<cache_data.size()<<"\n";
  for(unsigned i = 0; i < cache_data.size(); i=i+sizeof(K)) {
    K word_candi = 0;
     //printf(" abpd act %d\n",cache_data[i]);
    for(int j = sizeof(K)-1; j >= 0; j--) {
	         K tmp = cache_data[i+j];
		 //cout<<"abpd .. \t"<<tmp<<"\n";
      tmp = (tmp << (8*j));
       //cout<<"abpd after.. \t"<<tmp<<"\n";

      //cout<<tmp<<"unsigned \n";
      word_candi += tmp;
    }
     //cout<<"abpd word \t"<<word_candi<<"\n";
    packet+=int_to_hex(word_candi);
    //word_list.push_back(word_candi);
//    printf("\tword -- ");
//    hpcl_dict_elem<K>::print_word_data(word_candi);
//    printf("\n");
  }
  //assert(word_list.size() == cache_data.size()/sizeof(K));
}

template<class K>
string hpcl_comp_abpd_local_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
 // cout<<stream.str()<<" str\n";
  return stream.str();
}

template <class K>
void hpcl_comp_abpd_local_pl_proc<K>::lookup_hexa()
{
	lc_cmp.clear();

	lc_cmp.insert(make_pair('A',10));
	lc_cmp.insert(make_pair('B',11));
	lc_cmp.insert(make_pair('C',12));
	lc_cmp.insert(make_pair('D',13));
	lc_cmp.insert(make_pair('E',14));
	lc_cmp.insert(make_pair('F',15));
	lc_dcp.insert(make_pair(10,'A'));
	lc_dcp.insert(make_pair(11,'B'));
	lc_dcp.insert(make_pair(12,'C'));
	lc_dcp.insert(make_pair(13,'D'));
	lc_dcp.insert(make_pair(14,'E'));
	lc_dcp.insert(make_pair(15,'F'));	
}



#endif
