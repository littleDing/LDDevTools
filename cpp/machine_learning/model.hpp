#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <vector>
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "macros.h"
using std::vector;

template<typename FeatureType>
class Model{
	EMPTY_SERIAL();	
public:
	virtual double train(const vector<vector<FeatureType> >& x,const vector<vector<FeatureType> >& y)=0;
	virtual void fit(const vector<vector<FeatureType> >& x,const vector<vector<FeatureType> >& y)=0;
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

#endif
