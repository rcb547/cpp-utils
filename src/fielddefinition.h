/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

//---------------------------------------------------------------------------
#ifndef fielddefinitionH
#define fielddefinitionH

#include <stdlib.h>
#include <cstring>
#include <vector>
#include <float.h>

#include "general_utils.h"
#include "blocklanguage.h"

#define COLDEF_COLUMNVALUE  0
#define COLDEF_NEGATIVECOLUMNVALUE  1
#define COLDEF_DEFAULTVALUE 2
#define COLDEF_UNAVAILABLE  3

class FieldDefinition{

private:

	char op = ' ';
	double opval = 0.0;
	size_t firstcolumn = 1;
	int coldeftype;
	size_t column;	
	std::vector<double> defaultvector;	

	void ifnullconvert2zero(int& val) const {
		//temporary hack to handle Nulls
		if (val == -999 || val == -9999) val = 0;		
	}

	void ifnullconvert2zero(double& val) const {
		//temporary hack to handle Nulls
		if (val == -999.99) val = 0.0;		
	}

	void applyoperator(int& val) const {
		if (op == ' ') return;
		else if (op == '+') val += (int) opval;
		else if (op == '-') val -= (int) opval;
		else if (op == '*') val *= (int) opval;
		else if (op == '/') val /= (int) opval;
		else warningmessage("FieldDefinition::applyoperator() unknown operator %c\n", op);		
		return;
	}

	void applyoperator(double& val) const {
		if (op == ' ') return;
		else if (op == '+') val += opval;
		else if (op == '-') val -= opval;
		else if (op == '*') val *= opval;
		else if (op == '/') val /= opval;
		else warningmessage("FieldDefinition::applyoperator() unknown operator %c\n",op);
		return;
	}

public:
	
	FieldDefinition() { }

	void set(const cBlock& b, const std::string& fieldname)
	{
		int col;
		std::string rhs = b.getstringvalue(fieldname);
		if (strncasecmp(rhs, "Column", 6) == 0){			
			coldeftype = COLDEF_COLUMNVALUE;						
			int n = sscanf(&(rhs.c_str()[6]), "%d %c %lf", &col, &op, &opval);
			column = (size_t)col;
			if (n == 1){ op = ' '; opval = 0.0; }
		}
		else if (strncasecmp(rhs, "-Column", 7) == 0){
			coldeftype = COLDEF_NEGATIVECOLUMNVALUE;
			int n = sscanf(&(rhs.c_str()[7]), "%d %c %lf", &col, &op, &opval);
			column = (size_t)col;			
			if (n == 1){ op = ' '; opval = 0.0; }			
		}
		else if (strncasecmp(rhs, "Unavailable", 11) == 0){
			coldeftype = COLDEF_UNAVAILABLE;
			column = 0;
			op = ' '; opval = 0.0;
		}
		else{			
			coldeftype = COLDEF_DEFAULTVALUE;
			column = (size_t)0;
			defaultvector = b.getdoublevector(fieldname);
			op = ' '; opval = 0.0;
		}
	}
	
	int intvalue(const std::vector<std::string>& fields) const
	{
		int v;
		if (coldeftype == COLDEF_COLUMNVALUE){
			sscanf(fields[column - firstcolumn].c_str(), "%d", &v);
			ifnullconvert2zero(v);
			applyoperator(v);
			return v;
		}
		else if (coldeftype == COLDEF_NEGATIVECOLUMNVALUE){
			sscanf(fields[column - firstcolumn].c_str(), "%d", &v);
			ifnullconvert2zero(v);
			v = -v;
			applyoperator(v);
			return v;
		}
		else if (coldeftype == COLDEF_DEFAULTVALUE){
			if (defaultvector.size() == 0){
				return ud_int();
			}
			return (int)defaultvector[0];
		}
		else if (coldeftype == COLDEF_UNAVAILABLE){
			return ud_int();
		}
		else{
			warningmessage("FieldDefinition::intvalue() unknown column definition\n");
		}
		return ud_int();
	}

	double doublevalue(const std::vector<std::string>& fields) const
	{
		double v;
		if (coldeftype == COLDEF_COLUMNVALUE){
			sscanf(fields[column - firstcolumn].c_str(), "%lf", &v);
			ifnullconvert2zero(v);
			applyoperator(v);
			return v;
		}
		else if (coldeftype == COLDEF_NEGATIVECOLUMNVALUE){
			sscanf(fields[column - firstcolumn].c_str(), "%lf", &v);
			ifnullconvert2zero(v);
			v = -v;
			applyoperator(v);
			return v;
		}
		else if (coldeftype == COLDEF_DEFAULTVALUE){
			if (defaultvector.size() == 0){
				return ud_double();
			}
			return defaultvector[0];
		}
		else if (coldeftype == COLDEF_UNAVAILABLE){
			return ud_double();
		}
		else{
			warningmessage("FieldDefinition::doublevalue() unknown column definition\n");
		}
		return ud_double();
	}

	std::vector<int> intvector(const std::vector<std::string>& fields, const size_t& n) const
	{
		int v;
		if (coldeftype == COLDEF_COLUMNVALUE){
			std::vector<int> vec(n);
			for (size_t i = 0; i<n; i++){
				sscanf(fields[i + column - firstcolumn].c_str(), "%d", &v);
				ifnullconvert2zero(v);
				applyoperator(v);
				vec[i] = v;
			}
			return vec;
		}
		else if (coldeftype == COLDEF_NEGATIVECOLUMNVALUE){
			std::vector<int> vec(n);
			for (size_t i = 0; i<n; i++){
				sscanf(fields[i + column - firstcolumn].c_str(), "%d", &v);
				ifnullconvert2zero(v);
				v = -v;
				applyoperator(v);
				vec[i] = v;
			}
			return vec;
		}
		else if (coldeftype == COLDEF_DEFAULTVALUE){
			size_t deflen = defaultvector.size();
			std::vector<int> vec(n);
			for (size_t i = 0; i<n; i++){
				if (deflen == 1) vec[i] = (int)defaultvector[0];
				else vec[i] = (int)defaultvector[i];
			}
			return vec;
		}
		else if (coldeftype == COLDEF_UNAVAILABLE){
			std::vector<int> vec(0);
			return vec;
		}
		else{
			warningmessage("FieldDefinition::intvector() unknown column definition type\n");
		}
		std::vector<int> vec(0);
		return vec;
	}
	
	std::vector<double> doublevector(const std::vector<std::string>& fields, const size_t& n) const
	{
		double v;
		if (coldeftype == COLDEF_COLUMNVALUE){
			std::vector<double> vec(n);
			for (size_t i = 0; i<n; i++){
				sscanf(fields[i + column - firstcolumn].c_str(), "%lf", &v);
				ifnullconvert2zero(v);				
				applyoperator(v);
				vec[i] = v;
			}
			return vec;
		}
		else if (coldeftype == COLDEF_NEGATIVECOLUMNVALUE){
			std::vector<double> vec(n);
			for (size_t i = 0; i<n; i++){
				sscanf(fields[i + column - firstcolumn].c_str(), "%lf", &v);
				ifnullconvert2zero(v);
				v = -v;
				applyoperator(v);
				vec[i] = v;
			}
			return vec;
		}
		else if (coldeftype == COLDEF_DEFAULTVALUE){
			std::vector<double> vec(n);
			int deflen = (int)defaultvector.size();
			for (size_t i = 0; i<n; i++){
				if (deflen == 1)vec[i] = defaultvector[0];
				else vec[i] = defaultvector[i];
			}
			return vec;
		}
		else if (coldeftype == COLDEF_UNAVAILABLE){
			std::vector<double> vec(0);
			return vec;
		}
		else{
			warningmessage("FieldDefinition::doublevector() unknown column definition type\n");
		}
		std::vector<double> vec(0);
		return vec;
	}

};

#endif
