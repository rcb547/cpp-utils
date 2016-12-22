/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _stacktrace_H
#define _stacktrace_H

#include <vector>

//It is best to define USEGLOBALSTACKTRACE as a
//compiler preprocessor definition so that it is defined
//across all source units
//#define USEGLOBALSTACKTRACE

#ifdef USEGLOBALSTACKTRACE
	#define _GSTITEM_ cTraceItem trace(__FILE__, __FUNCTION__, __LINE__);	
	#define _GSTPUSH_ globalstacktrace.push(__FILE__, __FUNCTION__, __LINE__);
	#define _GSTPOP_ globalstacktrace.pop();
	#define _GSTPRINT_ globalstacktrace.printf();
#else
	#define _GSTITEM_	
	#define _GSTPUSH_
	#define _GSTPOP_
	#define _GSTPRINT_
#endif

class cTraceItem; //forward declaration only
class cStackTrace; //forward declaration only

//extern only here - declare the actual global instance of the object in the main program .cpp file
extern class cStackTrace globalstacktrace; 

class cStackTrace {

private:
	std::vector<std::string> stack;
	
public:

	cStackTrace(){ };
	
	void push(const char* file, const char* function, const int& linenumber){
		char buf[500];
		std::sprintf(buf,"File: %s\t Function:%s\t Line:%d", file, function, linenumber);
		stack.push_back(std::string(buf));
	}

	void pop(){
		stack.pop_back();
	}

	void printf(){		
		std::printf("---Stack Trace----------------------------\n");
		for (size_t i = stack.size(); i > 0; i--){
			std::printf("%s\n", stack[i-1].c_str());
		}
		std::printf("------------------------------------------\n");
	}
};

class cTraceItem {

public:

	cTraceItem(const char* file, const char* function, const int& linenumber){
		globalstacktrace.push(file,function,linenumber);
	}

	~cTraceItem(){
		globalstacktrace.pop();
	}

};

#endif

