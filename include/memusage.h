/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _memusage_H
#define _memusage_H


#if defined(_WIN32)
#include "windows.h"
#else
#endif

#include <stdio.h>
#include <string>
#include <iostream>
#include "general_utils.h"

class memUsage {

#if defined(_WIN32)
private:
	MEMORYSTATUS memstart;

public:
	std::string nametag;

	memUsage(const std::string name = "") {
		nametag = name;
		rename(name);
		reset();
		report();
	}

	void memUsage::rename(std::string name) {
		nametag = name;
	}

	void memUsage::reset() {
		GlobalMemoryStatus(&memstart);
	}

	void memUsage::report() {
		reset();
		std::printf("============>>Memory used: %d%%\n", memstart.dwMemoryLoad);
		fflush(stdout);
	}

	void memUsage::report1() {

		reset();
		int bytespermegabyte = 1048576;
		std::printf("-----------------------------------%s---------------------------------------\n", nametag.c_str());
		std::printf("Total physical memory: %d Mb\n", memstart.dwTotalPhys / bytespermegabyte);
		std::printf("Total page file memory: %d Mb\n", memstart.dwTotalPageFile / bytespermegabyte);
		std::printf("Total virtual memory: %d Mb\n", memstart.dwTotalVirtual / bytespermegabyte);
		std::printf("\n");

		std::printf("Available physical memory: %d Mb\n", memstart.dwAvailPhys / bytespermegabyte);
		std::printf("Available page file memory: %d Mb\n", memstart.dwAvailPageFile / bytespermegabyte);
		std::printf("Available virtual memory: %d Mb\n", memstart.dwAvailVirtual / bytespermegabyte);
		std::printf("\n");

		std::printf("Used physical memory: %d Mb\n", (memstart.dwTotalPhys - memstart.dwAvailPhys) / bytespermegabyte);
		std::printf("Used page file memory: %d Mb\n", (memstart.dwTotalPageFile - memstart.dwAvailPageFile) / bytespermegabyte);
		std::printf("Used virtual memory: %d Mb\n", (memstart.dwTotalVirtual - memstart.dwAvailVirtual) / bytespermegabyte);
		std::printf("\n");

		std::printf("Memory used: %d%%\n", memstart.dwMemoryLoad);
		std::printf("-----------------------------------%s---------------------------------------\n", nametag.c_str());
		fflush(stdout);

	}

#else
	
	memUsage(const std::string name = "") {
		nametag = name;
		rename(name);
		reset();
		report();
	}
	void reset() {}
	void report() {}
	void report1() {}
	void rename(std::string name) {
		nametag = name;
	}
#endif

};

#endif


