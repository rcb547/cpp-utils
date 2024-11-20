/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _fielddefinition_H
#define _fielddefinition_H

#include <stdlib.h>
#include <cstring>
#include <vector>
#include <functional>
#include <float.h>

#include "string_utils.hpp"
#include "general_utils.hpp"
#include "blocklanguage.hpp"

class cFieldDefinition {

private:

	bool initialised = false;

	bool isnumeric(const std::string& rhs)
	{
		std::vector<std::string> tokens = tokenise(rhs, " \t,");
		if (tokens.size() == 0)return false;
		char* str_end;
		double d = std::strtod(tokens[0].c_str(), &str_end);
		if (*str_end) return false;
		return true;
	};

public:

	std::string keyname = std::string();//A tag name for the definition
	enum class TYPE { VARIABLENAME, COLUMNNUMBER, NUMERIC, UNAVAILABLE };
	size_t coff = 1;//First column in ascii files for user perspective
	TYPE type = TYPE::UNAVAILABLE;//The type of definition
	char op = ' ';
	double opval = 0.0;
	bool flip = false;//flip polarity or not

	std::string varname = std::string();//Variable name for variablename type defs
	size_t column = undefinedvalue<size_t>();//Ascii file start column number for COLUMNUMBER type defs
	std::vector<double> numericvalue;//Numeric value

	cFieldDefinition() { }

	cFieldDefinition(const cBlock& b, const std::string& key) {
		initialise(b, key);
	}

	void initialise(const cBlock& b, const std::string& key)
	{
		int col;
		keyname = key;
		std::string rhs = b.getstringvalue(key);
		if (rhs == undefinedvalue<std::string>()) {
			type = cFieldDefinition::TYPE::UNAVAILABLE;
			column = 0;
			initialised = false;
		}
		else if (rhs.size() == 0) {
			type = cFieldDefinition::TYPE::UNAVAILABLE;
			column = 0;
			initialised = true;
		}
		else if (strncasecmp(rhs, "Unavailable", 11) == 0) {
			type = cFieldDefinition::TYPE::UNAVAILABLE;
			column = 0;
			initialised = true;
		}
		else if (isnumeric(rhs)) {
			type = cFieldDefinition::TYPE::NUMERIC;
			column = 0;
			numericvalue = b.getdoublevector(key);
			initialised = true;
		}
		else if (strncasecmp(rhs, "Column", 6) == 0) {
			type = cFieldDefinition::TYPE::COLUMNNUMBER;
			flip = false;
			int n = sscanf(&(rhs.c_str()[6]), "%d %c %lf", &col, &op, &opval);
			column = (size_t)col;
			if (n == 1) { op = ' '; opval = 0.0; }
			initialised = true;
		}
		else if (strncasecmp(rhs, "-Column", 7) == 0) {
			type = cFieldDefinition::TYPE::COLUMNNUMBER;
			flip = true;
			int n = sscanf(&(rhs.c_str()[7]), "%d %c %lf", &col, &op, &opval);
			column = (size_t)col;
			if (n == 1) { op = ' '; opval = 0.0; }
			initialised = true;
		}
		else {
			if (rhs[0] == '-') {
				type = cFieldDefinition::TYPE::VARIABLENAME;
				flip = true;
				rhs = rhs.substr(1, rhs.size() - 1);
				std::istringstream iss(rhs);
				iss >> varname;
				iss >> op;
				iss >> opval;
				initialised = true;
			}
			else {
				type = cFieldDefinition::TYPE::VARIABLENAME;
				flip = false;
				std::istringstream iss(rhs);
				iss >> varname;
				iss >> op;
				iss >> opval;
				initialised = true;
			}
		}
	}

	bool isinitialised() const {
		return initialised;
	}

	TYPE definitiontype() const {
		return type;
	}

	template<typename T>
	inline void getcolumn_val(const std::vector<std::string>& colstrings, const size_t& columnnumber, T& v) const
	{
		if (columnnumber >= colstrings.size()) {
			std::string msg = _SRC_;
			msg += strprint("\n\tError trying to access column %zu when there are only %zu columns in the current record string (check format and delimiters)\nCurrent record is\n", columnnumber + 1, colstrings.size());
			throw(std::runtime_error(msg));
		}
		else {
			if (colstrings[columnnumber].size() == 0) {
				v = undefinedvalue<T>();
			}
			else {
				std::istringstream(colstrings[columnnumber]) >> v;
				if (flip) apply_flip(v);
			}
		}
	}

	template<typename T>
	bool getvalue(const std::vector<std::string>& fields, T& v) const
	{
		if (type == cFieldDefinition::TYPE::NUMERIC) {
			v = (T)numericvalue[0];
			return true;
		}
		else if (type == cFieldDefinition::TYPE::COLUMNNUMBER) {
			getcolumn_val(fields, column - coff, v);
			return true;
		}
		else if (type == cFieldDefinition::TYPE::UNAVAILABLE) {
			v = undefinedvalue<T>();
			return false;
		}
		else if (type == cFieldDefinition::TYPE::VARIABLENAME) {
			std::string msg = _SRC_;
			msg += strprint("\n\tcFieldDefinition::TYPE::VARIABLENAME not allowed here\n");
			throw(std::runtime_error(msg));
		}
		else {
			std::string msg = _SRC_;
			msg += strprint("\n\tUnknown cFieldDefinition::TYPE\n");
			throw(std::runtime_error(msg));
		}
		return true;
	}

	template<typename T>
	bool getvalue(const std::vector<std::string>& fields, std::vector<T>& vec, const size_t& n) const
	{
		vec.resize(n);
		if (type == cFieldDefinition::TYPE::NUMERIC) {
			size_t len = numericvalue.size();
			for (size_t i = 0; i < n; i++) {
				if (len == 1) vec[i] = (T)numericvalue[0];
				else vec[i] = (T)numericvalue[i];
			}
			return true;
		}
		else if (type == cFieldDefinition::TYPE::COLUMNNUMBER) {
			for (size_t i = 0; i < n; i++) {
				getcolumn_val(fields, i + column - coff, vec[i]);
			}
			return true;
		}
		else if (type == cFieldDefinition::TYPE::UNAVAILABLE) {
			vec = std::vector<T>(n, undefinedvalue<T>());
			return false;
		}
		else if (type == cFieldDefinition::TYPE::VARIABLENAME) {
			std::string msg = _SRC_;
			msg += strprint("\n\tcFieldDefinition::TYPE::VARIABLENAME not allowed here\n");
			throw(std::runtime_error(msg));
			return false;
		}
		else {
			std::string msg = _SRC_;
			msg += strprint("\n\tUnknown cFieldDefinition::TYPE\n");
			throw(std::runtime_error(msg));
		}
		return true;
	}

	template<typename T>
	inline void ifnullconvert2zero(T& val) const {
		//temporary hack to handle Nulls
		if (val == (T)-999 || val == (T)-9999) val = (T)0;
	}

	private:
	template<typename T>
	inline void apply_operator(T& val) const {
		if (val == undefinedvalue<T>()) return; //don't apply to undefined values
		else if (op == ' ') return;//nothing to do
		else if (op == '+') {
			val += (T)opval;
		}
		else if (op == '-') {
			val -= (T)opval;
		}
		else if (op == '*') val *= (T)opval;
		else if (op == '/') val /= (T)opval;
		else glog.warningmsg(_SRC_, "Unknown operator %c\n", op);
		return;
	}

	private:
	template<typename T> 
	inline void apply_flip(T& val) const {
		if (flip == false) return;
		if (val == undefinedvalue<T>()) return; //don't apply to undefined values
		val *= (T)-1;
		return;
	}

	public:
	template<typename T> 
	inline void apply_flip_and_operator(T& val, const T& nullval = undefinedvalue<T>()) const {
		if (flip == false && op == ' ') return;
		if (val == nullval) return; //don't apply to null values
		apply_flip(val);
		apply_operator(val);
		return;
	}

	public:
	template<typename T>
	inline void apply_flip_and_operator(std::vector<T>& vec, const T& nullval = undefinedvalue<T>()) const {
		if (flip == false && op == ' ') return;
		for (size_t i = 0; i < vec.size(); i++) {
			apply_flip_and_operator(vec[i],nullval);
		}
		return;
	}
};

typedef std::map<std::string, cFieldDefinition, caseinsensetiveless<std::string>> cFDMap;
typedef std::pair<cFieldDefinition, cVrnt> cFDVar;


class cFdVrnt {

public:
	cFdVrnt(const cFieldDefinition& _fd, const cVrnt& _vrnt) {
		fd = _fd;
		vnt = _vrnt;
	}

	cFieldDefinition fd;
	cVrnt vnt;
};

#endif
