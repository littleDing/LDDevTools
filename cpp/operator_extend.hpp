#pragma once 

#include <iostream>
#include <vector>
using namespace std;

template<class T>
ostream& operator<<(ostream& out,const vector<T>& a){
	out<<"<";
	for(int i=0;i<a.size();++i){
		if(i!=0) out<<",";
		out<<a[i];
	}
	out<<">";
	return out;
}

