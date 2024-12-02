/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <cstring>
#include <vector>
#include <fstream>

#include "undefinedvalues.hpp"
#include "string_utils.hpp"
#include "general_utils.hpp"
#include "file_utils.hpp"


class cFmt {
public:
	char fmtchar = 0;
	size_t width = 0;
	size_t decimals = 0;

	cFmt(const char _fmtchar, const size_t _width, const size_t _decimals) {
		fmtchar = _fmtchar;
		width = _width;
		decimals = _decimals;
	}

	bool operator==(const cFmt& rhs) const {
		if (fmtchar == rhs.fmtchar && width == rhs.width && decimals == rhs.decimals) return true;
		else return false;
	}

	bool operator!=(const cFmt& rhs) const {
		if (fmtchar != rhs.fmtchar || width != rhs.width || decimals != rhs.decimals) return true;
		else return false;
	}
};

class cAsciiColumnFormat {

public:
	enum class Type { REAL, INTEGER, CHAR };

private:


public:

	inline const static std::string validfmttypes = "aAiIeEfF";
	char fmtchar = 'E';//Field type notation
	size_t width = 15;//Total width of field
	size_t decimals = 6;//Nuber of places after the decimal point	

	Type datatype() const {
		if (fmtchar == 'I' || fmtchar == 'i') {
			return Type::INTEGER;
		}
		else if (fmtchar == 'F' || fmtchar == 'f') {
			return Type::REAL;
		}
		else if (fmtchar == 'E' || fmtchar == 'e') {
			return Type::REAL;
		}
		if (fmtchar == 'A' || fmtchar == 'a') {
			return Type::CHAR;
		}
		else {
			std::string msg = strprint("Unknown data type <%c>\n%s\n", fmtchar, _SRC_.c_str());
			std::cerr << msg << std::endl;
		}
		return Type::REAL;
	};

	bool ischar() const {
		if (datatype() == Type::CHAR) return true;
		return false;
	};

	bool isinteger() const {
		if (datatype() == Type::INTEGER) return true;
		return false;
	};

	bool isreal() const {
		if (datatype() == Type::REAL) return true;
		return false;
	};

	bool valid_fmttype() const {
		return string_contains(validfmttypes, fmtchar);
	};

};

class cAsciiColumnField : public cAsciiColumnFormat {

private:

	static std::string space_free_field_name(const std::string& name) {
		std::string newname = name;
		for (size_t i = 0; i < newname.size(); i++) {
			if (newname[i] == ' ') {
				newname[i] = '_';
			}
		}
		return newname;
	};

public:
	static constexpr const char* NULLSTR = "NULL";
	static constexpr const char* UNITS = "UNITS";
	static constexpr const char* DATUM = "DATUM";
	static constexpr const char* PROJECTION = "PROJECTION";
	static constexpr const char* LONGNAME = "LONGNAME";
	static constexpr const char* DESC = "DESC";


	std::string name;//String used as the shorthand name of the field
	size_t fileorder = 0;//Zero based order of field in the file
	size_t nbands = 0;//Number of bands in the field
	size_t startcolumn = 0;//Zero based column index
	size_t startchar = 0;//Zero based character index	

	cKeyVecCiStr atts;//Additional key=value pairs
	using KeyVal = std::pair<std::string, std::string>;

	cAsciiColumnField() {};

	cAsciiColumnField(const size_t _order, const size_t _startcolumn, const std::string _name, const char _fmttype, const size_t _fmtwidth, const size_t _fmtdecimals, const size_t _nbands = 1) {
		fileorder = _order;
		startcolumn = _startcolumn;
		name = _name;
		fmtchar = _fmttype;
		width = _fmtwidth;
		decimals = _fmtdecimals;
		nbands = _nbands;
	};

	int startcol() const //Zero based start column index
	{
		return (int)startcolumn;
	};

	int endcol() const //Zero based end column index
	{
		return (int)(startcolumn + nbands - 1);
	};

	int endchar() const
	{
		return (int)(startchar + (nbands * width) - 1);
	};

	bool add_att(const std::string& key, const std::string& value) {
		return atts.add(key, value);
	};

	bool has_att(const std::string& key) const {
		if (atts.keyindex(key) < 0) {
			return false;
		}
		else return true;
	};

	std::string get_att(const std::string& key) const {
		std::string val;
		atts.get(key, val);
		return val;
	};

	bool hasnullvalue() const {
		return has_att(NULLSTR);
	};

	template <typename T>
	T nullvalue() const {
		std::stringstream iss(nullstring());
		T val = 0;
		iss >> val;
		return val;
	}

	std::string longname() const {
		return get_att(LONGNAME);
	};

	std::string units() const {
		return get_att(UNITS);
	};

	std::string description() const {
		return get_att(DESC);
	};

	std::string nullstring() const {
		return get_att(NULLSTR);
	}

	std::string fmtstr_single() const {
		char fmt = (char)toupper(fmtchar);
		std::ostringstream s;
		s << fmt;
		s << width;
		if (fmt != 'I') {
			s << "." << decimals;
		}
		return s.str();
	}

	std::string fmtstr() const {
		std::ostringstream s;
		if (nbands > 1) s << nbands;
		s << fmtstr_single();
		return s.str();
	}

	bool parse_format_string(const std::string& formatstr) {

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
		else if (std::sscanf(formatstr.c_str(), "%d%c%d", &inbands, &fmtchar, &iwidth) == 3) {
			nbands = inbands;
			width = iwidth;
			decimals = 0;
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

	std::string simple_header_record() const {
		std::string fixed_name = space_free_field_name(name);
		std::ostringstream s;
		if (nbands == 1) {
			s << startcolumn + 1 << "\t" << fixed_name << std::endl;
		}
		else {
			s << startcolumn + 1 << "-" << endcol() + 1 << "\t" << fixed_name << std::endl;
		}
		return s.str();
	}

	std::string aseggdf_header_record() const {

		std::string fixed_name = space_free_field_name(name);

		std::ostringstream s;
		s << "DEFN " << fileorder + 1 << " ST=RECD,RT=; " << fixed_name << " : ";
		s << fmtstr();

		if (atts.size() > 0) {
			s << " :";
			int k = 0;
			for (const auto& [key, value] : atts) {
				if (value.size() > 0) {
					if (k > 0) s << ',';
					s << " " << key << "=" << value;
					k++;
				}
			}
		}
		s << std::endl;
		return s.str();
	}

	std::string i3_header_record() const {
		// https://my.seequent.com/support/search/help/oasismontaj--content_ss_import_data_import_file_types_template_files_i3_import_template_file.htm?page=0&types=&product=&keyword=i3&kbtypes=&language=en_US&name=I3%20Import%20Template%20File
		std::string fixed_name = space_free_field_name(name);

		double scale = 1.0;
		double base = 0.0;
		std::string dummystr = "";

		std::ostringstream channelname;
		channelname << fixed_name;
		if (nbands > 1) channelname << "{" << nbands << "}";

		std::string typestr;
		std::string readformat;
		if (std::tolower(fmtchar) == 'i') {
			typestr = "LONG";
			readformat = "NORMAL";
		}
		else if (std::tolower(fmtchar) == 'f') {
			typestr = "DOUBLE";
			readformat = "NORMAL";
		}
		else if (std::tolower(fmtchar) == 'e') {
			typestr = "DOUBLE";
			readformat = "EXP";
		}

		bool writechan = true;
		std::string tag = "DATA";
		std::ostringstream s;
		if (ciequal(fixed_name, "line") ||
			ciequal(fixed_name, "linenumber") ||
			ciequal(fixed_name, "line_number") ||
			ciequal(fixed_name, "fltline")) {
			tag = "LINENUMBER";
			writechan = false;
		}

		std::ostringstream registry;
		registry << "Label=" << fixed_name << ';';
		for (const auto& [key, value] : atts) {
			if (value.size() > 0) registry << key << "=" << value << ';';
		}

		s << tag << "\t" << startchar << ", " << width << ", " << readformat << ", " << scale << ", " << base << ", " << dummystr << ", " << std::endl;
		if (writechan) {
			//s << "CHAN\t" << channelname.str() << ", " << typestr << ", " << readformat << ", " << width << ", " << decimals << ", " << "LABEL=" << fixed_name << std::endl;
			s << "CHAN\t" << channelname.str() << ", " << typestr << ", " << readformat << ", " << width << ", " << decimals << ", " << registry.str() << std::endl;
		}
		return s.str();
	}

	std::string str() {
		char sep = ':';
		std::ostringstream oss;
		oss << "name=" << name << sep;
		oss << " insert_order=" << fileorder << sep;
		oss << " bands=" << nbands << sep;
		oss << " startcol=" << startcolumn << sep;
		oss << " startchar=" << startchar << sep;
		oss << " endchar=" << startchar << sep;
		oss << " type=" << fmtchar << sep;
		oss << " width=" << width << sep;
		oss << " decimals=" << decimals << sep;

		for (const auto& [key, value] : atts) {
			oss << key << "=" << value << sep;
		}
		return oss.str();
	};

	bool isnull(const double v) const {
		if (hasnullvalue()) {
			if (v == nullvalue<double>()) {
				return true;
			}
		}
		return false;
	}

	bool set_variant_type(cVrnt& vrnt) const {
		if (isreal()) {
			if (nbands == 1) {
				vrnt = (double)0.0;
			}
			else {
				std::vector<double> vec;
				vrnt = vec;
			}
		}
		else if (isinteger()) {
			if (nbands == 1) {
				vrnt = (int)0;
			}
			else {
				std::vector<int> vec;
				vrnt = vec;
			}
		}
		else if (ischar()) {
			if (nbands == 1) {
				vrnt = (char)0;
			}
			else {
				std::vector<char> vec;
				vrnt = vec;
			}
		}
		return true;
	}
};

class cOutputFileInfo {

	size_t lastfield = 0;
	size_t lastcolumn = 0;
	bool   allowmorefields = true;

public:

	std::vector<cAsciiColumnField> fields;

	cOutputFileInfo() {};

	void lockfields() {
		allowmorefields = false;
	}

	void addfield(const std::string& _name, const char& _form, const size_t _width, const size_t& _decimals, const size_t& _nbands = 1) {
		//Stop compiler whinging
		addfield_impl(_name, _form, (int)_width, (int)_decimals, (int)_nbands);
	}

	void addfield(const cAsciiColumnField& c) {
		//Stop compiler whinging
		addfield_impl(c);
	}

	void addfield_impl(const std::string& _name, const char& _form, const int _width, const int& _decimals, const int& _nbands = 1) {
		if (allowmorefields) {
			cAsciiColumnField cf(lastfield, lastcolumn, _name, _form, _width, _decimals, _nbands);
			fields.push_back(cf);
			lastfield++;
			lastcolumn += _nbands;
		}
	}

	void addfield_impl(const cAsciiColumnField& c) {
		if (allowmorefields) {
			fields.push_back(c);
			lastfield++;
			lastcolumn += c.nbands;
		}
	}

	void add_att(const std::string& key, const std::string& value) {
		if (allowmorefields) {
			fields[lastfield - 1].add_att(key, value);
		}
	}

	void setunits(const std::string& units) {
		add_att(cAsciiColumnField::UNITS, units);
	}

	void setnullvalue(const std::string& nullstr) {
		add_att(cAsciiColumnField::NULLSTR, nullstr);
	}

	void setdescription(const std::string& description) {
		add_att(cAsciiColumnField::DESC, description);
	}

	cKeyVecCiStr collect_all_att_names() {
		cKeyVecCiStr v;
		for (size_t i = 0; i < fields.size(); i++) {
			cAsciiColumnField& f = fields[i];
			for (const auto& [key, value] : f.atts) {
				v.add(key, std::string());
			}
		}
		return v;
	}

	void write_simple_header(const std::string pathname) {
		std::ofstream ofs = ofstream_ex(pathname);
		for (size_t i = 0; i < fields.size(); i++) {
			std::string s = fields[i].simple_header_record();
			ofs << strprint("%s", s.c_str());
		}
	};

	void write_csv_header(const std::string pathname) {

		cKeyVecCiStr v = collect_all_att_names();

		std::ofstream ofs(pathname);

		ofs << "Name,Bands,Format";
		for (size_t j = 0; j < v.size(); j++) {
			ofs << "," << v[j].first;
		}
		ofs << std::endl;

		for (size_t i = 0; i < fields.size(); i++) {
			const cAsciiColumnField& f = fields[i];
			ofs << f.name << ",";
			ofs << f.nbands << ",";
			ofs << f.fmtstr_single();
			for (size_t j = 0; j < v.size(); j++) {
				std::string& key = v[j].first;
				std::string val = f.get_att(key);
				ofs << ",";
				if (val.size() > 0) ofs << val;
			}
			ofs << std::endl;
		}
	};

	void write_PAi3_header(const std::string pathname) {
		std::ofstream ofs = ofstream_ex(pathname);
		ofs << strprint("[IMPORT ARCHIVE]\n");
		ofs << strprint("FILEHEADER\t1\n");
		ofs << strprint("RECORDFORM\tFIXED\n");
		ofs << strprint("SKIPSTRING\t\"/\"\n");
		for (size_t i = 0; i < fields.size(); i++) {
			std::string s = fields[i].i3_header_record();
			ofs << strprint("%s", s.c_str());
		}
	};

	void write_aseggdf_header(const std::string pathname) {
		std::ofstream ofs = ofstream_ex(pathname);
		ofs << strprint("DEFN   ST=RECD,RT=COMM;RT:A4;COMMENTS:A76\n");
		for (size_t i = 0; i < fields.size(); i++) {
			std::string s = fields[i].aseggdf_header_record();
			ofs << strprint("%s", s.c_str());
		}
		ofs << strprint("DEFN %zu ST=RECD,RT=;END DEFN\n", fields.size() + 1);
	};

};

inline int field_index_by_name_impl(const std::vector<cAsciiColumnField>& fields, const std::string& fieldname) {
	int index = -1;
	for (int fi = 0; fi < fields.size(); fi++) {
		if (ciequal(fields[fi].name, fieldname)) {
			index = fi;
			break;
		}
	}
	return index;
}

class cHDRHeader {

private:
	bool valid = false;
	std::vector<cAsciiColumnField> fields;

public:

	cHDRHeader(const std::string& hdrpath) {
		valid = read(hdrpath);
	};

	const bool& isvalid() const {
		return valid;
	}

	int field_index_by_name(const std::string& fieldname) const {
		int index = field_index_by_name_impl(fields, fieldname);
		if (isvalid()) return index;
		else return -1;
	};

	cRange<int> column_range(const int& fieldindex) const {
		cRange<int> r(-1, -1);
		if (fieldindex >= 0 && fieldindex < fields.size()) {
			r.from = (int)fields[fieldindex].startcol();
			r.to = (int)fields[fieldindex].endcol();
		}
		return r;
	};

	cRange<int> column_range_by_name(const std::string& fieldname) const {
		int fieldindex = field_index_by_name(fieldname);
		return column_range(fieldindex);
	};

	const std::vector<cAsciiColumnField>& getfields() const {
		return fields;
	};

	static bool is_of_format(const std::string& filepath) {
		std::vector<cAsciiColumnField> fvec;
		bool status = read_static(filepath, fvec);
		return status;
	};

	bool read(const std::string& hdrfile) {
		std::vector<cAsciiColumnField> _fields;
		bool status = read_static(hdrfile, _fields);
		if (status) {
			fields = _fields;
		}
		return status;
	};

	static bool read_static(const std::string& hdrfile, std::vector<cAsciiColumnField>& fvec) {
		fvec.clear();

		std::ifstream file = fileopen(hdrfile);
		int k = 0;
		while (!file.eof()) {
			std::string cstr;
			std::string fstr;
			file >> cstr;
			file >> fstr;
			if (cstr.size() == 0) {
				break;
			}

			for (size_t i = 0; i < cstr.size(); i++) {
				if (std::isdigit(cstr[i]) == false && cstr[i] != '-') {
					return false;
				}
			}

			std::istringstream is(cstr);
			int col1 = -1;
			int col2 = -1;
			is >> col1;
			col2 = col1;
			if (!is.eof()) {
				is >> col2;
				col2 = -col2;
			}

			const size_t order = k;
			const size_t startcolumn = (size_t)col1 - 1;//Zero based index
			const int nbands = col2 - col1 + 1;
			const std::string name = fstr;
			const char fmttype = 0;
			const int fmtwidth = 0;
			const int fmtdecimals = 0;

			cAsciiColumnField f(order, startcolumn, name, fmttype, fmtwidth, fmtdecimals, nbands);
			fvec.push_back(f);
			k = k + 1;
		}
		return true;
	};
};

class cASEGGDF2Header {

private:

	bool valid = false;
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
			bool status = F2.parse_format_string(t[1]);
			return status;
		}
		return false;
	}

public:

	cASEGGDF2Header(const fs::path& dfnpath) {
		valid = read(dfnpath);
	};

	const bool& isvalid() const {
		return valid;
	};

	int field_index_by_name(const std::string& fieldname) const {
		if (isvalid()) return field_index_by_name_impl(fields, fieldname);
		else return -1;
	};

	cRange<int> column_range(const int& fieldindex) const {
		cRange<int> r(-1, -1);
		if (fieldindex >= 0 && fieldindex < fields.size()) {
			r.from = (int)fields[fieldindex].startcol();
			r.to = (int)fields[fieldindex].endcol();
		}
		return r;//invalid;
	};

	cRange<int> column_range_by_name(const std::string& fieldname) const {
		int fieldindex = field_index_by_name(fieldname);
		return column_range(fieldindex);
	};

	static bool is_of_format(const std::string& filepath) {
		std::vector<cAsciiColumnField> _fields;
		std::string _ST_string;
		std::string _RT_string;
		bool status = read_static(filepath, _fields, _ST_string, _RT_string);
		return status;
	};

	bool read(const fs::path& dfnfile) {
		std::vector<cAsciiColumnField> _fields;
		std::string _ST_string;
		std::string _RT_string;
		valid = read_static(dfnfile, _fields, _ST_string, _RT_string);
		if (valid) {
			fields = _fields;
			ST_string = _ST_string;
			RT_string = _RT_string;
		}
		return valid;
	}

	static bool read_static(const fs::path& dfnfile, std::vector<cAsciiColumnField>& _fields, std::string& _ST_string, std::string& _RT_string) {
		bool status = false;
		_fields.clear();

		std::ifstream ifs = ifstream_ex(dfnfile);
		std::string dfnrecord;
		bool reported_mixing = false;
		bool reported_badincrement = false;
		bool reported_geosoft = false;

		size_t datarec = 0;
		int dfnlinenum = 0;
		int lastfileorder = -1;
		while (filegetline_ifs(ifs, dfnrecord)) {
			dfnlinenum++;
			//std::cout << str << std::endl;

			auto tk_space = tokenise(dfnrecord, ' ');
			auto tk_semicolon = tokenise(dfnrecord, ';');
			bool processrecord = true;

			//Check for 'END DEFN' 
			for (size_t i = 0; i < tk_semicolon.size(); i++) {
				if (ciequal(tk_semicolon[i], "end defn")) {
					if (i == 0) {
						processrecord = false;
					}
				}
			}

			if (strncasecmp(tk_space[0], "defn", 4) != 0) {
				std::string msg;
				msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
				msg += strprint("\tSkipping DFN entry that does not begin with 'DEFN' or 'END DEFN'\n");
				if (my_rank() == 0) std::cerr << msg << std::endl;
				processrecord = false;
			}

			if (processrecord) {
				cAsciiColumnField F;

				auto t1 = tokenise(tk_semicolon[0], ',');
				auto t2 = tokenise(t1[0], '=');
				auto t3 = tokenise(t1[1], '=');

				_ST_string = t2[1];
				std::string rt = t3[1];

				if (_ST_string != "RECD") {
					std::string msg;
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tThe key 'ST' should be 'ST=RECD,'\n");
					throw(std::runtime_error(msg));
				}

				if (rt == "COMM" || rt == "comm") {
					//don't warn about skipping comments
					continue;
				}

				if (rt != "" && rt != "DATA") {
					std::string msg;
					msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tSkipping DFN entry that does not have a record type 'RT=DATA;' or 'RT=;'\n");
					if (my_rank() == 0) std::cerr << msg << std::endl;
					continue;
				}

				//Detect and try to fix Geosoft style DFN				
				if (reported_geosoft == false && tk_semicolon.size() > 2) {
					bool geosoft = detect_geosoft_error(tk_semicolon[1], tk_semicolon[2]);
					if (geosoft) {
						auto tmp1 = tokenise(tk_semicolon[1], ':');
						auto tmp2 = tokenise(tk_semicolon[2], ':');
						std::string msg;
						msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
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
				//if (n == 0) {};
				F.fileorder = (size_t)order;
				if (lastfileorder == -1) {
					if (F.fileorder != 1 && F.fileorder != 0) {
						std::string msg;
						msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
						msg += strprint("\tDEFN number does not start at 0 or 1. Check recommended.\n");
						std::cerr << msg << std::endl;
					}
				}
				else {
					if (reported_badincrement == false) {
						if ((int)F.fileorder != (lastfileorder + 1)) {
							std::string msg;
							msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
							msg += strprint("\tDEFN numbers are not incrementing by 1. Check recommended.\n");
							std::cerr << msg << std::endl;
							reported_badincrement = true;
						}
					}
				}


				//Data DEFN
				auto colon_tokens = tokenise(tk_semicolon[1], ':');
				if (colon_tokens.size() < 1) {
					std::string msg;
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tNo name or format provided\n");
					throw(std::runtime_error(msg));
				}

				if (ciequal(colon_tokens[0], "end defn")) {
					//forget warning about this - it seems pretty common/standard
					//std::string msg;
					//msg += strprint("Warning: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					//msg += strprint("\t'END DEFN' is out of place\n");
					//std::cerr << msg << std::endl;
					continue;
				}

				F.name = colon_tokens[0];

				if (colon_tokens.size() < 2) {
					std::string msg;
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tNo format provided\n");
					throw(std::runtime_error(msg));
				}

				std::string formatstr = colon_tokens[1];
				F.parse_format_string(formatstr);
				if (F.valid_fmttype() == false) {
					std::string msg;
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tFormat %s must start with one of '%s'\n", formatstr.c_str(), F.validfmttypes.c_str());
					throw(std::runtime_error(msg));
				}

				if (F.nbands < 1 || F.width < 1 || F.decimals < 0) {
					std::string msg;
					msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
					msg += strprint("\tCould not decipher the format %s\n", formatstr.c_str());
					glog.errormsg(_SRC_, msg);
				}

				if (colon_tokens.size() > 3) {
					for (size_t k = 3; k < colon_tokens.size(); k++) {
						colon_tokens[2] += "," + colon_tokens[k];
					}
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
							std::string key = strprint("extra%d", nea);
							std::string& value = tk_comma[i];
							F.atts.add(key, value);
						}
						else if (tk_equal.size() == 2) {
							std::string key = tk_equal[0];
							const std::string& value = tk_equal[1];
							if (ciequal(key, "unit")) key = cAsciiColumnField::UNITS;
							if (ciequal(key, "units")) key = cAsciiColumnField::UNITS;
							if (ciequal(key, "description")) key = cAsciiColumnField::DESC;
							if (ciequal(key, "nullvalue")) key = cAsciiColumnField::NULLSTR;
							if (ciequal(key, "name")) key = cAsciiColumnField::LONGNAME;
							F.add_att(key, value);
						}
						else {
							std::string msg;
							msg += strprint("Error: Parsing line %d of DFN file %s\n\t%s\n", dfnlinenum, dfnfile.c_str(), dfnrecord.c_str());
							msg += strprint("\tunknown erroe\n");
							throw(std::runtime_error(msg));
						}
					}
				}

				if (datarec == 0) {
					_RT_string = rt;
				}
				else {
					if (reported_mixing == false && rt != _RT_string) {
						std::string msg = strprint("\tDetected mixing of RT=; and RT=DATA; ");
						msg += strprint("at line %d of DFN file %s. ", dfnlinenum, dfnfile.c_str());
						msg += strprint("Making all data record types RT=%s;.\n", _RT_string.c_str());
						std::cerr << msg << std::endl;
						reported_mixing = true;
					}
				}

				if (datarec == 0 && rt == "DATA") {
					if (!F.ischar() && F.width != 4) {
						cAsciiColumnField R;
						R.name = "RT";
						R.fileorder = (size_t)0;
						R.fmtchar = 'A';
						R.width = 4;
						R.decimals = 0;
						R.nbands = 1;
						_fields.push_back(R);
						datarec++;
					}
				}
				//F.print();
				lastfileorder = (int)F.fileorder;
				_fields.push_back(F);
				status = true;
				datarec++;
			}
		};
		std::cerr << std::flush;

		size_t startchar = 0;
		size_t startcolumn = 0;
		for (size_t i = 0; i < _fields.size(); i++) {
			_fields[i].startchar = startchar;
			startchar = _fields[i].endchar() + 1;

			_fields[i].startcolumn = startcolumn;
			startcolumn += _fields[i].nbands;
		}
		return status;
	};

	void write(const std::string& dfnpath) {
		std::ofstream ofs = ofstream_ex(dfnpath);
		ofs << strprint("DEFN   ST=RECD,RT=COMM;RT:A4;COMMENTS:A76\n");
		for (size_t i = 0; i < fields.size(); i++) {
			std::string s = fields[i].aseggdf_header_record();
			ofs << strprint("%s", s.c_str());
		}
		ofs << strprint("DEFN %zu ST=RECD,RT=;END DEFN\n", fields.size() + 1);
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


class cFieldManager {

private:

public:
	std::vector<cAsciiColumnField> fields;

	cFieldManager() {};

	cFieldManager(const std::vector<cAsciiColumnField> _fields) {
		fields = _fields;
	};

	int field_index_by_name(const std::string& fieldname) const {
		return field_index_by_name_impl(fields, fieldname);
	};

	size_t ncolumns() const {
		size_t n = 0;
		for (size_t i = 0; i < fields.size(); i++) {
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

	cColumnFile() { initialise(); };

	cColumnFile(const std::string& datafile, const std::string& headerfile) {
		initialise();
		openread(datafile);
		cASEGGDF2Header A(headerfile);
		F = cFieldManager(A.getfields());
	};

	~cColumnFile() {
		if (file.is_open())file.close();
	};

	void initialise() {
		recordsreadsuccessfully = 0;
	};

	const cAsciiColumnField& fields(const size_t fi) {
		return F.fields[fi];
	}

	size_t nfields() const {
		return F.fields.size();
	}

	size_t ncolumns() const {
		return F.ncolumns();
	}

	const char* currentrecord_cstr() { return currentrecord.c_str(); };

	const std::string& currentrecordstring() { return currentrecord; };

	bool openread(const std::string& datafilename) {
		std::string path = datafilename;
		fixseparator(path);
		file.open(path, std::fstream::in);
		if (file.is_open())return true;
		else {
			std::string msg = strprint("Could not open file (%s)\n", path.c_str());
			glog.errormsg(_SRC_, msg);
		}
		return false;
	};

	void close() { file.close(); };

	bool readnextrecord() {
		if (file.eof())return false;
		std::getline(file, currentrecord);
		recordsreadsuccessfully++;
		return true;
	}

	size_t parse_record() {
		currentcolumns = parsestrings(currentrecord, " ,\t\r\n");
		return currentcolumns.size();
	}

	bool getcolumn(const size_t columnnumber, int& v) {
		v = atoi(currentcolumns[columnnumber].c_str());
		return true;
	}

	bool getcolumn(const size_t columnnumber, double& v) {
		v = atof(currentcolumns[columnnumber].c_str());
		return true;
	}

	bool getfield(const size_t findex, int& v) {
		size_t base = fields(findex).startcol();
		v = atoi(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, double& v) {
		size_t base = fields(findex).startcol();
		v = atof(currentcolumns[base].c_str());
		return true;
	}

	bool getfield(const size_t findex, std::vector<int>& v) {
		size_t base = fields(findex).startcol();
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {
			v[bi] = atoi(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfield(const size_t findex, std::vector<double>& v) {
		size_t base = fields(findex).startcol();
		size_t nb = fields(findex).nbands;
		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {
			v[bi] = atof(currentcolumns[base].c_str());
			base++;
		}
		return true;
	}

	bool getfieldlog10(const size_t findex, std::vector<double>& v) {
		size_t base = fields(findex).startcol();
		size_t nb = fields(findex).nbands;

		v.resize(nb);
		for (size_t bi = 0; bi < nb; bi++) {
			v[bi] = atof(currentcolumns[base].c_str());
			if (fields(findex).isnull(v[bi]) == false) {
				v[bi] = log10(v[bi]);
			}
			base++;
		}
		return true;
	}

	size_t readnextgroup(const size_t fgroupindex, std::vector<std::vector<int>>& intfields, std::vector<std::vector<double>>& doublefields) {

		if (file.eof())return 0;

		intfields.clear();
		doublefields.clear();
		intfields.resize(nfields());
		doublefields.resize(nfields());

		int lastline;
		size_t count = 0;
		do {
			if (recordsreadsuccessfully == 0) readnextrecord();
			if (parse_record() != ncolumns()) {
				continue;
			}
			int line;
			getfield(fgroupindex, line);

			if (count == 0)lastline = line;

			if (line != lastline) {
				return count;
			}

			for (size_t fi = 0; fi < nfields(); fi++) {
				size_t nbands = fields(fi).nbands;
				if (fields(fi).datatype() == cAsciiColumnField::Type::INTEGER) {
					std::vector<int> v;
					getfield(fi, v);
					for (size_t bi = 0; bi < nbands; bi++) {
						intfields[fi].push_back(v[bi]);
					}
				}
				else {
					std::vector<double> v;
					getfield(fi, v);
					for (size_t bi = 0; bi < nbands; bi++) {
						doublefields[fi].push_back(v[bi]);
					}
				}
			}
			count++;
		} while (readnextrecord());
		return count;
	};
};

