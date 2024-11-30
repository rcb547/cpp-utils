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

#undef _HAS_STACK_TRACE_
//#define _HAS_STACK_TRACE_
#if defined  _HAS_STACK_TRACE_
	#include <stacktrace>
#endif

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

class SourceCodeLocation {

private:
	std::string location;

public: 

	SourceCodeLocation(const char* file, const char* function, const int& linenumber){
		const std::filesystem::path p(file);
		location = strprint("File: %s\t Function:%s\t Line:%d", p.filename().string().c_str(), function, linenumber);
	};

	const char* c_str() const {
		return location.c_str();
	}

	size_t size() const {
		return location.size();
	}

};

class cLogger; //forward declaration only
extern class cLogger glog; //The global instance of the log file manager
#define _SRC_ SourceCodeLocation(__FILE__, __FUNCTION__, __LINE__)

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
		const size_t i = (size_t) threadindex();
		if (ofs.size() < i + 1) {
			ofs.resize(i + 1);
		}

		std::filesystem::path dirpath = std::filesystem::path(logfilename).make_preferred().parent_path();
		if (dirpath.string().size() > 0) {
			std::filesystem::create_directories(dirpath);
		}

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

	void log_to_cout(const std::string& msg) {
		std::cout << msg << std::flush;
	};

	void log_to_file(const std::string& msg) {
		std::ofstream& fs = ostrm();
		if (ofs.size() > 0 && fs.is_open()) fs << msg << std::flush;
	};

	void logmsg(const std::string& msg) {
		log_to_file(msg);
		log_to_cout(msg);
	};

	void logmsg(const int stdout_rank, const std::string& msg) {
		log_to_file(msg);
		if (cLogger::mpi_openmp_rank() == stdout_rank) {
			log_to_cout(msg);
		}
	};
	
	void logmsg(const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		logmsg(msg);
	}

	void logmsg(const int stdout_rank, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		logmsg(stdout_rank, msg);
	}
	
	void warningmsg_impl(const std::string& msg, const SourceCodeLocation& srccodeloc)
	{
		std::string fullmsg = "**Warning: " + msg + "\n";
		if (srccodeloc.size() > 0) fullmsg += strprint("Warning is from %s\n", srccodeloc.c_str());

		#if defined MATLAB_MEX_FILE
			mexWarnMsgTxt(fullmsg.c_str());
		#else
			logmsg(fullmsg);
		#endif
	}

	void warningmsg(const SourceCodeLocation& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = "**Warning: " + strprint_va(fmt, vargs);
		va_end(vargs);
		warningmsg_impl(msg, srccodeloc);
	}

	void warningmsg(const SourceCodeLocation& srccodeloc, const std::string& msg){
		warningmsg_impl(msg, srccodeloc);
	}

	void append_stacktrace(std::string& msg) {
		#ifdef  _HAS_STACK_TRACE_
			msg += "\n======= Stack Trace =========================\n";
			msg += std::to_string(std::stacktrace::current());
			msg += "\n=============================================\n";
		#endif	
	};

	void errormsg_impl(const std::string& msg, const SourceCodeLocation& srccodeloc) {
		std::string fullmsg = "***Error: " + msg;
		if(srccodeloc.size()>0) fullmsg += strprint("Exception thrown from %s\n", srccodeloc.c_str());
		append_stacktrace(fullmsg);

		#if defined MATLAB_MEX_FILE
			mexErrMsgTxt(fullmsg.c_str());
		#else
			log_to_file(fullmsg);
			throw(std::runtime_error(fullmsg));
		#endif
	}

	void errormsg(const SourceCodeLocation& srccodeloc, const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		std::string msg = strprint_va(fmt, vargs);
		va_end(vargs);
		errormsg_impl(msg, srccodeloc);
	}

	void errormsg(const SourceCodeLocation& srccodeloc, const std::string& msg)
	{
		errormsg_impl(msg, srccodeloc);
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
		int omprank = 0;
		int mpirank = 0;
		#if defined _OPENMP
			omprank = omp_get_thread_num();
		#endif

		#ifdef ENABLE_MPI
			int mpi_initialised;
			int ierr = MPI_Initialized(&mpi_initialised);
			if (mpi_initialised) {
				MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
			}
		#endif
		return std::max(mpirank,omprank);
	}

};

