/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "general_utils.hpp"

class cFileSplitter{

private:
	bool atstart;
	std::string buf;

	size_t splitindex;
	std::ifstream file;
	std::string filename;
	size_t nheaderlines;

	std::string splitfield(const std::string& s){
		std::string f;
		std::istringstream ss(s);
		for (size_t i = 0; i <= splitindex; i++) ss >> f;
		return f;
	}

	bool getnextrecord(std::string& s)
	{
		if (file.eof()){
			s.clear();
			return false;
		}
		std::getline(file, s);
		if (s.size() > 0) return true;
		else return false;
	}

public:

	cFileSplitter(const std::string _filename, const size_t _nheaderlines, const size_t _splitindex)
	{
		initialise(_filename, _nheaderlines, _splitindex);		
	}

	void initialise(const std::string _filename, const size_t _nheaderlines, const size_t _splitindex)
	{
		filename = _filename;
		nheaderlines = _nheaderlines;
		splitindex = _splitindex;
		rewind();
	}

	void rewind(){

		if (file.is_open()) file.close();
		atstart = true;
		buf.clear();

		file.open(filename, std::ifstream::in);
		std::string s;
		for (size_t i = 0; i < nheaderlines; i++){
			getnextrecord(s);
		}
	}

	void test(){
		std::vector<std::string> L;
		while (getnextgroup(L)){
			std::cout << L.size() << std::endl;
			//for (size_t i = 0; i < L.size(); i++){
			//	std::cout << L[i].size() << ":";
			//	std::cout << L[i].substr(0, 80) << std::endl;
			//}
			//std::cout << std::endl;
		}
	}

	size_t getnextgroup(std::vector<std::string>& L)
	{
		std::string r, g, thisgroup;
		L.clear();
		if (atstart == false && buf.size() > 0){
			g = splitfield(buf);
			thisgroup = g;
			L.push_back(buf);
			buf.clear();
		}

		while (getnextrecord(r)){
			g = splitfield(r);
			if (L.size() == 0) thisgroup = g;
			if (strcasecmp(g, thisgroup) != 0){
				buf = r;
				break;
			}
			L.push_back(r);
		}
		atstart = false;
		return L.size();
	}
};

