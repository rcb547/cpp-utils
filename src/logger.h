/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _logfile_H
#define _logfile_H

#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <vector>

#if defined _OPENMP
	#include <omp.h>
#endif

#if defined MATLAB_MEX_FILE
#include "mex.h"
#endif

#include "string_utils.h"

extern bool makedirectorydeep(std::string dirname);
extern std::string extractfiledirectory(const std::string& pathname);
extern std::string extractfilename(const std::string& filename);
extern const std::string timestamp();
extern const int mpi_openmp_rank();

class cLogger; //forward declaration only
extern class cLogger glog; //The global instance of the log file manager
#define _SRC_ cLogger::src_code_location(__FILE__, __FUNCTION__, __LINE__)

class cLogger  
{
	private:			
		std::vector<std::ofstream> ofs;
			
		void closeindex(const int i)
		{						
			if (ofs.size() > i) {
				if (ofs[i].is_open()) {
					ofs[i] << "Logfile closed on " << timestamp() << std::endl;
					ofs[i].close();										
				}
			}
		}		

		const int threadindex()
		{			
			#if defined _OPENMP
				return omp_get_thread_num();
			#else
				return 0;
			#endif
		}

		void flushindex(const int i)
		{
			ostrm() << std::flush;			
		}
		
public:    

	cLogger() {};

	void set_num_omp_threads(const size_t& n){
		ofs.resize(n);
	}
	
	bool open(const std::string& logfilename)
	{				
		const int i = threadindex();		
		if (ofs.size() < i + 1) {			
			ofs.resize(i + 1);
		}
		makedirectorydeep(extractfiledirectory(logfilename));
		ofs[i].open(logfilename, std::ios_base::out);		

		if (ofs[i].fail()) {
			glog.errormsg(_SRC_,"Failed to open Log file %s", logfilename.c_str());
		}		
		ofs[i] << "Logfile opened on " << timestamp() << std::endl << std::flush;
		return true;
	}

	void flush()
	{		
		flushindex(threadindex());
	}
	
	void close()
	{		
		closeindex(threadindex());
	}

	std::ofstream& ostrm()
	{		
		if (ofs.size() == 0) {
			ofs.resize(1);
			return ofs[0];
		}		
		const int i = threadindex();
		return ofs[i];	
	}
		
	~cLogger()
	{		
		for (size_t i = 0; i < ofs.size(); i++) {
			closeindex((int)i);
		}
	};

	void logmsg(const std::string& msg){
		std::ofstream& fs = ostrm();
		if (ofs.size()>0 && fs.is_open()) fs << msg << std::flush;
		std::cout << msg << std::flush;		
	};

	void log(const std::string& msg) {
		std::ofstream& fs = ostrm();
		if (ofs.size() > 0 && fs.is_open()) fs << msg << std::flush;
	};

	void logmsg(const char* fmt, ...)
	{				
		va_list vargs;
		va_start(vargs, fmt);		
		std::string msg = strprint(fmt, vargs);
		va_end(vargs);
		logmsg(msg);
	}

	void log(const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint(fmt, vargs);
		va_end(vargs);
		log(msg);
	}

	void logmsg(const int& rank, const std::string& msg) {		
		log(msg);
		if (mpi_openmp_rank() == rank) {
			std::cout << msg << std::flush;			
		}
	};

	void logmsg(const int& rank, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint(fmt, vargs);
		va_end(vargs);
		logmsg(rank, msg);
	}	

	void warningmsg(const std::string& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = "**Warning: " + strprint(fmt, vargs);
		va_end(vargs);

		#if defined MATLAB_MEX_FILE
			mexWarnMsgTxt(msg.c_str());
		#else
			logmsg(msg);		
		#endif		
		logmsg(strprint("%s\n",srccodeloc.c_str()));
	}

	void errormsg(const std::string& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = "**Error: " + strprint(fmt, vargs);
		va_end(vargs);
				
		#if defined MATLAB_MEX_FILE
			mexErrMsgTxt(msg.c_str());
		#else
			logmsg(msg);		
			throw(strprint("Exception throw from %s\n", srccodeloc.c_str()));
		#endif		
	}

	static std::string src_code_location(const char* file, const char* function, const int& linenumber)
	{		
		std::string s = strprint("File: %s\t Function:%s\t Line:%d", extractfilename(file).c_str(), function, linenumber);
		return s;
	}

};

#endif

