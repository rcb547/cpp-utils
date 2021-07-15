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
	size_t recordsreadsuccessfully = 0;
	std::string currentrecord;		
	std::vector<std::string> currentcolumns;
	bool charpositions_adjusted = false;

public:

	std::string ST_string;
	std::string RT_string;
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

	bool parse_aseggdf2_header(const std::string& dfnfile) {

		fields.clear();
			

		FILE* fp = fileopen(dfnfile, "r");
		std::string str;

		size_t datarec = 0;
		int dfnlinenum = 0;
		while (filegetline(fp, str)) {
			dfnlinenum++;
			
			if (strcasecmp(str, "end defn") == 0) {
				break;
			}

			std::vector<std::string> tokens = tokenise(str, ';');
			if (strcasecmp(tokens[1], "end defn") == 0) {
				break;
			}

			cAsciiColumnField F;
			
			int intorder = 0;
			int n = std::sscanf(tokens[0].c_str(), "DEFN %d",&intorder);
			F.order = (size_t)intorder;
			
			std::vector<std::string> t1 = tokenise(tokens[0], ',');
			std::vector<std::string> t2 = tokenise(t1[0], '=');
			std::vector<std::string> t3 = tokenise(t1[1], '=');
			
			ST_string = t2[1];
			std::string rt = t3[1];

			if (ST_string != "RECD") {				
				std::string msg = _SRC_;
				msg += strprint("\n\tError parsing line %d of DFN file %s\n",dfnlinenum,dfnfile.c_str());
				msg += strprint("\tThe key 'ST' should be 'ST=RECD,'\n");
				throw(std::runtime_error(msg));
			}

			if (rt != "" && rt != "DATA") {
				std::string msg = strprint("\tSkipping DFN entry that does not have a record type 'RT=DATA;' or 'RT=;' on line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
				glog.logmsg(0, msg);
				continue;
			}

			//Detect and try to fix Geosoft style DFN
			static bool geosoft_reported = false;
			if (geosoft_reported == false && tokens.size() > 2) {
				bool geosoft = detect_geosoft_error(tokens[1], tokens[2]);
				if (geosoft) {
					std::vector<std::string> tmp1 = tokenise(tokens[1], ':');
					std::vector<std::string> tmp2 = tokenise(tokens[2], ':');
					std::string msg = strprint("\n\tDetected Geosoft style DFN with two format specifiers (%s and %s) in the one entry ", tmp1[1].c_str(), tmp2[1].c_str());
					msg += strprint("on line %d of DFN file %s. ", dfnlinenum, dfnfile.c_str());
					msg += strprint("Removing %s.\n", tokens[1].c_str());
					msg += strprint("\t%s\n", str.c_str());
					geosoft_reported = true;
					glog.logmsg(0, msg);
					for (size_t i = 1; i < tokens.size() - 1; i++) {
						tokens[i] = tokens[i + 1];
					}
					tokens.pop_back();
				}
			}
			
			//Data DEFN
			tokens = tokenise(tokens[1], ':');
			F.name = tokens[0];
			std::string formatstr = tokens[1];

			F.parse_format_string(formatstr);
					
			if (F.valid_fmttype() == false) {
				std::string msg = _SRC_;
				msg += strprint("\tError parsing line %d of DFN file %s\n", dfnlinenum, dfnfile.c_str());
				msg += strprint("\tFormat %s must start with one of '%s'\n", formatstr.c_str(), F.validfmttypes.c_str());
				throw(std::runtime_error(msg));
			}

			if(F.nbands < 1 || F.fmtwidth < 1 || F.fmtdecimals < 0){
				std::string msg = _SRC_;
				msg += strprint("\tError parsing line %d of DFN file %s\n", dfnlinenum, dfnfile.c_str());
				msg += strprint("\tCould not decipher the format %s\n", formatstr.c_str());
				throw(std::runtime_error(msg));
			}

			//Parse the rest
			if (tokens.size() > 2) {
				std::string remainder = tokens[2];
				tokens = tokenise(remainder, ',');
				for (size_t i = 0; i < tokens.size(); i++) {
					std::vector<std::string> t = tokenise(tokens[i], '=');
					if (strcasecmp(t[0], "unit") == 0 || strcasecmp(t[0], "units") == 0) {
						F.units = t[1];
					}
					else if (strcasecmp(t[0], "name") == 0) {
						F.expandedname = t[1];
					}
					else if (strcasecmp(t[0], "null") == 0) {
						F.nullvaluestr = t[1];
						F.nullvalue = atof(t[1].c_str());
					}
					else {
						F.description += tokens[i];
					}
				}
			}

			if (datarec == 0) {
				RT_string = rt;
			}
			else {
				static bool reported_mixing = false;				
				if (reported_mixing == false && rt != RT_string) {
					std::string msg = strprint("\tDetected mixing of RT=; and RT=DATA; ");
					msg += strprint("at line %d of DFN file %s. ", dfnlinenum, dfnfile.c_str());
					msg += strprint("Making all data record types RT=%s;.\n", RT_string.c_str());
					glog.logmsg(0, msg);
					reported_mixing = true;
				}
			}
			

			if (datarec==0 && rt == "DATA") {
				if (!F.ischar() && F.fmtwidth != 4) {					
					cAsciiColumnField R;
					R.name = "RT";
					R.expandedname = "Record type";
					R.order = (size_t)0;
					R.fmttype = 'A';
					R.fmtwidth = 4;					
					fields.push_back(R);					
					datarec++;
				}
			}
			fields.push_back(F);
			datarec++;			
		};
		fclose(fp);

		size_t k = 0;
		size_t startcolumn = 1;
		for (size_t i = 0; i < fields.size(); i++) {			
			fields[i].startchar = k;
			fields[i].endchar   = k - 1 + fields[i].fmtwidth * fields[i].nbands;
			k = fields[i].endchar + 1;
			fields[i].startcolumn = startcolumn;
			startcolumn += fields[i].nbands;
		}
		return true;
	};

	static size_t nullfieldindex(){
		return UINT64_MAX;		
	};

	bool detect_geosoft_error(std::string s1, std::string s2) {				
		s1.erase(remove_if(s1.begin(), s1.end(), isspace), s1.end());
		s2.erase(remove_if(s2.begin(), s2.end(), isspace), s2.end());		
		std::transform(s1.begin(), s1.end(), s1.begin(), ::toupper);
		
		if (s1 == "RT:A4") {
			std::vector<std::string> t = tokenise(s2, ':');
			cAsciiColumnField F2;
			bool sttus = F2.parse_format_string(t[1]);
			return true;
		}
		return false;
	}

	bool contains_non_numeric_characters(const std::string& str, size_t startpos)
	{
		const static std::string validchars = "0123456789.+-eE ,\t\r\n";
		size_t pos = str.find_first_not_of(validchars, startpos);
		if (pos == std::string::npos) return false;
		else return true;
	}

	bool is_record_valid() {

		size_t startpos = RT_string.size();//character pos to start non-numeric check from
		if (RT_string.size() == 0){
			if (fields[0].fmttype == 'A' || fields[0].fmttype == 'a') {
				startpos = fields[0].fmtwidth;
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
		
		bool nonnumeric = contains_non_numeric_characters(currentrecord, startpos);
		if (nonnumeric) return false;
		else return true;
	}
	
	void adjust_character_positions(const size_t& offset) {
		for (size_t i = 0; i < fields.size(); i++) {			
			fields[i].startchar += offset;
			fields[i].endchar   += offset;
		}
		charpositions_adjusted = true;
	}

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
				cs.push_back(currentrecord.substr(f.startchar + j * f.fmtwidth, f.fmtwidth));				
			}
		}
		return cs;
	}

	size_t parserecord(){
		if (fields.size() > 0) {
			currentcolumns = fixed_width_parse();
		}
		else {
			currentcolumns = delimited_parse();			
		}
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
		if (columnnumber >= currentcolumns.size()) {
			glog.errormsg(_SRC_,"Error trying to access column %zu when there are only %zu columns in the current record string (check format and delimiters)\nCurrent record is\n%s\n", columnnumber+1, currentcolumns.size(),currentrecord.c_str());
			return false;
		}
		else {
			std::istringstream(currentcolumns[columnnumber]) >> v;
		}
		return true;
	}
		
	template<typename T>
	bool getfield(const size_t& findex, T& v) const 
	{
		size_t base = fields[findex].startcolumn-1;
		getcolumn(base,v);	
		if (fields[findex].isnull(v) == true) {
			return false;
		}
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
			if (fields[findex].isnull((double)vec[bi]) == true) {
				return false;
			}
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
			if (fields[findex].isnull(vec[bi]) == true) {
				return false;
			}
			else {
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
			if (recordsreadsuccessfully == 0) {
				readnextrecord();
			}
			
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
		}while(readnextrecord());
		return count;
	};
};

#endif

 