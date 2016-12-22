/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#include <iostream>
#include <stdio.h>
#include "general_utils.h"
#include "memusage.h"


#if defined (_WIN32)//windows
memUsage::memUsage(std::string name){
	nametag=name;
	rename(name);
    reset();
	report();
}

/////////////////////////////////////////////////////////////////////
void memUsage::reset() {
	GlobalMemoryStatus(&memstart);    
}
///////////////////////////////////////////////////////////////////////////
void memUsage::report() {	
	reset();	
	message("============>>Memory used: %d%%\n",memstart.dwMemoryLoad);  
	fflush(stdout);	
}
////////////////////////////////////////////////////////////////////////////
void memUsage::report1() {
	
	reset();
	int bytespermegabyte = 1048576;	
	message("-----------------------------------%s---------------------------------------\n",nametag.c_str());	
	message("Total physical memory: %d Mb\n",memstart.dwTotalPhys/bytespermegabyte);  	
	message("Total page file memory: %d Mb\n",memstart.dwTotalPageFile/bytespermegabyte);  	
	message("Total virtual memory: %d Mb\n",memstart.dwTotalVirtual/bytespermegabyte);  	
	message("\n");  	
	
	message("Available physical memory: %d Mb\n",memstart.dwAvailPhys/bytespermegabyte);  	
	message("Available page file memory: %d Mb\n",memstart.dwAvailPageFile/bytespermegabyte);  	
	message("Available virtual memory: %d Mb\n",memstart.dwAvailVirtual/bytespermegabyte);  	
	message("\n");  	
	
	message("Used physical memory: %d Mb\n",(memstart.dwTotalPhys-memstart.dwAvailPhys)/bytespermegabyte);  	
	message("Used page file memory: %d Mb\n",(memstart.dwTotalPageFile-memstart.dwAvailPageFile)/bytespermegabyte);  	
	message("Used virtual memory: %d Mb\n",(memstart.dwTotalVirtual-memstart.dwAvailVirtual)/bytespermegabyte);  	
	message("\n");  	
	
	message("Memory used: %d%%\n",memstart.dwMemoryLoad);  	
	message("-----------------------------------%s---------------------------------------\n",nametag.c_str());
	fflush(stdout);

}
/////////////////////////////////////////////////////////////////////
void memUsage::rename(std::string name) {
    nametag=name;        
}

#else //unix

memUsage::memUsage(std::string name){
	nametag=name;
	rename(name);
    reset();
	report();
}

/////////////////////////////////////////////////////////////////////
void memUsage::reset() {

}
////////////////////////////////////////////////////////////////////////////
void memUsage::report() {
	

}

/////////////////////////////////////////////////////////////////////
void memUsage::rename(std::string name) {
	nametag=name;	
}

#endif

