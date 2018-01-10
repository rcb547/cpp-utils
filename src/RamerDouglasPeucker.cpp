//2D implementation of the Ramer-Douglas-Peucker algorithm
//By Tim Sheerman-Chase, 2016
//Released under CC0
//https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm

//Downloaded from https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
//Modified to split into .cpp and .h files

#include <cmath>
#include <utility>
#include <vector>
#include <stdexcept>

#include "RamerDouglasPeucker.h"

namespace RDP{

	double PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd)
	{
		double dx = lineEnd.first - lineStart.first;
		double dy = lineEnd.second - lineStart.second;

		//Normalise
		double mag = std::pow(std::pow(dx, 2.0) + std::pow(dy, 2.0), 0.5);
		if (mag > 0.0)
		{
			dx /= mag; dy /= mag;
		}

		double pvx = pt.first - lineStart.first;
		double pvy = pt.second - lineStart.second;

		//Get dot product (project pv onto normalized direction)
		double pvdot = dx * pvx + dy * pvy;

		//Scale line direction vector
		double dsx = pvdot * dx;
		double dsy = pvdot * dy;

		//Subtract this from pv
		double ax = pvx - dsx;
		double ay = pvy - dsy;

		return std::pow(std::pow(ax, 2.0) + std::pow(ay, 2.0), 0.5);
	}

	void RamerDouglasPeucker(const std::vector<Point> &pointList, double epsilon, std::vector<Point> &out)
	{
		if (pointList.size() < 2)
			throw std::invalid_argument("Not enough points to simplify");

		// Find the point with the maximum distance from line between start and end
		double dmax = 0.0;
		size_t index = 0;
		size_t end = pointList.size() - 1;
		for (size_t i = 1; i < end; i++)
		{
			double d = PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
			if (d > dmax)
			{
				index = i;
				dmax = d;
			}
		}

		// If max distance is greater than epsilon, recursively simplify
		if (dmax > epsilon)
		{
			// Recursive call
			std::vector<Point> recResults1;
			std::vector<Point> recResults2;
			std::vector<Point> firstLine(pointList.begin(), pointList.begin() + index + 1);
			std::vector<Point> lastLine(pointList.begin() + index, pointList.end());
			RamerDouglasPeucker(firstLine, epsilon, recResults1);
			RamerDouglasPeucker(lastLine, epsilon, recResults2);

			// Build the result list
			out.assign(recResults1.begin(), recResults1.end() - 1);
			out.insert(out.end(), recResults2.begin(), recResults2.end());
			if (out.size() < 2)
				throw std::runtime_error("Problem assembling output");
		}
		else
		{
			//Just return start and end points
			out.clear();
			out.push_back(pointList[0]);
			out.push_back(pointList[end]);
		}
	}

};

	/*
	int main()
	{
	vector<Point> pointList;
	vector<Point> pointListOut;

	pointList.push_back(Point(0.0, 0.0));
	pointList.push_back(Point(1.0, 0.1));
	pointList.push_back(Point(2.0, -0.1));
	pointList.push_back(Point(3.0, 5.0));
	pointList.push_back(Point(4.0, 6.0));
	pointList.push_back(Point(5.0, 7.0));
	pointList.push_back(Point(6.0, 8.1));
	pointList.push_back(Point(7.0, 9.0));
	pointList.push_back(Point(8.0, 9.0));
	pointList.push_back(Point(9.0, 9.0));

	RamerDouglasPeucker(pointList, 1.0, pointListOut);

	cout << "result" << endl;
	for(size_t i=0;i< pointListOut.size();i++)
	{
	cout << pointListOut[i].first << "," << pointListOut[i].second << endl;
	}

	return 0;
	}
	*/
