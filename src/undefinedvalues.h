/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _undefinedvalues_H
#define _undefinedvalues_H

#include <cstdint>
#include <cfloat>
#include <string>
#include <climits>

inline size_t ud_size_t(){
	//return UINT64_MAX; 
	return (std::numeric_limits<size_t>::max)();	
}
inline short  ud_short(){
	//return SHRT_MIN; 
	return (std::numeric_limits<short>::min)();
}
inline int    ud_int(){
	//return INT_MIN;
	return (std::numeric_limits<int>::min)();
}
inline float  ud_float(){
	//return  -FLT_MAX;
	return (std::numeric_limits<float>::lowest)();
}
inline double ud_double(){ 
	//return -DBL_MAX; 
	return (std::numeric_limits<double>::lowest)();
}
inline std::string ud_string(){
	//return "*ENTRYNOTFOUND*";
	return std::string("Undefined std::string");	
}

inline short undefinedvalue(const short& v){
	return ud_short();
}

inline int undefinedvalue(const int& v){
	return ud_int();
}

inline size_t undefinedvalue(const size_t& v){
	return ud_size_t();
}

inline float undefinedvalue(const float& v){
	return ud_float();
}

inline double undefinedvalue(const double& v){
	return ud_double();
}

inline std::string undefinedvalue(const std::string& v){
	return ud_string();
}

template <class T>
inline bool isdefined(const T& v){
	if (v == undefinedvalue(v)) return false;
	return true;
}

#endif