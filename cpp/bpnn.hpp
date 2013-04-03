#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>
using namespace std;

template<class T>
ostream& operator<<(ostream& out,const vector<T>& a){
	out<<" [";
	for(int i=0;i<a.size();++i){
		if(i!=0) out<<",";
		out<<a[i];
	}
	out<<"] ";
	return out;
}
class Func{
public:
	virtual double f(double x)=0;
	virtual double df(double x)=0;
};
class Liner : public Func{
public:
	double f(double x){return x;}
	double df(double x){return 1;}
};
class Sigmoid : public Func{
public:
	double f(double x){
		return 1/(1+exp(-x));
	}
	double df(double x){
		double y = f(x);
		return y*(1-y);
	}
};

inline double _random(){
	return rand()*1.0/RAND_MAX;
}

class Cell{
protected:
	Func *kernel;
	double bias, learning;
	double stateIn,stateOut;

	double delta;
	struct Link{
		Cell* other;
		double weight;
	};
	vector<Link>  input;
	double collectInput(){
		double x = 0;
		for(int i=0;i<input.size();++i){
			x += input[i].other->getState() * input[i].weight;
		}
		return x;
	}
public:
	void reset(Func *k,double l = 0.01){
		kernel = k;
		bias = _random();
		learning = l;
		delta = 0.0;
		update();
		input.clear();
	}
	double getState(){return stateOut;}
	double setState(double s){stateOut=s;}
	double addDelta(double d){delta+=d;}
	void addLinkin(Cell* cell,double weight=0.0){
		Link link={cell,weight};
		input.push_back(link);
	}
	void update(){
		stateIn  = collectInput() + bias; 
		stateOut = kernel->f(stateIn);
		delta = 0;
	}
	void upgrade(double l=-1,double regular=0.01){
		if(l<0) l = learning;
		double dldy = delta ;
		double dydx = kernel->df(stateIn);
		bias -= (dldy*dydx + regular*bias )*l;
		for(int i=0;i<input.size();++i){
			double dxdw = input[i].other->getState();
			double dxds = input[i].weight;
			input[i].weight -= (dldy*dydx*dxdw + regular*input[i].weight)*l;
			input[i].other->addDelta(dldy*dydx*dxds);
		}
		delta = 0;
	}
};

double mse(const vector<vector<double> >& tags,const vector<vector<double> >& output){
	double loss = 0.0;
	for(int i=0;i<tags.size();++i){
		for(int j=0;j<tags[i].size();++j){
			double tmp = tags[i][j] - output[i][j];
			loss +=tmp*tmp;
		}		
	}
	loss/=tags.size();
	return loss;
}

class Net{
public:
	vector<vector<Cell*> > layers;
	void reset(vector<Cell*>& cells,int num,const vector<Cell*>& from,Func *kernel){
		cells.resize(num);
		for(int i=0;i<num;++i){
			cells[i] = new Cell();
			cells[i]->reset(kernel);
			for(int j=0;j<from.size();++j){
				cells[i]->addLinkin(from[j],_random());
			}	
		}
	}
public:
	void reset(int numInput,int numOutput,const vector<int>& numHidden){
		int nh = numHidden.size();
		layers.resize(nh+2);
		vector<Cell*> empty;
		Func *liner = new Liner(),*sigmoid = new Sigmoid();
		reset(layers[0],numInput,empty,liner);
		for(int i=0;i<nh;++i) reset(layers[i+1],numHidden[i],layers[i],sigmoid);
		reset(layers[nh+1],numOutput,layers[nh],liner);
	}
	void train(const vector<double>& data,const vector<double>& tag,double learning,double regular){
		vector<double> output;
		predict(data,output);
		int nl = layers.size();
		for(int i=0;i<layers[nl-1].size();++i){	
			layers[nl-1][i]->addDelta(output[i]-tag[i]);
		}
		for(int i=nl-1;i>0;--i){
			for(int j=0;j<layers[i].size();++j){
				layers[i][j]->upgrade(learning,regular);
			}
		}
	}
	void train(const vector<vector<double> >& datas,const vector<vector<double> >& tags
				,int rounds=100,double eps=0.01,double learning=-1,double regular=0.01,int repeat=1,int loground=5){
		vector<int> indexs(datas.size()); for(int i=0;i<datas.size();++i) indexs[i]=i;
		vector<vector<double> > output(datas.size());
		for(int r = 0;r<rounds;++r){
			for(int p=0;p<repeat;++p){
				random_shuffle(indexs.begin(),indexs.end());
				for(int i=0;i<indexs.size();++i){
					int j=indexs[i];
					//for(int h=0;h<10;++h)
					train(datas[j],tags[j],learning,regular);
				}
			}
			predict(datas,output);
			double loss = mse(tags,output);
			
			if(loss<eps || loground>0 && r%loground==0){
				cout<<"round "<<r<<" loss="<<loss<<endl;
				//cout<<"before:"<<endl;
				//cout<<x<<endl<<y<<endl;
				//cout<<"after:"<<endl;
				//cout<<output<<endl;
			}
			if(loss<eps) break;
		}
	}
	void predict(const vector<vector<double> >& datas,vector<vector<double> >& output){
		for(int i=0;i<datas.size();++i){
			predict(datas[i],output[i]);
		}
	}
	void predict(const vector<double>& data,vector<double>& output){
		int nl = layers.size();
		for(int i=0;i<layers[0].size();++i){
			layers[0][i]->setState(data[i]);
		}
		for(int i=1;i<nl;++i){
			for(int j=0;j<layers[i].size();++j){
				layers[i][j]->update();
			}
		}
		output.resize(layers[nl-1].size());
		for(int i=0;i<layers[nl-1].size();++i) {
			output[i] = layers[nl-1][i]->getState();
		}
	}
};
