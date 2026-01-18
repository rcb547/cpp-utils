/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include "logger.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <complex>
#include <typeinfo>

namespace CppUtils {
	constexpr short _undefined_short_ = std::numeric_limits<short>::lowest();
	constexpr int _undefined_int_ = std::numeric_limits<int>::lowest();
	constexpr size_t _undefined_size_t_ = (std::numeric_limits<size_t>::max)();
	constexpr float _undefined_float_ = std::numeric_limits<float>::lowest();
	constexpr double _undefined_double_ = std::numeric_limits<double>::lowest();
	constexpr char _undefined_stdstring_[] = "Undefined std::string";

	template <typename T>
	T _undefinedvalue() {
		const std::type_info& ti = typeid(T);
		glog.errormsg(_SRC_, "_undefinedvalue() not allowed for type (%s).\n", ti.name());
		return T(0);
	};

	template<> short _undefinedvalue<short>() { return _undefined_short_; };
	template<> int _undefinedvalue<int>() { return _undefined_int_; };
	template<> size_t _undefinedvalue<size_t>() { return _undefined_size_t_; };
	template<> float _undefinedvalue<float>() { return _undefined_float_; };
	template<> double _undefinedvalue<double>() { return _undefined_double_; };
	template<> std::string _undefinedvalue<std::string>() { return _undefined_stdstring_; };

	////Does not make sense to have an undefined char really
	//template<>
	//char _undefinedvalue<char>() {
	//	return 0;
	//};

	template <class T>
	inline T undefinedvalue() {
		return _undefinedvalue<T>();
	};

	template <>
	inline std::complex<double> undefinedvalue<std::complex<double>>() {
		const double v = undefinedvalue<double>();
		return std::complex<double>(v, v);
	};

	template <class T>
	bool isdefined(const T& v) {
		if (v == undefinedvalue<T>()) return false;
		return true;
	};
};