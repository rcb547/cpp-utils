//2D implementation of the Ramer-Douglas-Peucker algorithm
//By Tim Sheerman-Chase, 2016
//Released under CC0
//https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm

//Downloaded from https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
//Modified to split into .cpp and .h files

#include <utility>
#include <vector>

#ifndef _RamerDouglasPeucker_H
#define _RamerDouglasPeucker_H

namespace RDP{
	typedef std::pair<double, double> Point;
	void RamerDouglasPeucker(const std::vector<Point> &pointList, double epsilon, std::vector<Point> &out);
};

#endif