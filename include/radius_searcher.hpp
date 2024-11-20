/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <vector>
#include "vector_utils.hpp"

class cRadiusSearcherTile{

public:
	std::vector<size_t> pindex;

};

class cRadiusSearcher{

public:
	std::vector<double> x;
	std::vector<double> y;
	std::vector<double> elevation;
	double radius;
	double radiussquared;

	size_t nxtiles;
	size_t nytiles;
	std::vector<std::vector<cRadiusSearcherTile>> tiles;
	std::vector<bool> pointincluded;
	std::vector<size_t> pointrank;

	double x1;
	double x2;
	double y1;
	double y2;

	cRadiusSearcher(){};

	cRadiusSearcher(const std::vector<double>& _x, const std::vector<double>& _y, const std::vector<double>& _elevation, const double& _radius){
		x = _x;
		y = _y;
		elevation = _elevation;
		initialise(_radius);
	};

	void initialise(double _radius){
		radius = _radius;
		radiussquared = radius*radius;

		size_t np = x.size();
		x1 = min(x);
		x2 = max(x);
		y1 = min(y);
		y2 = max(y);
		nxtiles = (size_t)ceil((x2 - x1) / radius);
		nytiles = (size_t)ceil((y2 - y1) / radius);

		tiles.resize(nxtiles);
		for (size_t i = 0; i < nxtiles; i++){
			tiles[i].resize(nytiles);
		}

		for (size_t pi = 0; pi < np; pi++){
			tiles[ixt(x[pi])][iyt(y[pi])].pindex.push_back(pi);
		}
	};

	size_t ixt(const double& px){
		return (size_t)((px - x1) / radius);
	}

	size_t iyt(const double& py){
		return (size_t)((py - y1) / radius);
	}

	void getsearchtilerange(const double& px, const double& py, size_t& tx1, size_t& tx2, size_t& ty1, size_t& ty2){
		size_t tx = ixt(px);
		if (tx <= 0)tx1 = 0;
		else tx1 = tx - 1;
		if (tx >= nxtiles - 1)tx2 = nxtiles - 1;
		else tx2 = tx + 1;

		size_t ty = iyt(py);
		if (ty <= 0)ty1 = 0;
		else ty1 = ty - 1;
		if (ty >= nytiles - 1)ty2 = nytiles - 1;
		else ty2 = ty + 1;
		return;
	}

	std::vector<size_t> findneighbourstopoint(const double& px, const double& py, std::vector<double>& distances, double maxdistance = -1.0){

		if (maxdistance < 0) maxdistance = radius;
		double maxdistancesquared = maxdistance*maxdistance;

		std::vector<size_t> neighbours;
		distances.resize(0);

		size_t tx1, tx2, ty1, ty2;
		getsearchtilerange(px, py, tx1, tx2, ty1, ty2);
		for (size_t ix = tx1; ix <= tx2; ix++){
			for (size_t iy = ty1; iy <= ty2; iy++){
				size_t n = tiles[ix][iy].pindex.size();
				for (size_t j = 0; j < n; j++){
					size_t k = tiles[ix][iy].pindex[j];
					double dx = x[k] - px;
					double dy = y[k] - py;
					double r2 = dx*dx + dy*dy;
					if (r2 <= maxdistancesquared){
						neighbours.push_back(k);
						distances.push_back(std::sqrt(r2));
					}
				}
			}
		}
		return neighbours;
	}

	std::vector<size_t> findneighbours(const size_t index, std::vector<double>& distances, double maxdistance = -1.0){
		
		std::vector<size_t> n;
		std::vector<double> d;
		n = findneighbourstopoint(x[index], y[index], d, maxdistance);
		
		std::vector<size_t> neighbours(n.size()-1);
		distances.resize(n.size() - 1);
		size_t k = 0;
		for (size_t i = 0; i < n.size(); i++){
			if (n[i] != index){
				neighbours[k] = n[i];
				distances[k]  = d[i];
				k++;
			}
		}
		return neighbours;
	}

};

