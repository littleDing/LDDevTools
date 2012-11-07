#include <iostream>
#include "EasyBMP.h"
#include <string>
#include <map>
#include <set>
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

bool operator<(const RGBApixel& a,const RGBApixel& b){
	if(a.Red!=b.Red) return a.Red < b.Red;
	else if(a.Blue!=b.Blue) return a.Blue < b.Blue;
	else return a.Green < b.Green;
}

template<class T,class Distance>
void kmeans(const vector<T>& objs
			,vector<T>& centers
			,vector<int>& tags
			,Distance distance
			,int k
			,int max_round = -1,double eps=1e-6){
	int round = 0,n=objs.size();
	centers.clear();
	for(int i=0;i<k;++i){
		int k = rand()%n;
		centers.push_back(objs[k]);
	}
	//cerr<<centers<<endl;
	tags.resize(n);
	while(max_round==-1 || round<max_round){
		round++;
		cerr<<"round :"<<round<<endl;
		//update tags
		bool tag_change=false;
		for(int i=0;i<n;++i){
			double d = distance(objs[i],centers[0]);
			int tag = tags[i], pick = 0;
			for(int j=1;j<k;j++){
				double tmp = distance(objs[i],centers[j]);	
				if(tmp < d){
					d = tmp;
					pick = j;
				}
			}
			if(pick != tag){
				tags[i] = pick;
				tag_change = true;
			}
		}
		cerr<<"tags updated"<<endl;
		//update centers
		bool center_updated = false;
		vector<T> new_centers;
		vector<int> cnts;
		new_centers.resize(k);
		for(int i=0;i<k;++i){
			cnts.push_back(0);
		}
		for(int i=0;i<n;++i){
			int t = tags[i];
			if(cnts[t] == 0){
				new_centers[t] = objs[i];
			}else{
				new_centers[t] +=  objs[i];
			}
			cnts[t]++;
		}
		//cerr<<centers<<"::"<<cnts<<endl;
		for(int i=0;i<k;++i){
			new_centers[i] /= cnts[i];
			if(distance(new_centers[i],centers[i])>eps){
				centers[i] = new_centers[i];
				center_updated = true;
			}
		}
		cerr<<"center updated"<<endl;
		if(!center_updated){
			break;
		}
	}
	//cerr<<centers<<endl;
}

template<class T,size_t K>
class Tuple{
	T data[K];
public:
	const T& operator[](size_t index) const {
		return data[index];
	}
	T& operator[](size_t index){
		return data[index];
	}
	Tuple<T,K>& operator /= (const double& d){
		for(int i=0;i<K;++i)
			data[i]/=d;
		return *this;
	}
	Tuple<T,K>& operator +=(const Tuple<T,K>& other){
		for(int i=0;i<K;++i)
			data[i]+=other.data[i];
		return *this;
	}
	struct Distance{
		double operator()(const Tuple<T,K>& a,const Tuple<T,K>& b){
			double d =0;
			for(int i=0;i<K;++i){
				double t = a.data[i]-b.data[i];
				d+=t*t;
			}
			return sqrt(d);
		}
	};
};
template<class T,size_t K>
ostream& operator<<(ostream& out,const Tuple<T,K>& a){
	out<<" <";
	for(int i=0;i<K;++i){
		if(i!=0) out<<",";
		out<<a[i];
	}
	out<<"> ";
	return out;
}

class Colors{
	map<RGBApixel,int> colors;
	int cnt;	
	vector<RGBApixel> centers;
	typedef Tuple<double,3> D3;
	RGBApixel tran(D3 d){
		RGBApixel c;
		c.Red = d[0];
		c.Blue = d[1];
		c.Green = d[2];
		return c;
	}
	D3 tran(RGBApixel c){
		D3 d;
		d[0]=c.Red;
		d[1]=c.Blue;
		d[2]=c.Green;
		return d;
	}
public:
	void clear(){
		colors.clear();
		cnt = 0;
	}
	void addColor(RGBApixel* color){
		colors[*color] = cnt;
		cnt ++;
	}
	void culculate(int num){
		cerr<<"totally "<<colors.size()<<" colors here"<<endl;
		vector<D3 > objs,cs;
		vector<RGBApixel> origin;
		for(map<RGBApixel,int>::iterator it=colors.begin();it!=colors.end();++it){
			objs.push_back(tran(it->first));
			origin.push_back(it->first);
		}
		vector<int> tags;
		kmeans(objs,cs,tags,Tuple<double,3>::Distance(),num,-1,1);
		for(int i=0;i<objs.size();++i){
			colors[origin[i]] = tags[i];
		}
		for(int i=0;i<cs.size();++i){
			centers.push_back(tran(cs[i]));
		}
		cerr<<"culculate ends"<<endl;
	}
	void mapColor(RGBApixel* color){
		int k = colors[*color];
		RGBApixel c = centers[k];
		color->Red = c.Red;
		color->Blue = c.Blue;
		color->Green = c.Green;
	}
};




int solve(int num,string filename){
	BMP input,output;
	input.ReadFromFile(filename.c_str());
	int h =input.TellHeight() ,w=input.TellWidth();
	output.SetSize(w,h);
	RangedPixelToPixelCopy(input,0,w-1,h-1,0,output,0,0);
	output.SetBitDepth(input.TellBitDepth());
	Colors colors;
	for(int i=0;i<w;++i){
		for(int j=0;j<h;++j){
			RGBApixel* color = output(i,j);
			colors.addColor(color);
		}
	}
	colors.culculate(num);
	for(int i=0;i<w;++i){
		for(int j=0;j<h;++j){
			RGBApixel* color = output(i,j);
			colors.mapColor(color);
		}
	}
	output.WriteToFile("output.bmp");
}

int main(int argc,char** argv){
	cout<<"hello world!"<<endl;
	int num = 0;
	sscanf(argv[1],"%d",&num);
	solve(num,argv[2]);
	return 0;
}
