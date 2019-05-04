/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _asciicolumnfile_H
#define _asciicolumnfile_H

#include <cstdlib>
#include <cstring>
#include <vector>

#include "general_utils.h"
#include "file_utils.h"
#include "file_formats.h"

class cAsciiColumnFile {

private:	

	std::ifstream ifs;
	std::string currentrecord;
	std::vector<std::string> currentcolumns;
	size_t recordsreadsuccessfully = 0;

public:

	std::vector<cAsciiColumnField> fields;

	cAsciiColumnFile() { };

	cAsciiColumnFile(const std::string& filename){		
		openfile(filename);		
	};
		
	const std::string& currentrecord_string() const { return currentrecord; };
	
	const std::vector<std::string>& currentrecord_columns() const { return currentcolumns; };

	bool openfile(const std::string& datafilename){
		std::string name = datafilename;
		fixseparator(name);
		ifs.open(datafilename, std::ifstream::in);
		if (!ifs) {
			glog.errormsg(_SRC_,"Could not open file %s\n",datafilename.c_str());
		}
		return true;
	};

	std::vector<std::string> tokenise(const std::string& str, const char delim){
		std::string s = trim(str);
		std::vector<std::string> tokens;
		size_t p = s.find_first_of(delim);
		while (p < s.size()){
			tokens.push_back(trim(s.substr(0, p)));
			s = s.substr(p + 1, s.length());
			p = s.find_first_of(delim);
		}
		tokens.push_back(trim(s));
		return tokens;
	}

	bool parse_aseggdf2_header(const std::string& dfnfile){

		fields.clear();

		FILE* fp = fileopen(dfnfile, "r");
		std::string str;		

		size_t startcolumn = 1;
		filegetline(fp, str);
		while (filegetline(fp, str)){

			std::vector<std::string> tokens = tokenise(str, ';');			
			if (strcasecmp(tokens[1], "end defn") == 0){
				break;
			}
			
			cAsciiColumnField F;

			int intorder;
			sscanf(tokens[0].c_str(), "DEFN %d ST = RECD, RT = ", &intorder);
			F.order = (size_t)intorder;
			F.startcolumn = startcolumn;
			
			
			tokens = tokenise(tokens[1], ':');
			F.name = tokens[0];
			std::string formatstr = tokens[1];
			
			int nbands = 1;
			int width = 0;
			int decimals = 0;
			
			if (sscanf(formatstr.c_str(), "%d%c%d.%d", &nbands, &F.fmttype, &width, &decimals) == 4){
				//
			}
			else if (sscanf(formatstr.c_str(), "%c%d.%d", &F.fmttype, &width, &decimals) == 3){
				//
			}
			else if (sscanf(formatstr.c_str(), "%c%d", &F.fmttype, &width) == 2){
				//
			}
			else{
				printf("%s\n", formatstr.c_str());
			}			
			F.nbands = nbands;
			F.fmtwidth = width;
			F.fmtdecimals = decimals;			
			startcolumn += F.nbands;

			if (tokens.size() > 2){
				std::string remainder = tokens[2];
				tokens = tokenise(remainder, ',');
				for (size_t i = 0; i < tokens.size(); i++){
					std::vector<std::string> t = tokenise(tokens[i], '=');
					if (strcasecmp(t[0], "unit") == 0 || strcasecmp(t[0], "units") == 0){
						F.units = t[1];
					}
					else if (strcasecmp(t[0], "name") == 0){
						F.expandedname = t[1];
					}
					else if (strcasecmp(t[0], "null") == 0){
						F.nullvaluestr = t[1];
						F.nullvalue = atof(t[1].c_str());
					}
					else{
						F.comment += tokens[i];
					}
				}
			}			
			fields.push_back(F);
		};
		fclose(fp);

		size_t k = 0;
		for (size_t i = 0; i < fields.size(); i++){
			fields[i].startchar = k;
			fields[i].endchar   = k - 1 + fields[i].fmtwidth * fields[i].nbands;
			k = fields[i].endchar + 1;
		}
		return true;		
	};

	static size_t nullfieldindex(){
		return UINT64_MAX;		
	};

	size_t fieldindexbyname(const std::string& fieldname) const
	{
		for (size_t fi = 0; fi < fields.size(); fi++){
			if (strcasecmp(fields[fi].name, fieldname) == 0) return fi;
		}
		return nullfieldindex();
	}

	bool fieldindexbyname(const std::string& fieldname, size_t& index) const
	{		
		for (size_t fi = 0; fi < fields.size(); fi++){
			if (strcasecmp(fields[fi].name, fieldname) == 0){
				index = fi;
				return true;
			}
		}
		index = nullfieldindex();
		return false;
	}
	
	bool skiprecords(const size_t& nskip) {		
		for (size_t i = 0; i < nskip; i++) {
			//status = filegetline(filepointer, currentrecord);
			if (ifs.eof()) return false;
			std::getline(ifs, currentrecord);			
			recordsreadsuccessfully++;			
		}
		return true;
	}
	   	  
	bool readnextrecord() {
		if (ifs.eof()) return false;
		std::getline(ifs, currentrecord);
		if (ifs.eof() && currentrecord.size() == 0){
			return false;			
		}	
		return true;		
	}

	size_t parserecord(){						
		currentcolumns = fieldparsestring(currentrecord.c_str(), " ,\t\r\n");		
		return currentcolumns.size();
	}

	size_t ncolumns(){
		size_t n = 0;
		for (size_t i = 0; i < fields.size(); i++){
			n += fields[i].nbands;
		}
		return n;
	}
	
	template<typename T>
	bool getcolumn(const size_t& columnnumber, T& v) const
	{			
		std::istringstream(currentcolumns[columnnumber]) >> v;
		return true;
	}
		
	template<typename T>
	bool getfield(const size_t& findex, T& v) const 
	{
		size_t base = fields[findex].startcolumn-1;
		getcolumn(base,v);		
		return true;
	}

	template<typename T>
	bool getfield(const size_t& findex, std::vector<T>& vec) const 
	{
		size_t base = fields[findex].startcolumn - 1;
		size_t nb = fields[findex].nbands;
		vec.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {			
			getcolumn(base,vec[bi]);
			base++;
		}
		return true;
	}

	template<typename T>
	bool getfieldlog10(const size_t& findex, std::vector<T>& vec) const
	{
		size_t base = fields[findex].startcolumn - 1;
		size_t nb = fields[findex].nbands;
		vec.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {
			getcolumn(base,vec[bi]);
			if (fields[findex].isnull(vec[bi]) == false) {
				vec[bi] = log10(vec[bi]);
			}
			base++;
		}
		return true;
	}

	size_t readnextgroup(const size_t& fgroupindex, std::vector<std::vector<int>>& intfields, std::vector<std::vector<double>>& doublefields){
		
		size_t nfields = fields.size();
		size_t numcolumns = ncolumns();
		if (ifs.eof()) return 0;

		intfields.clear();
		doublefields.clear();
		intfields.resize(fields.size());
		doublefields.resize(fields.size());

		int lastline;
		size_t count = 0;
		do{
			if(recordsreadsuccessfully == 0)readnextrecord();
			if (parserecord() != numcolumns){
				continue;
			}
			int line;
			getfield(fgroupindex, line);

			if (count == 0)lastline = line;

			if (line != lastline) return count;			

			for (size_t fi = 0; fi < nfields; fi++){
				size_t nbands = fields[fi].nbands;
				if (fields[fi].datatype() == eFieldType::INTEGER){
					std::vector<int> v;
					getfield(fi, v);
					for (size_t bi = 0; bi < nbands; bi++){
						intfields[fi].push_back(v[bi]);
					}
				}
				else{
					std::vector<double> v;
					getfield(fi, v);
					for (size_t bi = 0; bi < nbands; bi++){
						doublefields[fi].push_back(v[bi]);
					}
				}
			}
			count++;
		} while (readnextrecord());
		return count;
	};
};

#endif

 