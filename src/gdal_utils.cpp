/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#include "gdal_priv.h"
#include <ogr_spatialref.h>

#include "gdal_utils.h"

OGRSpatialReference getsrs(const std::string& datum, const std::string& projection, const std::string& units)
{
	OGRSpatialReference srs;	
	OGRErr err = srs.importFromERM(datum.c_str(), projection.c_str(), units.c_str());
	return srs;
}

OGRSpatialReference getsrs(const int epsgcode)
{
	OGRSpatialReference srs;
	OGRErr err = srs.importFromEPSG(epsgcode);	
	return srs;
}


bool transform(
	const int& epsgcodein,
	const std::vector<double>& xin,
	const std::vector<double>& yin,
	const int& epsgcodeout,
	std::vector<double>& xout,
	std::vector<double>& yout)
{
	GDALAllRegister();
	OGRErr err;
	OGRSpatialReference inSRS, outSRS;
	err = inSRS.importFromEPSG(epsgcodein);
	err = outSRS.importFromEPSG(epsgcodeout);	

	xout = xin;
	yout = yin;

	OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(&inSRS, &outSRS);
	if (poCT == NULL){
		printf("Transformation creation failed.\n");
		return false;
	}

	if (!poCT->Transform(xout.size(), xout.data(), yout.data())){
		printf("Transformation operation failed.\n");
		return false;
	}

	return true;
}
