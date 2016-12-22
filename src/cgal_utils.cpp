
#  pragma warning (push)
#  pragma warning (disable : 4127)
#  pragma warning (disable : 4191)
#  pragma warning (disable : 4242)
#  pragma warning (disable : 4244)
#  pragma warning (disable : 4365)
#  pragma warning (disable : 4371)
#  pragma warning (disable : 4512)
#  pragma warning (disable : 4571)
#  pragma warning (disable : 4619)
#  pragma warning (disable : 4640)
#  pragma warning (disable : 4702)
#  pragma warning (disable : 4625)
#  pragma warning (disable : 4626)
#  pragma warning (disable : 4365)

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/algorithm.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polyline_simplification_2/simplify.h>
#include <CGAL/Polyline_simplification_2/Stop_below_count_threshold.h>
#include <CGAL/Polyline_simplification_2/Stop_above_cost_threshold.h>
#  pragma warning (pop)

#include <vector>
#include <list>
#include "cgal_utils.h"
#include "general_utils.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::FT FT;
typedef K::Point_2 Point;
typedef K::Segment_2 Segment;
typedef CGAL::Alpha_shape_vertex_base_2<K> Vb;
typedef CGAL::Alpha_shape_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
typedef CGAL::Delaunay_triangulation_2<K,Tds> Triangulation_2;
typedef CGAL::Alpha_shape_2<Triangulation_2> Alpha_shape_2;
typedef Alpha_shape_2::Alpha_shape_edges_iterator Alpha_shape_edges_iterator;

typedef CGAL::Polygon_2<K> Polygon;
namespace PS = CGAL::Polyline_simplification_2;

bool line_data_alpha_shape_polygon_ch(
	const std::vector<unsigned int>& line_index_start,
	const std::vector<unsigned int>& line_index_count,
	const std::vector<double>& x,
	const std::vector<double>& y,
	const double nullx,
	const double nully,	
	size_t maxvertices,
	std::vector<double>& px,
	std::vector<double>& py)
{	
	double minsepmetres = 100.0;
	double minsepdeg    = minsepmetres / 30.0 / 3600.0;//assumes 1 arc second is 30m.

	std::list<Point> points;
	size_t nl = line_index_start.size();
	for (size_t li = 0; li < nl; li++){	
		size_t ns = line_index_count[li];
		size_t start = line_index_start[li];		
		std::list<Point> plist;		
		double lastx = 0.0;
		double lasty = 0.0;
		for (size_t si = 0; si < ns; si++){
			const double xp = x[start + si];
			const double yp = y[start + si];
			if (xp == nullx || yp == nully)continue;			
			
			double d = std::sqrt((xp - lastx)*(xp - lastx) + (yp - lasty)*(yp - lasty));
			if (si==0 || si==(ns-1) || d >= minsepdeg){
				plist.push_back(Point(xp, yp));
				lastx = xp;
				lasty = yp;
			}			
		}				
		//std::list<Point> hull;		
		//CGAL::convex_hull_2(plist.begin(), plist.end(), std::inserter(hull, hull.begin()));		
		points.splice(points.end(), plist);
	}
	//printf("Number of filtered points = %lu\n", points.size());

	Alpha_shape_2 A(points.begin(), points.end(), Alpha_shape_2::REGULARIZED);
	auto ait = A.find_optimal_alpha(1);
	double r = 1.5*sqrt(*ait);
	A.set_alpha(r*r);
	
	int nsc = A.number_of_solid_components();
	//printf("alpha radius=%lf\n", r);
	//printf("Number of solid components=%d\n", nsc);
	
	//printf("Ordering edges\n");	
	std::list<Segment> src;
	for (auto it = A.alpha_shape_edges_begin(); it != A.alpha_shape_edges_end(); ++it){				
		if (A.classify(*it) == Alpha_shape_2::Classification_type::REGULAR){
			src.push_back(A.segment(*it));
		}
	}

	//Move first of src into dst
	std::list<Segment> dst;
	dst.splice(dst.end(), src, src.begin());

	//Empty rest of src into dst in order	
	Point p = dst.back().target();
	while (src.size() > 0){
		size_t n1 = src.size();
		for (auto it = src.begin(); it != src.end(); ++it){
			if ((*it).source() == p){
				dst.splice(dst.end(), src, it);
				p = dst.back().target();
				break;
			}
		}
		if (n1 == src.size()){
			printf("Could not empty source list\n");			
			break;
		}
	}
	
	//Create a polygon
	Polygon polygon;
	for (auto it = dst.begin(); it != dst.end(); ++it){
		double vx = (*it).source().x();
		double vy = (*it).source().y();
		Point p = Point(vx, vy);
		//if (A.classify(p) == Alpha_shape_2::Classification_type::INTERIOR)continue;
		//if (A.classify(p) == Alpha_shape_2::Classification_type::EXTERIOR)continue;
		//if (A.classify(p) == Alpha_shape_2::Classification_type::SINGULAR)continue;
		polygon.push_back(p);
	}
	size_t nhull = polygon.container().size();
	//printf("Number of hull vertices = %lu\n", nhull);

	if (true){
		for (r = 10; r <= 5000; r = r + 10){
			double thr_m = r;
			double thr_d = thr_m / (3600.0 * 30.0);
			PS::Stop_above_cost_threshold stop(thr_d*thr_d);
			PS::Squared_distance_cost cost;
			//PS::Scaled_squared_distance_cost cost;
			Polygon simple = polygon;
			simple = PS::simplify(simple, cost, stop);
			//printf("%lf %lu\n", r, simple.container().size());
			polygon = simple;
			if (simple.container().size() <= 64){
				break;
			}
		}
	}

	//printf("Building polygon\n");
	px.resize(polygon.container().size());
	py.resize(polygon.container().size());
	size_t k = 0;
	for (auto it = polygon.vertices_begin(); it != polygon.vertices_end(); ++it){		
		px[k] = (*it).x();
		py[k] = (*it).y();
		k++;
	}		
	return true;
}






