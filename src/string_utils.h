/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _string_utils_H
#define _string_utils_H

#include <cfloat>
#include <cstring>
#include <cstdarg>
#include <string>
#include <istream>
#include <sstream>
#include <iterator>
#include <algorithm>

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

inline bool instring(const std::string& str, const char& c)
{
	return instring(str.c_str(), c);	
}

inline std::string stringvalue(const double value, const char* fmt)
{
	if (value == -DBL_MAX)return std::string("Undefined");
	if (fmt == NULL) return strprint("%lf", value);
	return strprint(fmt, value);
}

inline std::string stringvalue(const size_t value, const char* fmt)
{
	if (fmt == NULL) return strprint("%zu", value);
	return strprint(fmt, value);
}

inline std::string stringvalue(const int value, const char* fmt)
{
	if (fmt == NULL) return strprint("%d", value);
	return strprint(fmt, value);
}

inline std::string stringvalue(const bool value)
{
	if (value == true)return std::string("True");
	return std::string("False");
}

inline int strcasecmp(const std::string& A, const std::string& B)
{
#if defined _MSC_VER //Microsoft Visual Studio compiler does not seem to define strcasecmp
	return _stricmp(A.c_str(), B.c_str());
#else
	return strcasecmp(A.c_str(), B.c_str());
#endif
}

inline int strncasecmp(const std::string& A, const std::string& B, const size_t n)
{
#if defined _MSC_VER //Microsoft Visual Studio compiler does not seem to define strncasecmp
	return _strnicmp(A.c_str(), B.c_str(), n);
#else
	return strncasecmp(A.c_str(), B.c_str(), n);
#endif
}

inline std::vector<std::string>& split(const std::string& str, const char delim, std::vector<std::string>& elems) {
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

inline std::vector<std::string> split(const std::string& str, const char delim) {
	std::vector<std::string> elems;
	split(str, delim, elems);
	return elems;
}

inline std::string trim(const std::string& s)
{
	if (s.length() == 0) return s;
	size_t index1 = s.find_first_not_of(" \t\r\n");
	size_t index2 = s.find_last_not_of(" \t\r\n");
	if (index1 == std::string::npos || index2 == std::string::npos) {
		return std::string("");
	}
	else {
		return s.substr(index1, index2 - index1 + 1);
	}
}

inline std::vector<std::string> trimsplit(const std::string& str, const char delim) {
	std::vector<std::string> elems;
	split(str, delim, elems);
	for (size_t i = 0; i < elems.size(); i++) {
		elems[i] = trim(elems[i]);
	}
	return elems;
}

inline std::vector<std::string> tokenise(const std::string& str, const char delim) {
	std::string s = trim(str);
	std::vector<std::string> tokens;
	size_t p = s.find_first_of(delim);
	while (p < s.size()) {
		tokens.push_back(trim(s.substr(0, p)));
		s = s.substr(p + 1, s.length());
		p = s.find_first_of(delim);
	}
	tokens.push_back(trim(s));
	return tokens;
}

inline std::vector<std::string> tokenise(const std::string& str, const char* delims) {
	std::string s = trim(str);
	std::vector<std::string> tokens;
	size_t p = s.find_first_of(delims);
	while (p < s.size()) {
		tokens.push_back(trim(s.substr(0, p)));
		s = s.substr(p + 1, s.length());
		p = s.find_first_of(delims);
	}
	tokens.push_back(trim(s));
	return tokens;
}

inline std::string stripquotes(const std::string& s)
{
	if (s.length() == 0) return s;
	if (s[0] == '"' && s[s.size() - 1] == '"') {
		return s.substr(1, s.size() - 2);
	}
	return s;
}

inline std::vector<std::string> tokenize(const std::string& str)
{
	std::stringstream strstr(str);
	std::istream_iterator<std::string> it(strstr);
	std::istream_iterator<std::string> end;
	std::vector<std::string> results(it, end);
	return results;
}

inline std::vector<std::string> parsestrings(const std::string& str, const std::string& delims)
{
	size_t len = strlen(str.c_str());
	char* workstr = new char[len + 1];
	strcpy(workstr, str.c_str());

	std::vector<std::string> list;
	char* token = strtok(workstr, delims.c_str());
	while (token != NULL) {
		list.push_back(std::string(token));
		token = strtok(NULL, delims.c_str());
	}
	delete[] workstr;
	return list;
}

inline std::vector<std::string> fieldparsestring(const char* s, const char* delims)
{
	std::vector<std::string> fields;
	fields.reserve(400);
	std::string work = std::string(s);
	char* p = strtok(&(work[0]), delims);
	while (p != NULL)
	{
		fields.push_back(std::string(p));
		p = strtok(NULL, delims);
	}
	return fields;
}

inline void settolower(std::string& s){
	std::transform(s.begin(), s.end(), s.begin(), std::tolower);
}

inline std::string tolower(const std::string& s) {
	std::string t = s;
	settolower(t);
	return t;
}



inline void settoupper(std::string& s) {	
	std::transform(s.begin(), s.end(), s.begin(), std::toupper);
}

inline std::string toupper(const std::string& s) {
	std::string t = s;
	settoupper(t);
	return t;
}

//case insensitive equal function
inline bool ciequal(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
		[](char x, char y) {
			return tolower(x) == tolower(y);
		});
}

//case insensitive lass function
inline bool ciless(const std::string& a, const std::string& b)
{
	return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
		[](char x, char y) {
			return tolower(x) < tolower(y);
		});
}

//case insensitive equal functor
template <class T> struct caseinsensetiveequal {
	bool operator() (const T& a, const T& b) const {
		return std::equal(a.begin(), a.end(), b.begin(), b.end(),
			[](char x, char y) {
				return tolower(x) == tolower(y);
			});
	}
};

//case insensitive less functor
template <class T> struct caseinsensetiveless {
	bool operator() (const T& a, const T& b) const {
		return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
			[](char x, char y) {
				return tolower(x) < tolower(y);
			});
	}
};

#endif


