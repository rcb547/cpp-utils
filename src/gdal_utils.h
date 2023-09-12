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
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#pragma warning( pop )   

#include <valarray>
#include "stopwatch.h"

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

std::ostream& operator<<(std::ostream& os, GDALDataset* dataset) {

	os << "Driver: " <<
		dataset->GetDriver()->GetDescription() << "/" <<
		dataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME) << std::endl;

	os << "Description: " << dataset->GetDescription() << std::endl;

	os << "Size: " <<
		dataset->GetRasterXSize() << "x" <<
		dataset->GetRasterYSize() << "x" <<
		dataset->GetRasterCount() << std::endl;

	if (dataset->GetProjectionRef() != NULL) {
		std::cout << "Projection is " << dataset->GetProjectionRef() << std::endl;
	}
	else {
		std::cout << "Projection is not NULL" << std::endl;
	}
	return os;
}

class PixelSubset {
public:
	int nXOff;
	int nYOff;
	int nXSize;
	int nYSize;
	int nBufXSize;
	int nBufYSize;
};

class GDALHelper {

private:

public:

	GDALHelper() {
		GDALAllRegister();
	};

	static void report_basic_envs() {
		report_env("GDAL_CONFIG_FILE");
		report_env("GDAL_ROOT");
		report_env("GDAL_DATA");
		report_env("GDAL_CACHEMAX");
		report_env("GDAL_NUM_THREADS");
		report_env("GDAL_FORCE_CACHING");
		report_env("GDAL_MAX_DATASET_POOL_RAM_USAGE");
		report_env("GDAL_DEFAULT_WMS_CACHE_PATH");
	};

	static void report_env(const std::string& key) {
		std::cout << key << " = " << get_env(key) << std::endl;
	};

	static std::string get_env(const std::string& key) {
		std::string value(CPLGetConfigOption(key.c_str(), ""));
		return value;
	}

	void set_env(const std::string& key, const std::string& value) {
		CPLSetConfigOption(key.c_str(), value.c_str());
	};

	template <class T, std::enable_if_t<std::is_same_v<T, std::uint8_t>, bool> = false>
	GDALDataType GetGDALDataType(const T& x) {
		return GDALDataType::GDT_Byte;
	}

	template <class T, std::enable_if_t<std::is_same_v<T, int>, bool> = false>
	GDALDataType GetGDALDataType(const T& x) {
		return GDALDataType::GDT_Int32;
	}

	const PixelSubset get_pixelsubset(GDALDataset* dataset, double x1, double y1, double x2, double y2, const int& nxsize, const int& nysize) {

		PixelSubset ps;
		ps.nBufXSize = nxsize;
		ps.nBufYSize = nysize;

		const OGRSpatialReference* pInSRS = dataset->GetSpatialRef();

		OGRSpatialReference OutSRS;
		OGRErr err = OutSRS.importFromEPSG(7843);
		OutSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

		OGRCoordinateTransformation* pCT = OGRCreateCoordinateTransformation(&OutSRS, pInSRS);
		if (pCT == NULL) {
			std::cout << "Transformation creation failed." << std::endl;
			return ps;
		}

		pCT->Transform(1, &x1, &y1);
		pCT->Transform(1, &x2, &y2);

		double pixeltogeo[6];
		double geotopixel[6];
		dataset->GetGeoTransform(pixeltogeo);
		GDALInvGeoTransform(pixeltogeo, geotopixel);

		double px1, px2, py1, py2;
		GDALApplyGeoTransform(geotopixel, x1, y1, &px1, &py1);
		GDALApplyGeoTransform(geotopixel, x2, y2, &px2, &py2);
		ps.nXOff = px1;
		ps.nYOff = py1;
		ps.nXSize = px2 - px1;
		ps.nYSize = py2 - py1;
		return ps;
	};

	template <typename T>
	bool get_subset(GDALDataset* dataset, const PixelSubset& ps, std::valarray<T>& buffer) {

		const GDALDataType eBufType = GDALDataType::GDT_Byte;

		const GDALRWFlag eRWFlag = GDALRWFlag::GF_Read;
		const GSpacing nPixelSpace = 0;
		const GSpacing nLineSpace = 0;
		const GSpacing nBandSpace = 0;
		const int nBandCount = 3;
		int panBandMap[3] = { -1, -1, -1 };

		const int nsrcbands = dataset->GetRasterCount();
		for (int i = 0; i < nsrcbands; i++) {
			GDALRasterBand* band = dataset->GetRasterBand(i + 1);
			GDALColorInterp ci = band->GetColorInterpretation();
			if (ci == GCI_RedBand) {
				panBandMap[0] = i + 1;
			}
			else if (ci == GCI_GreenBand) {
				panBandMap[1] = i + 1;
			}
			else if (ci == GCI_BlueBand) {
				panBandMap[2] = i + 1;
			}
		}

		int len = ps.nBufXSize * ps.nBufYSize * nBandCount;
		if (buffer.size() != len) {
			buffer.resize(len);
		}

		CPLErr err = dataset->RasterIO(eRWFlag, ps.nXOff, ps.nYOff, ps.nXSize, ps.nYSize, &(buffer[0]), ps.nBufXSize, ps.nBufYSize, eBufType, nBandCount, panBandMap, nPixelSpace, nLineSpace, nBandSpace, nullptr);
		if (err != CE_None) {
			std::string msg(CPLGetLastErrorMsg());
			std::cout << msg << std::endl;
			CPLError(err, CPLGetLastErrorNo(), "MyError");
		}
		return true;
	};

	int test_wmts_subset()
	{

		// See index https://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer

		//std::string wmts = "WMTS:https://gibs.earthdata.nasa.gov/wmts/epsg4326/best/1.0.0/WMTSCapabilities.xml,layer=BlueMarble_NextGeneration";
		//std::string wmts = "WMTS:http://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/WMTS/1.0.0/WMTSCapabilities.xml";
		std::string wmts = "g:\\wmts\\world_imagery.xml";

		GDALDataset* dataset = (GDALDataset*)GDALOpen(wmts.c_str(), GA_ReadOnly);
		if (dataset == nullptr) {
			std::cout << "Null dataset returned" << std::endl;
			return false;
		}

		std::cout << dataset;

		double x1, x2, y1, y2;
		x1 = 141; x2 = 142;
		y1 = -34; y2 = -35;

		double x0 = 149.1300;
		double y0 = -35.2809;

		x0 = 149.14511;
		y0 = -35.3288;
		double delta = 0.001;
		x1 = x0 - delta; x2 = x0 + delta;
		y1 = y0 + delta; y2 = y0 - delta;

		int nx = 850;
		int ny = nx;
		PixelSubset ps = get_pixelsubset(dataset, x1, y1, x2, y2, nx, ny);

		std::valarray<std::uint8_t> buffer;

		cStopWatch sw;
		sw.reset();
		get_subset(dataset, ps, buffer);
		sw.reportnow();

		std::ofstream ostrm("g:\\wmts\\wmts1.txt");
		for (int k = 0; k < buffer.size(); k++) {
			ostrm << (int)buffer[k] << std::endl;
		}

		return 0;
	};

};
#endif


