#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <vector>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "macros.h"
#include <cmath>
using std::fabs;
using std::vector;

template<typename FeatureType>
class Model{
	EMPTY_SERIAL();	
protected:
	int rounds,repeats;
	double eps;
	const char* saveDir;
public:	
	void reset(int _rounds=100,double _eps=1e-6,const char *_saveDir="model"){
		rounds = _rounds; eps = _eps; saveDir = _saveDir;
	}
	virtual resetModel(const vector<vector<FeatureType> >& x,const vector<vector<FeatureType> >& y)=0;
	virtual double train(const vector<vector<FeatureType> >& x,const vector<vector<FeatureType> >& y)=0;
	virtual void fit(const vector<vector<FeatureType> >& x,const vector<vector<FeatureType> >& y){
		resetModel(x,y);
		double last = 99e99;
		int unchange = 0;
		for(int r=0;r<rounds;++r){
			double error = train(x,y);
			if(saveDir){
				char path[1024]; sprintf(path,"%s/%d.model",saveDir,r);
				save(path);
			}
			if( fabs(error-last) < eps ){
				unchange++;
				if(unchange >=3) break;
			}else unchange = 0;
			last = error;
		}
	}
	virtual void predict(const vector<vector<FeatureType> >& x,vector<vector<FeatureType> >& y)=0;
	void save(const char *path){
		std::ofstream fout(path);
		boost::archive::binary_oarchive oa(fout);
		oa<<*this;
	}
	void load(const char *path){
		std::ifstream fin(path);
		boost::archive::binary_iarchive ia(fin);
		ia>>(*this);
	}
};

inline double _random(double from=0.0,double to=1.0){return rand()*1.0/RAND_MAX*(to-from)+from;}
template<typename FeatureType>
double rmse(const vector<vector<FeatureType> > & y,const vector<vector<FeatureType> >& predict){
	FeatureType ret = 0.0;
	for(int i=0;i<y.size();++i){
		for(int j=0;j<y[i].size();++j){
			double t = y[i][j] - predict[i][j];
			ret += t*t;
		}
	}
	return sqrt(ret/y.size());
}

template<typename FeatureType>
inline FeatureType fix(FeatureType value,FeatureType from,FeatureType to){
	if(value<from) return from;
	else if(to<value) return to;
	else return value;
}

#endif
