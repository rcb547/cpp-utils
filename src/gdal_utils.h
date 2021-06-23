/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _gdal_utils_H
#define _gdal_utils_H

#pragma warning( push )  
#pragma warning (disable: 4251)
#include <ogr_spatialref.h>
#include <gdal_priv.h>
#pragma warning( pop )   

int erm2epsgcode(const std::string& datum, const std::string& projection = "GEODETIC");

int getepsgcode(const std::string& datum,
	const std::string& projection = "GEODETIC",
	const std::string& units = "METERS");

OGRSpatialReference getsrs(
	const std::string& datum,
	const std::string& projection,
	const std::string& units
	);

OGRSpatialReference getsrs(const int& epsgcode);
	

bool transform(
	const int& epsgcodein,
	const std::vector<double>& xin,
	const std::vector<double>& yin,
	const int& epsgcodeout,
	std::vector<double>& xout,
	std::vector<double>& yout
	);

std::string WellKnownText(const int epsgcode);

#endif


