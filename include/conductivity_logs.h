/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _conductivity_logs_H
#define _conductivity_logs_H

#include <vector>

#include "file_utils.h"
#include "general_utils.h"

class cConductivityLog{

public:
	std::string name = "";
	std::string source = "";
	std::string lasfile = "";
	double x = 0;
	double y = 0;
	double z = 0;
	std::vector<double> depth;
	std::vector<double> conductivity;
	;
	cConductivityLog(){};

	cConductivityLog(const std::string& confile, const bool readheaderonly = false){
		load_confile(confile, readheaderonly);
	}

	bool load_confile(const std::string& confile, const bool readheaderonly = false){
		std::string s;
		std::vector<std::string> t;
		source = confile;
		FILE* fp = fileopen(confile, "r");

		double usf = 1.0;
		//Read header
		while (filegetline(fp, s)){			
			t = parsestrings(s, " ,:");
			if (t[0] == "Bore") name = t[1];
			if (t[0] == "Lasfile") lasfile = t[1];
			if (t[0] == "Easting") x = std::atof(t[1].c_str());
			if (t[0] == "Northing") y = std::atof(t[1].c_str());
			if (t[0] == "Elevation") z = std::atof(t[1].c_str());
			if (t[0] == "Depth(m)"){
				if (t[1] == "Conductivity(mS/m)") usf = 0.001;
				if (t[1] == "Conductivity(S/m)") usf = 1.000;
				break;
			}
		}

		if (readheaderonly){
			fclose(fp);
			return true;
		}

		//Read data
		while (filegetline(fp, s)){
			t = tokenize(s);
			if (t[0] == "NaN")continue;
			else if (t[1] == "NaN")continue;
			double d = std::atof(t[0].c_str());
			double c = std::atof(t[1].c_str());
			if (d < 0)continue;
			else if (c <= 0)continue;
			else{
				depth.push_back(d);
				conductivity.push_back(usf*c);
			}
		}
		fclose(fp);
		return true;
	};

	std::pair<size_t, size_t> first_last_index(const double& d1, const double& d2){

		auto start = std::find_if(
			depth.begin(), depth.end(),
			[&d1](const double& item){ return item >= d1; }
		);

		auto end = std::find_if(
			start, depth.end(),
			[&d2](const double& item){ return item > d2; }
		);

		size_t i1 = start - depth.begin();
		size_t i2 = end - depth.begin();
		std::pair<size_t, size_t> p(i1, i2);
		return p;
	};

	size_t interval_nsamples(const double& d1, const double& d2){
		std::pair<size_t, size_t> p = first_last_index(d1, d2);
		size_t n = p.second - p.first;
		return n;
	}

	bool interval_has_overlap(const double& d1, const double& d2){
		if (interval_nsamples(d1, d2)) return true;
		else return false;
	}

	bool interval_means(const double& d1, const double& d2, size_t& n, double& linear_mean, double& log10_mean){

		linear_mean = 0.0;
		log10_mean = 0.0;
		const std::pair<size_t, size_t> p = first_last_index(d1, d2);
		n = p.second - p.first;
		if (n <= 0)return false;
		for (size_t k = p.first; k < p.second; k++){
			linear_mean += conductivity[k];
			log10_mean += std::log10(conductivity[k]);
		}
		linear_mean /= (double)n;
		log10_mean /= (double)n;
		log10_mean = std::pow(10.0, log10_mean);
		return true;
	};

	std::string infostring()
	{
		std::string s;
		s += strprint("Name: %s ", name.c_str());
		s += strprint("X: %.1lf ", x);
		s += strprint("Y: %.1lf ", y);
		s += strprint("Z: %.1lf ", z);
		s += strprint("Log depth: %.1lf ", depth.back());
		s += strprint("Source: %s", source.c_str());
		return s;
	};
};

#endif

