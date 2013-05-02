#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include "bp.hpp"
#include "operator_extend.hpp"
using namespace std;

void make(int n,int nx,vector<vector<double> >& x,vector<vector<double> >& y){
	x.resize(n); y.resize(n);
	for(int i=0;i<n;i++){
		x[i].resize(nx);
		y[i].resize(1);
		bool ans = true;
		for(int j=0;j<nx;++j) {
			bool tmp = rand()%2;
			ans ^= tmp;
			x[i][j] = tmp;
		}
		y[i][0] = ans;
	}
}


double precision(const vector<vector<double> >& tags,const vector<vector<double> >& output,double eps=0.1){
	double loss = 0.0;
	for(int i=0;i<tags.size();++i){
		for(int j=0;j<tags[i].size();++j){
			bool t = tags[i][j]>=0.5;
			if(t&&output[i][j]>=1-eps || !t&&output[i][j]<=eps)
				loss +=1;
		}		
	}
	loss/=tags.size();
	return loss;
}


bool path_exist(const char *path){
	ifstream fin(path);
	if(!fin) return false;
	else return true;
}

void test(){
	Network net;
	int nx = 100,ny=1,n=nx*log(nx);
	vector<vector<double> > x,y,yy(n);
	char path[1024]; sprintf(path,"bpnn.xor.nx%d.ny%d.model",nx,ny);
	if(path_exist(path)){
		net.load(path);
	}else{
		make(n,nx,x,y);	
		LayerType input,output;
		input.size = nx;  input.kernel = new Linear();
		output.size =ny;  output.kernel = new Sigmoid();
		int all = 0,last = nx;
		vector<LayerType> hiddens(1);
		for(int i=0;i<hiddens.size();++i){
			hiddens[i].kernel = new Sigmoid();
			hiddens[i].size = nx;
			hiddens[i].density = 1;
			cout<<"hidden size of "<<hiddens[i].size<<endl;
			all += hiddens[i].size * last * hiddens[i].density;
			last = hiddens[i].size;
		}
		all +=last*ny;
		cout<<"all links should be :"<<all<<endl;
		net.reset(input,hiddens,output);
		double rounds=100,eps = 0.001,learning=0.1,regular=0.0001,momentum=0.1,batch=1,repeat=100,vanE=-1,vanish=1e-3;
		net.train(x,y,learning,regular,momentum,rounds,eps,batch,repeat,vanE,vanish,"models");
		net.dump(path);
	}
	net.predict(x,yy);
	for(int i=0;i<n;++i){
	//	cerr<<x[i]<<"\t"<<y[i]<<"\t"<<yy[i]<<endl;
	}
	make(n,nx,x,y);
	net.predict(x,yy);
	double loss = rmse(y,yy);
	cout<<"loss="<<loss<<endl;
	for(int i=0;i<5;++i){
		cout<<"precision@"<<0.1*i<<"="<<precision(y,yy,0.1*i)<<endl;
	}
	for(int i=0;i<n;++i){
	//	cerr<<x[i]<<"\t"<<y[i]<<"\t"<<yy[i]<<endl;
	}
	//cout<<yy<<endl;
}

int main(){
	cout<<"hello world"<<endl;
	cout<<log(10)<<" "<<log(100)<<endl;
	test();
	return 0;
}
