/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <sstream>
#include "gdal_utilities.hpp"

class cCRS{

public:
	std::string name = "";
	std::string epsg_string = "";
	double semi_major_axis = 0;
	double inverse_flattening = 0;
	bool valid = false;


	cCRS(const std::string& datum){
		if (datum == "GDA94"){
			name = datum;
			epsg_string = "EPSG:4283";
			semi_major_axis = 6378137.0;
			inverse_flattening = 298.257222101;
			valid = true;
		}
		else if (datum == "WGS84"){
			name = datum;
			epsg_string = "EPSG:4326";
			semi_major_axis = 6378137.0;
			inverse_flattening = 298.257223563;
			valid = true;
		}
		else if (datum == "AGD66"){
			name = datum;
			epsg_string = "EPSG:4326";
			semi_major_axis = 6378160.0;
			inverse_flattening = 298.25;
			valid = true;
		}
		else{
			name = datum;
		}
	};

	static int epsgcode(const std::string& datumprojection){		
		
		int code;
		if (std::sscanf(datumprojection.c_str(), "EPSG:%d", &code) == 1){
			return code;
		}
		else if (datumprojection == "GDA94|GEODETIC") return 4283;
		else if (datumprojection == "WGS84|GEODETIC") return 4326;		
		else if (datumprojection == "AGD66|GEODETIC") return 4202;
		else if (datumprojection == "AGD84|GEODETIC") return 4203;		
		else if (datumprojection == "GDA94|MGA49") return 28349;
		else if (datumprojection == "GDA94|MGA50") return 28350;
		else if (datumprojection == "GDA94|MGA51") return 28351;
		else if (datumprojection == "GDA94|MGA52") return 28352;
		else if (datumprojection == "GDA94|MGA53") return 28353;
		else if (datumprojection == "GDA94|MGA54") return 28354;
		else if (datumprojection == "GDA94|MGA55") return 28355;
		else if (datumprojection == "GDA94|MGA56") return 28356;		
		else if (datumprojection == "GDA2020|Zone49") return 28349;
		else if (datumprojection == "GDA2020|Zone50") return 28350;
		else if (datumprojection == "GDA2020|Zone51") return 28351;
		else if (datumprojection == "GDA2020|Zone52") return 28352;
		else if (datumprojection == "GDA2020|Zone53") return 28353;
		else if (datumprojection == "GDA2020|Zone54") return 28354;
		else if (datumprojection == "GDA2020|Zone55") return 28355;
		else if (datumprojection == "GDA2020|Zone56") return 28356;
		else return -1;
	}

	std::string wellknowntext() const
	{
		std::string wkt = WellKnownText(epsgcode(epsg_string));
		return wkt;
	}

};

