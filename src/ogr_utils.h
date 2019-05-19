/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef shape_utils_H
#define shape_utils_H

#include <vector>
#include <variant>
#include "gdal.h"
#include "ogrsf_frmts.h"
#include "stacktrace.h"

class cGeoDataset;//forward declaration

class cAttribute{

private:
	

public:
	std::string name;
	std::variant<std::string, int, double> value;

	cAttribute() {};

	cAttribute(const std::string& _name, const std::string& _value) {
		name  = _name;
		value = _value;
	};

	cAttribute(const std::string& _name, const int& _value) {
		name  = _name;
		value = _value;
	};

	cAttribute(const std::string& _name, const double& _value) {
		name  = _name;
		value = _value;
	};

	size_t index() const { return value.index(); }

	OGRFieldType fieldtype() const {
		if (value.index() == 0) return OFTString;
		else if (value.index() == 1) return OFTInteger;
		else if (value.index() == 2) return OFTReal;
	}

	std::string fieldtypename() const {		
		const char* tn = OGRFieldDefn::GetFieldTypeName(fieldtype());
		return std::string(tn);
	}

	void set(OGRFeature* poFeature) const {
		if (index() == 0) {
			poFeature->SetField(name.c_str(), (std::get<std::string>(value)).c_str());
		}
		else if(index() == 1) poFeature->SetField(name.c_str(), std::get<int>(value));
		else if(index() == 2) poFeature->SetField(name.c_str(), std::get<double>(value));
	};

	void get(OGRFeature* poFeature) {		
		if (index() == 0) value = poFeature->GetFieldAsString(name.c_str());
		else if (index() == 1) value = poFeature->GetFieldAsInteger(name.c_str());
		else if (index() == 2) value = poFeature->GetFieldAsDouble(name.c_str());		
	};

	static void fill(std::vector<cAttribute>& attributes, OGRFeature* poFeature){		
		for (auto i = 0; i < attributes.size(); i++){
			attributes[i].get(poFeature);
		}
	};

	void print() const {
		std::cout << name << " : " << fieldtypename() << " : ";
		if (index() == 0) std::cout << std::get<std::string>(value);
		else if (index() == 1) std::cout << std::get<int>(value);
		else if (index() == 2) std::cout << std::get<double>(value);
		std::cout << std::endl;
	}

};

class cFeature : public OGRFeature {

public:
	
	void getAttributes(std::vector<cAttribute>& attributes)
	{		
		cAttribute::fill(attributes, this);		
	}
};

class cPointFeature : public cFeature {

public:
	
	bool getGeometry(double& x, double& y)
	{
		OGRGeometry* poGeometry = GetGeometryRef();
		if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
			OGRPoint* p = poGeometry->toPoint();
			x = p->getX();
			y = p->getY();
			return true;
		}
		return false;
	}

	bool get(std::vector<cAttribute>& attributes, double& x, double& y) {
		getAttributes(attributes);
		getGeometry(x, y);
		return true;
	}

};

class cLineStringFeature : public cFeature {

public:

	bool getGeometry(std::vector<double>& x, std::vector<double>& y)
	{		
		OGRGeometry* poGeometry = GetGeometryRef();
		if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
			OGRLineString*  l = poGeometry->toLineString();
			int np = l->getNumPoints();
			x.resize(np);
			y.resize(np);
			int i = 0;
			for (auto&& p : *l) {
				x[i] = p.getX();
				y[i] = p.getY();
				i++;
			}	
			return true;
		}	
		return false;
	}

	bool get(std::vector<cAttribute>& attributes, std::vector<double>& x, std::vector<double>& y) {
		getAttributes(attributes);
		getGeometry(x, y);
		return true;
	}

};

class cPolygonFeature : public cFeature {

public:

	bool getGeometry(std::vector<double>& x, std::vector<double>& y)
	{
		OGRGeometry* poGeometry = GetGeometryRef();
		if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {						
			OGRLinearRing* l = poGeometry->toPolygon()->getExteriorRing();
			int np = l->getNumPoints();
			x.resize(np);
			y.resize(np);			
			int i = 0;
			for (auto&& p : *l) {
				x[i] = p.getX();
				y[i] = p.getY();
				i++;
			}	
			return true;
		}
		return false;
	}

	bool get(std::vector<cAttribute>& attributes, std::vector<double>& x, std::vector<double>& y) {
		getAttributes(attributes);
		getGeometry(x, y);
		return true;
	}
};

class  cLayer : public OGRLayer {

private:
	
public:
		
	std::vector<cAttribute> get_fields() {
		
		OGRFeatureDefn& ld = *GetLayerDefn();
		int n = ld.GetFieldCount();
		std::vector<cAttribute> a(n);
		for (size_t i = 0; i < n; i++) {
			OGRFieldDefn& fd = *ld.GetFieldDefn(i);			
			a[i].name = std::string(fd.GetNameRef());
			if (fd.GetType() == OFTString) a[i].value = std::string();
			else if (fd.GetType() == OFTInteger) a[i].value = (int)0;
			else if (fd.GetType() == OFTReal) a[i].value = (double)0.0;
		}


		return a;
	}

	void add_field(const cAttribute& a) {	
		OGRFieldDefn oField(a.name.c_str(), a.fieldtype());				
		const char* tn  = oField.GetFieldTypeName(a.fieldtype());
		if (a.fieldtype() == OFTString) oField.SetWidth(32);						
		if (CreateField(&oField) != OGRERR_NONE){
			printf("Creating field %s (%s) failed.\n", a.name.c_str(),tn);
		}
	}

	void add_fields(const std::vector<cAttribute>& atts) {
		for (auto i = 0; i < atts.size(); i++) {
			add_field(atts[i]);
		}
	}
	
	void add_point_feature(const std::vector<cAttribute> a, const double& x, const double& y)
	{
		OGRFeature *poFeature;
		poFeature = OGRFeature::CreateFeature(GetLayerDefn());
		
		for (auto i = 0; i < a.size(); i++) {
			a[i].set(poFeature);
			a[i].print();
		}
		
		poFeature->SetGeometry(&OGRPoint(x, y));
		if (CreateFeature(poFeature) != OGRERR_NONE){
			printf("Failed to create point feature in shapefile.\n");
			exit(1);
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	void add_linestring_feature(const std::vector<cAttribute> a, const std::vector<double>& x, const std::vector<double>& y)
	{			
		OGRFeature *poFeature;
		poFeature = OGRFeature::CreateFeature(GetLayerDefn());
		for (auto i = 0; i < a.size(); i++) {
			a[i].set(poFeature);
		}

		OGRLineString ls;
		ls.setPoints(x.size(), x.data(), y.data());
		poFeature->SetGeometry(&ls);
		if (CreateFeature(poFeature) != OGRERR_NONE){
				printf("Failed to create line feature in shapefile.\n");
				exit(1);
		}
		OGRFeature::DestroyFeature(poFeature);		
	}
	
	void add_polygon_feature(const std::vector<cAttribute> a, const std::vector<double>& x, const std::vector<double>& y)
	{
		OGRFeature *poFeature;
		poFeature = OGRFeature::CreateFeature(GetLayerDefn());
		for (auto i = 0; i < a.size(); i++) {
			a[i].set(poFeature);
		}

		OGRLinearRing c;
		c.setPoints(x.size(), x.data(), y.data());

		OGRPolygon p;
		p.addRing(&c);		
		poFeature->SetGeometry(&p);
		if (CreateFeature(poFeature) != OGRERR_NONE){
			printf("Failed to create line feature in shapefile.\n");
			exit(1);
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	//cLineStringFeature get_linestring_feature(OGRFeature* poFeature, const std::vector<cAttribute>& attributes)
	//{
	//	return cLineStringFeature(poFeature, attributes);		
	//}	
};

class  cGeoDataset : public GDALDataset {
	
private:	

	static GDALDriver* get_driver(const std::string DriverName)
	{		
		int nd = GetGDALDriverManager()->GetDriverCount();
		for (int i = 0; i < nd; i++) {
			GDALDriver* poDriver = GetGDALDriverManager()->GetDriver(i);
			std::cout << poDriver->GetDescription() << std::endl;
		}

		GDALDriver* poDriver;
		poDriver = GetGDALDriverManager()->GetDriverByName(DriverName.c_str());
		if (poDriver == NULL) {
			glog.errormsg(_SRC_, "%s driver not available.\n", DriverName.c_str());
		}
		return poDriver;
	}

public:			
	
	static cGeoDataset* open_existing(const std::string shapepath) {
		_GSTITEM_		
		cGeoDataset* poDS = (cGeoDataset*)GDALDataset::Open(shapepath.c_str(), GDAL_OF_VECTOR);		
		return (cGeoDataset*)poDS;
	}

	cGeoDataset* create_shapefile(const std::string shapepath) {
		_GSTITEM_		
		GDALDriver* poDriver = get_driver("ESRI Shapefile");				
		cGeoDataset* poDS = (cGeoDataset*) poDriver->Create(shapepath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
		if (poDS == NULL){
			glog.errormsg(_SRC_,"Creation of output file failed.\n");			
		}				
		return poDS;		
	}

	int  nlayers(){ return GetLayerCount(); }

	std::vector<std::string> filelist() {
		std::vector<std::string> list;
		char** fl = GetFileList();
		int k = 0;
		while (fl[k] != NULL) {
			list.push_back(std::string(fl[k]));
			k++;
		}
		return list;		
	}
	
	cLayer* create_layer(const std::string& layername, OGRwkbGeometryType layertype){
		OGRSpatialReference srs;
		srs.importFromEPSG(4283);				
		cLayer* poLayer = (cLayer*)CreateLayer(layername.c_str(), &srs, layertype, NULL);
		if (poLayer == NULL) {
			printf("Layer creation failed.\n");
			exit(1);
		}
		//return cLayer(poLayer);		
		return poLayer;
	}

};

#endif




