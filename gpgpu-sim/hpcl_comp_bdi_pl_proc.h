
// added by abpd

#ifndef HPCL_COMP_BDI_PL_PROC_H_
#define HPCL_COMP_BDI_PL_PROC_H_

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



//added by abpd
#include "hpcl_comp_anal.h"
#include "hpcl_blocker.h"
#include "hpcl_comp_analysis.h"

using namespace std;
extern abpd_comp_anal *abpd_anal_ptr;

extern hpcl_comp_anal* g_hpcl_comp_anal;
///
#define ABPD_DEBUG 0

template<class K>
class hpcl_comp_bdi_pl_proc {
private:
  hpcl_comp_pl_data* m_input;
  hpcl_comp_pl_data* m_output;
  unordered_map<char,int> lc_cmp;
  unordered_map<int,char> lc_dcp;

  int m_pl_index;

public:
  hpcl_comp_bdi_pl_proc();
  ~hpcl_comp_bdi_pl_proc() {
    if(m_input)	 	delete m_input;
    if(m_output)	delete m_output;
    if(m_loc_dict)	delete m_loc_dict;
  };
  void set_pl_index(int pl_index);
  void set_output(hpcl_comp_pl_data* output);
  hpcl_comp_pl_data* get_output();
  hpcl_comp_pl_data* get_input();
  void reset_output();

private:
  hpcl_dict<K>* m_loc_dict;		//1B word
  unsigned simul_local_comp(hpcl_dict<K>& loc_dict, vector<unsigned char>& cache_data);
  void form_string(std::vector<unsigned char>& cache_data,string& res);
  void lookup_hexa();
  bool check_zero(string packet,int data_size,vector<string>& part_output,int base,int delta);
  bool zero_diff(string src,string dst,int delta,string& p_input);
	bool check_base(vector<string>& part_output,int base,int delta,string& base_str);
	bool convert_base(string src,string dst,int base,int delta,string& processed);
	long long convert_int(string s);
	string convert_hexa(long long num,int delta);
	vector<char> get_binary_c(vector<char> b);
	vector<char> get_binary(long long num,int limit);
	long long get_two_c(long long num,int delta);
	long long get_int(vector<char> binary,bool is_neg);
	vector<char> get_binary_hexa(string str);
	string form_output(vector<string> part_output);
	string int_to_hex( K i );

	void decompose_data(std::vector<unsigned char>& cache_data, string& packet);

	void convert_output(string output,vector<unsigned char>& data_to_send,int delta,int base);


public:
  void run(vector<pair<string,Blocker*> > tracker);
  mem_fetch* get_mf_input();
  void set_mf_input(mem_fetch *mf);
  bool fetch_data;
  bool execute_data;

};
template <class K>
void hpcl_comp_bdi_pl_proc<K>::lookup_hexa()
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
template <class K>
hpcl_comp_bdi_pl_proc<K>::hpcl_comp_bdi_pl_proc() {
  m_input = new hpcl_comp_pl_data;
  m_output = NULL;
  m_pl_index = -1;
  m_loc_dict = new hpcl_dict<K>(128, HPCL_LFU);
  m_loc_dict->clear();
}

template <class K>
void hpcl_comp_bdi_pl_proc<K>::set_pl_index(int pl_index) {
  m_pl_index = pl_index;
}

template <class K>
void hpcl_comp_bdi_pl_proc<K>::set_output(hpcl_comp_pl_data* output) {
  m_output = output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_bdi_pl_proc<K>::get_output() {
  return m_output;
}

template <class K>
hpcl_comp_pl_data* hpcl_comp_bdi_pl_proc<K>::get_input() {
  return m_input;
}

// added by abpd
template <class K>
mem_fetch* hpcl_comp_bdi_pl_proc<K>::get_mf_input() {
	mem_fetch* mf = m_input->get_mem_fetch();

  return mf;
}
template <class K>
void hpcl_comp_bdi_pl_proc<K>::set_mf_input(mem_fetch *mf) {

	m_input->set_mem_fetch(mf);
}


template <class K>
void hpcl_comp_bdi_pl_proc<K>::reset_output() {
  m_output->clean();
}
template<class K>
string hpcl_comp_bdi_pl_proc<K>::int_to_hex( K i )
{
  stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(K)*2) 
	  << std::hex << i;
 // cout<<stream.str()<<" str\n";
  return stream.str();
}
template <class K>
void hpcl_comp_bdi_pl_proc<K>::decompose_data(std::vector<unsigned char>& cache_data, string& packet)
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
string get_zero_out(int num)
{
	string res;
	for(int i=0;i<num;i++)
	{
		res+="0";
	}
	return res;
}
bool all_zero_check(string packet,string& output)
{
	string zero;

	for(int i =0;i<packet.size();i++)
	{
		zero+="0";
	}
	if(packet.compare(zero)==0)
	{
		string tmp;
		for(int i=0;i<packet.size()/8;i++)
			tmp+="0";
		output="0000"+tmp;
		return true;
	}
	
	return false;
}
unsigned int hex_to_int(string x)
{
	unsigned int y;
	std::stringstream stream;
	stream << std::hex << x;
	stream >> y;
	return y;
}
void prepare_data_to_send(vector<unsigned int> word_list,vector<unsigned char>& data_to_send)
{
	for(int i=0;i<word_list.size();i++)
	{
		unsigned int v = word_list[i];
		int count = sizeof(unsigned int) -1;
		while(count>=0)
		{
			unsigned char mov = 255;
			unsigned int op = mov << (count*8);
			v=v&op;
			v=v >> (count*8);
			//cout<<"abdp pre- > "<<v<<"\n";
			data_to_send.push_back(v);
			count--;

		}
	}
}
template<class K>
void  hpcl_comp_bdi_pl_proc<K>::convert_output(string output,vector<unsigned char>& data_to_send,int delta,int base)
{

	string type_bdi = output.substr(0,4);

	if(type_bdi.compare("0000")==0)
	{
			string rem_a = output.substr(4);

		// we consider non compressed as sector of 4 bytes for checking rep
		if(abpd_anal_ptr)
		{
			
			for(int i=0;i<rem_a.size();i+=8)
			{
				string sub_a = rem_a.substr(i,8);
				if(abpd_anal_ptr->bdi_anal.find(sub_a)==abpd_anal_ptr->bdi_anal.end())
				{
					unordered_map<string,int> tmp;
					tmp.insert(make_pair(sub_a,1));
					abpd_anal_ptr->bdi_anal.insert(make_pair(sub_a,tmp));
				}
				else
				{
					abpd_anal_ptr->bdi_anal[sub_a][sub_a]++;	
				}
			}
			
		
		}

		while(output.size()%8!=0)
		{
			output = output + "0";
		}
		for(int i=0;i<output.size()/8;i++)
		{
			unsigned char a ='0';
			data_to_send.push_back(a);
		}
	}
	else if(type_bdi.compare("1111")==0)
	{
			string rem_a = output.substr(4);
		if(abpd_anal_ptr)
		{
			
			for(int i=0;i<rem_a.size();i+=8)
			{
				string sub_a = rem_a.substr(i,8);
				if(abpd_anal_ptr->bdi_anal.find(sub_a)==abpd_anal_ptr->bdi_anal.end())
				{
					unordered_map<string,int> tmp;
					tmp.insert(make_pair(sub_a,1));
					abpd_anal_ptr->bdi_anal.insert(make_pair(sub_a,tmp));

				}
				else
				{
					abpd_anal_ptr->bdi_anal[sub_a][sub_a]++;	
				}

			}
			

		}

		string decode = output.substr(4);
		decode = "F"+decode;
		vector<K> back_list;
		int i =0;
		//cout<<"Out abpd "<<output<<"\n";
		//cout<<"Dec abpd "<<decode<<"\n";
		while(i+2<=decode.size())
		{
			string sub = decode.substr(i,2);
			K val_get=hex_to_int(sub);
			data_to_send.push_back((unsigned char)val_get);
			i+=2;
		}

		if(i<decode.size())
		{
			string sub = decode.substr(i);
			while(sub.size()%2!=0)
			{
				sub="0"+sub;
			}
			K val_get=hex_to_int(sub);
			data_to_send.push_back((unsigned char)val_get);
		}
		//prepare_data_to_send(back_list,data_to_send);
	}
	else
	{
		string binary = type_bdi;
		string s_base = output.substr(4,base);	
		vector<char> bi = get_binary_hexa(s_base);

		for(int i=0;i<bi.size();i++)
		{
			binary.push_back(bi[i]);
		}

		for(int i=4+base;i<output.size();i+=(delta+1))
		{
			if(abpd_anal_ptr)
			{	
				
				string it = output.substr(i+1,delta);
				string rem_a = output.substr(i,1);
				if(abpd_anal_ptr->bdi_anal.find(rem_a)==abpd_anal_ptr->bdi_anal.end())
				{
					unordered_map<string,int> tmp;
					tmp.insert(make_pair(it,1));
					abpd_anal_ptr->bdi_anal.insert(make_pair(rem_a,tmp));
				}
				else
				{
					if(abpd_anal_ptr->bdi_anal[rem_a].find(it)==abpd_anal_ptr->bdi_anal[rem_a].end())
					{
						abpd_anal_ptr->bdi_anal[rem_a].insert(make_pair(it,1));
					}
					else
					{
						abpd_anal_ptr->bdi_anal[rem_a][it]++;	
					}

				}
				
			}

			binary = binary+output.substr(i,1);

			string hex_form = output.substr(i+1,delta);
			vector<char> binary_base = get_binary_hexa(hex_form);
			for(int i=0;i<binary_base.size();i++)
			{
				binary.push_back(binary_base[i]);
			}
		}
		// convert bin string to unsig char

		int i=0;
		while(i+8<=binary.size())
		{
			vector<char> to_send;
			for(int j=i;j<i+8;j++)
			{
				to_send.push_back(binary[i]);
			}
			data_to_send.push_back((unsigned char)get_int(to_send,false));
			i+=8;
		}

		string last = binary.substr(i);
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
}
// added by abpd
template<class K>
void hpcl_comp_bdi_pl_proc<K>::run(vector<pair<string,Blocker*> > tracker)
{

	//cout<<"=========================ABPD==============================\n";
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
	cout<<" SID .. "<<mf->get_sid()<<"\n";
	unsigned comp_bit_size = 0;

	// init local dict
	lookup_hexa();

	cout<<"new code -> checkk checkk \n";
	cout<<"abpd mod_data = 0x"<<packet<<"\n";

	string output;
	string base_str;
	int used_base =0,used_delta=0;
	bool is_last_ex=true;
	//comp_bit_size+=4;
	bool zero_flag=all_zero_check(packet,output);
	if(zero_flag==false)
	{
	
		for(int i=0;i<tracker.size();i++)
		{
			std::vector<string> part_output;
		
			bool all_zero=check_zero(packet,packet.size(),part_output,tracker[i].second->base*2,tracker[i].second->delta*2);
			used_base=tracker[i].second->base*2;
			used_delta=tracker[i].second->delta*2;
			cout<<"Part output"<<part_output.size()<<endl;
			if(all_zero==true)
			{
				base_str.assign(get_zero_out(used_base));
				output.assign(tracker[i].second->code+base_str+form_output(part_output));
				is_last_ex=false;
				break;
			}

			if(check_base(part_output,tracker[i].second->base*2,tracker[i].second->delta*2,base_str))
			{
				cout<<" check base must be true \n";
				cout<<tracker[i].second->code<<"\t"<<base_str<<endl;
				output.assign(tracker[i].second->code+base_str+form_output(part_output));
				is_last_ex=false;
				break;

			}
	
		}
		if(is_last_ex==true)
		{
			used_base=-1;
			used_delta=-1;
			output.assign("1111"+packet);
			comp_bit_size+=4+(packet.size()*4);

		}
		else 
		{
	
			for(int i=(4+used_base);i<output.size();i+=used_delta+1)
			{
				comp_bit_size+=(1+(used_delta*4));
			}
				cout<<"\nused_base="<<used_base<<"\tused_delta="<<used_delta<<endl;
			comp_bit_size+=(4 + used_base *4);
			cout<<"comp_bit="<<comp_bit_size<<endl;

		}

	}
	else if (zero_flag)
	{
		comp_bit_size+=(4+(packet.size()/8));
		cout<<"Enter here zero flag\tsize=\t"<<comp_bit_size<<endl;
	}

	cout<<"Abpd Output String "<<output<<" Output Size : "<<output.size()<<"\n";

	// decode output list to unsig char 
	vector<unsigned char> data_to_send;

	convert_output(output,data_to_send,used_delta,used_base);

	if(abpd_anal_ptr)
	{
		abpd_anal_ptr->display_red();
		abpd_anal_ptr->bdi_anal.clear();
	}
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
//function which try to compress it with zero base
template <class K>
bool hpcl_comp_bdi_pl_proc<K>::check_zero(string packet,int data_size,vector<string>& part_output,int base,int delta)
{
	bool all_zero=true;

	// base = base * 2
	string zero;
		
	for(int i=0;i<base;i++)
	{
		zero+="0";
	}
		
	for(int i=0;i<packet.size();i+=base)
	{
		string processed_input;
		string sub=packet.substr(i,base);
		if(zero_diff(sub,zero,delta,processed_input)==false)
		{
			part_output.push_back("#"+sub);
			all_zero=false;
		}
		else
		{
			part_output.push_back(processed_input);
		}
					
	}
	return all_zero;		
}
template <class K>
bool  hpcl_comp_bdi_pl_proc<K>::zero_diff(string src,string dst,int delta,string& p_input)
{
	// dst is our base in this case

	string src_delta=src.substr(0,src.size()-delta);
	string dst_delta=dst.substr(0,dst.size()-delta);
	


	if(src_delta.compare(dst_delta)!=0)
	{
		return false;
	}

	p_input.assign("0"+src.substr(src.size()-delta));
	return true;
}


template <class K>
void hpcl_comp_bdi_pl_proc<K>::form_string(std::vector<unsigned char>& cache_data,string& res)
{
		for(unsigned i = 0; i < cache_data.size(); i++)
		{
			res.push_back(static_cast<char>(cache_data[i]));
		}

}
template <class K>
bool hpcl_comp_bdi_pl_proc<K>::check_base(vector<string>& part_output,int base,int delta,string& base_str)
{
	bool is_base_set=false;
	string processed;

		
	for(int i=0;i<part_output.size();i++)
	{
		string cur=part_output[i];
		if(cur[0]=='#')
		{
			cur.erase(cur.begin());
			if(is_base_set==false)
			{
				base_str.assign(cur);
				is_base_set=true;
			}
			
			if(convert_base(cur,base_str,base,delta,processed)==false)
			{
				return false;
			}
			part_output[i].assign("1"+processed);	
		}
	}

	return true;
}
template <class K>
bool hpcl_comp_bdi_pl_proc<K>::convert_base(string src,string dst,int base,int delta,string& processed)
{
	
	
	string src_delta=src.substr(0,src.size()-delta);
	string dst_delta=dst.substr(0,dst.size()-delta);

	if(src_delta.compare(dst_delta)!=0)
	{
		return false;
	}


	//cout<<"base "<<dst<<"\n";
	vector<char> binary_base = get_binary_hexa(dst);

	long long a=0,b=0;
	if(binary_base[0]=='1')
		a = get_int(binary_base,true);
	else
		a = get_int(binary_base,false);
	
	//cout<<"Delta "<<src.substr(src.size()-delta)<<"\n";	
	vector<char> binary_delta = get_binary_hexa(src.substr(src.size()-delta));
			
			
	if(binary_delta[0]=='1')
		b = get_int(binary_delta,true);
	else	
		b = get_int(binary_delta,false);

	//cout<<"Compress base delta\n";
	//cout<<a<<"\t"<<b<<" encode uuu "<<"\n";


	long long two_c=a*-1;

	//cout<<"tw_c"<<two_c<<"\n";
	//cout<<"base "<<b<<"src "<<a<<"\n";		
	processed.assign(convert_hexa(b+two_c,delta));

	//cout<<"base c "<<processed<<"\n";
	return true;
	
}
template <class K>
long long hpcl_comp_bdi_pl_proc<K>::convert_int(string s)
{
	//cout<<" int st "<<s<<"\n";
	long long output=0;
	int size=s.size()-1; 
	// string to int ( string by def is hexadecimal )
	for(int i=0;i<s.size();i++) 
	{
		char p = s[i];

		if(lc_cmp.find(p)!=lc_cmp.end())
		{
			output+=lc_cmp[p]*pow(16,size);	
		}
		else
		{
			output+=(p-'0')*pow(16,size);
		}
		size--;		
	}
		
	return output;
}
template <class K>
string hpcl_comp_bdi_pl_proc<K>::convert_hexa(long long num,int delta)
{
	
	// convert int to hexa and return output in form of string
	int limit=delta*4;
	bool is_neg=false;

	string output;

	if(num<0)
	is_neg=true;
	
	vector<char> binary = get_binary(abs(num),limit);
		
	vector<char> two_c;
	if(is_neg==true)
	two_c=get_binary_c(binary);
	else
	two_c=binary;
	

	for(int i=0;i<two_c.size();i+=4)
	{
		int val=0;
		int count=3;
		for(int j=i;j<i+4;j++)
		{
			val+=(two_c[j]-'0')*pow(2,count);
			count--;
		}

		if(lc_dcp.find(val)!=lc_dcp.end())
		{
			string s;
			s.push_back(lc_dcp[val]);
			output+=s;
		}
		else
		{
			stringstream ss;
			ss << val;
			string tmp = ss.str();
			/*
			string tmp;
			int tmp_v=val;
			cout<<tmp_v<<"\n";
			if(val==0) tmp+="0";
			else
			{
			while(tmp_v>0)
			{
				cout<<tmp<<"\n";
				tmp.insert(tmp.begin(),tmp_v%10+'0');
				tmp_v=tmp_v/10;
			}
			}
			cout<<" IMP ABPD "<<tmp<<"\n";
			*/
			output+=tmp;
		//	output+=static_cast<string>(val);
		}

	}
	//cout<<"hex out "<<output<<"\n";
	return output;	
}
template <class K>
vector<char> hpcl_comp_bdi_pl_proc<K>::get_binary_c(vector<char> b)
{
	vector<char> res;
	int carry=1;
	for(int i=b.size()-1;i>=0;i--)
	{
		if(b[i]=='0')
		{
			int val=(1^carry);
			res.insert(res.begin(),'0'+val);
			carry= 1 & carry;
		}
		else
		{
			int val2=(0^carry);
			res.insert(res.begin(),'0'+val2);
			carry = 0;
		}	
	}
	return res;
}
template <class K>
vector<char> hpcl_comp_bdi_pl_proc<K>::get_binary(long long num,int limit)
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
long long hpcl_comp_bdi_pl_proc<K>::get_two_c(long long num,int delta)
{
	long long output=0;
	while(num>0)
	{
		int val =(num & 1);
		if(val!=1)
		output = output | 1;	
		num>>=1;
		if(num>0)
		output<<=1;
	}

	return output;	
}
template <class K>
string hpcl_comp_bdi_pl_proc<K>::form_output(vector<string> part_output)
{
	string res;

	for(int i=0;i<part_output.size();i++)
	{
		string s = part_output[i];
		res+=s;
	}
	return res;
}
template <class K>
vector<char> hpcl_comp_bdi_pl_proc<K>::get_binary_hexa(string str)
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
		vector<char> temp= get_binary(val,4);
		for(int j=0;j<temp.size();j++)
		{
			char p = temp[j];
			res.push_back(p);
		}
	}

	return res;
}
// get binary corresponding to negative number
template <class K>
long long hpcl_comp_bdi_pl_proc<K>::get_int(vector<char> binary,bool is_neg)
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

#endif /* HPCL_COMP_PL_PROC_H_ */
