/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <ctime>
#include <iterator>
#include <sstream>
#include <vector>
#include <filesystem>

#include "string_print.hpp"
#include "logger.hpp"
#include "general_constants.hpp"
#include "general_types.hpp"
#include "string_utils.hpp"

#if defined _WIN32
#define NOMINMAX 
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <errno.h>
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef ENABLE_MPI
#include "mpi_wrapper.hpp"
#endif

#ifdef MATLAB_MEX_FILE
#include "mex.h"
#endif

inline std::string commandlinestring(int argc, char** argv) {
	std::string str = "Executing:";
	for (int i = 0; i < argc; i++) {
		str += strprint(" %s", argv[i]);
	}
	return str;
};

inline std::string versionstring(const std::string& version, const std::string& compiletime, const std::string& compiledate) {
	std::string s = strprint("Version: %s Compiled at %s on %s", version.c_str(), compiletime.c_str(), compiledate.c_str());
	return s;
};

inline int my_size() {
#ifdef ENABLE_MPI
	if (cMpiEnv::isinitialised()) {
		return cMpiEnv::world_size();
	}
	else return 1;
#else
	return 1;
#endif
};

inline int my_rank() {
	int threadnum = 0;
#if defined _OPENMP
	threadnum = omp_get_thread_num();
#endif

#ifdef ENABLE_MPI
	if (cMpiEnv::isinitialised()) {
		return cMpiEnv::world_rank();
	}
	else return threadnum;
#else
	return threadnum;
#endif
}

inline int mpi_openmp_rank() {
	int rank = 0;

#if defined _OPENMP
	rank = omp_get_thread_num();
	if (rank > 0)return rank;
#endif

#ifdef ENABLE_MPI
	if (cMpiEnv::isinitialised()) {
		rank = cMpiEnv::world_rank();
		return rank;
	}
#endif
	return rank;
}

inline void rb_sleep(double secs)
{

#if defined _WIN32
	int ms = (int)(secs * 1000);
	Sleep((DWORD)ms);
#else        
	int s = (int)(secs);
	sleep(s);
#endif
}

inline void debug(const char* msg)
{
	glog.logmsg("Debug: %s\n", msg);
}

inline void prompttocontinue()
{

#if defined MATLAB_MEX_FILE

#else
	std::printf("Press enter to continue...\n");
	std::getchar();
#endif
	return;
}

inline void prompttoexit()
{
#if defined MATLAB_MEX_FILE		
	mexErrMsgTxt("Error");
#else
	std::printf("Press enter to exit...\n");
	std::getchar();
	exit(0);
#endif
	return;
}

inline bool wildcmp(const char* wildpattern, const char* stringpattern)
{
	char* wild = new char[strlen(wildpattern) + 1];
	strcpy(wild, wildpattern);
	char* owild = wild;//oringinal pointer for delete

	char* str = new char[strlen(stringpattern) + 1];
	strcpy(str, stringpattern);
	char* ostring = str;//oringinal pointer for delete


	while ((*str) && (*wild != '*')) {
		if ((*wild != *str) && (*wild != '?')) {
			delete owild;
			delete ostring;
			return false;
		}
		wild++;
		str++;
	}

	char* cp = (char*)NULL;
	char* mp = (char*)NULL;
	while (*str) {
		if (*wild == '*') {
			if (!*++wild) {
				delete owild;
				delete ostring;
				return true;
			}
			mp = wild;
			cp = str + 1;
		}
		else if ((*wild == *str) || (*wild == '?')) {
			wild++;
			str++;
		}
		else {
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	bool rval = (bool)!*wild;
	delete owild;
	delete ostring;
	return rval;
}

inline double correlation_coefficient(std::vector<double>x, std::vector<double>y)
{
	double N = (double)x.size();
	double sumx = 0.0;
	double sumy = 0.0;
	double sumxx = 0.0;
	double sumyy = 0.0;
	double sumxy = 0.0;
	for (size_t i = 0; i < N; i++) {
		sumx += x[i];
		sumy += y[i];
		sumxx += x[i] * x[i];
		sumyy += y[i] * y[i];
		sumxy += x[i] * y[i];
	}
	return (N * sumxy - sumx * sumy) / std::sqrt((N * sumxx - sumx * sumx) * (N * sumyy - sumy * sumy));

}

inline bool regression(double* x, double* y, size_t n, double* gradient, double* intercept)
{
	//regression line y=mx + b // Thomas/Finney pp887
	//user must ensure line is not vertical	
	double sx = 0.0;
	double sy = 0.0;
	double sxy = 0.0;
	double sxx = 0.0;
	for (size_t i = 0; i < n; i++) {
		sx += x[i];
		sy += y[i];
		sxy += x[i] * y[i];
		sxx += x[i] * x[i];
	}

	if ((sx * sx - n * sxx) == 0.0)return false;

	double m = (sx * sy - n * sxy) / (sx * sx - n * sxx);
	double b = (sy - m * sx) / (double)n;

	*gradient = m;
	*intercept = b;
	return true;
}

inline bool regression(const std::vector<double>& x, const std::vector<double>& y, double& gradient, double& intercept)
{
	//regression line y=mx + b // Thomas/Finney pp887
	//user must ensure line is not vertical
	double n = (double)x.size();
	if (n <= 0)return false;

	double sx = 0.0;
	double sy = 0.0;
	double sxy = 0.0;
	double sxx = 0.0;
	for (size_t i = 0; i < x.size(); i++) {
		sx += x[i];
		sy += y[i];
		sxy += x[i] * y[i];
		sxx += x[i] * x[i];
	}

	if ((sx * sx - n * sxx) == 0.0)return false;

	gradient = (sx * sy - n * sxy) / (sx * sx - n * sxx);
	intercept = (sy - gradient * sx) / (double)n;

	return true;
}

inline bool bestfitlineendpoints(const std::vector<double>& x, const std::vector<double>& y, double& x1, double& y1, double& x2, double& y2)
{
	size_t n = x.size();
	double m = 0, c = 0;
	if (std::fabs(x[n - 1] - x[0]) > std::fabs(y[n - 1] - y[0])) {
		//predominantly EW line
		regression(x, y, m, c);
		x1 = (x[0] + m * (y[0] - c)) / (m * m + 1);
		x2 = (x[n - 1] + m * (y[n - 1] - c)) / (m * m + 1);
		y1 = x1 * m + c;
		y2 = x2 * m + c;
	}
	else {
		//predominantly NS line
		regression(y, x, m, c);
		y1 = (y[0] + m * (x[0] - c)) / (m * m + 1);
		y2 = (y[n - 1] + m * (x[n - 1] - c)) / (m * m + 1);
		x1 = y1 * m + c;
		x2 = y2 * m + c;
	}
	return true;
}

inline const std::string timestamp()
{
	std::time_t result = std::time(nullptr);
	const char* t = std::asctime(std::localtime(&result));
	std::string str;
	if (t) {
		str = std::string(t);
		if (str[str.length() - 1] == '\n') str.erase(str.length() - 1, 1);
	}
	return str;
};

inline const std::string timestring_fmt(const std::string format, std::time_t t = 0) {
	if (t == 0) t = std::time(NULL);
	//"%Y%m%d";
	char str[100];
	std::strftime(str, sizeof(str), format.c_str(), std::localtime(&t));
	return std::string(str);
}

inline int isinsidepolygon(int npol, double* xp, double* yp, double x, double y)
{
	//The following code is by Randolph Franklin, it returns 1 for interior points and 0 for exterior points. 
	int i, j, c = 0;
	for (i = 0, j = npol - 1; i < npol; j = i++) {
		if ((((yp[i] <= y) && (y < yp[j])) ||
			((yp[j] <= y) && (y < yp[i]))) &&
			(x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
			c = !c;
	}
	return c;
}

inline bool eq(double& a, double& b)
{
	if (std::fabs(a - b) < DBL_EPSILON)return true;
	else return false;
}

inline bool gt(double& a, double& b)
{
	if ((a - b) > DBL_EPSILON)return true;
	else return false;
}

inline bool lt(double& a, double& b)
{
	if ((b - a) > DBL_EPSILON)return true;
	else return false;
}

inline bool le(double& a, double& b)
{
	if (eq(a, b) || lt(a, b))return true;
	else return false;
}

inline bool ge(double& a, double& b)
{
	if (eq(a, b) || gt(a, b))return true;
	else return false;
}

inline void planeequation(const double& x1, const double& y1, const double& z1, const double& x2, const double& y2, const double& z2, const double& x3, const double& y3, const double& z3, double& A, double& B, double& C, double& D)
{
	//Ax+By+Cz+D=0
	double ax, ay, az, bx, by, bz;
	ax = x2 - x1;
	ay = y2 - y1;
	az = z2 - z1;

	bx = x3 - x1;
	by = y3 - y1;
	bz = z3 - z1;

	A = ay * bz - by * az;
	B = az * bx - bz * ax;
	C = ax * by - bx * ay;
	D = -(A * x1 + B * y1 + C * z1);
}

inline bool interplineline(const std::vector<double>& xin, const std::vector<double>& yin, std::vector<double>& xout, std::vector<double>& yout, double& dl)
{
	double d, x1, y1, x2, y2;
	double dx, dy, gradient, intercept;
	size_t n = xin.size();

	gradient = 0.0;
	intercept = 0.0;
	if (std::fabs(xin[n - 1] - xin[0]) > std::fabs(yin[n - 1] - yin[0])) {
		regression(xin, yin, gradient, intercept);
		x1 = xin[0];
		y1 = x1 * gradient + intercept;
		x2 = xin[n - 1];
		y2 = x2 * gradient + intercept;

		d = std::sqrt(std::pow(2.0, (x2 - x1)) + std::pow(2.0, (y2 - y1)));

		size_t nl = (size_t)std::floor(0.5 + d / dl);
		xout.resize(nl);
		yout.resize(nl);

		dx = (x2 - x1) / (double)(nl - 1);
		for (size_t i = 0; i < nl; i++) {
			xout[i] = x1 + (double)i * dx;
			yout[i] = xout[i] + gradient * intercept;
		}
	}
	else {
		regression(yin, xin, gradient, intercept);
		y1 = yin[0];
		x1 = y1 * gradient + intercept;
		y2 = yin[n - 1];
		x2 = y2 * gradient + intercept;

		d = std::sqrt(std::pow(2.0, (x2 - x1)) + std::pow(2.0, (y2 - y1)));

		size_t nl = (size_t)std::floor(0.5 + d / dl);
		xout.resize(nl);
		yout.resize(nl);

		dy = (y2 - y1) / (double)(nl - 1);
		for (size_t i = 0; i < nl; i++) {
			yout[i] = y1 + (double)i * dy;
			xout[i] = yout[i] + gradient * intercept;
		}
	}

	return true;
}

inline int findindex(const size_t n, const double* x, const double& xtarget)
{
	int N = (int)n;

	if (xtarget < x[0])return -1;
	if (xtarget > x[n - 1])return N;

	int lo = 0;
	int hi = N - 1;

	while (lo <= hi) {

		if (hi == lo + 1)return lo;

		int mid = (lo + hi) / 2;
		if (xtarget <= x[mid]) {
			hi = mid;
		}
		else {
			lo = mid;
		}
	}
	return lo;
}

inline int findindex(const std::vector<double>& x, const double& xtarget)
{
	return findindex(x.size(), &(x[0]), xtarget);
}

inline double linearinterp(const double& x1, const double& y1, const double& x2, const double& y2, const double& xtarget)
{
	return ((y2 - y1) / (x2 - x1)) * (xtarget - x1) + y1;
}

inline double linearinterp(const size_t n, const double* x, const double* y, const double& xtarget)
{
	int k = findindex(n, x, xtarget);
	if (k < 0)k = 0;
	else if (k >= (int)(n)-1)k = (int)(n)-2;
	return linearinterp(x[k], y[k], x[k + 1], y[k + 1], xtarget);
}

inline double linearinterp(const std::vector<double>& x, const std::vector<double>& y, const double& xtarget)
{
	return linearinterp(x.size(), &(x[0]), &(y[0]), xtarget);
}

inline void   linearinterp(const size_t n, const double* x, const double* y, size_t ni, const double* xi, double* yi)
{
	for (size_t i = 0; i < ni; i++) {
		yi[i] = linearinterp(n, x, y, xi[i]);
	}
}

inline std::vector<double> linearinterp(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& xi)
{
	std::vector<double> yi(xi.size());
	for (size_t i = 0; i < xi.size(); i++) {
		yi[i] = linearinterp(x, y, xi[i]);
	}
	return yi;
}

inline size_t bytesallocated(const std::vector<int>& v)
{
	return v.capacity() * sizeof(int);
}

inline size_t bytesallocated(const std::vector<double>& v)
{
	return v.capacity() * sizeof(double);
}

inline bool isreportable(int rec)
{
	int k, interval;

	if (rec < 1000)k = 2;
	else        k = (int)std::log10((double)rec);

	interval = (int)std::pow(10.0, (double)k);

	if (rec % interval == 0) return true;
	else return false;
}

inline bool isinrange(const cRange<int>& r, const int& i)
{
	if (i<r.from || i>r.to)return false;
	else return true;
}

inline double overlap(const double al, const double ah, const double bl, const double bh)
{
	//this returns the amount of overlap between the ranges al-ah with bl-bh
	if (al >= bh || ah <= bl)return 0;//no overlap 
	else if (al <= bl) {
		if (ah >= bh)return bh - bl;//a overlaps all of b
		else return ah - bl;//a overlaps low end of b 
	}
	else {
		if (ah <= bh)return ah - al;//a is all inside b
		else return bh - al;//a overlaps high end of b
	}
}

inline double fractionaloverlap(const double al, const double ah, const double bl, const double bh)
{
	double o = overlap(al, ah, bl, bh);
	return o / (ah - al);
}

inline std::vector<double> overlaps(const double& a1, const double& a2, const std::vector<double>& b)
{
	std::vector<double> o(b.size() - 1);
	o.resize(b.size() - 1);
	for (size_t bi = 0; bi < b.size() - 1; bi++) {
		o[bi] = overlap(a1, a2, b[bi], b[bi + 1]);
	}
	return o;
}

inline std::vector<double> fractionaloverlaps(const double& a1, const double& a2, const std::vector<double>& b)
{
	std::vector<double> o(b.size() - 1);
	o.resize(b.size() - 1);
	for (size_t bi = 0; bi < b.size() - 1; bi++) {
		o[bi] = fractionaloverlap(a1, a2, b[bi], b[bi + 1]);
	}
	return o;
}

inline std::vector<std::vector<double>> overlaps(const std::vector<double>& a, const std::vector<double>& b)
{
	std::vector<std::vector<double>> o(a.size() - 1);
	for (size_t ai = 0; ai < a.size() - 1; ai++) {
		o[ai].resize(b.size() - 1);
		for (size_t bi = 0; bi < b.size() - 1; bi++) {
			o[ai][bi] = overlap(a[ai], a[ai + 1], b[bi], b[bi + 1]);
		}
	}
	return o;
}

inline std::vector<std::vector<double>> fractionaloverlaps(const std::vector<double>& a, const std::vector<double>& b)
{
	std::vector<std::vector<double>> o(a.size() - 1);
	for (size_t ai = 0; ai < a.size() - 1; ai++) {
		o[ai].resize(b.size() - 1);
		for (size_t bi = 0; bi < b.size() - 1; bi++) {
			o[ai][bi] = overlap(a[ai], a[ai + 1], b[bi], b[bi + 1]);
		}
	}
	return o;
}

inline std::chrono::high_resolution_clock::time_point gettime_hr() {
	return std::chrono::high_resolution_clock::now();
}

inline double time_diff_hr(const std::chrono::high_resolution_clock::time_point& t1, const std::chrono::high_resolution_clock::time_point& t2) {
	return (double)1.0e-6 * (double)std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

inline double gettime() {
	auto tp = std::chrono::high_resolution_clock::now();
	auto t = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
	return (double)t / (double)1e3;
}

inline char* temppath(const char* s, int set)
{
	static char* p = (char*)NULL;
	if (set == 1) {
		if (p)delete[]p;
		p = new char[strlen(s) + 1];
		strcpy(p, s);
		return (char*)NULL;
	}
	else return p;
}

inline void settemppath(const char* s)
{
	temppath(s, 1);
}

inline std::string gettemppath()
{
	char* s = (char*)NULL;
	char* p = temppath(s, 0);
	return std::string(p);
}

inline int floatcompare(const void* pa, const void* pb)
{
	float& a = *(float*)pa; float& b = *(float*)pb;
	if (a < b)return -1;
	else if (a == b)return 0;
	else return 1;
}

inline void sort(float* x, const size_t n)
{
	qsort(x, n, sizeof(float), floatcompare);
}

inline int doublecompare(const void* pa, const void* pb)
{
	double& a = *(double*)pa; double& b = *(double*)pb;
	if (a < b)return -1;
	else if (a == b)return 0;
	else return 1;
}

inline void sort(double* x, const size_t n)
{
	qsort(x, n, sizeof(double), doublecompare);
}

inline int stringcompare(const void* pa, const void* pb)
{
	char** a = (char**)pa;
	char** b = (char**)pb;
	return strcmp(*a, *b);
}

inline void sort(char** strings, const size_t n)
{
	qsort(strings, n, sizeof(char*), stringcompare);
}

inline int intcompare(const void* pa, const void* pb)
{
	int& a = *(int*)pa; int& b = *(int*)pb;
	return a - b;
}

inline void sort(int* x, const size_t n)
{
	qsort(x, n, sizeof(int), intcompare);
}

#if defined _WIN32
inline double reportusage()
{
	MEMORYSTATUSEX memstatus;
	GlobalMemoryStatusEx(&memstatus);
	return memstatus.dwMemoryLoad;
}
#else
inline double reportusage()
{
	int pid = getpid();
	double vsize, pcpu, pmem;
	std::string tmpfile = strprint("ps.%d.tmp", pid);
	std::string cmd = strprint("ps --pid %d --format pcpu,vsize,pmem > %s\n", pid, tmpfile.c_str());
	int status = system(cmd.c_str());
	FILE* fp = std::fopen(tmpfile.c_str(), "r");
	char buf[201];
	char* dummy;
	dummy = fgets(buf, 200, fp);
	dummy = fgets(buf, 200, fp);
	sscanf(buf, "%lf %lf %lf", &pcpu, &vsize, &pmem);
	fclose(fp);
	std::filesystem::remove(tmpfile);
	glog.logmsg("Percent CPU used: %.2lf\n", pcpu);
	glog.logmsg("Percent memory used: %.2lf\n", pmem);
	glog.logmsg("Virtual memory used (Mb): %.2lf\n", vsize / 1000.0);
	return pmem;
}
#endif

#if defined _WIN32
inline double percentmemoryused()
{
	MEMORYSTATUSEX memstatus;
	GlobalMemoryStatusEx(&memstatus);
	return memstatus.dwMemoryLoad;
}
#else
inline double percentmemoryused()
{
	int pid = getpid();
	double vsize, pcpu, pmem;
	std::string tmpfile = strprint("ps.%d.tmp", pid);
	std::string cmd = strprint("ps --pid %d --format pcpu,vsize,pmem > %s\n", pid, tmpfile.c_str());
	int status = system(cmd.c_str());
	FILE* fp = std::fopen(tmpfile.c_str(), "r");
	char buf[201];
	char* dummy;
	dummy = fgets(buf, 200, fp);
	dummy = fgets(buf, 200, fp);
	sscanf(buf, "%lf %lf %lf", &pcpu, &vsize, &pmem);
	fclose(fp);
	std::filesystem::remove(tmpfile.c_str());
	return pmem;
}
#endif

inline void guage(int ntot, int n, int pdiv1, int pdiv2)
{
	double d1 = std::ceil(((double)pdiv1 / 100.0) * (double)ntot);
	double d2 = std::ceil(((double)pdiv2 / 100.0) * d1);

	if (n == 0) {
		printf("<");
	}
	else if (n >= ntot - 1) {
		printf(">");
	}
	else if (n % (int)d1 == 0) {
		printf("+");
	}
	else if (n % (int)d2 == 0) {
		printf(".");
	}
}

inline void allocate1darray(int*& a, const size_t n)
{
	a = new int[n];
	if (a == (int*)NULL) {
		printf("allocate1darray(int*& a, int n) returned NULL\n");
		prompttocontinue();
	}
}

inline void allocate1darray(double*& a, const size_t n)
{
	a = new double[n];
	if (a == (double*)NULL) {
		printf("allocate1darray(double*& a, int n) returned NULL\n");
		prompttocontinue();
	}
}

inline void allocate2darray(double**& a, const size_t nrows, const size_t ncols)
{
	a = new double* [nrows];
	for (size_t i = 0; i < nrows; i++)a[i] = new double[ncols];
}

inline void deallocate2darray(double**& a, const size_t nrows)
{
	if (a) {
		for (size_t i = 0; i < nrows; i++) {
			if (a[i])delete[]a[i];
		}
		delete[]a;
		a = (double**)NULL;
	}
}

inline void deallocate1darray(double*& a)
{
	if (a) {
		delete[]a;
		a = (double*)NULL;
	}
}

inline void deallocate1darray(int*& a)
{
	if (a) {
		delete[]a;
		a = (int*)NULL;
	}
}

inline double median(const double* v, const size_t n)
{
	double* d = new double[n];
	for (size_t i = 0; i < n; i++) d[i] = v[i];

	sort(d, n);

	double m = d[n / 2];

	delete[]d;
	return m;
}

inline std::vector<cRange<int>> parserangelist(std::string& str)
{
	std::vector<std::string> items = parsestrings(str, ",");

	std::vector<cRange<int>> list;
	cRange<int> r;

	for (size_t i = 0; i < items.size(); i++) {
		int f, t;
		int n = sscanf(items[i].c_str(), "%d to %d", &f, &t);
		//printf("%s\n",items[i].c_str());
		if (n == 1) {
			r.from = f;
			r.to = f;
			list.push_back(r);
		}
		else if (n == 2) {
			r.from = f;
			r.to = t;
			list.push_back(r);
		}
	}
	return list;
}

inline std::vector<double> getdoublevector(const char* str, const char* delims)
{
	double v;
	std::vector<double> vec;
	std::vector<std::string> fields = fieldparsestring(str, delims);
	for (size_t i = 0; i < fields.size(); i++) {
		int n = sscanf(fields[i].c_str(), "%lf", &v);
		if (n == 1) vec.push_back(v);
		else {
			glog.warningmsg(_SRC_, "Could not parse field %zu from string\n", i);
		}
	}
	return vec;
}

inline std::vector<float> dvec2fvec(std::vector<double>& vd)
{
	std::vector<float> vf(vd.size());
	for (size_t i = 0; i < vd.size(); i++)vf[i] = (float)vd[i];
	return vf;
}

inline unsigned int factorial(unsigned int n) {
	if (n <= 1) return 1;
	unsigned int fact = 1;
	for (unsigned int i = 2; i <= n; i++) {
		fact *= i;
	}
	return fact;
}

inline int LevenshteinDistance(char* s, int len_s, char* t, int len_t)
{
	//Adapted from https://en.wikipedia.org/wiki/Levenshtein_distance#Example
	int cost;
	/* base case: empty strings */
	if (len_s == 0) return len_t;
	if (len_t == 0) return len_s;
	/* test if last characters of the strings match */
	if (s[len_s - 1] == t[len_t - 1])cost = 0;
	else cost = 1;

	/* return minimum of delete char from s, delete char from t, and delete char from both */
	//return std::min(LevenshteinDistance(s, len_s - 1, t, len_t) + 1, LevenshteinDistance(s, len_s, t, len_t - 1) + 1, LevenshteinDistance(s, len_s - 1, t, len_t - 1) + cost);

	int a = LevenshteinDistance(s, len_s - 1, t, len_t) + 1;
	int b = LevenshteinDistance(s, len_s, t, len_t - 1) + 1;
	int c = LevenshteinDistance(s, len_s - 1, t, len_t - 1) + cost;
	if (a <= b && a <= c)return a;
	else if (b <= c)return b;
	return c;
}

template <typename T>
int roundnearest(const T& x, int nearest)
{
	return nearest * (int)std::round(x / nearest);
}

template <typename T>
T roundnearest(const T& x, const double& nearest)
{
	return (T)(nearest * std::round(x / nearest));
}

template <typename T>
T roundupnearest(const T& x, const double& nearest)
{
	return (T)(nearest * std::ceil(x / nearest));
}

template <typename T>
T rounddownnearest(const T& x, const double& nearest)
{
	T f = std::floor(x / nearest);
	T v = nearest * f;
	return v;
}

template <typename T>
T pow10(const T& x)
{
	return std::pow((T)10.0, x);
};

template <typename T>
T distance(const T& x1, const T& y1, const T& x2, const T& y2)
{
	return std::sqrt(std::pow(x2 - x1, 2.0) + std::pow(y2 - y1, 2.0));
}

template <typename T>
T distance(const T& x, const T& y)
{
	return std::sqrt(x * x + y * y);
}

inline bool isbigendian()
{
	const int i = 1;
	return (bool)(!*((char*)&i));
}

template <typename T>
T swap_endian(const T u)
{
	//from stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c	

	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;
	source.u = u;
	for (size_t k = 0; k < sizeof(T); k++) {
		dest.u8[k] = source.u8[sizeof(T) - k - 1];
	}
	return dest.u;
}

template <typename T>
void swap_endian(T* array, size_t num)
{
	for (size_t i = 0; i < num; i++) {
		array[i] = swap_endian(array[i]);
	}
}

template <typename T>
void swap_endian(std::vector<T>& array)
{
	for (size_t i = 0; i < array.size(); i++) {
		array[i] = swap_endian(array[i]);
	}
}

template<typename T>
inline const T sign(const T& a, const T& b) { return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a); }

template<typename T>
double covariance(const std::vector<T>& x, const std::vector<T>& y)
{
	size_t n = x.size();
	cStats<T> sx(x);
	cStats<T> sy(y);
	std::vector<T> v(n);
	for (size_t i = 0; i < x.size(); i++) {
		v[i] = (x[i] - sx.mean) * (y[i] - sy.mean);
	}
	cStats<T> sv(v);
	return sv.mean;
}

template<typename T>
double correlation(const std::vector<T>& x, const std::vector<T>& y)
{
	size_t n = x.size();
	double sx = 0.0;
	double sy = 0.0;
	double sxx = 0.0;
	double syy = 0.0;
	double sxy = 0.0;
	for (size_t i = 0; i < x.size(); i++) {
		sx += x[i];
		sy += y[i];
		sxx += x[i] * x[i];
		syy += y[i] * y[i];
		sxy += x[i] * y[i];
	}
	double r = (n * sxy - sx * sy) / std::sqrt(n * sxx - sx * sx) / std::sqrt(n * syy - sy * sy);
	return r;
}

constexpr auto SORT_UP = 0;
constexpr auto SORT_DOWN = 1;
template<typename T> void quicksortindex(T* a, int* index, const int& leftarg, const int& rightarg, int sortupordown)
{
	if (leftarg < rightarg) {
		T pivotvalue = a[leftarg];
		int left = leftarg - 1;
		int right = rightarg + 1;
		for (;;) {
			if (sortupordown == SORT_UP) {
				while (a[--right] > pivotvalue);
				while (a[++left] < pivotvalue);
			}
			else {
				while (a[--right] < pivotvalue);
				while (a[++left] > pivotvalue);
			}
			if (left >= right) break;

			T temp = a[right];
			a[right] = a[left];
			a[left] = temp;

			if (index) {
				int tempind = index[right];
				index[right] = index[left];
				index[left] = tempind;
			}
		}
		int pivot = right;
		quicksortindex(a, index, leftarg, pivot, sortupordown);
		quicksortindex(a, index, pivot + 1, rightarg, sortupordown);
	}
}

template<typename T>
bool bwrite(FILE* fp, const T& v) {
	size_t status = fwrite(&v, sizeof(T), 1, fp);
	if (status != 1) {
		glog.errormsg(_SRC_, "Error writing to binary file\n");
	}
	return true;
}

template<typename T>
bool bwrite(FILE* fp, const std::vector<T>& v) {
	size_t status = fwrite(&(v[0]), sizeof(T), v.size(), fp);
	if (status != v.size()) {
		glog.errormsg(_SRC_, "Error in writing to binary file\n");
	}
	return true;
}

