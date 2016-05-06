// file for various analysis stats
#ifndef __COMP_ANAL__
#define __COMP_ANAL__

#include<unordered_map>
#include<utility>
#include<fstream>

using namespace std;

class abpd_comp_anal
{
	public:
		ofstream red_file;
		abpd_comp_anal()
		{
			red_file.open("red_file_for_comp.txt");
		}
		~abpd_comp_anal()
		{
			red_file.close();
		}
		unordered_map<string,unordered_map<string,int> > bdi_anal;
		void display_red()
		{
			auto it = bdi_anal.begin();
			for(;it!=bdi_anal.end();it++)
			{
				pair<string,unordered_map<string,int> > tmp = *it;
				cout<<" KEY ABPD "<<tmp.first<<"\t";
				auto pt = tmp.second.begin();
				for(;pt!=tmp.second.end();pt++)
				{
					pair<string,int> tmp1= *pt;
					cout<<tmp1.first<<"\t"<<tmp1.second<<"\n";		
				}
			}
		}
		void write_to_file()
		{
			auto it = bdi_anal.begin();
			for(;it!=bdi_anal.end();it++)
			{
				pair<string,unordered_map<string,int> > tmp = *it;
				red_file<<tmp.first<<"\t";
				auto pt = tmp.second.begin();
				for(;pt!=tmp.second.end();pt++)
				{
					pair<string,int> tmp1= *pt;
					red_file<<tmp1.first<<"\t"<<tmp1.second<<"\n";
					
				}
				red_file<<"==============================================\n";
			}

		}

};

#endif
