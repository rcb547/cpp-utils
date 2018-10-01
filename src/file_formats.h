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
#include <fstream>

#include "general_utils.h"
#include "file_utils.h"

enum eFieldType {REAL,INTEGER};

class cAsciiColumnField {

public:
	size_t order;
	size_t startcolumn;
	size_t endcolumn() const
	{
		return startcolumn + nbands - 1;
	}
	size_t startchar;
	size_t endchar;
	std::string name;
	std::string expandedname;

	size_t nbands;

	char   fmttype;
	size_t fmtwidth;
	size_t fmtdecimals;
	
	std::string units;

	bool hasnullvalue;
	std::string nullvaluestr;
	double nullvalue;
	std::string comment;

	cAsciiColumnField(){
		initialise();
	};

	cAsciiColumnField(const size_t _order, const size_t _startcolumn, const std::string _name, const char _fmttype, const size_t _fmtwidth, const size_t _fmtdecimals, const size_t _nbands = 1){
		initialise();
		order = _order;
		startcolumn = _startcolumn;
		name = _name;
		fmttype = _fmttype;
		fmtwidth = _fmtwidth;
		fmtdecimals = _fmtdecimals;
		nbands = _nbands;
	};

	void initialise(){
		nbands = 1;
		fmtdecimals = 0;
		hasnullvalue = false;
		nullvalue = -32767;
	};

	std::string simple_header_record(){

		std::string s;
		if (nbands == 1){
			s = strprint("%lu\t%s\n", startcolumn + 1, name.c_str());
		}
		else{
			s = strprint("%lu-%lu\t%s\n", startcolumn + 1, startcolumn + nbands, name.c_str());
		}
		return s;
	}

	std::string i3_header_record(){

		std::string channelname = name;
		if (nbands > 1){
			channelname = strprint("%s{%lu}", name.c_str(), nbands);
		}

		std::string typestr;
		if (fmttype == 'I'){
			typestr = "INTEGER";
		}
		else if (fmttype == 'F'){
			typestr = "DOUBLE";
		}

		std::string s;
		s += strprint("DATA\t%lu, %lu, NORMAL, , , ,\n", startcolumn, fmtwidth);
		s += strprint("CHAN\t%s, %s, NORMAL, %lu, %lu, LABEL = \"%s\"\n", channelname.c_str(), typestr.c_str(), fmtwidth, fmtdecimals, name.c_str());
		return s;
	}

	std::string aseggdf_header_record(){
		std::string fmtstr;
		if (nbands > 1) fmtstr += strprint("%lu", nbands);
		fmtstr += strprint("%c", fmttype);
		fmtstr += strprint("%lu", fmtwidth);
		if (fmttype != 'I')fmtstr += strprint(".%lu", fmtdecimals);

		std::string s;
		s += strprint("DEFN %lu ST=RECD,RT=; %s : %s", order + 1, name.c_str(), fmtstr.c_str());

		char comma = ',';
		char colon = ':';
		std::string o = " :";
		if (units.size() > 0){
			if (o[o.size() - 1] != colon && o[o.size() - 1] != comma) o += comma;
			o += strprint(" UNITS = %s ", units.c_str());
		}

		if (nullvaluestr.size() > 0){
			if (o[o.size() - 1] != colon && o[o.size() - 1] != comma) o += comma;
			o += strprint(" NULL = %s ", nullvaluestr.c_str());
		}

		if (comment.size() > 0){
			if (o[o.size() - 1] != colon && s[s.size() - 1] != comma) o += comma;
			o += strprint(" %s", comment.c_str());
		}

		if (o.size() > 2) s += o;
		s += strprint("\n");
		return s;
	}

	void print(){		
		printf("\n");
		printf(" name=%s", name.c_str());
		printf(" order=%lu", order);
		printf(" startcolumn=%lu", startcolumn);		
		printf(" bands=%lu", nbands);
		printf(" type=%c", fmttype);
		printf(" width=%lu", fmtwidth);
		printf(" decimals=%lu", fmtdecimals);
		printf(" units=%s", units.c_str());
		printf(" nullvalue=%s", nullvaluestr.c_str());
		printf(" nullvalue=%lf", nullvalue);
		printf(" expandedname=%s", expandedname.c_str());
		printf(" comment=%s", comment.c_str());
		printf("\n");
	}

	eFieldType datatype() const {		
		if (fmttype == 'I' || fmttype == 'i'){
			return eFieldType::INTEGER;
		}
		else if (fmttype == 'F' || fmttype == 'f'){
			return eFieldType::REAL;
		}
		else if (fmttype == 'E' || fmttype == 'e'){
			return eFieldType::INTEGER;
		}
		else{			
			warningmessage("Unknown data type <%c>\n", fmttype);			
		}
		return eFieldType::REAL;
	};

	bool isinteger() const {
		if ( datatype() == eFieldType::INTEGER) return true;
		return false;
	}

	bool isreal() const {
		if (datatype() == eFieldType::REAL) return true;
		return false;
	}

	bool isnull(const double v) const {
		if (hasnullvalue){
			if (v == nullvalue)return true;
		}
		return false;
	}
};

class cOutputFileInfo{

	size_t lastfield;
	size_t lastcolumn;
	bool   allowmorefields;

	public:

	std::vector<cAsciiColumnField> fields;

	cOutputFileInfo(){
		lastfield  = 0;
		lastcolumn = 0;
		allowmorefields = true;
	}

	void lockfields(){		
		allowmorefields = false;
	}

	void addfield(const std::string _name, const char _form, const size_t _width, const size_t _decimals, const size_t _nbands = 1){
		if (allowmorefields){
			cAsciiColumnField cf(lastfield,lastcolumn, _name, _form, _width, _decimals, _nbands);
			fields.push_back(cf);
			lastfield++;
			lastcolumn += _nbands;
		}
	}

	void setunits(const std::string _units){
		if (allowmorefields){			
			fields[lastfield-1].units = _units;			
		}
	}

	void setnullvalue(const std::string _nullvaluestr){
		if (allowmorefields){
			fields[lastfield - 1].nullvaluestr = _nullvaluestr;
		}
	}

	void setcomment(const std::string _comment){
		if (allowmorefields){
			fields[lastfield-1].comment = _comment;
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
		fprintf(fp, "DEFN %lu ST=RECD,RT=;END DEFN\n", fields.size()+1);
		fclose(fp);
	};

};

class cASEGGDF2Header {

private:

	std::vector<cAsciiColumnField> fields;

public:

	cASEGGDF2Header(const std::string& dfnpath){
		read(dfnpath);
	}

	bool read(const std::string& dfnpath){

		fields.clear();

		FILE* fp = fileopen(dfnpath, "r");
		std::string str;

		size_t startcolumn = 1;
		filegetline(fp, str);
		while (filegetline(fp, str)){

			std::vector<std::string> tokens = trimsplit(str, ';');
			if (strcasecmp(tokens[1], "end defn") == 0){
				break;
			}

			cAsciiColumnField F;

			int intorder;
			sscanf(tokens[0].c_str(), "DEFN %d ST = RECD, RT = ", &intorder);
			intorder--;
			F.order = (size_t)intorder;
			F.startcolumn = startcolumn;

			tokens = trimsplit(tokens[1], ':');
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
				std::string msg = _SRC_ + strprint("Unknown format string in ASEGGDF2 DFN File (%s)\n",formatstr.c_str());
				throw(std::runtime_error(msg));				
			}
			F.nbands = nbands;
			F.fmtwidth = width;
			F.fmtdecimals = decimals;
			startcolumn += F.nbands;

			if (tokens.size() > 2){
				std::string remainder = tokens[2];
				tokens = trimsplit(remainder, ',');
				for (size_t i = 0; i < tokens.size(); i++){
					std::vector<std::string> t = trimsplit(tokens[i], '=');
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
						if (i < tokens.size() - 1) 
							F.comment += tokens[i] + ", ";
						else{
							F.comment += tokens[i];
						}
							
					}
				}
			}
			fields.push_back(F);
		};
		fclose(fp);

		size_t k = 0;
		for (size_t i = 0; i < fields.size(); i++){
			fields[i].startchar = k;
			fields[i].endchar = k - 1 + fields[i].fmtwidth * fields[i].nbands;
			k = fields[i].endchar + 1;
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
		fprintf(fp, "DEFN %lu ST=RECD,RT=;END DEFN\n", fields.size() + 1);
		fclose(fp);
	};

	const std::vector<cAsciiColumnField>& getfields() const {
		return fields;
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
		size_t base = fields(findex).startcolumn - 1;
		v = atoi(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, double& v){
		size_t base = fields(findex).startcolumn - 1;
		v = atof(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, std::vector<int>& v){
		size_t base = fields(findex).startcolumn - 1;
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++){
			v[bi] = atoi(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfield(const size_t findex, std::vector<double>& v){
		size_t base = fields(findex).startcolumn - 1;
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++){
			v[bi] = atof(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfieldlog10(const size_t findex, std::vector<double>& v){
		size_t base = fields(findex).startcolumn - 1;
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

 