/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _general_types_H
#define _general_types_H

#include <cstring>
#include <complex>
#include <vector>
#include <variant>
#include <map>
#include <optional>
#include <iomanip>
#include "undefinedvalues.h"
#include "string_utils.h"

//Short cut for setting std::fixed output witdh/decimals
class ixd {

public:
	const std::streamsize& width;	
	ixd(const std::streamsize& _width)
		: width(_width)
	{	};

	friend std::ostream& operator << (std::ostream& os, const ixd& fmt)
	{
		os << std::fixed;
		os << std::setw(fmt.width);
		//os << std::setprecision(0);
		return os;
	}

};

//Short cut for setting std::fixed output witdh/decimals
class fxd {

public:
	const std::streamsize& width;
	const std::streamsize& decimals;
	fxd(const std::streamsize& _width, const std::streamsize& _decimals)
		: width(_width), decimals(_decimals)
	{	};

	friend std::ostream& operator << (std::ostream& os, const fxd& fmt)
	{
		os << std::fixed;
		os << std::setw(fmt.width);
		os << std::setprecision(fmt.decimals);
		return os;
	}

};

//Short cut for setting std::scientific output witdh/decimals
class exd {

public:
	const std::streamsize& width;
	const std::streamsize& decimals;
	exd(const std::streamsize& _width, const std::streamsize& _decimals)
		: width(_width), decimals(_decimals)
	{	};

	friend std::ostream& operator << (std::ostream& os, const exd& fmt)
	{
		os << std::scientific;
		os << std::setw(fmt.width);
		os << std::setprecision(fmt.decimals);
		return os;
	}
};



typedef std::variant<double, int, float, char, std::vector<double>, std::vector<int>, std::vector<float>, std::vector<char>> cVrnt;
template <typename KeyType, typename ValType, typename Compare = std::equal_to<KeyType>>

//Unsorted vector of unique-key key,value pairs
class cKeyVec : public std::vector<std::pair<KeyType, ValType>> {

public:

	Compare compare;
	int keyindex(const KeyType& key) const {
		for (int i = 0; i < size(); i++) {			
			if (compare(key, (*this)[i].first)) return i;
		}
		return -1;
	}

	bool add(const std::pair<KeyType, ValType>& p) {
		if (keyindex(p.first) < 0) {
			push_back(p);
			return true;
		}
		return false;
	}

	bool add(const KeyType& key, const ValType& val) {
		if (keyindex(key) < 0) {
			push_back(std::pair(key, val));
			return true;
		}
		return false;
	}

	bool get(const KeyType& key, ValType& val) const {
		int i = keyindex(key);
		if (i < 0) {
			val = ValType{};
			return  false;
		}
		else val = (*this)[i].second;
		return true;
	}

	//ValType& operator[](const KeyType& key) {
	//	const& int i = keyindex(key);		
	//	return ((*this)[i]).second;
	//}
	
	std::pair<KeyType, ValType>&  pair(const KeyType& key) {
		const int& i = keyindex(key);
		std::pair<KeyType, ValType>& p = (*this)[i];
		return p;
	}
	
	ValType& cref(const KeyType& key) {
		const int& i = keyindex(key);
		return ((*this)[i]).second;		
	}

	std::optional<ValType> oget(const KeyType& key) {
		const int& i = keyindex(key);
		if (i < 0) return std::optional<ValType>{};
		else return std::optional<ValType>(((*this)[i]).second);
			
	}
	
	cKeyVec preferred_sort(const std::vector<std::string>& order) const {				
		const cKeyVec& in = *this;
		if (in.size() < 2)return *this;
		
		cKeyVec out;
		for (size_t i = 0; i < order.size(); i++) {
			int ki = in.keyindex(order[i]);
			if (ki >= 0) {
				out.add(in[ki]);
			}
		}

		for (size_t i = 0; i < in.size(); i++) {			
			out.add(in[i]);
		}
		return out;
	}
};

//Case insensitive std::string KeyVec
using cKeyVecCiStr = cKeyVec<std::string, std::string, caseinsensetiveequal<std::string>>;


template<typename T>
class cHistogramStats{

public:
	size_t nbins=0;//number of bins in histogram
	size_t nsamples=0;//number of samples in histogram
	T min=0;//minimum of sample
	T max=0;//maximum of sample
	T mean=0;
	T std=0;
	T var=0;
	T mode=0;
	T p10=0;
	T p50=0;
	T p90=0;

	cHistogramStats(){
		nbins = 0;
	}

	template<typename U>
	cHistogramStats(const std::vector<T>& bins, const U* counts){
		compute(bins, counts);
	};

	template<typename U>
	void compute(const std::vector<T>& bins, const U* counts)
	{
		nbins = bins.size();

		min = bins[nbins-1];
		max = bins[0];

		T sum = 0.0;
		nsamples = 0;
		std::vector<size_t> cumcounts(nbins + 1, 0);
		for (size_t i = 0; i < nbins; i++){
			U c = counts[i];
			nsamples += c;
			cumcounts[i + 1] = cumcounts[i] + c;

			T s = bins[i] * (T)c;
			sum += s;

			if (c>0){
				if (bins[i] < min)min = bins[i];
				if (bins[i] > max)max = bins[i];
			}
		}
		mean = sum / (T)nsamples;
		T np10 = 0.1*(T)nsamples;
		T np50 = 0.5*(T)nsamples;
		T np90 = 0.9*(T)nsamples;

		size_t modebin = 0;
		T sumdsqr = 0.0;
		//nearest-rank method for percentiles
		bool set10 = true;
		bool set50 = true;
		bool set90 = true;
		for (size_t i = 0; i < nbins; i++){
			T d = bins[i] - mean;
			sumdsqr += d*d*(T)counts[i];

			if (cumcounts[i] <= np10 && cumcounts[i + 1] >= np10 && set10) {
				p10 = bins[i];
				set10 = false;
			}
			if (cumcounts[i] <= np50 && cumcounts[i + 1] >= np50 && set50) {
				p50 = bins[i];
				set50 = false;
			}
			if (cumcounts[i] <= np90 && cumcounts[i + 1] >= np90 && set90) {
				p90 = bins[i];
				set90 = false;
			}

			if (counts[i] > counts[modebin]){
				mode = bins[i];
				modebin = i;
			}
		}
		var = sumdsqr / (T)nsamples;
		std = sqrt(var);
	}
};

template<typename T, typename U>
class cHistogram{

public:
	size_t nbins;
	std::vector<T> edge;
	std::vector<T> centre;
	std::vector<U> count;

	cHistogram(){
		nbins = 0;
	}

	cHistogram(const std::vector<T>& v, T hmin, T hmax, size_t _nbins){
		compute(v, hmin, hmax, _nbins);
	}


	void compute(const std::vector<T>& v, T hmin, T hmax, size_t _nbins)
	{
		nbins = _nbins;
		T dx = (hmax - hmin) / (nbins);
		T e = hmin;
		edge.push_back(e);
		for (size_t i = 0; i < nbins; i++){
			centre.push_back(e + dx / 2.0);
			e += dx;
			edge.push_back(e);
		}
		count.resize(nbins, 0);

		for (size_t i = 0; i < v.size(); i++){
			if (v[i] <= hmin) count[0]++;
			else if (v[i] >= hmax) count[nbins - 1]++;
			else {
				size_t b = (size_t)floor((v[i] - hmin) / dx);
				count[b]++;
			}
		}
	}
};


template<typename T>
class cStats{

public:
	size_t nulls;
	size_t nonnulls;
	T min;
	T max;
	T mean;
	T var;
	T std;

	cStats(){
		nulls = 0;
		nonnulls = 0;
	}

	cStats(const std::vector<T>& v){
		compute(v);
	}

	cStats(const std::vector<T>& v, T nullvalue){
		compute_with_nulls(v,nullvalue);
	}

	void compute(const std::vector<T>& v)
	{
		nulls    = 0;
		nonnulls = 0;
		min = v[0];
		max = v[0];

		double sx  = 0.0;
		double sx2 = 0.0;
		for (size_t i = 0; i < v.size(); i++){
			nonnulls++;

			if (v[i] < min) min = v[i];
			else if (v[i] > max) max = v[i];

			sx  += v[i];
			sx2 += (v[i] * v[i]);
		};
		mean = sx / nonnulls;
		var  = (sx2 - (sx*sx) / nonnulls) / (nonnulls - 1.0);
		std  = sqrt(var);
	}

	void compute_with_nulls(const std::vector<T>& v, const T nullvalue)
	{
		nulls    = 0;
		nonnulls = 0;

		double sx  = 0.0;
		double sx2 = 0.0;
		for (size_t i = 0; i < v.size(); i++){
			if (v[i] == nullvalue){
				nulls++;
				continue;
			}
			nonnulls++;

			if(nonnulls==1){
				min = v[i];
				max = v[i];
			}
			else if (v[i] < min) min = v[i];
			else if (v[i] > max) max = v[i];

			sx  += v[i];
			sx2 += (v[i] * v[i]);
		};
		mean = sx / nonnulls;
		var  = (sx2 - (sx*sx) / nonnulls) / (nonnulls - 1.0);
		std  = sqrt(var);
	}
};

template<typename T>
class cRange{

public:
	T from;
	T to;

	cRange(){
		from = undefinedvalue(from);
		to   = undefinedvalue(to);
	}

	cRange(const T& _from, const T& _to){
		from = _from;
		to   = _to;
	}

	bool valid(){
		if (isdefined(from) && isdefined(to)) return true;
		return false;
	}

};

struct sBoundingBox{
	double xlow;
	double xhigh;
	double ylow;
	double yhigh;
};

class cPoint{

public:

	double x=0;
	double y=0;

	cPoint(){};

	cPoint(const double& px, const double& py){
		x = px;
		y = py;
	}

	bool operator==(const cPoint& p){
		if (x == p.x && y == p.y)return true;
		return false;
	}

};

template <typename T>
class c3DArray{

private:
	std::vector<std::vector<std::vector<T>>> data;

public:
	c3DArray<T>(int ni = 0, int nj = 0, int nk = 0){ resize(ni, nj, nk); }
	void resize(int ni, int nj, int nk){
		data.resize(ni);
		for (int i = 0; i < ni; ++i){
			data[i].resize(nj);
			for (int j = 0; j < nj; ++j){
				data[i][j].resize(nk);
			}
		}
	}

	void initialise(const T& v){
		for (int i = 0; i < ni(); ++i){
			for (int j = 0; j < nj(); ++j){
				for (int k = 0; k < nk(); ++k){
					data[i][j][k] = v;
				}
			}
		}
	}

	std::vector<std::vector<T>>& operator[](int index) {
		return data[index];
	}

	c3DArray<T>& operator=(const T& v) {
		initialise(v);
		return *this;
	}

	int ni() const { return (int) data.size(); }
	int nj() const { return (int) data[0].size(); }
	int nk() const { return (int) data[0][0].size(); }
};


#endif
