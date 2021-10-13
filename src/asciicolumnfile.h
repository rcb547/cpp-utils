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

#include "csv.hpp"
#include "general_utils.h"
#include "file_utils.h"
#include "file_formats.h"

class cFieldDefinition;

class cAsciiColumnFile {

private:	

	std::ifstream ifs;
	size_t recordsreadsuccessfully = 0;
	std::string currentrecord;		
	std::vector<std::string> colstrings;
	bool charpositions_adjusted = false;

	std::string ST_string;
	std::string RT_string;
	
public:
	enum class HeaderType { DFN, CSV, NONE } headertype = HeaderType::NONE;
	enum class ParseType { FIXEDWIDTH, DELIMITED } parsetype = ParseType::FIXEDWIDTH;
	
	const std::vector<std::string>& cref_colstrings() const
	{
		return colstrings;
	};
	std::vector<cAsciiColumnField> fields;

	cAsciiColumnFile() {};

	cAsciiColumnFile(const std::string& filename){		
		openfile(filename);		
	};

	void read_dfn(const std::string& dfnpath) {
		cASEGGDF2Header H(dfnpath);
		fields = H.getfields();
		ST_string = H.get_ST_string();
		RT_string = H.get_RT_string();
	}
		
	const std::string& currentrecord_string() const { return currentrecord; };
	
	const std::vector<std::string>& currentrecord_columns() const { return colstrings; };

	bool openfile(const std::string& datafilename){
		std::string name = datafilename;
		fixseparator(name);
		ifs.open(datafilename, std::ifstream::in);
		if (!ifs) {
			glog.errormsg(_SRC_,"Could not open file %s\n",datafilename.c_str());
		}
		return true;
	};

	static std::vector<std::string> tokenise(const std::string& str, const char delim){
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

	bool parse_csv_header(const std::string& csvfile) {

		fields.clear();

		csv::CSVFormat csvfm;
		std::vector<char> dc{ ',' };
		std::vector<char> ws{ ' ','\t' };		
		csvfm.delimiter(dc);
		csvfm.trim(ws);
		csvfm.header_row(0);
		csv::CSVReader R(csvfile, csvfm);		
		csv::CSVRow row;		
		std::vector<std::string> cnames = R.get_col_names();				
		size_t iname  = R.index_of("Name");
		size_t inbands = R.index_of("Bands");
		size_t ifmt   = R.index_of("Format");
		size_t inullstr = R.index_of("NullString");
		
		size_t iunits = R.index_of("Units");		
		size_t idesc = R.index_of("Description");
		size_t ilongn = R.index_of("LongName");

		//Name,Bands,Format,NullString,LongName
		while (R.read_row(row)) {
			cAsciiColumnField F;
			F.fileorder = R.n_rows();
			F.name = row[iname].get<std::string>();
			F.nullvaluestring = row[inullstr].get<std::string>();
			std::string formatstr = row[ifmt].get<std::string>();
			F.parse_format_string(formatstr);
			F.nbands = row[inbands].get<size_t>();			
			fields.push_back(F);
			if(iunits != csv::CSV_NOT_FOUND) F.units = row[iunits].get<std::string>();
			if(ilongn != csv::CSV_NOT_FOUND) F.longname = row[ilongn].get<std::string>();
			if(idesc  != csv::CSV_NOT_FOUND) F.description = row[idesc].get<std::string>();
			int dummy = 0;
		}		

		size_t startchar = 0;
		size_t startcolumn = 0;
		for (size_t i = 0; i < fields.size(); i++) {
			fields[i].startchar = startchar;
			startchar = fields[i].endchar() + 1;
			
			fields[i].startcolumn = startcolumn;
			startcolumn += fields[i].nbands;
		}
		return true;
	};
	
	static int nullfieldindex(){
		return INT_MAX;		
	};

	bool contains_non_numeric_characters(const std::string& str, size_t startpos)
	{
		const static std::string validchars = "0123456789.+-eE ,\t\r\n";
		size_t pos = str.find_first_not_of(validchars, startpos);
		if (pos == std::string::npos) return false;
		else return true;
	}

	bool is_record_valid() {

		if (currentrecord.size() == 0) return false;

		size_t startpos = 0;
		if (headertype == cAsciiColumnFile::HeaderType::DFN) {
			startpos = RT_string.size();//character pos to start non-numeric check from
			if (RT_string.size() == 0) {
				if (fields[0].fmtchar == 'A' || fields[0].fmtchar == 'a') {
					startpos = fields[0].width;
				}
				else {
					if (charpositions_adjusted == false) {
						//Check if record starts with DATA or COMM that is not declared as a field in the DFN and adjust character positions accordingly
						if (strncasecmp(currentrecord.c_str(), "DATA", 4) == 0) {
							glog.logmsg(0, "\nDetected DATA at start of record that is not specified in the DFN file as a column. Adjusting character positions accordingly\n%s\n", currentrecord.c_str());
							RT_string = currentrecord.substr(0, 4);
							adjust_character_positions(RT_string.size());
							startpos = RT_string.size();
						}
						else if (strncasecmp(currentrecord.c_str(), "COMM", 4) == 0) {
							glog.logmsg(0, "\nDetected COMM at start of record that is not specified in the DFN file as a column. Adjusting character positions accordingly\n%s\n", currentrecord.c_str());
							RT_string = currentrecord.substr(0, 4);
							adjust_character_positions(RT_string.size());
							startpos = RT_string.size();
						}
					}
				}
			}
			int reclen = fields.back().endchar();
			if (currentrecord.size() < reclen) return false;
		}
				
		bool nonnumeric = contains_non_numeric_characters(currentrecord, startpos);
		if (nonnumeric) return false;
		else return true;
	}
	
	void adjust_character_positions(const size_t& offset) {
		for (size_t i = 0; i < fields.size(); i++) {			
			fields[i].startchar += offset;			
		}
		charpositions_adjusted = true;
	}

	int fieldindexbyname(const std::string& fieldname) const
	{
		for (size_t fi = 0; fi < fields.size(); fi++){
			if (strcasecmp(fields[fi].name, fieldname) == 0) return fi;
		}
		return -1;
	}
		
	bool skiprecords(const size_t& nskip) {		
		for (size_t i = 0; i < nskip; i++) {			
			if (ifs.eof()) return false;
			std::getline(ifs, currentrecord);						
		}
		return true;
	}
	   	  
	bool readnextrecord() {
		if (ifs.eof()) return false;
		std::getline(ifs, currentrecord);
		if (ifs.eof() && currentrecord.size() == 0){
			return false;			
		}	
		recordsreadsuccessfully++;
		return true;		
	}

	std::vector<std::string> delimited_parse(){
		std::vector<std::string> cs;
		cs = fieldparsestring(currentrecord.c_str(), " ,\t\r\n");		
		return cs;
	}

	std::vector<std::string> fixed_width_parse() {
		std::vector<std::string> cs;
		for (size_t i = 0; i < fields.size(); i++) {
			cAsciiColumnField& f = fields[i];
			for (size_t j = 0; j < f.nbands; j++) {
				const std::string s = trim(currentrecord.substr(f.startchar + j * f.width, f.width));				
				if (s == f.nullvaluestring){
					cs.push_back(std::string());
				}
				else cs.push_back(s);
			}
		}
		return cs;
	}

	size_t parserecord(){
		if (fields.size() > 0) {
			colstrings = fixed_width_parse();
		}
		else {
			colstrings = delimited_parse();			
		}
		return colstrings.size();
	}

	size_t ncolumns(){
		size_t n = 0;
		for (size_t i = 0; i < fields.size(); i++){
			n += fields[i].nbands;
		}
		return n;
	}
	
	template<typename T>
	inline void getcolumn(const size_t& columnnumber, T& v) const
	{					
		if (columnnumber >= colstrings.size()) {
			std::string msg = _SRC_;			
			msg += strprint("\n\tError trying to access column %zu when there are only %zu columns in the current record string (check format and delimiters)\nCurrent record is\n%s\n", columnnumber+1, colstrings.size(),currentrecord.c_str());
			throw(std::runtime_error(msg));			
		}
		else {			
			if (colstrings[columnnumber].size() == 0) {
				v = undefinedvalue(v);
			}
			else {
				std::istringstream(colstrings[columnnumber]) >> v;				
			}
		}		
	}

	template<typename T>
	inline void getcolumns(const size_t& columnnumber, std::vector<T>& vec, const int& n) const
	{
		vec.resize(n);
		for (size_t i = 0; i < n; i++) {
			getcolumn(i + columnnumber, vec[i]);
		}
	}
		
	template<typename T>
	void getfieldbyindex(const size_t& findex, T& v) const 
	{
		const size_t& cnum = fields[findex].startcol();
		getcolumn(cnum,v);		
	}

	template<typename T>
	void getfieldbyindex(const size_t& findex, std::vector<T>& vec) const 
	{
		size_t cnum = fields[findex].startcol();
		const size_t& n = fields[findex].nbands;
		getcolumns(cnum, vec, n);	
	}

	template<typename T>
	void getfieldlog10(const size_t& findex, std::vector<T>& vec) const
	{
		size_t base = fields[findex].startcol();
		size_t nb = fields[findex].nbands;
		vec.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {
			getcolumn(base,vec[bi]);
			if (!undefined(vec[bi])) {
				vec[bi] = log10(vec[bi]);
			}
			base++;
		}		
	}
	
	template<typename T>
	bool getvec_fielddefinition(const cFieldDefinition& fd, std::vector<T>& vec, const size_t& n) const
	{	
		const T udval = undefinedvalue((T)0);
		vec.resize(n);
		if (fd.type == cFieldDefinition::TYPE::NUMERIC) {
			size_t deflen = fd.numericvalue.size();
			for (size_t i = 0; i < n; i++) {
				if (deflen == 1) vec[i] = (T)fd.numericvalue[0];
				else vec[i] = (T)fd.numericvalue[i];
			}
			return true;
		}
		else if (fd.type == cFieldDefinition::TYPE::COLUMNNUMBER) {
			getcolumns(fd.column-1, vec, n);
			if (fd.flip) {
				for (size_t i = 0; i < n; i++) {					
					if (isdefined(vec[i])) {
						vec[i] *= -1.0;
					}
				}
			}
			return true;
		}
		else if (fd.type == cFieldDefinition::TYPE::VARIABLENAME) {			
			int findex = fieldindexbyname(fd.varname);
			if (findex < 0) {
				glog.errormsg(_SRC_, "Could not find a field named %s\n", fd.varname.c_str());
			}
			getfieldbyindex(findex, vec);
			if (fd.flip) {
				for (size_t i = 0; i < n; i++) {
					if (isdefined(vec[i])) vec[i] *= -1.0;
				}
			}
			return true;
		}
		else if (fd.type == cFieldDefinition::TYPE::UNAVAILABLE) {
			vec = std::vector<T>(n, udval);
			return false;
		}
		else {
			vec = std::vector<T>(n, udval);
			return false;
		}
	};
	
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
			if (recordsreadsuccessfully == 0) {
				readnextrecord();
			}
			
			if (parserecord() != numcolumns){
				continue;
			}
			int line;
			getfieldbyindex(fgroupindex, line);

			if (count == 0)lastline = line;

			if (line != lastline) return count;			

			for (size_t fi = 0; fi < nfields; fi++){
				size_t nbands = fields[fi].nbands;
				if (fields[fi].datatype() == eFieldType::INTEGER){
					std::vector<int> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++){
						intfields[fi].push_back(vec[bi]);
					}
				}
				else{
					std::vector<double> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++){
						doublefields[fi].push_back(vec[bi]);
					}
				}
			}
			count++;
		}while(readnextrecord());
		return count;
	};
};

#endif

 