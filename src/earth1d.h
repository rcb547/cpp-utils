/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _earth1d_H
#define _earth1d_H

//#include <climits>
//#include <cstring>
//#include <complex>
#include <vector>
#include <cassert>


class cEarth1D { 
	
public:
	std::vector<double> conductivity;	
	std::vector<double> thickness;
	
	cEarth1D(){};

	cEarth1D(const size_t nlayers){
		conductivity.resize(nlayers);
		thickness.resize(nlayers-1);
	}

	cEarth1D(const std::vector<double>& _conductivity, const std::vector<double>& _thickness){		
		assert(_conductivity.size() == _thickness.size() + 1);
		conductivity = _conductivity;
		thickness    = _thickness;
	}

	inline const size_t nlayers() const {return conductivity.size();}
	inline const size_t nl() const { return conductivity.size(); }

	void print() const {
		for(size_t i=0; i<nlayers()-1; i++){
			printf("%d\t%8.6lf\t%6.2lf\n",(int)i,conductivity[i],thickness[i]);
		}
		printf("%d\t%8.6lf\n\n",(int)nlayers(),conductivity[nlayers()-1]);
	}

	double meanlog10conductivity() const 
	{
		if (nl() == 1)return conductivity[0];
		//Returns the thickness weighted mean (in linear space) conductivity (but calculated in 10g10 space)
		double sumc = 0.0;
		double sumt = 0.0;
		for (size_t i = 0; i<nl(); i++){
			if (i<(nl() - 1)){
				sumc += log10(conductivity[i])*thickness[i];
				sumt += thickness[i];
			}
			else{
				sumc += log10(conductivity[i])*sumt; //make basement layer as thick as sum of all overlying
				sumt += sumt;
			}
		}
		return pow(10.0, sumc / sumt);
	}

	double meanconductivity() const 
	{
		if (nl() == 1)return conductivity[0];
		//Returns the thickness weighted mean (in linear space) conductivity (but calculated in linear space)
		double sumc = 0.0;
		double sumt = 0.0;
		for (int i = 0; i<nl(); i++){
			if (i<(nl() - 1)){
				sumc += conductivity[i] * thickness[i];
				sumt += thickness[i];
			}
			else{
				sumc += conductivity[i] * sumt; //make basement layer as thick as sum of all overlying
				sumt += sumt;
			}
		}
		return sumc / sumt;
	}
};

#endif
