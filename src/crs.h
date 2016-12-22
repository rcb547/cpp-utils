/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _crs_H
#define _crs_H

class cCRS{

public:
	std::string name = "";
	std::string epsg_code = "";
	double semi_major_axis = 0;
	double inverse_flattening = 0;
	bool valid = false;

	cCRS(std::string datum){
		if (datum == "GDA94"){
			name = datum;
			epsg_code = "EPSG:4283";
			semi_major_axis = 6378137.0;
			inverse_flattening = 298.257222101;
			valid = true;
		}
		else if (datum == "WGS84"){
			name = datum;
			epsg_code = "EPSG:4326";
			semi_major_axis = 6378137.0;
			inverse_flattening = 298.257223563;
			valid = true;
		}
		else if (datum == "AGD66"){
			name = datum;
			epsg_code = "EPSG:4326";
			semi_major_axis = 6378160.0;
			inverse_flattening = 298.25;
			valid = true;
		}
		else{
			name = datum;
		}
	};

};

#endif

