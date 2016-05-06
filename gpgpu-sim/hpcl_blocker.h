// file for blocker data

#include<cstring>
#include<iostream>

using namespace std;

class Blocker
{

public:
	string code;
	string name;
	int base;
	int delta;
	int size;
	
	Blocker(string cd,string na,int bs,int dl,int sz)
	{
		code.assign(cd);
		name.assign(na);
		base =bs;
		delta=dl;
		size=sz;
	}
	~Blocker()
	{
	}
};
