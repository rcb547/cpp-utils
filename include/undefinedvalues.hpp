/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <cstdint>
#include <limits>
#include <string>

constexpr short _undefined_short_ = std::numeric_limits<short>::lowest();
constexpr int _undefined_int_ = std::numeric_limits<int>::lowest();
constexpr size_t _undefined_size_t_ = (std::numeric_limits<size_t>::max)();
constexpr float _undefined_float_ = std::numeric_limits<float>::lowest();
constexpr double _undefined_double_ = std::numeric_limits<double>::lowest();
constexpr char _undefined_stdstring_[] = "Undefined std::string";

inline short _undefinedvalue(const short&){
	return _undefined_short_;
}

inline int _undefinedvalue(const int&){
	return _undefined_int_;
}

inline size_t _undefinedvalue(const size_t&){
	return _undefined_size_t_;
}

inline float _undefinedvalue(const float&) {
	return _undefined_float_;
}

inline double _undefinedvalue(const double&){
	return _undefined_double_;
}

inline std::string _undefinedvalue(const std::string&){
	return _undefined_stdstring_;
}

//Does not make sense to have an undefined char really
//inline char _undefinedvalue(const char&) {
//	return 0;
//}

template <class T>
inline T undefinedvalue(){
	static T v;
	return _undefinedvalue(v);
}

template <class T>
bool isdefined(const T& v) {
	if (v == undefinedvalue<T>()) return false;
	return true;
}
