/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _RANDOM_UTILS_H_
#define _RANDOM_UTILS_H_
#include <vector>
#include <chrono>
#include <random>

template<typename T>
T irand(const T& imin, const T& imax)
{
	std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<T> dist(imin, imax);
	return dist(gen);
};

template<typename T>
T urand(const T& rmin=0.0, const T& rmax=1.0)
{
	std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<T> dist(rmin, rmax);
	return dist(gen);	
};

template<typename T>
T nrand(const T& mean = 0.0, const T& stddev = 1.0) {
	std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::normal_distribution<T> dist(mean, stddev);	
	return dist(gen);
};

template<typename T>
void nrand(size_t n, T* x, const T& mean=0.0, const T& stddev=1.0)
{
	std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::normal_distribution<T> dist(mean, stddev);
	for (size_t i = 0; i < n; i++) {
		x[i] = dist(gen);
	}
};

template<typename T>
std::vector<T> nrand(const size_t& n, const T& mean = 0.0, const T& stddev = 1.0)
{	
	std::vector<T> x(n);
	nrand<T>(x.data(), mean, stddev);	
	return x;
};

template<typename T>
double gaussian_pdf(const T& mean, const T& std, const T& x)
{
	T p = std::exp(-0.5 * std::pow((x - mean) / std, 2.0)) / (std::sqrt(TWOPI) * std);
	return p;
};

#endif
