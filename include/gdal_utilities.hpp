/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#pragma warning(push)
#pragma warning (disable: 4251)
#include <ogr_spatialref.h>
#include <gdal_priv.h>
#pragma warning(pop)

inline void checkogrerror(const OGRErr& err)
{
	if(err != OGRERR_NONE){
		std::printf("Warning OGRErr %d\n",err);
	}
}

inline int erm2epsgcode(const std::string& datum, const std::string& projection = "GEODETIC"){	
	if (datum == "GDA94" && projection == "GEODETIC") return 4283;
	else if (datum == "WGS84" && projection == "GEODETIC") return 4326;	
	else if (datum == "AGD66" && projection == "GEODETIC") return 4202;
	else if (datum == "AGD84" && projection == "GEODETIC") return 4203;
	else if (datum == "GDA94" && projection == "MGA49") return 28349;
	else if (datum == "GDA94" && projection == "MGA50") return 28350;
	else if (datum == "GDA94" && projection == "MGA51") return 28351;
	else if (datum == "GDA94" && projection == "MGA52") return 28352;
	else if (datum == "GDA94" && projection == "MGA53") return 28353;
	else if (datum == "GDA94" && projection == "MGA54") return 28354;
	else if (datum == "GDA94" && projection == "MGA55") return 28355;
	else if (datum == "GDA94" && projection == "MGA56") return 28356;
	else return -1;
}

inline int getepsgcode(const std::string& datum, const std::string& projection = "GEODETIC", const std::string& units = "METERS"){
	OGRSpatialReference srs;	
	//OGRErr err   = srs.importFromERM(projection.c_str(), datum.c_str(), units.c_str());
	//This seem to not work
	//int epsgcode = srs.GetEPSGGeogCS();
	int epsgcode = erm2epsgcode(datum, projection);
	return epsgcode;
}

inline OGRSpatialReference getsrs(const std::string& datum, const std::string& projection, const std::string& units)
{
	OGRSpatialReference srs;	
	OGRErr err = srs.importFromERM(projection.c_str(), datum.c_str(), units.c_str());
	checkogrerror(err);
	return srs;
}

inline OGRSpatialReference getsrs(const int& epsgcode)
{
	OGRSpatialReference srs;
	OGRErr err = srs.importFromEPSG(epsgcode);	
	checkogrerror(err);
	return srs;
}

inline std::string WellKnownText(const int epsgcode)
{
	OGRSpatialReference srs;
	OGRErr err = srs.importFromEPSG(epsgcode);
	checkogrerror(err);
	char* wkt = NULL;
	err = srs.exportToWkt(&wkt);
	checkogrerror(err);
	return std::string(wkt);
}


inline bool transform(
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
	checkogrerror(err);
	err = outSRS.importFromEPSG(epsgcodeout);	
	checkogrerror(err);

	//Note !poCT->Transform below seems to return lon as y and lat as x
	//hende swapped here.  Not sure if it were a EN to EN transform
	xout = xin;
	yout = yin;

	OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(&inSRS, &outSRS);
	if (poCT == NULL){
		printf("Transformation creation failed.\n");
		return false;
	}

	
	if (!poCT->Transform((int) xout.size(), xout.data(), yout.data())){
		printf("Transformation operation failed.\n");
		return false;
	}

	return true;
}

