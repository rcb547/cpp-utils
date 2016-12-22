/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#if !defined(memusage_h)
#define memusage_h

#if defined(_WIN32)//windows
#include "windows.h"
#else//unix
#endif

#include "string"
using namespace std;

class memUsage {

#if defined(_WIN32)	//windows

private:
    MEMORYSTATUS memstart;    		
  
#endif 

public:
    memUsage(std::string name = "");
	std::string nametag;
	void reset();  
	void rename(std::string name);
	void report();  
	void report1();  

};

#endif
