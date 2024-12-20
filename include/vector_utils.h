/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _vector_utils_H
#define _vector_utils_H

#include <cmath>
#include <numeric>
#include <complex>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream> 
#include <iomanip> 
#include <fstream> 

//Vector scalar unary op
template<typename T, typename S> std::vector<T>& operator+=(std::vector<T>& a, const S& s)
{
	std::for_each(a.begin(), a.end(), [&s](T& item) { item += s; });
	return a;
};

template<typename T, typename S> std::vector<T>& operator-=(std::vector<T>& a, const S& s)
{
	std::for_each(a.begin(), a.end(), [&s](T& item) { item -= s; });
	return a;
};

template<typename T, typename S> std::vector<T>& operator*=(std::vector<T>& a, const S& s)
{
	std::for_each(a.begin(), a.end(), [&s](T& item) { item *= s; });
	return a;
};

template<typename T, typename S> std::vector<T>& operator/=(std::vector<T>& a, const S& s)
{
	std::for_each(a.begin(), a.end(), [&s](T& item) { item /= s; });
	return a;
};

//Vector scalar binary op
template<typename T, typename S> std::vector<T> operator+(const std::vector<T>& a, const S& s)
{
	std::vector<T> b = a;
	return b += s;
};

template<typename T, typename S> std::vector<T> operator-(const std::vector<T>& a, const S& s)
{
	std::vector<T> b = a;
	return b -= s;
};

template<typename T, typename S> std::vector<T> operator*(const std::vector<T>& a, const S& s)
{
	std::vector<T> b = a;
	return b *= s;
};

template<typename T, typename S> std::vector<T> operator/(const std::vector<T>& a, const S& s)
{
	std::vector<T> b = a;
	return b /= s;
};

template<typename T, typename S> std::vector<T> operator+(const S& s, const std::vector<T>& a)
{
	std::vector<T> b(a.size());
	for (size_t i = 0; i < b.size(); i++) b[i] = s + a[i];
	return b;
};

template<typename T, typename S> std::vector<T> operator-(const S& s, const std::vector<T>& a)
{
	std::vector<T> b(a.size());
	for (size_t i = 0; i < b.size(); i++) b[i] = s - a[i];
	return b;
};

template<typename T, typename S> std::vector<T> operator*(const S& s, const std::vector<T>& a)
{
	std::vector<T> b(a.size());
	for (size_t i = 0; i < b.size(); i++) b[i] = s * a[i];
	return b;
};

template<typename T, typename S> std::vector<T> operator/(const S& s, const std::vector<T>& a)
{
	std::vector<T> b(a.size());
	for (size_t i = 0; i < b.size(); i++) b[i] = s / a[i];
	return b;
};

//Vector vector unary op
template<typename T> std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b)
{
	for (size_t i = 0; i < a.size(); i++) a[i] += b[i];
	return a;
};

template<typename T> std::vector<T>& operator-=(std::vector<T>& a, const std::vector<T>& b)
{
	for (size_t i = 0; i < a.size(); i++) a[i] -= b[i];
	return a;
};

template<typename T> std::vector<T>& operator*=(std::vector<T>& a, const std::vector<T>& b)
{
	for (auto i = 0; i < b.size(); i++) a[i] *= b[i];
	return a;
};

template<typename T> std::vector<T>& operator/=(std::vector<T>& a, const std::vector<T>& b)
{
	for (size_t i = 0; i < a.size(); i++) a[i] /= b[i];
	return a;
};

//vector vector binary op
template<typename T> std::vector<T> operator*(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> c = a;
	return c *= b;
};

template<typename T> std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> c = a;
	return c += b;
};

template<typename T> std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> c = a;
	return c -= b;
};

template<typename T> std::vector<T> operator/(const std::vector<T>& a, const std::vector<T>& b)
{
	std::vector<T> c = a;
	return c /= b;
};


//Functions
template<typename T> void pow10_apply(std::vector<T>& v)
{
	std::for_each(v.begin(), v.end(), [](T& item){ item = std::pow(10.0, item); });
};

template<typename T> std::vector<T> pow10(const std::vector<T>& v)
{
	std::vector<T> a = v;
	pow10_apply(a); return a;
};

template<typename T> void log10_apply(std::vector<T>& v)
{
	std::for_each(v.begin(), v.end(), [](T& item){ item = std::log10(item); });
};

template<typename T> std::vector<T> log10(const std::vector<T>& v)
{
	std::vector<T> a = v;
	log10_apply(a); return a;
};

template<typename T> T min(const std::vector<T>& v)
{
	return *std::min_element(v.cbegin(), v.cend());
};

template<typename T> T max(const std::vector<T>& v)
{
	return *std::max_element(v.cbegin(), v.cend());
};

template<typename T> T sum(const std::vector<T>& v)
{
	T init = 0;
	return std::accumulate(v.cbegin(), v.cend(), init);
};

template<typename T> T mean(const std::vector<T>& v)
{
	return sum(v) / v.size();
};

template<typename T> T variance(const std::vector<T>& v)
{
	T vmean = mean(v);
	T init = 0.0;
	T sum = std::accumulate(v.cbegin(), v.cend(), init, [&vmean](T& total, const T& item){ return total += std::pow(item - vmean, 2.0); });
	return sum / v.size();
};

template<typename T> T stddev(const std::vector<T>& v)
{
	return sqrt(variance(v));
};

template<typename T>
void append(std::vector<T>& a, const std::vector<T>& b) {
	//This does not work for Intel: a.insert(std::end(a), std::begin(b), std::end(b));	
	a.insert(a.end(), b.begin(), b.end());
};

template<typename T>
void prepend(std::vector<T>& a, const std::vector<T>& b) {
	//This does not work for Intel: a.insert(std::begin(a), std::begin(b), std::end(b));
	a.insert(a.begin(), b.begin(), b.end());
};

template<typename T>
std::vector<T> concaternate(const std::vector<T>& a, const std::vector<T>& b) {
	std::vector<T> c = a;
	//This does not work for Intel: c.insert(std::end(c), std::begin(b), std::end(b));
	c.insert(c.end(), b.begin(), b.end());
	return c;
};

//Resize 2d array
template<typename T> void resize(std::vector<std::vector<T>>& m, size_t nrows, size_t ncols)
{
	m.resize(nrows);
	for (size_t i = 0; i < nrows; i++){
		m[i].resize(ncols);
	}
};

template<typename TA, typename TB> void cast(const std::vector< std::vector<TA> >& a, std::vector< std::vector<TB> >& b)
{
	size_t nr = a.size();
	b.resize(nr);
	for (size_t i = 0; i < nr; i++) {
		size_t nc = a[i].size();
		b[i].resize(nc);
		for (size_t j = 0; j < nc; j++) {
			b[i][j] = a[i][j];
		}
	}
};

//Stats on raw pointer
template<typename T> T min(const size_t n, const T* v)
{
	T m = *std::min_element(v, v + n);
	return m;
};

template<typename T> T max(const size_t n, const T* v)
{
	T m = *std::max_element(v, v + n);
	return m;
};

template<typename T> T sum(const size_t n, const T* v)
{
	return std::accumulate(v, v + n, 0.0);
};

template<typename T> T mean(const size_t n, const T* v)
{
	return sum(n, v) / n;
};

template<typename T> T stddev(const size_t n, const T* v)
{
	T vmean = mean(n, v);
	T init = 0.0;
	T sum = std::accumulate(v, v+n, init, [&vmean](T& total, const T& item){return total += std::pow(item - vmean, 2.0);});
	return sum / n;
};

template<typename T>
std::vector<T> increment(const size_t n, const T start = 0, const T inc = 1)
{
	std::vector<T> v(n);
	v[0] = start;
	for (size_t i = 1; i<n; i++) v[i] = v[i - 1] + inc;
	return v;
};

template<typename T>
std::vector<T> cumulative_sum(const std::vector<T>& v)
{
	std::vector<T> csum(v.size());
	std::partial_sum(v.begin(), v.end(), csum.begin(), std::plus<double>());	
	return csum;
};

template<typename T>
void write(const std::string filename, const std::vector<T>& x)
{
	std::ofstream of(filename);
	of.setf(std::ios_base::scientific);
	for (size_t i = 0; i < x.size(); i++) {
		of << x[i] << std::endl;
	}
};

template<typename T>
void write(const std::string filename, const std::vector<std::complex<T>>& x)
{
	std::ofstream of(filename);
	of.setf(std::ios_base::scientific);	
	for (auto i = 0; i < x.size(); i++) {
		of << x[i].real() << " " << x[i].imag() << std::endl;
	}
};

template<typename T>
void write(std::string filename, const T& x)
{
	std::ofstream of(filename);
	of.setf(std::ios_base::scientific);
	of << x << std::endl;
};

template<typename T>
std::vector<T> evenspace(const T& x1, const T& dx, const size_t& n)
{
	std::vector<T> xi(n);
	for (size_t i = 0; i < n; i++) {
		xi[i] = x1 + (T)(i)*dx;
	}
	return xi;
};

template<typename T>
std::vector<T> linspace(const T& x1, const T& x2, const size_t n)
{
	T dx = (x2 - x1) / (T)(n - 1);
	std::vector<T> xi(n);
	xi[0] = x1;
	for (size_t i = 1; i < n; i++) {
		xi[i] += xi[i - 1] + dx;
	}
	return xi;
};

template<typename T>
std::vector<T> log10space(const T& x1, const T& x2, const size_t n)
{
	std::vector<T> v = linspace(std::log10(x1), std::log10(x2), n);
	for (size_t i = 0; i < n; i++) {
		v[i] = std::pow(10.0, v[i]);
	}
	return v;
};

#endif

