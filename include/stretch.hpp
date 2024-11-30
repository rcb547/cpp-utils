/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <cstdint>
#include "general_utils.hpp"
#include "blocklanguage.hpp"

enum eStretchType {
	LINEAR,
	LOG10
};

class cStretch {

public:
	eStretchType type = eStretchType::LINEAR;
	double lowclip  = 0;
	double highclip = 1;	
	int nbins = 256;
	
	cStretch(){ }

	cStretch(const double& _lowclip, const double& _highclip, const eStretchType& _type, const int& nbins=256){
		lowclip  = _lowclip;
		highclip = _highclip;
		type     = _type;
	}

	cStretch(const cBlock& b){

		if(b.getvalue("LowClip",lowclip) == false){
			std::string msg("Stretch LowClip not set\n");
			glog.errormsg(_SRC_, msg);
		}

		if (b.getvalue("HighClip", highclip) == false){
			std::string msg("Stretch HighClip not set\n");
			glog.errormsg(_SRC_, msg);
		}				

		std::string s = b.getstringvalue("Type");
		int v = strcasecmp(s, "LINEAR");
		if (strcasecmp(s, "LINEAR") == 0){ 
			type = eStretchType::LINEAR;
		}
		else if (strcasecmp(s, "LOG10") == 0){
			type = eStretchType::LOG10;
		}
		else{
			std::string msg("ColourStretch type not set\n");
			glog.errormsg(_SRC_, msg);
		}		
	}
	

	int index(const double& val) const {
		if (type == eStretchType::LINEAR){
			return linearstretch(val, lowclip, highclip, nbins);
		}
		else{
			return log10stretch(val, lowclip, highclip, nbins);
		}		
	}

	static int linearstretch(const double val, const double lowclip, const double highclip, const int nbins = 256)
	{
		int bin;
		if (val <= lowclip) bin = 0;
		else if (val >= highclip) bin = nbins - 1;
		else bin = (int)((double)nbins * (val - lowclip) / (highclip - lowclip));
		//if (bin >= nbins) bin = nbins - 1;
		//if (bin < 0) bin = 0;
		return bin;
	}

	static double inverselinearstretch(const int bin, const double lowclip, const double highclip, const int nbins = 256)
	{
		return lowclip + ((double)bin / (double)nbins) * (highclip - lowclip);
	}

	static int log10stretch(const double val, const double lowclip, const double highclip, const int nbins = 256)
	{
		if (val <= lowclip) return  0;
		if (val >= highclip) return nbins-1;

		int bin;
		double logl = log10(lowclip);
		double logh = log10(highclip);
		if (val <= 0.0) bin = 0;
		else bin = (int)((double)nbins * (log10(val) - logl) / (logh - logl));
		//if (bin >= nbins) bin = (nbins - 1);
		//if (bin < 0) bin = 0;
		return bin;
	}

	static double inverselog10stretch(const int bin, const double lowclip, const double highclip, const int nbins = 256)
	{
		double logl = log10(lowclip);
		double logh = log10(highclip);
		double lval = logl + ((double)bin / (double)nbins)*(logh - logl);
		return pow(10.0, lval);
	}

	
};

