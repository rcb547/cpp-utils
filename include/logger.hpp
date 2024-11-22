/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <iostream>
#include <ctime>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
#include <filesystem>

#include "string_print.hpp"

#if defined ENABLE_MPI
	#include <mpi.h>
#endif

#if defined _OPENMP
	#include <omp.h>
#endif

#if defined MATLAB_MEX_FILE
	#include "mex.h"
#endif

class cLogger; //forward declaration only
extern class cLogger glog; //The global instance of the log file manager
#define _SRC_ cLogger::src_code_location(__FILE__, __FUNCTION__, __LINE__)

class cLogger
{
	private:
		std::vector<std::ofstream> ofs;

		void closeindex(const int i)
		{
			if ((int)ofs.size() > i) {
				if (ofs[i].is_open()) {
					ofs[i] << "Logfile closed on " << timestamp() << std::endl;
					ofs[i].close();
				}
			}
		}

		int threadindex()
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

		std::ofstream& ostrm()
		{
			if ((int)ofs.size() == 0) {
				ofs.resize(1);
				return ofs[0];
			}
			const int i = threadindex();
			return ofs[i];
		};


public:

	cLogger() {};

	void set_num_omp_threads(const size_t& n){
		ofs.resize(n);
	}

	bool open(const std::string& logfilename)
	{
		const size_t i = (size_t)threadindex();
		if (ofs.size() < i + 1) {
			ofs.resize(i + 1);
		}

		std::filesystem::path p(logfilename); p.make_preferred();
		std::filesystem::create_directories(p.parent_path());

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

	~cLogger()
	{
		for (size_t i = 0; i < ofs.size(); i++) {
			closeindex((int)i);
		}
	};

	void log(const std::string& msg) {
		std::ofstream& fs = ostrm();
		if (ofs.size() > 0 && fs.is_open()) fs << msg << std::flush;
	};

	void log(const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		log(msg);
	}

	void logmsg(const std::string& msg){
		std::ofstream& fs = ostrm();
		if ((int)ofs.size()>0 && fs.is_open()) fs << msg << std::flush;
		std::cout << msg << std::flush;
	};

	void logmsg(const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		logmsg(msg);
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
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		logmsg(rank, msg);
	}
	
	void warningmsg(const std::string& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = "**Warning: " + strprint_va(fmt, vargs);
		va_end(vargs);

		#if defined MATLAB_MEX_FILE
			mexWarnMsgTxt(msg.c_str());
		#else
			logmsg(msg);
		#endif
		logmsg(strprint("%s\n",srccodeloc.c_str()));
	}

	void errormsg(const std::string& msg) {
		errormsg(msg.c_str());
	}

	void errormsg(const char* str)
	{		
		std::string msg = "**Error: " + std::string(str);		
		#if defined MATLAB_MEX_FILE
			mexErrMsgTxt(msg.c_str());
		#else			
			throw(std::runtime_error(msg));
		#endif
	}

	void errormsg(const std::string& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = "**Error: " + strprint_va(fmt, vargs);
		va_end(vargs);

		#if defined MATLAB_MEX_FILE
			mexErrMsgTxt(msg.c_str());
		#else
			logmsg(msg);
			throw(std::runtime_error(strprint("Exception thrown from %s\n", srccodeloc.c_str())));
		#endif
	}

	static std::string src_code_location(const char* file, const char* function, const int& linenumber)
	{
		const std::filesystem::path p(file);
		std::string s = strprint("File: %s\t Function:%s\t Line:%d", p.filename().string().c_str(), function, linenumber);
		return s;
	}

	inline static const std::string timestamp()
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

	inline static int mpi_openmp_rank() {
		int rank = 0;
		#if defined _OPENMP
			rank = omp_get_thread_num();
			if (rank > 0) return rank;
		#endif

		#ifdef ENABLE_MPI
			int mpi_initialised;
			int ierr = MPI_Initialized(&mpi_initialised);
			if (mpi_initialised) {
				MPI_Comm_rank(MPI_COMM_WORLD, &rank);
				return rank;
			}
		#endif
		return rank;
	}

};

