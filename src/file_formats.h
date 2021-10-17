/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _file_formats_H
#define _file_formats_H

#include <cstring>
#include <vector>
#include <map>
#include <fstream>

#include "stacktrace.h"
#include "string_utils.h"
#include "general_utils.h"
#include "file_utils.h"



enum class eFieldType { REAL, INTEGER, CHAR };

class cAsciiColumnField {

private:
	

public:
	inline const static std::string validfmttypes = "aAiIeEfF";

	std::string name;//String used as the shorthand name of the field
	size_t fileorder = 0;//Zero based order of field in the file
	size_t nbands = 0;//Number of bands in the field
	char fmtchar = 'E';//Field type notation
	size_t width = 15;//Total width of field
	size_t decimals = 6;//Nuber of places after the decimal point
	size_t startchar = 0;//Zero based character index
	size_t startcolumn = 0;//Zero based column index
		
	std::string nullvaluestring;//String used as the null value
	std::string longname;//String used as the long version of the name
	std::string description;//String used as the description	
	std::string units;//String used as the units			
	std::map<std::string, std::string> atts;//Additional key=value pairs

	cAsciiColumnField (){}
	
	cAsciiColumnField(const size_t _order, const size_t _startcolumn, const std::string _name, const char _fmttype, const int _fmtwidth, const int _fmtdecimals, const int _nbands = 1) {
		fileorder = _order;
		startcolumn = _startcolumn;
		name = _name;
		fmtchar = _fmttype;
		width = _fmtwidth;
		decimals = _fmtdecimals;
		nbands = _nbands;
	};


	double nullvalue() const {
		return atof(nullvaluestring.c_str());
	}
	
	const size_t& startcol() const //Zero based start column index
	{
		return startcolumn;
	}

	const size_t endcol() const //Zero based end column index
	{
		return startcolumn - 1 + nbands;
	}

	const size_t endchar() const
	{		
		return startchar - 1 + (nbands*width);
	}
	
	bool hasnullvalue() const {
		if (nullvaluestring.size() > 0) return true;
		return false;
	}

	std::string fmtstr_single() const {
		std::string s;		
		s += strprint("%c", fmtchar);
		s += strprint("%zu", width);
		if (fmtchar != 'I') s += strprint(".%zu", decimals);
		return s;
	}

	std::string fmtstr() const {
		std::string s;
		if (nbands > 1) s += strprint("%zu", nbands);
		s += fmtstr_single();		
		return s;
	}
		
	bool valid_fmttype(){		
		return instring(validfmttypes, fmtchar);
	}

	bool parse_format_string(const std::string& formatstr){

		int inbands, iwidth, idecimals;
		if (std::sscanf(formatstr.c_str(), "%d%c%d.%d", &inbands, &fmtchar, &iwidth, &idecimals) == 4) {			
			nbands = inbands;
			width = iwidth;
			decimals = idecimals;
		}
		else if (std::sscanf(formatstr.c_str(), "%c%d.%d", &fmtchar, &iwidth, &idecimals) == 3) {
			nbands = 1;
			width = iwidth;
			decimals = idecimals;
		}
		else if (std::sscanf(formatstr.c_str(), "%c%d", &fmtchar, &iwidth) == 2) {
			nbands = 1;
			width = iwidth;
			decimals = 0;
		}
		else {
			return false;
		}
		
		bool stat = valid_fmttype();
		if (stat && nbands > 0 && width > 0 && decimals >= 0)return true;
		return false;
	}

	std::string simple_header_record(){

		std::string s;
		if (nbands == 1){
			s = strprint("%zu\t%s\n", startcolumn + 1, name.c_str());
		}
		else{
			s = strprint("%zu-%zu\t%s\n", startcolumn + 1, startcolumn + nbands, name.c_str());
		}
		return s;
	}

	std::string i3_header_record(){

		std::string channelname = name;
		if (nbands > 1){
			channelname = strprint("%s{%lu}", name.c_str(), nbands);
		}

		std::string typestr;
		if (fmtchar == 'I'){
			typestr = "INTEGER";
		}
		else if (fmtchar == 'F'){
			typestr = "DOUBLE";
		}

		std::string s;
		s += strprint("DATA\t%lu, %lu, NORMAL, , , ,\n", startcolumn, width);
		s += strprint("CHAN\t%s, %s, NORMAL, %lu, %lu, LABEL = \"%s\"\n", channelname.c_str(), typestr.c_str(), width, decimals, name.c_str());
		return s;
	}

	std::string aseggdf_header_record(){
		std::string fmtstr;
		if (nbands > 1) fmtstr += strprint("%lu", nbands);
		fmtstr += strprint("%c", fmtchar);
		fmtstr += strprint("%zu", width);
		if (fmtchar != 'I')fmtstr += strprint(".%zu", decimals);

		std::string s;
		s += strprint("DEFN %lu ST=RECD,RT=; %s : %s", fileorder + 1, name.c_str(), fmtstr.c_str());

		char comma = ',';
		char colon = ':';
		std::string o = " :";
		if (units.size() > 0){
			if (o[o.size() - 1] != colon && o[o.size() - 1] != comma) o += comma;
			o += strprint(" UNITS = %s ", units.c_str());
		}

		if (nullvaluestring.size() > 0){
			if (o[o.size() - 1] != colon && o[o.size() - 1] != comma) o += comma;
			o += strprint(" NULL = %s ", nullvaluestring.c_str());
		}

		if (description.size() > 0){
			if (o[o.size() - 1] != colon && s[s.size() - 1] != comma) o += comma;
			o += strprint(" %s", description.c_str());
		}

		if (o.size() > 2) s += o;
		s += strprint("\n");
		return s;
	}

	void print(){				
		printf(" name=%s:", name.c_str());
		printf(" order=%zu:", fileorder);
		printf(" bands=%zu:", nbands);
		printf(" startcol=%zu:", startcolumn);
		printf(" startchar=%zu:", startchar);
		printf(" endchar=%zu:", startchar);
		printf(" type=%c:", fmtchar);
		printf(" width=%zu:", width);
		printf(" decimals=%zu:", decimals);
		printf(" units=%s:", units.c_str());
		printf(" nullvalue=%s:", nullvaluestring.c_str());
		printf(" nullvalue=%lf:", nullvalue());
		printf(" longname=%s:", longname.c_str());
		printf(" description=%s:", description.c_str());		
		for (const auto& [key, value] : atts) {
			printf(" %s=%s:", key.c_str(),value.c_str());			
		}					
		printf("\n");
	}

	eFieldType datatype() const {		
		if (fmtchar == 'I' || fmtchar == 'i'){
			return eFieldType::INTEGER;
		}
		else if (fmtchar == 'F' || fmtchar == 'f'){
			return eFieldType::REAL;
		}
		else if (fmtchar == 'E' || fmtchar == 'e'){
			return eFieldType::REAL;
		}
		if (fmtchar == 'A' || fmtchar == 'a') {
			return eFieldType::CHAR;
		}
		else{						
			std::string msg = strprint("Unknown data type <%c>\n%s\n", fmtchar,_SRC_.c_str());			
			std::cerr << msg << std::endl;
		}
		return eFieldType::REAL;
	};

	bool ischar() const {
		if (datatype() == eFieldType::CHAR) return true;
		return false;
	}

	bool isinteger() const {
		if ( datatype() == eFieldType::INTEGER) return true;
		return false;
	}

	bool isreal() const {
		if (datatype() == eFieldType::REAL) return true;
		return false;
	}

	bool isnull(const double v) const {
		if (hasnullvalue()){
			if (v == nullvalue()) {
				return true;
			}
		}
		return false;
	}
};

class cOutputFileInfo{

	size_t lastfield = 0;
	size_t lastcolumn = 0;
	bool   allowmorefields = true;

	public:

	std::vector<cAsciiColumnField> fields;

	cOutputFileInfo() {	};

	void lockfields() {
		allowmorefields = false;
	}

	void addfield(const std::string& _name, const char& _form, const size_t _width, const size_t& _decimals, const size_t& _nbands = 1) {
		//Stop compiler whinging
		addfield_impl(_name, _form, (int)_width, (int)_decimals, (int)_nbands);
	}

	void addfield_impl(const std::string& _name, const char& _form, const int _width, const int& _decimals, const int& _nbands = 1){
		if (allowmorefields){
			cAsciiColumnField cf(lastfield,lastcolumn, _name, _form, _width, _decimals, _nbands);
			fields.push_back(cf);
			lastfield++;
			lastcolumn += _nbands;
		}
	}

	void setunits(const std::string& _units){
		if (allowmorefields){			
			fields[lastfield-1].units = _units;			
		}
	}

	void setnullvalue(const std::string& _nullvaluestr){
		if (allowmorefields){
			fields[lastfield - 1].nullvaluestring = _nullvaluestr;
		}
	}

	void setdescription(const std::string& _description){
		if (allowmorefields){
			fields[lastfield-1].description = _description;
		}
	}

	void write_simple_header(const std::string pathname){
		FILE* fp = fileopen(pathname.c_str(), "w");		
		for (size_t i = 0; i < fields.size(); i++){
			std::string s = fields[i].simple_header_record();
			fprintf(fp, s.c_str());
		}
		fclose(fp);
	};

	void write_csv_header(const std::string pathname) {
		FILE* fp = fileopen(pathname.c_str(), "w");
		fprintf(fp, "Name,Bands,Format,NullString,Units,Description\n");
		for (size_t i = 0; i < fields.size(); i++) {
			std::string s;
			s += strprint("%s,",fields[i].name.c_str());
			s += strprint("%zu,",fields[i].nbands);
			s += strprint("%s,", fields[i].fmtstr_single().c_str());
			s += strprint("%s,", fields[i].nullvaluestring.c_str());
			s += strprint("%s,", fields[i].units.c_str());
			s += strprint("%s", fields[i].description.c_str());
			s += strprint("\n");
			fprintf(fp, s.c_str());
		}
		fclose(fp);
	};

	void write_PAi3_header(const std::string pathname){
		FILE* fp = fileopen(pathname.c_str(), "w");
		fprintf(fp, "[IMPORT ARCHIVE]\n");
		fprintf(fp, "FILEHEADER\t1\n");
		fprintf(fp, "RECORDFORM\tFIXED\n");
		fprintf(fp, "SKIPSTRING\t\"/\"\n");
		for (size_t i = 0; i < fields.size(); i++){
			std::string s = fields[i].i3_header_record();
			fprintf(fp, s.c_str());
		}	
		fclose(fp);
	};

	void write_aseggdf_header(const std::string pathname){
		FILE* fp = fileopen(pathname.c_str(), "w");		
		fprintf(fp,"DEFN   ST=RECD,RT=COMM;RT:A4;COMMENTS:A76\n");
		for (size_t i = 0; i < fields.size(); i++){
			std::string s = fields[i].aseggdf_header_record();
			fprintf(fp, "%s", s.c_str());
		}
		fprintf(fp, "DEFN %zu ST=RECD,RT=;END DEFN\n", fields.size()+1);
		fclose(fp);
	};

};


class cASEGGDF2Header {

private:

	std::vector<cAsciiColumnField> fields;	
	std::string ST_string;
	std::string RT_string;

	static bool detect_geosoft_error(std::string s1, std::string s2) {
		s1.erase(remove_if(s1.begin(), s1.end(), isspace), s1.end());
		s2.erase(remove_if(s2.begin(), s2.end(), isspace), s2.end());		
		toupper(s1);

		if (s1 == "RT:A4") {
			auto t = tokenise(s2, ':');
			cAsciiColumnField F2;
			bool sttus = F2.parse_format_string(t[1]);
			return true;
		}
		return false;
	}

public:

	cASEGGDF2Header(const std::string& dfnpath){		
		read(dfnpath);
	}

	bool read(const std::string& dfnfile) {
		fields.clear();

		FILE* fp = fileopen(dfnfile, "r");
		std::string str;
		bool reported_mixing = false;
		bool reported_badincrement = false;
		bool reported_geosoft = false;

		size_t datarec = 0;
		int dfnlinenum = 0;
		int lastfileorder = -1;
		while (filegetline(fp, str)) {
			dfnlinenum++;
			//std::cout << str << std::endl;

			auto tk_space = tokenise(str, ' ');
			auto tk_semicolon = tokenise(str, ';');
			bool enddefn = false;
			bool processrecord = true;

			//Check for 'END DEFN' 
			for (size_t i = 0; i < tk_semicolon.size(); i++) {
				if (strcasecmp(tk_semicolon[i], "end defn") == 0) {
					enddefn = true;
					if (i == 0) {
						processrecord = false;
					}
				}
			}

			if (strncasecmp(tk_space[0], "defn",4) != 0) {
				std::string msg;
				msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
				msg += strprint("\tSkipping DFN entry that does not begin with 'DEFN' or 'END DEFN'\n");
				std::cerr << msg << std::endl;
				processrecord = false;
			}

			if (dfnlinenum == 28) {
				int dummy = 0;
			}

			if (processrecord) {
				cAsciiColumnField F;
								
				auto t1 = tokenise(tk_semicolon[0], ',');
				auto t2 = tokenise(t1[0], '=');
				auto t3 = tokenise(t1[1], '=');

				ST_string = t2[1];
				std::string rt = t3[1];

				if (ST_string != "RECD") {
					std::string msg;					
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
					msg += strprint("\tThe key 'ST' should be 'ST=RECD,'\n");					
					throw(std::runtime_error(msg));
				}

				if (rt != "" && rt != "DATA") {
					std::string msg;
					msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
					msg += strprint("\tSkipping DFN entry that does not have a record type 'RT=DATA;' or 'RT=;'\n");
					std::cerr << msg << std::endl;
					continue;
				}

				//Detect and try to fix Geosoft style DFN				
				if (reported_geosoft == false && tk_semicolon.size() > 2) {
					bool geosoft = detect_geosoft_error(tk_semicolon[1], tk_semicolon[2]);
					if (geosoft) {
						auto tmp1 = tokenise(tk_semicolon[1], ':');
						auto tmp2 = tokenise(tk_semicolon[2], ':');
						std::string msg;
						msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
						msg += strprint("\tDetected Geosoft style DFN with two format specifiers (%s and %s) in the one entry ", tmp1[1].c_str(), tmp2[1].c_str());
						msg += strprint("\tRemoving %s.\n", tk_semicolon[1].c_str());
						std::cerr << msg << std::endl;
						reported_geosoft = true;						
						for (size_t i = 1; i < tk_semicolon.size() - 1; i++) {
							tk_semicolon[i] = tk_semicolon[i + 1];
						}
						tk_semicolon.pop_back();
					}
				}

				//Get DEFN Number
				int order = 0;
				int n = std::sscanf(tk_semicolon[0].c_str(), "DEFN%d", &order);
				F.fileorder = (size_t)order;
				if (lastfileorder == -1) {
					if (F.fileorder != 1 && F.fileorder != 0) {
						std::string msg;
						msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
						msg += strprint("\tDEFN number does not start at 0 or 1. Check recommended.\n");
						std::cerr << msg << std::endl;
					}
				}
				else {
					if (reported_badincrement == false) {
						if (F.fileorder != (int)(lastfileorder + 1)) {
							std::string msg;
							msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
							msg += strprint("\tDEFN numbers are not incrementing by 1. Check recommended.\n");
							std::cerr << msg << std::endl;
							reported_badincrement = true;
						}
					}
				}

				
				//Data DEFN				
				auto colon_tokens = tokenise(tk_semicolon[1], ':');
				F.name = colon_tokens[0];
				if (strcasecmp(F.name,"end defn")==0){
					std::string msg;
					msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
					msg += strprint("\t'END DEFN' is out of place\n");
					std::cerr << msg << std::endl;
					continue;					
				}

				std::string formatstr = colon_tokens[1];

				F.parse_format_string(formatstr);

				if (F.valid_fmttype() == false) {
					std::string msg;					
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
					msg += strprint("\tFormat %s must start with one of '%s'\n", formatstr.c_str(), F.validfmttypes.c_str());
					throw(std::runtime_error(msg));
				}

				if (F.nbands < 1 || F.width < 1 || F.decimals < 0) {
					std::string msg = _SRC_;					
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
					msg += strprint("\tCould not decipher the format %s\n", formatstr.c_str());
					throw(std::runtime_error(msg));
				}

				//Parse the rest
				if (colon_tokens.size() > 2) {
					int nea = 0;
					std::string remainder = colon_tokens[2];
					auto tk_comma = tokenise(remainder, ',');
					for (size_t i = 0; i < tk_comma.size(); i++) {
						auto tk_equal = tokenise(tk_comma[i], '=');
						if (tk_equal.size() == 1) {
							nea++;
							std::string eaname = strprint("extra%d", nea);
							std::pair<std::string, std::string> p(eaname, tk_comma[i]);
							F.atts.insert(p);
						}
						else if (tk_equal.size() == 2) {
							if (strcasecmp(tk_equal[0], "unit") == 0 || strcasecmp(tk_equal[0], "units") == 0) {
								F.units = tk_equal[1];
							}
							else if (strcasecmp(tk_equal[0], "name") == 0) {
								F.longname = tk_equal[1];
							}
							else if (strcasecmp(tk_equal[0], "null") == 0) {
								F.nullvaluestring = tk_equal[1];
							}
							else if (strcasecmp(tk_equal[0], "desc") == 0 || strcasecmp(tk_equal[0], "description") == 0) {
								F.description = tk_equal[1];
							}
							else {
								std::pair<std::string, std::string> p(tolower(tk_equal[0]), tk_equal[1]);
								F.atts.insert(p);
							}
						}
						else {
							std::string msg;							
							msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), str.c_str());
							msg += strprint("\tunknown erroe\n");
							throw(std::runtime_error(msg));
						}
					}
				}

				if (datarec == 0) {
					RT_string = rt;
				}
				else {					
					if (reported_mixing == false && rt != RT_string) {
						std::string msg = strprint("\tDetected mixing of RT=; and RT=DATA; ");
						msg += strprint("at line %d of DFN file %s. ", dfnlinenum, dfnfile.c_str());
						msg += strprint("Making all data record types RT=%s;.\n", RT_string.c_str());
						std::cerr << msg << std::endl;
						reported_mixing = true;
					}
				}

				if (datarec == 0 && rt == "DATA") {
					if (!F.ischar() && F.width != 4) {
						cAsciiColumnField R;
						R.name = "RT";
						R.longname = "Record type";
						R.fileorder = (size_t)0;
						R.fmtchar = 'A';
						R.width = 4;
						R.decimals = 0;
						R.nbands = 1;
						fields.push_back(R);
						datarec++;
					}
				}
				//F.print();
				lastfileorder = F.fileorder;
				fields.push_back(F);
				datarec++;
			}
		};
		fclose(fp);
		std::cerr << std::flush;

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

	void write(const std::string& dfnpath){
		FILE* fp = fileopen(dfnpath, "w");
		fprintf(fp, "DEFN   ST=RECD,RT=COMM;RT:A4;COMMENTS:A76\n");
		for (size_t i = 0; i < fields.size(); i++){
			std::string s = fields[i].aseggdf_header_record();
			fprintf(fp, s.c_str());
		}
		fprintf(fp, "DEFN %zu ST=RECD,RT=;END DEFN\n", fields.size() + 1);
		fclose(fp);
	};

	const std::vector<cAsciiColumnField>& getfields() const {
		return fields;
	};

	const std::string get_ST_string() const {
		return ST_string;
	};

	const std::string get_RT_string() const {
		return RT_string;
	};
};


class cFieldManager{

private:

public:
	std::vector<cAsciiColumnField> fields;

	cFieldManager(){};

	cFieldManager(const std::vector<cAsciiColumnField> _fields){
		fields = _fields;
	};

	static size_t nullfieldindex(){ return UINT64_MAX; };

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

	size_t ncolumns() const {
		size_t n = 0;
		for (size_t i = 0; i < fields.size(); i++){
			n += fields[i].nbands;
		}
		return n;
	}
};

class cColumnFile {

private:
	std::ifstream file;
	std::string currentrecord;
	std::vector<std::string> currentcolumns;
	size_t recordsreadsuccessfully;

public:
	
	cFieldManager F;

	cColumnFile(){ initialise(); };

	cColumnFile(const std::string& datafile, const std::string& headerfile){
		initialise();
		openread(datafile);
		cASEGGDF2Header A(headerfile);
		F = cFieldManager(A.getfields());
	};

	~cColumnFile(){
		if (file.is_open())file.close();		
	};

	void initialise(){		
		recordsreadsuccessfully = 0;
	};

	const cAsciiColumnField& fields(const size_t fi){
		return F.fields[fi];
	}

	size_t nfields() const {
		return F.fields.size();
	}

	size_t ncolumns() const {
		return F.ncolumns();
	}

	const char* currentrecord_cstr(){ return currentrecord.c_str(); };

	const std::string& currentrecordstring(){ return currentrecord; };

	bool openread(const std::string& datafilename){
		std::string path = datafilename;
		fixseparator(path);
		file.open(path, std::fstream::in);
		if (file.is_open())return true;
		else{
			std::string msg = _SRC_ + strprint("Could not open file (%s)\n", path.c_str());
			throw(std::runtime_error(msg));
		}
		return false;
	};

	void close(){ file.close(); };

	bool readnextrecord(){
		if (file.eof())return false;
		std::getline(file,currentrecord);
		recordsreadsuccessfully++;
		return true;		
	}

	size_t parserecord(){
		currentcolumns = parsestrings(currentrecord, " ,\t\r\n");
		return currentcolumns.size();
	}

	bool getcolumn(const size_t columnnumber, int& v){
		v = atoi(currentcolumns[columnnumber].c_str());
		return true;
	}

	bool getcolumn(const size_t columnnumber, double& v){
		v = atof(currentcolumns[columnnumber].c_str());
		return true;
	}

	bool getfield(const size_t findex, int& v){
		size_t base = fields(findex).startcol();
		v = atoi(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, double& v){
		size_t base = fields(findex).startcol();
		v = atof(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, std::vector<int>& v){
		size_t base = fields(findex).startcol();
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++){
			v[bi] = atoi(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfield(const size_t findex, std::vector<double>& v){
		size_t base = fields(findex).startcol();
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++){
			v[bi] = atof(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfieldlog10(const size_t findex, std::vector<double>& v){
		size_t base = fields(findex).startcol();
		size_t nb   = fields(findex).nbands;

		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++){
			v[bi] = atof(currentcolumns[base].c_str());
			if (fields(findex).isnull(v[bi]) == false){
				v[bi] = log10(v[bi]);
			}
			base++;
		}
		return true;
	}

	size_t readnextgroup(const size_t fgroupindex, std::vector<std::vector<int>>& intfields, std::vector<std::vector<double>>& doublefields){
		
		if (file.eof())return 0;

		intfields.clear();
		doublefields.clear();
		intfields.resize(nfields());
		doublefields.resize(nfields());

		int lastline;
		size_t count = 0;
		do{
			if (recordsreadsuccessfully == 0) readnextrecord();
			if (parserecord() != ncolumns()){
				continue;
			}
			int line;
			getfield(fgroupindex, line);

			if (count == 0)lastline = line;

			if (line != lastline){
				return count;
			}

			for (size_t fi = 0; fi < nfields(); fi++){
				size_t nbands = fields(fi).nbands;
				if (fields(fi).datatype() == eFieldType::INTEGER){
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

 