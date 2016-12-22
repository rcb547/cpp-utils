/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _intrepid_H
#define _intrepid_H

#include <cmath>
#include <climits>
#include <cfloat>
#include <vector>
#include <list>

#include "file_utils.h"
#include "general_utils.h"
#include "geometry3d.h"
#include "stacktrace.h"

class IHeader;
class ILDataset;
class ILField;
class ILSegment;

struct SampleIndex{
	size_t lineindex;
	size_t sampleindex;
};

struct IndexTable{
	size_t start;
	size_t ns;
	size_t dummy1;
	size_t dummy2;
};

struct SortTable{
	double value;
	size_t sequence;
	struct IndexTable index;
};

struct SurveyInfoEntry{
	std::string identifer;
	std::string fieldname;
};

enum  edtype { dtBYTE, dtSHORT, dtINT, dtFLOAT, dtDOUBLE, dtUNKNOWN };

class IDatatype{

private:
	edtype type;

public:

	std::string name() const
	{
		_GSTITEM_
		if (type == dtBYTE) return "UnSigned8BitInteger";
		else if (type == dtSHORT) return "Signed16BitInteger";
		else if (type == dtINT) return "Signed32BitInteger";
		else if (type == dtFLOAT) return "IEEE4ByteReal";
		else if (type == dtDOUBLE) return "IEEE8ByteReal";
		else return "UNKNOWN";
	}

	static unsigned char bytenull() { return 0; }
	static int16_t  shortnull() { return -32767; }
	static int    intnull() { return -2147483647; }	
	static float  floatnull() { return -3.4E+38f; }
	static double doublenull() { return -5.0E+75; }

	IDatatype(){
		_GSTITEM_
		type = dtUNKNOWN;
	}

	IDatatype(const edtype t){
		_GSTITEM_
		type = t;
	}

	edtype etype(){ return type; }
	size_t size()
	{
		_GSTITEM_
		if (type == dtBYTE) return 1;
		else if (type == dtSHORT) return 2;
		else if (type == dtINT) return 4;
		else if (type == dtFLOAT) return 4;
		else if (type == dtDOUBLE) return 8;
		else {
			message("IDatatype::size() Unknown dadatype\n");
			return 0;
		}
	}

	bool isbyte() const { if (type == dtBYTE) return true; else return false; }
	bool isshort() const { if (type == dtSHORT) return true; else return false; }
	bool isint() const { if (type == dtINT) return true; else return false; }
	bool isfloat() const { if (type == dtFLOAT) return true; else return false; }
	bool isdouble() const { if (type == dtDOUBLE) return true; else return false;}

	double nullasdouble() const {
		_GSTITEM_
		switch (type) {
			case dtBYTE: return (double) bytenull(); break;
			case dtSHORT: return (double) shortnull(); break;
			case dtINT: return (double)intnull(); break;
			case dtFLOAT: return (double)floatnull(); break;
			case dtDOUBLE: return (double)doublenull(); break;
			default: return doublenull();
		}
	}

	static bool isnull(unsigned char number)
	{
		_GSTITEM_
		if (number == bytenull()) return true;
		else return false;
	}	

	static bool isnull(const float& number)
	{
		_GSTITEM_
		if (number == floatnull()) return true;
		else if (std::isfinite<float>(number) == false){
			return true;
		}		
		return false;
	}

	static bool isnull(const double& number)
	{
		_GSTITEM_
		if (number == doublenull()) return true;
		else if (std::isfinite<double>(number) == false){
			return true;
		}		
		return false;
	}

	static bool isnull(int16_t number)
	{
		_GSTITEM_
		if (number == shortnull()) return true;
		return false;
	}

	static bool isnull(int number)
	{
		_GSTITEM_
		if (number == intnull()) return true;
		return false;
	}	

};

template<typename T>
class IData{
	
private:
	std::vector<T> buffer;
	
	size_t ns;
	size_t nb;
	size_t nelements;
	bool groupby;
	
	std::vector<T> _gbbuf;
	std::vector<T> groupbybuffer(){
		std::vector<T> v(ns*nb);
		for (size_t bi = 0; bi < nb; bi++){
			for (size_t si = 0; si < ns; si++){
				v[bi*ns + si] = buffer[bi];
			}
		}
		return v;
	}

public:
	
	IData(){
		ns = 0;
		nb = 0;
		nelements = 0;
		groupby = false;
	}

	void resize(size_t _ns, size_t _nb = 1, bool _groupby = false)
	{		
		ns = _ns;
		nb = _nb;
		groupby = _groupby;
		
		if (groupby){
			nelements = nb;			
		}
		else{
			nelements = ns*nb;
		}
		buffer.resize(nelements,0);
	}
	
	T& operator()(size_t s, size_t b){
		if (groupby){
			return buffer[b];
		}
		else{
			//return buffer[b*ns + s];
			return buffer[s*nb + b];
		}
	}

	void* pvoid(){ return (void*)buffer.data(); }
	char* pchar(){ return (char*)buffer.data(); }
	void  swap_endian(){
		::swap_endian(buffer);
	}

	void* pvoid_groupby(){
		_gbbuf = groupbybuffer();
		return (void*)_gbbuf.data();		
	}
	

};

size_t nullindex(){
	return SIZE_MAX;
}

enum eatype  { atDIRECT, atINDEXED };
enum eftype  { ftLINE, ftINDEX, ftPOINT, ftPOLYGON, ftIMAGE};
enum ebptype { bptBSQ, bptBIL, bptBIP };

class IHeader{
	
public:
	bool valid;
	eftype filetype;
	eatype accesstype;
	ebptype bandpackingtype;
	size_t nlines;
	size_t maxspl;
	size_t nbands;
	IDatatype datatype;
	size_t headeroffset;
	bool endianswap;
	std::string indexname;
	
	IHeader(){
		valid = false;
	};

	IHeader(char* p){
		valid = parse(p);
	}

	bool parse(char* p)
	{		 
		int16_t* s = (int16_t*)p;

		//printf("sizeof(int)  = %lu\n",sizeof(int));
		//printf("sizeof(long) = %lu\n",sizeof(long));
		//printf("s[91]=%d\n",(int)s[91]);
		//printf("s[72]=%d\n",(int)s[72]);
		//printf("s[73]=%d\n",(int)s[73]);
		//printf("s[90]=%d\n",(int)s[90]);		
		//printf("d s[217]=%d\n",*(int32_t*)&s[217]);
		//printf("s[219]=%d\n",*(int32_t*)&s[219]);
		//printf("s[221]=%d\n", *(int32_t*)&s[221]);

		if (s[91] != 1 && s[91] != 2){
			endian_swap(p);
			//printf("swapped\n");
			if (s[91] != 1 && s[91] != 2){
				return false;
			}
			else endianswap = true;
		}
		else endianswap = false;

		switch (s[72]) {
			case 1000: filetype = ftLINE; break;
			case 1001: filetype = ftIMAGE; printf("filetype ftIMAGE is not supported"); return false;
			case 1002: filetype = ftINDEX; break;
			case 1004: filetype = ftPOLYGON; printf("filetype ftPOLYGON is not supported"); return false;
			case 1006: filetype = ftPOINT; printf("filetype ftPOINT is not supported"); return false;
			default: printf("Bad file type"); return false;
		}

		int16_t dt = s[73];
		int16_t ds = s[90];
		if (dt == 1 && ds == 8)	datatype = IDatatype(dtBYTE);		
		else if (dt == 2 && ds == 16) datatype = IDatatype(dtSHORT);
		else if (dt == 2 && ds == 32) datatype = IDatatype(dtINT);
		else if (dt == 3 && ds == 32) datatype = IDatatype(dtFLOAT);
		else if (dt == 3 && ds == 64) datatype = IDatatype(dtDOUBLE);
		else return false;
		
		if (filetype==ftINDEX){
			nlines = (size_t)*((int32_t*)&(s[217]));
			maxspl = (size_t)*((int32_t*)&(s[219]));
			nbands = (size_t)*((int32_t*)&(s[221]));
		}
		else if (filetype == ftLINE){
			maxspl = (size_t)*((int32_t*)&(s[217]));
			nlines = (size_t)*((int32_t*)&(s[219]));
			nbands = (size_t)*((int32_t*)&(s[221]));
		}
		
		//printf("nlines = %lu\n",nlines);
		//printf("maxspl = %lu\n",maxspl);
		//printf("nbands = %lu\n",nbands);
		//prompttocontinue();

		headeroffset = 512 * (size_t)s[81];
		
		switch (s[91]) {
			case 1: accesstype = atDIRECT; break;
			case 2: accesstype = atINDEXED; break;
			default:
				printf("Bad access type");
				return false;
		}

		bandpackingtype = bptBIL;
		if (nbands > 1){
			switch (s[78]) {
				case 0: bandpackingtype = bptBSQ; break;
				case 1: bandpackingtype = bptBIL; break;
				case 2: bandpackingtype = bptBIP; break;
				default:
					if (nbands == 1){
						bandpackingtype = bptBSQ;
					}
					else{
						printf("Bad band packing type");
						return false;
					}
				}
		}	

		char tmp[26];
		strncpy(tmp,(char*)&s[171],25);
		tmp[25] = 0;
		indexname = std::string(tmp);
		
		return true;
	};

	void endian_swap(void* p)
	{
		int16_t* s = (int16_t*)p;
		swap_endian((int32_t*)&s[10], 1);
		swap_endian((int32_t*)&s[12], 1);
		swap_endian((int16_t*)&s[32], 1);
		swap_endian((int16_t*)&s[33], 1);
		swap_endian((int16_t*)&s[34], 1);
		swap_endian((int16_t*)&s[72], 1);
		swap_endian((int16_t*)&s[73], 1);
		swap_endian((int16_t*)&s[78], 1);
		swap_endian((int16_t*)&s[79], 1);
		swap_endian((int16_t*)&s[81], 1);
		swap_endian((int16_t*)&s[87], 1);
		swap_endian((int16_t*)&s[89], 1);
		swap_endian((int16_t*)&s[90], 1);
		swap_endian((int16_t*)&s[91], 1);
		swap_endian((int16_t*)&s[92], 1);
		swap_endian((int16_t*)&s[93], 1);
		swap_endian((int32_t*)&s[115], 1);
		swap_endian((int32_t*)&s[117], 1);
		swap_endian((double*)&s[119], 1);
		swap_endian((double*)&s[123], 1);
		swap_endian((double*)&s[131], 1);
		swap_endian((double*)&s[135], 1);
		swap_endian((double*)&s[143], 1);
		swap_endian((double*)&s[147], 1);
		swap_endian((int16_t*)&s[188], 1);
		swap_endian((int16_t*)&s[189], 1);
		swap_endian((int32_t*)&s[217], 1);
		swap_endian((int32_t*)&s[219], 1);
		swap_endian((int32_t*)&s[221], 1);
	};
	static size_t nbytes() { return 512; }
};

class ILSegment{

private:
	ILField*  pField;	
	size_t lineindex;

public:
			
	IData<float> fdata;
	IData<double> ddata;
	IData<int32_t> idata;
	IData<int16_t> sdata;
	IData<char> cdata;		
	
	bool readbuffer();
	bool writebuffer();

	ILDataset* pDataset();	
	size_t startindex();
	size_t nsamples();
	size_t nbands();
	IDatatype datatype();
	FILE* filepointer();

	ILSegment()
	{
		pField = (ILField*)NULL;	
		lineindex = 0;		
	}

	void initialise(ILField*  _pField, size_t _lineindex)
	{
		pField     = _pField;	
		lineindex  = _lineindex;		
	}
	
	void createbuffer()
	{
		size_t ns = nsamples();
		size_t nb = nbands();
		bool groupby = isgroupbyline();

		switch (datatype().etype()){
		case dtFLOAT: fdata.resize(ns,nb,groupby); break;
		case dtDOUBLE: ddata.resize(ns, nb, groupby); break;
		case dtSHORT: sdata.resize(ns, nb, groupby); break;
		case dtINT: idata.resize(ns, nb, groupby); break;
		case dtBYTE: cdata.resize(ns, nb, groupby); break;
		default: printf("ILSegment::createbuffer() Unknown type");
		}
	}

	void* pvoid()
	{
		switch (datatype().etype()){
		case dtFLOAT: return fdata.pvoid(); break;
		case dtDOUBLE: return ddata.pvoid(); break;
		case dtSHORT: return sdata.pvoid(); break;
		case dtINT: return idata.pvoid(); break;
		case dtBYTE: return cdata.pvoid(); break;
		default: printf("ILSegment::pvoid() Unknown type"); return (void*)NULL;
		}
	}

	void* pvoid_groupby()
	{		
		switch (datatype().etype()){
		case dtFLOAT: return fdata.pvoid_groupby(); break;
		case dtDOUBLE: return ddata.pvoid_groupby(); break;
		case dtSHORT: return sdata.pvoid_groupby(); break;
		case dtINT: return idata.pvoid_groupby(); break;
		case dtBYTE: return cdata.pvoid_groupby(); break;
		default: printf("ILSegment::pvoid_groupby() Unknown type"); return (void*)NULL;
		}
	}

	double d(size_t s, size_t b = 0){		
		
		switch (datatype().etype()){

			case dtFLOAT:{
				if (IDatatype::isnull(fdata(s, b))) return IDatatype::doublenull();
				else return (double)fdata(s, b);
			}

			case dtDOUBLE:{
				return ddata(s, b);
			}

			case dtSHORT:{
				if (IDatatype::isnull(sdata(s, b))) return IDatatype::doublenull();
				else return (double)sdata(s, b);
			}

			case dtINT:{
				if (IDatatype::isnull(idata(s, b))) return IDatatype::doublenull();
				else return (double)idata(s, b);
			}

			case dtBYTE:{
				if (IDatatype::isnull(cdata(s, b))) return IDatatype::doublenull();
				else return (double)cdata(s, b);
			}

			default:{
				printf("ILSegment::d() Unknown type"); return false;
			}
		}
	}

	float f(size_t s, size_t b = 0)
	{
		switch (datatype().etype()){
		case dtFLOAT: return fdata(s, b); break;
		case dtDOUBLE: return (float)ddata(s, b); break;
		case dtSHORT: return (float)sdata(s, b); break;
		case dtINT: return (float)idata(s, b); break;
		case dtBYTE: return (float)cdata(s, b); break;
		default: printf("ILSegment::f() Unknown type"); return false;
		}
	}

	int32_t i(size_t s, size_t b = 0)
	{
		switch (datatype().etype()){
		case dtFLOAT: return (int32_t)fdata(s, b); break;
		case dtDOUBLE: return (int32_t)ddata(s, b); break;
		case dtSHORT: return (int32_t)sdata(s, b); break;
		case dtINT: return idata(s, b); break;
		case dtBYTE: return (int32_t)cdata(s, b); break;
		default: printf("ILSegment::i() Unknown type"); return false;
		}
	}

	int16_t s(size_t s, size_t b = 0)
	{
		switch (datatype().etype()){
		case dtFLOAT: return (int16_t)fdata(s, b); break;
		case dtDOUBLE: return (int16_t)ddata(s, b); break;
		case dtSHORT: return sdata(s, b); break;
		case dtINT: return (int16_t)idata(s, b); break;
		case dtBYTE: return (int16_t)cdata(s, b); break;
		default: printf("ILSegment::i() Unknown type"); return false;
		}
	}

	
	template <typename T>
	bool getband(std::vector<T>& v, size_t band=0)
	{
		size_t ns = nsamples();
		if (isgroupbyline()) ns = 1;		
		v.resize(ns);
		
		switch (datatype().etype()){
		case dtFLOAT: 			
			for (size_t i = 0; i< ns; i++){				
				v[i] = (T)fdata(i, band);
			}		
			return true; break;
		case dtDOUBLE: 
			for (size_t i = 0; i< ns; i++){				
				v[i] = (T)ddata(i, band);
			}
			return true; break;			
		case dtSHORT:
			for (size_t i = 0; i< ns; i++){				
				v[i] = (T)sdata(i, band);					
			}
			return true; break;
		case dtINT:
			for (size_t i = 0; i< ns; i++){				
				v[i] = (T)idata(i, band);
			}
			return true; break;
		case dtBYTE:
			for (size_t i = 0; i< ns; i++){				
				v[i] = (T)cdata(i, band);
			}			
			return true; break;
		default: printf("ILSegment::i() Unknown type"); return false;
		}
	}

	size_t nbands() const;
	size_t nsamples() const;
	size_t startindex() const;
	size_t nstored(){
		if (isindexed())return nsamples();
		else return 1;
	}
	size_t nelements(){
		return nstored()*nbands();
	}
	size_t nbytes(){
		return nelements() * datatype().size();
	}

	bool isgroupbyline();

	bool isindexed();

	long fileposition()
	{
		if (isgroupbyline()){
			long p = (long)(IHeader::nbytes() + lineindex * nbands() * datatype().size());
			return p;
		}
		else{
			long p = (long)(IHeader::nbytes() + startindex() * nbands() * datatype().size());
			return p;
		}
	}
	
	void change_nullvalue(const float& newnullvalue){
		_GSTITEM_
		if (datatype().isfloat()){
			float* fp = (float*)pvoid();
			for (size_t k = 0; k < nstored(); k++){
				if (datatype().isnull(fp[k])){
					fp[k] = newnullvalue;
				}
			}
		}
	}

	void change_nullvalue(const double& newnullvalue){

		if (datatype().isdouble()){
			double* fp = (double*)pvoid();
			for (size_t k = 0; k < nstored(); k++){
				if (datatype().isnull(fp[k])){
					fp[k] = newnullvalue;
				}
			}
		}

	}

	/*
	template<typename T>
	bool write(const std::vector<std::vector<T>>& array)
	{
		switch (datatype().etype()){
		case dtFLOAT: array2ptr((float*)buffer.data(), array); break;
		case dtDOUBLE: ptr2array((double*)buffer.data(), array); break;
		case dtSHORT: ptr2array((int16_t*)buffer.data(), array); break;
		case dtINT: ptr2array((int32_t*)buffer.data(), array); break;
		case dtBYTE: ptr2array((char*)buffer.data(), array); break;
		default: printf("ILSegment::write() Unknown type"); return false;
		}
		std::vector<char> buffer = array2ptr(array);
		return writebuffer();
	}*/

};

class ILField{

private:		
	IHeader Header;
	FILE* pFile;			

public:	
	ILDataset*  pDataset;
	std::string Name;
		
	std::vector<ILSegment> Segments;
	IDatatype datatype() const { return Header.datatype; }
	size_t nbands(){ return Header.nbands; };
	size_t nlines() const;			
	FILE* filepointer() { return pFile; }	
	bool endianswap() const { return Header.endianswap; }
	
	bool isgroupbyline() const
	{
		if (Header.accesstype == atDIRECT) return true; 			
		else return false;
	}
	bool isindexed() const
	{
		if (Header.accesstype == atINDEXED) return true;
		else return false;
	}
	
	ILField()
	{
		pFile     = (FILE*)NULL;
		pDataset  = (ILDataset*)NULL;		
	}

	ILField(ILDataset *_pDataset, const std::string& fieldname)
	{				
		initialise(_pDataset, fieldname);
	}

	~ILField()
	{
		close();
	}

	bool isvalid()
	{
		if (datafilepath().size() > 0)return true;
		return false;
	}

	bool initialise(ILDataset *_pDataset, const std::string& fieldname);	
	bool createnew(ILDataset *_pDataset, const std::string& fieldname, IDatatype _datatype, size_t _nbands, bool indexed=true);	
	bool open()
	{
		if (pFile != (FILE*)NULL)return true;
		if ((pFile = fileopen(datafilepath(), "rb")) == NULL) {
			message("ILField::open() cannot open file: %s\n\n", datafilepath().c_str());
			return false;
		}

		std::vector<char> buffer(IHeader::nbytes());
		fread(buffer.data(), IHeader::nbytes(), 1, pFile);
		Header = IHeader(buffer.data());
		if (Header.valid==false){
			printf("Could not read header in file: %s\n\n", datafilepath().c_str());
			fclose(pFile);
			pFile = (FILE*)NULL;
			return false;
		}		
		return true;
	}	
	bool initialisesegments()
	{		
		if (Segments.size() > 0) return true;
		Segments.resize(nlines());
		for (size_t li = 0; li < nlines(); li++){
			Segments[li].initialise(this,li);
		}
		return true;
	}	
	void close()
	{
		if (pFile){
			fclose(pFile);
		}
		pFile = (FILE*)NULL;
	}
	bool erase()
	{
		bool status = true;
		close();
		if (remove(datafilepath().c_str()) != 0){
			if (errno == EACCES)status = false;
		}

		if (remove(dotvecfilepath().c_str()) != 0){
			if (errno == EACCES)status = false;
		}

		if (remove(dotdotlinefilepath().c_str()) != 0){
			if (errno == EACCES)status = false;
		}
		return status;
	}

	std::string infostring(){
		std::string s;
		s += strprint(Name.c_str());
		s += strprint(" Type=%s ", Header.datatype.name().c_str());
		s += strprint(" Bands=%lu ", Header.nbands);
		if (isgroupbyline())s += strprint(" GroupBy ");
		if (isindexed())s += strprint(" Indexed ");
		s += strprint("\n");
		return s;
	}

	void printinfo()
	{
		std::string s = infostring();
		printf(s.c_str());
	}
	
	std::string datasetpath();
	
	std::string datafilepath()
	{
		return datasetpath() + Name + ".PD";		
	}

	std::string dotvecfilepath()
	{
		return datasetpath() + Name + ".PD.vec";		
	}

	std::string dotdotlinefilepath()
	{
		return datasetpath() + Name + "..LINE";		
	}

	size_t groupbyindex(int value);

	size_t get_data();

};

class ILDataset{

private:	
	IHeader Header;
	std::vector<SurveyInfoEntry> SurveyInfo;
			
	bool readsurveyinfo()
	{
		_GSTITEM_
		FILE* fsurveyinfo;
		if ((fsurveyinfo = fileopen(surveyinfopath, "r")) == NULL) {
			message("Cannot open file: %s\n\n", surveyinfopath.c_str());
			return false;
		}

		std::string lstr;
		while (filegetline(fsurveyinfo, lstr)){
			size_t epos = lstr.find_first_of("=");
			if (epos<lstr.size()){
				lstr = trim(lstr);
				size_t len = lstr.size();
				epos = lstr.find_first_of("=");
				std::string lhs = lstr.substr(0, epos);
				std::string rhs = lstr.substr(epos + 1, len - epos);
				lhs = trim(lhs);
				rhs = trim(rhs);

				SurveyInfoEntry e;
				e.identifer = lhs;
				e.fieldname = rhs;
				SurveyInfo.push_back(e);
				
			}
		}
		fclose(fsurveyinfo);
		return true;
	}
	bool getfields()
	{
		_GSTITEM_
		std::vector<std::string> filelist;
		filelist = getfilelist(datasetpath, "pd");
		if (filelist.size() == 0){
			filelist = getfilelist(datasetpath, "PD");
		}

		//Count valid fields		
		for (size_t i = 0; i<filelist.size(); i++){			
			std::string name = extractfilename_noextension(filelist[i]);
			std::string ext  = extractfileextension(filelist[i]);

			if (strcasecmp(name.c_str(), "INDEX") == 0){
				continue;
			}

			if (strcasecmp(ext.c_str(),".PD")==0){				
				Fields.push_back(ILField());
				Fields.back().initialise(this, name);
			}
		}		
		return true;
	}

public:
	bool valid;
	std::string datasetpath;
	std::string surveyinfopath;
	std::string indexpath;
	
	std::list<ILField> Fields;
	
	ILDataset(const std::string& _datasetpath)
	{
		_GSTITEM_
		valid = false;

		datasetpath = strippath(_datasetpath);		
		if (datasetpath.size() <= 0){
			message("ILDataset::ILDataset() Invalid Dataset Path: %s\n\n", datasetpath.c_str());
			valid = false;
			return;
		}
		
		indexpath = datasetpath + "INDEX.PD";
		FILE* findex = fileopen(indexpath, "rb");
		if (findex == NULL) {
			message("ILDataset::ILDataset() Cannot open file: %s\n\n", indexpath.c_str());
			valid = false;
			return;
		}
		
		surveyinfopath = datasetpath + "SurveyInfo";
		if (readsurveyinfo() == false){
			valid = false;
			return;
		}

		std::vector<char> buffer(IHeader::nbytes());		
		fread(buffer.data(), IHeader::nbytes(), 1, findex);		
		Header = IHeader(buffer.data());		
		if (Header.valid==false){
			message("ILDataset::ILDataset() could not read INDEX file: %s\n\n", indexpath.c_str());
			valid = false;
			return;
		}
		else if (Header.filetype != ftINDEX){		
			message("ILDataset::ILDataset() File %s is not an INDEX file\n\n", indexpath.c_str());
			valid = false;
			return;
		}

		indextable.resize(nlines());		
		std::vector<int> indexdata(nlines() * 4);		
		if (fread((void*)indexdata.data(), 16, nlines(), findex) != (size_t)nlines()){
			message("ILDataset::ILDataset() Error reading INDEX file: %s\n\n", indexpath.c_str());
			valid = false;
			return;
		}
		
		if(Header.endianswap){
			swap_endian(indexdata.data(), nlines() * 4);
		}

		for (size_t i = 0; i<nlines(); i++){
			indextable[i].start = (size_t)indexdata[i * 4];
			indextable[i].ns = (size_t)indexdata[i * 4 + 1];
			indextable[i].dummy1 = (size_t)indexdata[i * 4 + 2];
			indextable[i].dummy2 = (size_t)indexdata[i * 4 + 3];
			if (indextable[i].ns > Header.maxspl){
				Header.maxspl = indextable[i].ns;
			}
		}				
		fclose(findex);

		if (getfields() == true){
			valid = true;
		}
		return;
	}

	~ILDataset()
	{
		_GSTITEM_
	}

	std::vector<IndexTable> indextable;
	std::vector<cLineSeg> bestfitlinesegs;
	
	static std::string dbdirpath(const std::string& path){
		_GSTITEM_
		std::string ext = extractfileextension(path);
		size_t pos = path.find("..DIR");
		std::string db = path.substr(0, pos);
		return db;
	}

	static std::string dbname(const std::string& path){				
		_GSTITEM_
		std::string dp = dbdirpath(path);
		std::string dn = extractfilename(dp);
		return dn;
	}

	size_t nlines() { return Header.nlines; }
	size_t maxspl() {return Header.maxspl;	}

	size_t nfields(){ return Fields.size(); }			
	size_t nsamplesinline(const size_t segindex){ return indextable[segindex].ns; }
	size_t nsamples(){
		_GSTITEM_
		size_t n = 0;
		for (size_t li = 0; li < nlines(); li++){
			n += nsamplesinline(li);
		}
		return n;
	}
	size_t startindex(const size_t segindex){ return indextable[segindex].start; }
	
	std::vector<size_t> linesamplecount(){
		std::vector<size_t> count(nlines());
		for (size_t i = 0; i < nlines(); i++){
			count[i] = indextable[i].ns;
		}
		return count;
	}

	static std::string strippath(const std::string& path)
	{
		_GSTITEM_
		std::string p = path;
		fixseparator(p);

		size_t len = p.size();
		if (len >= 5){
			if (p.substr(len - 5, 5) == "..DIR"){
				p.resize(len - 5);;
			}
			else if (p.substr(len - 5, 5) == "..dir"){
				p.resize(len - 5);
			}
		}

		len = p.size();
		if (len >= 1){
			if (p[len - 1] != pathseparator()){
				p += pathseparatorstring();
			}
		}
		return p;
	}	
	
	static bool isdatabase(const std::string& path)
	{
		_GSTITEM_
		std::string p = strippath(path);
		size_t len = p.size();
		if (len <= 1) return false;

		std::string indexfile = p + "INDEX.PD";
		bool isvalid = exists(indexfile);
		if (isvalid)return true;

		indexfile = p + "index.pd";
		isvalid = exists(indexfile);
		if (isvalid)return true;
		else return false;
	}

	std::string infostring()
	{
		_GSTITEM_
		std::string s;
		s += strprint("Dataset Information\n");
		s += strprint(datasetpath.c_str());		
		s += strprint(indexpath.c_str());		
		s += strprint(surveyinfopath.c_str());		
		s += strprint("%lu Lines\n", nlines());
		s += strprint("Maximum samples per line = %lu\n\n", maxspl());
		s += strprint("Fields %lu\n", nfields());
		for (auto it = Fields.begin(); it != Fields.end(); ++it){
			s += "\t";
			s += it->infostring();
		}

		if (valid == true){
			s += strprint("Valid = Yes\n");
		}
		else{
			s += strprint("Valid = No\n");
		}
		return s;
	}

	void printinfo()
	{
		_GSTITEM_
		std::string s = infostring();
		printf(s.c_str());		
	}

	bool fieldexists(const std::string& fieldname)
	{
		_GSTITEM_
		for (auto it = Fields.begin(); it != Fields.end(); ++it){
			if (strcasecmp(it->Name.c_str(),fieldname.c_str())==0){
				return true;
			}
		}
		return false;
	}

	ILField* getfield(const std::string& fieldname)
	{
		_GSTITEM_
		for (auto it = Fields.begin(); it != Fields.end(); ++it){			
			if (strcasecmp(it->Name.c_str(), fieldname.c_str()) == 0){
				return (ILField*)&(*it);
			}
		}	
		return (ILField*)NULL;
	}

	ILField* getsurveyinfofield(const std::string& identifer)
	{
		_GSTITEM_
		std::string name = surveyinfofieldname(identifer);
		if (name.size() == 0){
			printf("Cannot find field %s from SurveyInfo:\n\n", identifer.c_str());
			return (ILField*)NULL;
		}
		return getfield(name);
	}	

	ILField& getsurveyinfofield_ref(const std::string& identifer)
	{
		_GSTITEM_
		ILField* pF = getsurveyinfofield(identifer);
		return *pF;
	}

	bool erasefield(const std::string& fieldname)
	{
		_GSTITEM_
		for (auto it = Fields.begin(); it != Fields.end(); ++it){
			if (strcasecmp(it->Name.c_str(), fieldname.c_str()) == 0){
				bool status = it->erase();
				if (status){
					Fields.erase(it);
				}
				return status;
			}
		}
		return false;			
	}
	
	bool addfield(const std::string& fieldname, IDatatype datatype, size_t nbands=1, bool isindexed=true)
	{
		_GSTITEM_
		if (fieldexists(fieldname)){
			printf("ILDataset::addfield() %s already exists\n\n", fieldname.c_str());
			return false;
		}

		Fields.push_back(ILField());
		Fields.back().createnew(this, fieldname, datatype, nbands, isindexed);						

		ILField* F = getfield(fieldname);
		for (size_t li = 0; li < nlines(); li++){
			ILSegment S = F->Segments[li];			
			S.createbuffer();
			S.writebuffer();
		}

		return true;
	}
	
	std::string fieldnamelike(const std::string& identifer) const
	{
		_GSTITEM_
		for (auto it = Fields.begin(); it != Fields.end(); ++it){			
			if (strcasecmp(it->Name.c_str(), identifer.c_str()) == 0){
				return it->Name;
			}
		}		
		//printf("Cannot find field like: identifer %s\n\n", identifer.c_str());
		return "";
	}

	bool hassurveyinfoid(const std::string& identifer)
	{
		_GSTITEM_
		for (size_t i = 0; i < SurveyInfo.size(); i++){
			if (strcasecmp(SurveyInfo[i].identifer.c_str(), identifer.c_str()) == 0){
				return true;
			}
		}
		return false;		
	}

	bool hassurveyinfoid_fieldexists(const std::string& identifer)
	{
		_GSTITEM_
		if(hassurveyinfoid(identifer)){
			std::string fname = surveyinfofieldname(identifer);
			return fieldexists(fname);
		}			
		return false;
	}

	std::string surveyinfofieldname(const std::string& identifer)
	{
		_GSTITEM_
		for (size_t i = 0; i < SurveyInfo.size(); i++){
			if (strcasecmp(SurveyInfo[i].identifer.c_str(), identifer.c_str()) == 0){
				return SurveyInfo[i].fieldname;
			}
		}
		//printf("Cannot find identifer %s in SurveyInfo:\n\n", identifer.c_str());
		return "";
	}

	void bestfitlines()
	{
		_GSTITEM_
		if (bestfitlinesegs.size() > 0)return;
		
		ILField& fX = *getsurveyinfofield("X");
		ILField& fY = *getsurveyinfofield("Y");

		for (size_t li = 0; li<nlines(); li++){
			ILSegment& sX = fX.Segments[li];
			ILSegment& sY = fY.Segments[li];
			size_t numsamples = sX.nsamples();

			////Find first and last non nulls
			size_t s = 0;
			size_t firstnonnull, lastnonnull;
			double xv, yv;
			do{
				xv = sX.d(s);
				yv = sY.d(s);
				if (IDatatype::isnull(xv) || IDatatype::isnull(yv)){
					s++;
				}
				else{
					firstnonnull = s;
					break;
				}
			} while (s<numsamples);

			s = numsamples - 1;
			do{
				xv = sX.d(s);
				yv = sY.d(s);
				if (IDatatype::isnull(xv) || IDatatype::isnull(yv)){
					s--;
				}
				else{
					lastnonnull = s;
					break;
				}
			} while (s>0);


			///////////////////
			size_t n = 40;
			double x[40];
			double y[40];

			size_t validsamples = lastnonnull - firstnonnull + 1;
			if (n>validsamples)n = validsamples;
			size_t di = validsamples / n;
			for (s = 0; s<n; s++){
				x[s] = sX.d(firstnonnull + s*di);
				y[s] = sY.d(firstnonnull + s*di);
			}

			cPnt p1, p2;
			cLineSeg seg;
			double gradient, intercept;
			if (fabs(x[0] - x[n - 1]) > fabs(y[0] - y[n - 1])){
				regression(x, y, n, &gradient, &intercept);
				p1.x = sX.d(firstnonnull);
				p2.x = sX.d(lastnonnull);
				p1.y = gradient * p1.x + intercept;
				p2.y = gradient * p2.x + intercept;
			}
			else{
				regression(y, x, n, &gradient, &intercept);
				p1.y = sY.d(firstnonnull);
				p2.y = sY.d(lastnonnull);
				p1.x = gradient * p1.y + intercept;
				p2.x = gradient * p2.y + intercept;
			}
			p1.z = 0.0;
			p2.z = 0.0;

			seg.set(p1, p2);
			bestfitlinesegs.insert(bestfitlinesegs.end(), seg);
		}
		fX.close();
		fY.close();
	}
	
	template <typename T>
	bool getlinenumbers(std::vector<T>& v){
		
		if (hassurveyinfoid_fieldexists("LineNumber")){
			ILField& f = *getsurveyinfofield("LineNumber");
			bool status = getgroupbydata(f, v);
			return status;
		}
		else if (fieldexists("LINE")){
			ILField& f = *getfield("LINE");
			bool status = getgroupbydata(f, v);
			return status;
		}		
		else return false;
	}

	template <typename T> 
	bool getgroupbydata(const ILField& f, std::vector<T>& v, const int band=0){
		v.resize(nlines());
		for (size_t li = 0; li < nlines(); li++){
			ILSegment S = f.Segments[li];
			S.readbuffer();
			std::vector<T> b;
			S.getband(b,band);
			v[li] = b[0];
		}
		return true;
	}

	double distancetobestfitline(cPnt p, size_t i)
	{
		_GSTITEM_
		bestfitlines();
		cPnt c = closestpoint(bestfitlinesegs[i], p);
		double d = c.distance(p);
		return d;
	}

	size_t nearestbestfitline(cPnt p)
	{
		_GSTITEM_
		bestfitlines();
		size_t index = 0;
		double mindistance, d;
		for (size_t li = 0; li<nlines(); li++){
			d = distancetobestfitline(p, li);
			if (li == 0)mindistance = d;
			if (d<mindistance){
				mindistance = d;
				index = li;
			}
		}
		return index;
	}

	double nearestsample(const cPnt p, size_t& lineindex, size_t& sampleindex, double& x, double& y)
	{
		_GSTITEM_
		lineindex = nearestbestfitline(p);
		sampleindex = 0;

		double mindistance;

		ILField& fX = *getsurveyinfofield("X");
		ILField& fY = *getsurveyinfofield("Y");

		ILSegment& sX = fX.Segments[lineindex];
		ILSegment& sY = fY.Segments[lineindex];

		for (size_t si = 0; si<sX.nsamples(); si++){
			double dx = p.x - sX.d(si);
			double dy = p.y - sY.d(si);
			double d = sqrt(dx*dx + dy*dy);
			if (si == 0)mindistance = d;
			if (d <= mindistance){
				mindistance = d;
				sampleindex = si;
				x = sX.d(si);
				y = sY.d(si);
			}
		}
		return mindistance;
	}

	std::vector<SampleIndex> sampleswithindistance(cPnt p, double distance)
	{
		_GSTITEM_
		bestfitlines();
		std::vector<SampleIndex> samples;

		ILField& fX = *getsurveyinfofield("X");
		ILField& fY = *getsurveyinfofield("Y");

		for (size_t li = 0; li<nlines(); li++){
			double d = distancetobestfitline(p, li);
			if (d<distance*2.0){
				ILSegment& sX = fX.Segments[li];
				ILSegment& sY = fY.Segments[li];
				size_t nsam = sX.nsamples();
				for (size_t si = 0; si<nsam; si++){
					cPnt p1(sX.d(si), sY.d(si), 0.0);
					if (p.distance(p1) <= distance){
						struct SampleIndex sam;
						sam.lineindex = li;
						sam.sampleindex = si;
						samples.push_back(sam);
					}
				}
			}
		}
		fX.close();
		fY.close();
		return samples;
	}

	SampleIndex linefid_index(int linenumber, int fidnumber)
	{
		_GSTITEM_
		ILField& fLine = *getsurveyinfofield("LineNumber");
		ILField& fFid  = *getsurveyinfofield("Fiducial");

		SampleIndex sindex;
		sindex.lineindex = fLine.groupbyindex(linenumber);
		if (sindex.lineindex == nullindex())return sindex;
		
		ILSegment& sFid  = fFid.Segments[sindex.lineindex];
		sindex.sampleindex = nullindex();
		for (size_t si = 0; si<sFid.nsamples(); si++){
			if (sFid.i(si) == fidnumber){
				sindex.sampleindex = si;
				break;
			}
		}
		return sindex;
	}

	cStats<double> fieldstats(const std::string& fieldname){
		_GSTITEM_
		ILField& F = *getfield(fieldname);
		vector<double> v;
		v.reserve(nsamples());		
		for (size_t li = 0; li < nlines(); li++){
			ILSegment& S = F.Segments[li];			
			S.readbuffer();	
			size_t nsamples = S.nsamples();			
			for (size_t si = 0; si < nsamples; si++){				
				double val = S.d(si);
				if (IDatatype::isnull(val)==false){
					v.push_back(val);					
				}				
			}
		}				
		cStats<double> stats(v);
		return stats;		
	}
	
	template<typename T>
	void getdata(const std::string& fieldname, std::vector<T> v){
		_GSTITEM_

		ILField& F = *getfield(fieldname);		
		v.reserve(nsamples());
		for (size_t li = 0; li < nlines(); li++){
			ILSegment& S = F.Segments[li];
			S.readbuffer();
			F.datatype();
			size_t nsamples = S.nsamples();
			for (size_t si = 0; si < nsamples; si++){
				T val = S.d(si);
				v.push_back(val);				
			}
		}		
	}

	bool get_line_start_end_points(std::vector<double>& x1, std::vector<double>& x2, std::vector<double>& y1, std::vector<double>& y2)
	{
		_GSTITEM_
		ILField& fx = getsurveyinfofield_ref("X");
		ILField& fy = getsurveyinfofield_ref("Y");

		size_t nl = nlines();

		IDatatype dt = fx.datatype();
		
		x1.resize(nl);
		y1.resize(nl);
		x2.resize(nl);
		y2.resize(nl);
		
		for (size_t li = 0; li < nl; li++){
			ILSegment& sx = fx.Segments[li];
			ILSegment& sy = fy.Segments[li];
			sx.readbuffer();
			sy.readbuffer();
			size_t ns = sx.nsamples();
			for (size_t k = 0; k<ns; k++){
				x1[li] = sx.d(k);
				y1[li] = sy.d(k);
				if (dt.isnull(x1[li]) == false && dt.isnull(y1[li]) == false)break;
			}

			for (size_t k = ns - 1; k != 0; k--){
				x2[li] = sx.d(k);
				y2[li] = sy.d(k);
				if (dt.isnull(x2[li]) == false && dt.isnull(y2[li]) == false)break;
			}
		}		
		return true;
	}
};


///////////////////////////////////////////////////////////////////
ILDataset* ILSegment::pDataset() { return pField->pDataset; }
size_t ILSegment::nbands() { return pField->nbands(); }
size_t ILSegment::nsamples() { return pDataset()->nsamplesinline(lineindex); }
size_t ILSegment::startindex() { return pDataset()->startindex(lineindex); }
IDatatype ILSegment::datatype() { return pField->datatype(); }
FILE* ILSegment::filepointer() { return pField->filepointer(); }
bool ILSegment::isgroupbyline()
{
	return pField->isgroupbyline();
}
bool ILSegment::isindexed()
{
	return pField->isindexed(); 
}

bool ILSegment::readbuffer()
{	
	size_t n;
	pField->open();
	long move = fileposition() - std::ftell(filepointer());
	std::fseek(filepointer(), move, SEEK_CUR);

	switch (datatype().etype()){
	case dtFLOAT:
		fdata.resize(nsamples(), nbands(), isgroupbyline());
		n = std::fread(fdata.pvoid(), nbytes(), 1, filepointer());
		if(pField->endianswap()) fdata.swap_endian();
		break;
	case dtDOUBLE:
		ddata.resize(nsamples(), nbands(), isgroupbyline());
		n = std::fread(ddata.pvoid(), nbytes(), 1, filepointer());
		if (pField->endianswap()) ddata.swap_endian();
		break;
	case dtSHORT:
		sdata.resize(nsamples(), nbands(), isgroupbyline());
		n = std::fread(sdata.pvoid(), nbytes(), 1, filepointer());
		if (pField->endianswap()) sdata.swap_endian();
		break;
	case dtINT:
		idata.resize(nsamples(), nbands(), isgroupbyline());
		n = std::fread(idata.pvoid(), nbytes(), 1, filepointer());
		if (pField->endianswap()){
			idata.swap_endian();
		}
		break;
	case dtBYTE:
		cdata.resize(nsamples(), nbands(), isgroupbyline());
		n = std::fread(cdata.pvoid(), nbytes(), 1, filepointer());
		if (pField->endianswap()) cdata.swap_endian();
		break;
	default: std::printf("ILSegment::read() Unknown type"); return false;
	}
		
	if (n != 1){
		std::printf("ILSegment::readbuffer Error reading file %s\n", pField->datafilepath().c_str());
		return false;
	}	
	return true;
}

bool ILSegment::writebuffer()
{
	size_t n;
	pField->open();
	long move = fileposition() - ftell(filepointer());
	fseek(filepointer(), move, SEEK_CUR);

	switch (datatype().etype()){
	case dtFLOAT:
		if (pField->endianswap()) fdata.swap_endian();				
		n = fwrite(fdata.pvoid(), nbytes(), 1, filepointer());
		break;
	case dtDOUBLE:
		if (pField->endianswap()) ddata.swap_endian();		
		n = fwrite(ddata.pvoid(), nbytes(), 1, filepointer());		
		break;
	case dtSHORT:
		if (pField->endianswap()) sdata.swap_endian();
		n = fwrite(sdata.pvoid(), nbytes(), 1, filepointer());		
		break;
	case dtINT:
		if (pField->endianswap()) idata.swap_endian();
		n = fwrite(idata.pvoid(), nbytes(), 1, filepointer());		
		break;
	case dtBYTE:
		if (pField->endianswap()) cdata.swap_endian();
		n = fwrite(cdata.pvoid(), nbytes(), 1, filepointer());		
		break;
	default: printf("ILSegment::read() Unknown type"); return false;
	}

	if (n != 1){
		printf("ILSegment::writebuffer Error writing to file %s\n", pField->datafilepath().c_str());
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////
std::string ILField::datasetpath()
{
	return pDataset->datasetpath;
}

size_t ILField::nlines() const { return pDataset->nlines(); }

bool ILField::initialise(ILDataset *_pDataset, const std::string& _fieldname)
{	
	pDataset = _pDataset;
	Name     = _fieldname;	
	initialisesegments();
	pFile = (FILE*)NULL;
	open();
	close();
	return true;
}

bool ILField::createnew(ILDataset *_pDataset, const std::string& _fieldname, IDatatype _datatype, size_t _nbands, bool _indexed)
{
	pDataset = _pDataset;
	Name     = _fieldname;	
	pFile    = (FILE*)NULL;
	
	int16_t hdata[256];
	for (size_t i = 0; i < 256; i++) hdata[i]=0;
		
	hdata[79] = (int16_t)IHeader::nbytes();
	hdata[81] = 1;

	Header.endianswap = false;
	Header.headeroffset = IHeader::nbytes();
	Header.nlines = pDataset->nlines();
	Header.nbands = _nbands;

	hdata[32] = (int16_t)pDataset->maxspl();
	hdata[33] = (int16_t)pDataset->nlines();
	hdata[34] = (int16_t)_nbands;

	hdata[217] = (int16_t)pDataset->maxspl();
	hdata[219] = (int16_t)pDataset->nlines();
	hdata[221] = (int16_t)_nbands;	

	if (_indexed){
		Header.accesstype = atINDEXED;
		Header.maxspl = pDataset->maxspl();
		hdata[91] = 2; //INDEXED		
	}
	else{
		Header.accesstype = atDIRECT;
		Header.maxspl = 1;
		hdata[91] = 1; //Group By		
	}

	Header.filetype = ftLINE;
	hdata[72] = 1000; //LINE

	Header.datatype = _datatype;
	if (_datatype.isbyte())       { hdata[73] = 1; hdata[90] = 8; }
	else if (_datatype.isshort()) { hdata[73] = 2; hdata[90] = 16; }
	else if (_datatype.isint())   { hdata[73] = 2; hdata[90] = 32; }
	else if (_datatype.isfloat()) { hdata[73] = 3; hdata[90] = 32; }
	else if (_datatype.isdouble()){ hdata[73] = 3; hdata[90] = 64; }
	
	hdata[93] = -1; //PROJECTION
	
	if (_nbands > 1){
		Header.bandpackingtype = bptBIP;
		hdata[78] = 2; //BIP
	}
	else{
		Header.bandpackingtype = bptBIL;
		hdata[78] = 1; //BIL
	}
	
	Header.indexname = "INDEX                    ";//INDEX FILE - need spaces
	strncpy((char*)&hdata[87], "OK", 2);  //???
	strncpy((char*)&hdata[89], "P1", 2);  //???
	strncpy((char*)&hdata[171], Header.indexname.c_str(),25);//INDEX FILE - need spaces
	hdata[185] = 1;
	hdata[186] = 251;

	if ((pFile = fileopen(datafilepath(), "w+b")) == NULL) {
		message("Cannot open file: %s\n\n", datafilepath().c_str());
		return false;
	}

	if (fwrite((char*)hdata, IHeader::nbytes(), 1, pFile) != 1){
		message("Cannot write header: %s\n\n", datafilepath().c_str());
		return false;
	}

	initialisesegments();
	for (size_t si = 0; si < Segments.size(); si++){
		size_t ns = Segments[si].nsamples();
		std::vector<std::vector<float>> array;
		array.resize(nbands());
		for (size_t bi = 0; bi < nbands(); bi++){
			array[bi].resize(ns,0.0);						
		}
		//Segments[si].writearray(array);
	}	
	close();
	
	FILE* lfile;
	if ((lfile = fileopen(dotdotlinefilepath(), "w+b")) == NULL) {
		message("Cannot open file: %s\n\n", dotdotlinefilepath().c_str());
		return false;
	}
	fclose(lfile);

	
	open();
	close();	
	return true;
}

size_t ILField::groupbyindex(int value)
{
	for (size_t li = 0; li < nlines(); li++){		
		int v = Segments[li].i(0,0);
		if (v == value) return li;
	}
	return nullindex();
}

#endif
