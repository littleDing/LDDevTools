#include <vector>
using namespace std;

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

