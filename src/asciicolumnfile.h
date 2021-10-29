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
#include <filesystem>

#include "csv.hpp"
#include "general_utils.h"
#include "file_utils.h"
#include "file_formats.h"
#include "fielddefinition.h"

#if defined _MPI_ENABLED
#include "mpi_wrapper.h"
#endif

class cAsciiColumnFile {

private:	
	static constexpr int newline = 10;
	static constexpr int carriagereturn = 13;
	std::ifstream IFS;
	std::string FileName;
	size_t FileSize = 0;
	size_t RecordLength=0;//Length in bytes of records including "\r\n" or "\n"
	std::string CurrentRecord;
	std::vector<std::string> colstrings;

	//Related to header parsing
	bool charpositions_adjusted = false;
	std::string ST_string;
	std::string RT_string;

	size_t determine_record_length_no_rewind() {		
		size_t k = 0;		
		while (true) {
			if (IFS.eof()) return k;
			k++;						
			if (IFS.get() == newline) return k;
		}		
		return k;		
	}

	size_t determine_record_length(){
		rewind();
		size_t rl = determine_record_length_no_rewind();
		size_t k = 0;
		while(k < 100 && IFS.eof()==false) {
			size_t n = determine_record_length_no_rewind();
			if (n != rl) {
				std::string msg;
				msg += strprint("%s is not a fixed record length\n", FileName.c_str());
				msg += strprint("\trecord 1 has length %zu\n", rl);
				msg += strprint("\trecord %zu has length %zu\n", k+1, n);
				throw(std::runtime_error(msg));
			}
			k++;
		}
		return rl;
	}


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
	
	const std::string& currentrecord_string() const { return CurrentRecord; };
	
	const std::vector<std::string>& currentrecord_columns() const { return colstrings; };

	void clear_currentrecord() { CurrentRecord.clear(); };

	void set_record_length(const size_t& length) {
		RecordLength = length;
	}

	size_t get_record_length() {
		return RecordLength;
	}

	size_t file_size() {
		return FileSize;
	}

	size_t nrecords() {
		return (size_t)std::ceil((double)FileSize / (double)RecordLength);
	}

	size_t nrecords_manual_count() {
		rewind();
		size_t nr = 0;
		std::streamsize gc = 0;
		while (IFS.ignore(RecordLength, newline)) {
			nr++;
			gc = IFS.gcount();
		}
		if (gc == 0) nr--;//In case of blank last line (newline after last line)
		IFS.clear();
		return nr;
	}
	
	bool goto_record(const size_t& n) {
		std::streamoff p = n * RecordLength;
		if (IFS.seekg(p, IFS.beg))return true;
		return false;
	}

	bool load_next_record() {
		if (std::getline(IFS, CurrentRecord)) {
			return true;
		}
		return false;
	}

	bool load_record(const size_t& n) {
		goto_record(n);
		if (std::getline(IFS, CurrentRecord)) {
			return true;
		}
		return false;
	}

	std::string get_next_record() {
		load_next_record();
		return CurrentRecord;		
	}

	std::string get_record(size_t n) {
		load_record(n);		
		return CurrentRecord;
	}

	
	void rewind() {
		IFS.clear();
		IFS.seekg(0);
	}

	bool openfile(const std::string& datafilename){
		FileName = datafilename;
		fixseparator(FileName);
		//Open in binary mode so \r\n does not get converted to \n
		IFS.open(datafilename, std::ifstream::in | std::ifstream::binary);
		if (!IFS) {
			glog.errormsg(_SRC_,"Could not open file %s\n", FileName.c_str());
		}		
		FileSize = std::filesystem::file_size(FileName);		

		#if defined _MPI_ENABLED
			if (cMpiEnv::world_rank() == 0) {
				RecordLength = determine_record_length();
				rewind();
			}
			cMpiComm c = cMpiEnv::world_comm();
			c.bcast(RecordLength);			
			cMpiEnv::world_barrier();
		#else
			RecordLength = determine_record_length();
		#endif		

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
		
	static int nullfieldindex(){
		return INT_MAX;		
	};

	void read_dfn(const std::string& dfnpath) {
		cASEGGDF2Header H(dfnpath);
		fields = H.getfields();
		ST_string = H.get_ST_string();
		RT_string = H.get_RT_string();
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
		size_t iname = R.index_of("Name");
		size_t inbands = R.index_of("Bands");
		size_t ifmt = R.index_of("Format");
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
			if (iunits != (size_t)csv::CSV_NOT_FOUND) F.units = row[iunits].get<std::string>();
			if (ilongn != (size_t)csv::CSV_NOT_FOUND) F.longname = row[ilongn].get<std::string>();
			if (idesc != (size_t)csv::CSV_NOT_FOUND) F.description = row[idesc].get<std::string>();
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

	bool contains_non_numeric_characters(const std::string& str, size_t startpos)
	{
		const static std::string validchars = "0123456789.+-eE ,\t\r\n";
		size_t pos = str.find_first_not_of(validchars, startpos);
		if (pos == std::string::npos) return false;
		else return true;
	}

	bool is_record_valid() {

		if (CurrentRecord.size() == 0) return false;

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
						if (strncasecmp(CurrentRecord.c_str(), "DATA", 4) == 0) {
							glog.logmsg(0, "\nDetected DATA at start of record that is not specified in the DFN file as a column. Adjusting character positions accordingly\n%s\n", CurrentRecord.c_str());
							RT_string = CurrentRecord.substr(0, 4);
							adjust_character_positions(RT_string.size());
							startpos = RT_string.size();
						}
						else if (strncasecmp(CurrentRecord.c_str(), "COMM", 4) == 0) {
							glog.logmsg(0, "\nDetected COMM at start of record that is not specified in the DFN file as a column. Adjusting character positions accordingly\n%s\n", CurrentRecord.c_str());
							RT_string = CurrentRecord.substr(0, 4);
							adjust_character_positions(RT_string.size());
							startpos = RT_string.size();
						}
					}
				}
			}
			size_t reclen = fields.back().endchar();
			if (CurrentRecord.size() < reclen) return false;
		}
				
		bool nonnumeric = contains_non_numeric_characters(CurrentRecord, startpos);
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
			if (strcasecmp(fields[fi].name, fieldname) == 0) return (int)fi;
		}
		return -1;
	}
		
	bool skiprecords(const size_t& nskip) {		
		for (size_t i = 0; i < nskip; i++) {			
			if (IFS.eof()) return false;
			std::getline(IFS, CurrentRecord);						
		}
		return true;
	}
	   		
	std::vector<std::string> delimited_parse(){
		std::vector<std::string> cs;
		cs = fieldparsestring(CurrentRecord.c_str(), " ,\t\r\n");		
		return cs;
	}

	std::vector<std::string> fixed_width_parse() {
		std::vector<std::string> cs;
		for (size_t i = 0; i < fields.size(); i++) {
			cAsciiColumnField& f = fields[i];
			for (size_t j = 0; j < f.nbands; j++) {
				const std::string s = trim(CurrentRecord.substr(f.startchar + j * f.width, f.width));				
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
			msg += strprint("\n\tError trying to access column %zu when there are only %zu columns in the current record string (check format and delimiters)\nCurrent record is\n%s\n", columnnumber+1, colstrings.size(),CurrentRecord.c_str());
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
	inline void getcolumns(const size_t& columnnumber, std::vector<T>& vec, const size_t& n) const
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
						vec[i] *= (T)-1;
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
					if (isdefined(vec[i])) vec[i] *= (T)-1;
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
	
	/*
	size_t readnextgroup(const size_t& fgroupindex, std::vector<std::vector<int>>& intfields, std::vector<std::vector<double>>& doublefields) {

		size_t nfields = fields.size();
		size_t numcolumns = ncolumns();
		if (IFS.eof()) return 0;

		intfields.clear();
		doublefields.clear();
		intfields.resize(fields.size());
		doublefields.resize(fields.size());

		int lastline;
		size_t count = 0;
		do {
			if (recordsreadsuccessfully == 0) {
				readnextrecord();//Put the first record in
			}

			if (parserecord() != numcolumns) {
				continue;
			}
			int line;
			getfieldbyindex(fgroupindex, line);

			if (count == 0)lastline = line;

			if (line != lastline) return count;

			for (size_t fi = 0; fi < nfields; fi++) {
				size_t nbands = fields[fi].nbands;
				if (fields[fi].datatype() == eFieldType::INTEGER) {
					std::vector<int> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++) {
						intfields[fi].push_back(vec[bi]);
					}
				}
				else {
					std::vector<double> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++) {
						doublefields[fi].push_back(vec[bi]);
					}
				}
			}
			count++;
		} while (readnextrecord());
		return count;
	};
	*/

	
	size_t readnextgroup(const size_t& fgroupindex, std::vector<std::vector<int>>& intfields, std::vector<std::vector<double>>& doublefields) {

		size_t nfields = fields.size();
		size_t numcolumns = ncolumns();
		if (IFS.eof()) return 0;

		intfields.clear();
		doublefields.clear();
		intfields.resize(fields.size());
		doublefields.resize(fields.size());

		int lastline;
		size_t count = 0;
		//Leave the last read record (from the next line) up the spout for next time
		do {
			if (CurrentRecord.empty()) {
				load_next_record();//Put the first record in
			}

			if (parserecord() != numcolumns) {
				continue;
			}

			int line;
			getfieldbyindex(fgroupindex, line);

			if (count == 0) lastline = line;

			if (line != lastline) return count;

			for (size_t fi = 0; fi < nfields; fi++) {
				size_t nbands = fields[fi].nbands;
				if (fields[fi].datatype() == eFieldType::INTEGER) {
					std::vector<int> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++) {
						intfields[fi].push_back(vec[bi]);
					}
				}
				else {
					std::vector<double> vec;
					getfieldbyindex(fi, vec);
					for (size_t bi = 0; bi < nbands; bi++) {
						doublefields[fi].push_back(vec[bi]);
					}
				}
			}
			count++;
		} while (load_next_record());
		return count;
	};
	

	size_t scan_for_line_index(const int& field_index, std::vector<unsigned int>& line_index_start, std::vector<unsigned int>& line_index_count, std::vector<unsigned int>& line_number)
	{
		_GSTITEM_		
		size_t fi = field_index;
		const size_t& i1 = fields[fi].startchar;
		const size_t& width = fields[fi].width;		
		
		unsigned int lastline=-1;		
		rewind();		
		unsigned int nread = 0;
		while (load_next_record()) {
			std::string t = CurrentRecord.substr(i1, width);
			unsigned int lnum = atoi(t.data());
			if (nread==0 || lnum != lastline) {
				line_number.push_back(lnum);
				line_index_start.push_back(nread);
				line_index_count.push_back(1);
				lastline = lnum;
			}
			else {
				line_index_count.back()++;
			}
			nread++;
		}
		rewind();
		return nread;
	}

	std::vector<bool>  scan_for_groupby_fields(const std::vector<unsigned int>& line_index_count)
	{
		_GSTITEM_						
		std::vector<bool> groupby(fields.size(), true);		
		size_t nltocheck = std::min((size_t)4, line_index_count.size());
		rewind();
		for (size_t li = 0; li < nltocheck; li++) {			
			std::string first = get_next_record();
			for (size_t si = 1; si < line_index_count[li]; si++) {				
				std::string current = get_next_record();				
				for (size_t fi = 0; fi < fields.size(); fi++) {
					if (groupby[fi] == true) {//do not check fields already known to not be groupby						
						const size_t& i1 = fields[fi].startchar;
						const size_t& width = fields[fi].width;						
						std::string a = first.substr(i1, width);
						std::string b = current.substr(i1, width);						
						if (a != b) groupby[fi] = false;
					}
				}
			}
		}
		rewind();
		return groupby;
	}
		
};

#endif

 