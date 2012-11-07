
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

