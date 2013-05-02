#include <vector>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#include <algorithm>
#include <cmath>
#include "operator_extend.hpp"
#include "macros.h"
#include "model.hpp"
using namespace std;
using namespace boost::archive;


class Kernel{
	EMPTY_SERIAL()
public:
	virtual double f(double x)=0;
	virtual double df(double x)=0;
};

class Sigmoid:public Kernel{
	EMPTY_SERIAL_BASE(Kernel);
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
	EMPTY_SERIAL_BASE(Kernel);
public:
	virtual double f(double x){
		return x;
	}
	virtual double df(double x){
		return 1;
	}
};

BOOST_CLASS_EXPORT(Linear)
BOOST_CLASS_EXPORT(Sigmoid)

class Cell;

struct Link{
	Cell* cell;
	double w,dw,dwm;
	SERIAL_ARG_2(cell,w)
};
ostream& operator<<(ostream& out,const Link& link){
	return out<<" cell:"<<link.cell<<" w:"<<link.w<<" ";
}

class Cell{
protected:
	Kernel* kernel;
	double bias,db,dbm;

	double input,output,error;

	vector<Link> linkIns;
	SERIAL_ARG_3(kernel,bias,linkIns)

	void collectInput(){
		input = bias;
		for(int i=0;i<linkIns.size();++i){
			input += linkIns[i].cell->getState() * linkIns[i].w;
		}
	}
public:
//methods for network
	Cell(Kernel* k=NULL):kernel(k),bias(_random(-0.5,0.5)),input(0),output(0),error(0),db(0){
		dbm=0;
	}
	void addLinkIn(Cell* cell,double w){
		Link link; 
		link.cell = cell; link.w = w; link.dw = 0; link.dwm =0;
		linkIns.push_back(link);	
	}
	void setBias(double b){ bias=b;}
	void addError(double e){error+=e;}
	void updateState(){
		collectInput();
		output = kernel->f(input);
		error = 0;
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
	void updateParameters(double learning=0.01,double regular=0.0,int n=1,double momentum=0.05,double eps=1e-3){
		bias -= (db/n + bias/n*regular + dbm*momentum)*learning;		
		dbm =db; db =0;
		for(int i=0;i<linkIns.size();++i){
			linkIns[i].w -= (linkIns[i].dw/n + linkIns[i].w/n*regular +linkIns[i].dwm*momentum)*learning;	
			if(eps>0 && fabs(linkIns[i].w)<eps && fabs(linkIns[i].dw/n) < eps/100){
				linkIns.erase(linkIns.begin()+i);
				--i;
				cerr<<" remove occurs"<<endl;
				continue;
			}
			linkIns[i].dwm = linkIns[i].dw; linkIns[i].dw =0;
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
	double density; // link density
	LayerType(Kernel* k=NULL,int s=0,bool l=false,double d=1):kernel(k),size(s),links(l),density(d){}
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


class Network : public Model<double> {
protected:
	vector<Cell*> inputs,hiddens,outputs;
	int ni,nh,no;
	SERIAL_ARG_6(inputs,hiddens,outputs,ni,nh,no);

	void addCells(vector<Cell*>& layer,LayerType cType,vector<Cell*>& pres,LayerType pType){
		int np = pres.size(),nl=layer.size();
		double eps = 1.0/np;
		for(int i=0;i<cType.size;++i){
			Cell *cell =new Cell(cType.kernel);
			for(int j=0;j<pType.size;++j){
				if(_random()<=cType.density){
					cell->addLinkIn(pres[np-1-j],_random(-eps,eps));
				}
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
				,double learning,double regular=0,double momentum=0.05
				,double vanish=0.0
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
		for(int i=0;i<no;++i) outputs[i]->updateParameters(learning,regular,to-from,momentum,vanish);
		for(int i=0;i<nh;++i) hiddens[i]->updateParameters(learning,regular,to-from,momentum,vanish);
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
		ni = inputs.size(); nh=hiddens.size(); no=outputs.size();
	}
// methods for applications
	double train(const vector<vector<double> >& x,const vector<vector<double> >& y){
		train(x,y,0.01,0,0,1);
		vector<vector<double> > predicts;
		predict(x,predicts);
		return rmse(y,predicts);
	}
	void fit(const vector<vector<double> >& x,const vector<vector<double> >& y){
		train(x,y,0.01);
	}
	void train(const vector<vector<double> >& x,const vector<vector<double> >& y
			,double learning,double regular=0.0,double momentum=0.05
			,int rounds=100,double eps=0.01
			,int batch=0,int repeat=1
			,int vanishEnd=10,double vanish=1e-3
			,const char *saveDir = NULL
			){
		int n = x.size();
		if(batch<=0) batch=n;
		vector<int> indexes(n);	for(int i=0;i<n;++i) indexes[i]=i;
		double last = 99e99,error=99e99;
		vector<vector<double> > predicts;
		double van = -1;
		for(int r=0;r<rounds;++r){
			for(int p=0;p<repeat;++p){
				random_shuffle(indexes.begin(),indexes.end());
				for(int i=0;i<n;i+=batch){
					doTrain(x,y,indexes,i,i+batch,learning,regular,momentum,van);
				}
			}
			if(saveDir){
				char path[1024]; sprintf(path,"%s/%d.model",saveDir,r);
				save(path);
			}
			predict(x,predicts);
			error = rmse(y,predicts);
			cout<<"R"<<r<<": rmse="<<error<<endl;
			if(error<eps || fabs(error-last) < eps*eps) break;
			//if(error<eps) break;
			last = error;
			if(r<vanishEnd) van = vanish;
			else van = -1;
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

