/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie
*/

#pragma once

#include <string>
#include <cstdarg>

inline std::string strprint_va(const char* fmt, va_list vargs)
{
	std::va_list vargscopy;
	va_copy(vargscopy, vargs);
	const int len = 1 + std::vsnprintf(nullptr, 0, fmt, vargscopy);
	va_end(vargscopy);

	std::string s(len-1,0);//initialise with null characters
	std::vsnprintf(&(s.front()), len, fmt, vargs);
	return s;
}

inline std::string strprint(const char* fmt, ...)
{
	std::va_list vargs;
	va_start(vargs, fmt);
	std::string s = strprint_va(fmt, vargs);
	va_end(vargs);
	return s;
}

inline std::string strprint(const std::string fmt, ...)
{
	va_list vargs;
	va_start(vargs, fmt);
	std::string s = strprint_va(fmt.c_str(), vargs);
	va_end(vargs);
	return s;
}


