/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _logfile_H
#define _logfile_H

#include <cstdio>
#include <vector>
#if defined _OPENMP
	#include <omp.h>
#endif

#include <iostream>
#include <fstream>

extern std::string extractfilename(const std::string& filename);
extern std::string timestamp();
//extern std::string strprint_va(const char* fmt, va_list vargs);
extern std::string strprint(const char* fmt, ...);
extern const int mpi_openmp_rank();

class cLogger; //forward declaration only
extern class cLogger glog; //The global instance of the log file manager

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

	bool open(const std::string& logfilename)
	{
		const int i = threadindex();		
		if (ofs.size() < i + 1) ofs.resize(i + 1);
		ofs[i].open(logfilename,std::ios_base::out);
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
		if (fs.is_open()) fs << msg << std::flush;
		std::cout << msg << std::flush;		
	};

	void log(const std::string& msg) {
		std::ofstream& fs = ostrm();
		if (fs.is_open()) fs << msg << std::flush;		
	};

	void logmsg(const char* fmt, ...)
	{		
		va_list vargs;
		va_start(vargs, fmt);
		//std::string msg = strprint_va(fmt, vargs);
		std::string msg = strprint(fmt, vargs);
		va_end(vargs);
		logmsg(msg);
	}

	void logmsg(const int& rank, const std::string& msg) {		
		logmsg(msg);
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
};

#endif

