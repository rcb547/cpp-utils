/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _blocklanguage_H
#define _blocklanguage_H

#include <iostream>
#include <cstring>
#include <climits>
#include <cfloat>

#include <cstdlib>
#include <vector>

#include "general_utils.hpp"
#include "file_utils.hpp"

class cBlock{

private:
	std::string delimiters = " ,\t";

	static std::string strip_fromandafter1(const std::string& entry, const std::string& tag) {
		size_t index = entry.find(tag);
		if (index != std::string::npos) {
			return entry.substr(0, index);
		}
		else return entry;
	};

	static std::string strip_comments(const std::string& entry) {
		return strip_fromandafter1(entry, "//");
	};

public:

	std::string Filename;
	std::string Name;
	std::vector<std::string> Entries;
	std::vector<cBlock> Blocks;

	cBlock() { }

	cBlock(const std::string& filename)
	{
		loadfromfile(filename);
	}

	bool empty() {
		if (Entries.empty() && Blocks.empty())return true;
		return false;
	}

	void loadfromfile(const std::string& filename)
	{
		Filename = filename;
		FILE* fp = fileopen(filename, "r");
		if (fp == NULL){
			glog.errormsg(_SRC_,"Could not open file: %s\n", filename.c_str());		
		}
		else {
			loadfromfilepointer(fp, true);
			fclose(fp);
		}
	}

	void loadfromfilepointer(FILE* fp, bool rootlevel)
	{
		std::string linestr;
		while (filegetline(fp, linestr)){
			std::string s = trim(linestr);
			if (s.length() == 0)continue;

			std::vector<std::string> vs = tokenize(s);
			if (vs.size() >= 2){
				if (strcasecmp(vs[1], "End") == 0){
					break;
				}
				else if (strcasecmp(vs[1], "Begin") == 0){
					if (rootlevel == true){
						Name = vs[0];
						rootlevel = false;
					}
					else{
						cBlock newblock;
						newblock.Name = vs[0];
						newblock.loadfromfilepointer(fp, false);
						Blocks.push_back(newblock);
					}
					continue;
				}
				Entries.push_back(s);
			}
			else{
				Entries.push_back(s);
			}
		}
	}

	const std::string get_as_string(const size_t n = 0) const
	{
		std::string s;

		for (size_t j = 0; j < n; j++) s += strprint("\t");
		s += strprint("%s Begin\n", Name.c_str());

		for (size_t i = 0; i < Entries.size(); i++){
			for (size_t j = 0; j < n + 1; j++)s += strprint("\t");
			s += strprint("%s\n", Entries[i].c_str());
		}

		for (size_t i = 0; i < Blocks.size(); i++){
			s += Blocks[i].get_as_string(n + 1);
		}

		for (size_t j = 0; j < n; j++) s += strprint("\t");
		s += strprint("%s End\n", Name.c_str());
		return s;
	}

	void print(const size_t n = 0) const
	{
		std::string s = get_as_string(n);
		std::cout << s.c_str();
	}

	void write(FILE* fp, size_t n = 0) const
	{
		if (fp == (FILE*)NULL) return;
		std::string s = get_as_string(n);
		fprintf(fp, "%s", s.c_str());		
		return;
	}
	
	void printidentifiers() const
	{
		for (size_t i = 0; i < Entries.size(); i++){
			std::cout << key(Entries.at(i)).c_str() << std::endl;
		}

		for (size_t i = 0; i < Blocks.size(); i++){
			Blocks.at(i).printidentifiers();
		}

	}
	
	std::string key(const std::string& entry) const
	{
		std::string s = strip_comments(entry);
		size_t index = s.find("=");
		std::string id = s.substr(0, index - 1);
		return trim(id);
	}
	
	std::string key(const size_t eindex) const
	{
		return key(Entries[eindex]);
	}

	std::string value(const std::string entry) const
	{
		std::string s = strip_comments(entry);
		size_t index = s.find("=");
		size_t len = s.size() - index - 1;
		if (len == 0) return std::string("");
		std::string v = s.substr(index + 1, len);
		return trim(v);
	}

	std::string value(const size_t eindex) const
	{
		return value(Entries[eindex]);
	}
	
	void printvalues()  const
	{
		for (size_t i = 0; i < Entries.size(); i++){
			std::cout << value(Entries.at(i)).c_str() << std::endl;
		}

		for (size_t i = 0; i < Blocks.size(); i++){
			Blocks.at(i).printvalues();
		}
	}
	
	std::string getentry(const std::string id) const
	{
		size_t index = id.find(".");
		if (index != std::string::npos){
			std::string start = id.substr(0, index);
			std::string end = id.substr(index + 1, id.size() - index - 1);
			if (strcasecmp(start, Name) == 0){
				return getentry(end);
			}
			else{
				cBlock b = findblock(start);
				return b.getentry(end);
			}
		}
		else{
			return findkey(id);
		}
	}
	
	const cBlock findblock(const std::string name) const
	{
		size_t index = name.find(".");

		if (index != std::string::npos){
			std::string start = name.substr(0, index);
			std::string end = name.substr(index + 1, name.size() - index - 1);
			cBlock b = findblock(start);
			return b.findblock(end);
		}

		for (size_t i = 0; i < Blocks.size(); i++){
			if (strcasecmp(Blocks[i].Name, name) == 0){
				return Blocks[i];
			}
		}
		return cBlock();
	}
	
	std::vector<cBlock> findblocks(const std::string name) const
	{
		std::vector<cBlock> v;
		size_t index = name.find(".");
		if (index != std::string::npos){
			std::string start = name.substr(0, index);
			std::string end = name.substr(index + 1, name.size() - index - 1);
			cBlock b = findblock(start);
			return b.findblocks(end);
		}

		for (size_t i = 0; i < Blocks.size(); i++){
			if (strcasecmp(Blocks[i].Name, name) == 0){
				v.push_back(Blocks[i]);
			}
		}
		return v;
	};

	size_t findkeyindex(const std::string id) const
	{
		for (size_t i = 0; i < Entries.size(); i++){
			if (strcasecmp(key(Entries[i]), id) == 0){
				return i;
			}
		}		
		return undefinedvalue<size_t>();
	}

	std::string findkey(const std::string id) const
	{
		for (size_t i = 0; i < Entries.size(); i++){
			if (strcasecmp(key(Entries[i]), id) == 0){
				std::string e = strip_comments(Entries[i]);
				return e;
			}
		}
		return undefinedvalue<std::string>();
	}

	std::string getstringvalue(const std::string id) const
	{
		return value(getentry(id));
	}

	short getshortvalue(const std::string id) const
	{
		short v;
		int status;
		std::string entry = getentry(id);
		if (strcasecmp(entry, undefinedvalue<std::string>()) == 0){
			return undefinedvalue<short>();
		}
		else{
			status = sscanf(value(entry).c_str(), "%hd", &v);
		}

		if (status == 1) return v;
		else return undefinedvalue<short>();
	}
	
	int getintvalue(const std::string id) const
	{
		int v;
		int status;
		std::string entry = getentry(id);
		if (strcasecmp(entry, undefinedvalue<std::string>()) == 0){
			return undefinedvalue<int>();
		}
		else{
			status = sscanf(value(entry).c_str(), "%d", &v);
		}

		if (status == 1) return v;
		else return undefinedvalue<int>();
	}
	
	size_t getsizetvalue(const std::string id) const
	{
		size_t v;
		int status;
		std::string entry = getentry(id);
		if (strcasecmp(entry, undefinedvalue<std::string>()) == 0){
			return undefinedvalue<size_t>();
		}
		else{
			long tmp;
			status = std::sscanf(value(entry).c_str(), "%ld", &tmp);
			v = (size_t)tmp;
		}

		if (status == 1) return v;
		return undefinedvalue<size_t>();
	}
	
	float getfloatvalue(const std::string id) const
	{
		float v;
		int status;
		std::string entry = getentry(id);
		if (strcasecmp(entry, undefinedvalue<std::string>()) == 0){
			return undefinedvalue<float>();
		}
		else{
			status = sscanf(value(entry).c_str(), "%f", &v);
		}

		if (status == 1) return v;
		else return undefinedvalue<float>();
	}
	
	double getdoublevalue(const std::string id) const
	{
		double v;
		int status;
		std::string entry = getentry(id);
		if (strcasecmp(entry, undefinedvalue<std::string>()) == 0){
			return undefinedvalue<double>();
		}
		else{
			status = sscanf(value(entry).c_str(), "%lf", &v);
		}

		if (status == 1) return v;
		else return undefinedvalue<double>();
	}

	std::vector<int> getintvector(const std::string id) const
	{		
		std::vector<int> vec;
		std::string s = getstringvalue(id);

		if (strcasecmp(s, undefinedvalue<std::string>()) == 0){
			return vec;
		}

		std::vector<std::string> fields = fieldparsestring(s.c_str(), delimiters.c_str());
		for (size_t i = 0; i < fields.size(); i++){			
			int v = std::atoi(fields[i].c_str());
			vec.push_back(v);
		}
		return vec;
	}

	std::vector<double> getdoublevector(const std::string id) const
	{		
		std::vector<double> vec;
		std::string s = getstringvalue(id);
		if (strcasecmp(s, undefinedvalue<std::string>()) == 0){
			return vec;
		}

		std::vector<std::string> fields = fieldparsestring(s.c_str(), delimiters.c_str());
		for (size_t i = 0; i < fields.size(); i++){			
			double v = std::atof(fields[i].c_str());
			vec.push_back(v);
		}
		return vec;
	}

	std::vector<std::string> getstringvector(const std::string id) const
	{
		std::vector<std::string> fields;
		std::string s = getstringvalue(id);

		if (strcasecmp(s, undefinedvalue<std::string>()) == 0){
			return fields;
		}

		fields = fieldparsestring(s.c_str(), delimiters.c_str());
		for (size_t i = 0; i < fields.size(); i++){
			fields[i] = trim(fields[i]);
		}
		return fields;
	}

	std::vector<std::vector<double>> getdoublematrix(const std::string id) const
	{
		std::vector<std::vector<double>> matrix;
		cBlock b = findblock(id);
		for (size_t i = 0; i < b.Entries.size(); i++){			
			std::vector<double> vec;
			std::string s = strip_comments(b.Entries[i]);
			if (s.size() == 0) continue;
			std::vector<std::string> fields = fieldparsestring(s.c_str(), delimiters.c_str());
			for (size_t j = 0; j < fields.size(); j++){
				double v = std::atof(fields[j].c_str());
				vec.push_back(v);
			}
			matrix.push_back(vec);
		}
		return matrix;
	}

	bool getboolvalue(const std::string id) const
	{
		std::string s = getstringvalue(id);
		size_t k = s.find_first_of(" \t\r\n");
		std::string value = s.substr(0, k);
		if (strcasecmp(value, "yes") == 0)return true;
		else if (strcasecmp(value, "no") == 0)return false;
		else if (strcasecmp(value, "true") == 0)return true;
		else if (strcasecmp(value, "false") == 0)return false;
		else if (strcasecmp(value, "1") == 0)return true;
		else if (strcasecmp(value, "0") == 0)return false;
		else if (strcasecmp(value, "off") == 0)return false;
		else if (strcasecmp(value, "on") == 0)return true;
		else return false;
	}

	bool getvalue(const std::string id, bool& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getboolvalue(id);
		return true;
	}
	bool getvalue(const std::string id, short& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getshortvalue(id);
		return true;
	}
	bool getvalue(const std::string id, int& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getintvalue(id);
		return true;
	}
	bool getvalue(const std::string id, size_t& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getsizetvalue(id);
		return true;
	}
	bool getvalue(const std::string id, float& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getfloatvalue(id);
		return true;
	}
	bool getvalue(const std::string id, double& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getdoublevalue(id);
		return true;
	}
	bool getvalue(const std::string id, std::string& value) const
	{
		if (getentry(id).compare(undefinedvalue<std::string>()) == 0){
			return false;
		}
		value = getstringvalue(id);
		return true;
	}

	//bookmark
	template<typename T>
	bool get(const std::string id, T& value, const T& defaultvalue) const
	{
		std::string e = getentry(id);
		if (e.compare(undefinedvalue<std::string>()) == 0) {
			value = defaultvalue;
			return false;
		}
		bool status = getvalue(id,value);
		return status;
	}

	std::vector<int> getmultipleints(const std::string id) const
	{
		std::vector<std::string> str = getmultiplestrings(id);
		std::vector<int> result;
		
		for (size_t i = 0; i < str.size(); i++){
			int val = std::atoi(str[i].c_str());
			result.push_back(val);
		}
		return result;
	}
	std::vector<double> getmultipledoubles(const std::string id) const
	{
		std::vector<std::string> str = getmultiplestrings(id);
		std::vector<double> result;
		
		for (size_t i = 0; i < str.size(); i++){
			double val = std::atof(str[i].c_str());
			result.push_back(val);
		}
		return result;
	}
	std::vector<std::string> getmultiplestrings(const std::string id) const
	{
		int i;
		std::vector<std::string> result;

		std::string str = getstringvalue(id);
		if (str.length() > 0 && strcasecmp(str, undefinedvalue<std::string>()) != 0){
			result.push_back(str);
		}
		for (i = 0; i < 100; i++){
			std::string token = id;
			token += strprint("%d", i);

			str = getstringvalue(token);
			if (strcasecmp(str, undefinedvalue<std::string>()) != 0){
				result.push_back(str);
			}
		}
		return result;
	}
	std::vector<std::string> getblockstrings(const std::string id) const
	{
		std::vector<std::string> result;
		cBlock b = findblock(id);
		for (size_t i = 0; i < b.Entries.size(); i++){
			std::string s = strip_comments(b.Entries[i]);
			result.push_back(s);
		}
		return result;
	}
	std::vector<std::vector<std::string>> getblockleftright(const std::string id) const
	{
		std::vector<std::string> s = getblockstrings(id);
		std::vector<std::vector<std::string>> v(s.size());
		for (size_t i = 0; i < s.size(); i++){
			v[i] = split(s[i], '=');
			if (v[i].size() == 1)v[i].push_back("");
			v[i][0] = trim(v[i][0]);
			v[i][1] = trim(v[i][1]);
		}
		return v;
	}
};

#endif
