/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#if defined _WIN32
	#define NOMINMAX 
	#include <windows.h>
	//#include <conio.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/resource.h>
	#include <errno.h>
#endif


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

