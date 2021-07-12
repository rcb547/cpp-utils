/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _string_utils_H
#define _string_utils_H

#include <cstring>
#include <cstdarg>
#include <string>
#include <sstream>

template<typename T>
void str2num(const std::string str, T& v)
{
	std::stringstream ss(str);
	ss >> v;
}

template<typename T>
void str2num(const char* str, T& v)
{
	std::stringstream ss(str);
	ss >> v;
}

inline std::string strprint_va(const char* fmt, va_list vargs)
{
	va_list vargscopy;	
	va_copy(vargscopy, vargs);
	const int len = 1 + std::vsnprintf(nullptr, 0, fmt, vargscopy);
	va_end(vargscopy);

	std::string s(len, '\0');
	std::vsnprintf(&(s.front()), len, fmt, vargs);	
	return std::string(s.c_str());
}

inline std::string strprint(const char* fmt, ...)
{		
	va_list vargs;
	va_start(vargs, fmt);
	std::string s = strprint_va(fmt, vargs);
	va_end(vargs);
	return s;
}


inline bool instring(const char* str, const char& c)
{
	const char* p = str;
	while (*p != 0){
		if (*p == c) return true;
		p++;
	}
	return false;
}

inline bool instring(const std::string str, const char& c)
{
	return instring(str.c_str(), c);	
}
#endif


