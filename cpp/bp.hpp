

#include <vector>
#include <algorithm>
#include <cmath>
#include "operator_extend.hpp"
using namespace std;

inline double _random(){return rand()*1.0/RAND_MAX;}

class Kernel{
public:
	virtual double f(double x)=0;
	virtual double df(double x)=0;
};

class Sigmoid:public Kernel{
public:
	virtual double f(double x){
		return 1.0/(1+exp(-x));
	}
	virtual double df(double x){
		double y = f(x);
		return y*(1-y);
	}
};

class Linear:public Kernel{
public:
	virtual double f(double x){
		return x;
	}
	virtual double df(double x){
		return 1;
	}
};

class Cell;

struct Link{
	Cell* cell;
	double w,dw;
};
ostream& operator<<(ostream& out,const Link& link){
	return out<<" cell:"<<link.cell<<" w:"<<link.w<<" ";
}

class Cell{
protected:
	Kernel* kernel;
	double bias,db;

	double input,output,error;

	vector<Link> linkIns;
	void collectInput(){
		input = bias;
		for(int i=0;i<linkIns.size();++i){
			input += linkIns[i].cell->getState() * linkIns[i].w;
		}
	}
public:
//methods for network
	Cell(Kernel* k):kernel(k),bias(_random()),input(0),output(0),error(0),db(0){}
	void addLinkIn(Cell* cell,double w){
		Link link; 
		link.cell = cell; link.w = w; link.dw = 0;
		linkIns.push_back(link);		
	}
	void setBias(double b){ bias=b;}
	void addError(double e){error+=e;}
	void updateState(){
		collectInput();
		output = kernel->f(input);
	}
	void backPropergate(){
		double dldx =error*kernel->df(input);
		db += dldx;
		for(int i=0;i<linkIns.size();++i){
			linkIns[i].cell->addError(dldx*linkIns[i].w);
			linkIns[i].dw += dldx*linkIns[i].cell->getState();
		}
		error = 0;
	}
	void updateParameters(double learning=0.01,double regular=0.0,int n=1){
		bias -= (db/n + bias*regular)*learning;		
		db = 0;
		for(int i=0;i<linkIns.size();++i){
			linkIns[i].w -= (linkIns[i].dw/n + linkIns[i].w*regular)*learning;	
			linkIns[i].dw =0;
		}
	}
//methods for other cell
	virtual double getState(){return output;}
	
	friend ostream& operator<<(ostream& out,const Cell& cell);
};

ostream& operator<<(ostream& out,const Cell& cell){
	out<<"{"<<" bias:"<<cell.bias<<" linkIns:"<<cell.linkIns<<" input:"<<cell.input<<" output:"<<cell.output<<"}";
	return out;
}

struct LayerType{
	Kernel *kernel;
	int size;
	bool links;	//links inside layer
	LayerType(Kernel* k=NULL,int s=0,bool l=false):kernel(k),size(s),links(l){}
};

double rmse(const vector<vector<double> > & y,const vector<vector<double> >& predict){
	double ret = 0.0;
	for(int i=0;i<y.size();++i){
		for(int j=0;j<y[i].size();++j){
			double t = y[i][j] - predict[i][j];
			ret += t*t;
		}
	}
	return sqrt(ret/y.size());
}

class Network{
protected:
	vector<Cell*> inputs,hiddens,outputs;
	int ni,nh,no;
	int hiddenLayers;

	void addCells(vector<Cell*>& layer,LayerType cType,vector<Cell*>& pres,LayerType pType){
		int np = pres.size(),nl=layer.size();
		for(int i=0;i<cType.size;++i){
			Cell *cell =new Cell(cType.kernel);
			for(int j=0;j<pType.size;++j){
				cell->addLinkIn(pres[np-1-j],_random()-0.5);
			}
			layer.push_back(cell);
		}
		if(cType.links){
			for(int i=0;i<cType.size;++i){
				for(int j=0;j<cType.size;++j){
					layer[nl+i]->addLinkIn(layer[nl+j],_random());
				}
			}
		}
	}
	void feedForward(const vector<double>& x){
		for(int i=0;i<ni;++i) {	inputs[i]->setBias(x[i]); inputs[i]->updateState();  } 
		for(int i=0;i<nh;++i) { hiddens[i]->updateState();  } 
		for(int i=0;i<no;++i) { outputs[i]->updateState();  } 
	}
	void doTrain(const vector<vector<double> >& x,const vector<vector<double> >& y
				,const vector<int>& indexes,int from,int to
				,double learning,double regular=0
				){
		if(to>=indexes.size()) to = indexes.size();
		for(int i=from;i<to;++i){
			feedForward(x[i]);
			for(int j=0;j<no;++j){
				outputs[j]->addError(outputs[j]->getState() - y[i][j] );
				outputs[j]->backPropergate();
			}
			for(int j=nh-1;j>=0;--j) hiddens[j]->backPropergate();
		}
		for(int i=0;i<no;++i) outputs[i]->updateParameters(learning,regular,to-from);
		for(int i=0;i<nh;++i) hiddens[i]->updateParameters(learning,regular,to-from);
	}
public:
	~Network(){	
	}
	void reset(LayerType input,const vector<LayerType>& hidden,LayerType output){
		inputs.clear();	outputs.clear(); hiddens.clear();
		LayerType empty;
		addCells(inputs,input,inputs,empty);
		addCells(hiddens,hidden[0],inputs,input);
		for(int i=1;i<hidden.size();++i){
			addCells(hiddens,hidden[i],hiddens,hidden[i-1]);
		}
		addCells(outputs,output,hiddens,hidden[hidden.size()-1]);
		hiddenLayers = hidden.size();
		ni = inputs.size(); nh=hiddens.size(); no=outputs.size();
	}
// methods for applications
	void train(const vector<vector<double> >& x,const vector<vector<double> >& y
			,double learning=0.01,double regular=0.0
			,int rounds=100,double eps=0.01
			,int batch=0,int repeat=1
			){
		int n = x.size();
		if(batch<=0) batch=n;
		vector<int> indexes(n);	for(int i=0;i<n;++i) indexes[i]=i;
		double last = 99e99,error=99e99;
		vector<vector<double> > predicts;
		for(int r=0;r<rounds;++r){
			for(int p=0;p<repeat;++p){
				random_shuffle(indexes.begin(),indexes.end());
				for(int i=0;i<n;i+=batch){
					doTrain(x,y,indexes,i,i+batch,learning,regular);
				}
			}
			predict(x,predicts);
			error = rmse(y,predicts);
			cout<<"R"<<r<<": rmse="<<error<<endl;
			if(error<eps || fabs(error-last) < eps*eps) break;
			//if(error<eps) break;
			last = error;
		}
	}
	void predict(const vector<vector<double> >& x,vector<vector<double> >& y){
		y.resize(x.size());
		int dimy = outputs.size();
		for(int i=0;i<x.size();++i){
			feedForward(x[i]);
			y[i].resize(dimy);
			for(int j=0;j<dimy;++j){
				y[i][j] = outputs[j]->getState();
			}
		}
	}
};

