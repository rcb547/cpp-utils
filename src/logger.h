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
#include <file_utils.h>

class cLogger  
{
	private:
		std::vector<FILE*> fptr;
		void closeindex(const int i)
		{
			if (fptr.size() > i) {
				if (fptr[i]){
					fprintf(fptr[i],"Logfile closed on %s\n", timestamp().c_str());
					fflush(fptr[i]);
					fclose(fptr[i]);
					fptr[i] = (FILE*)NULL;
				}				
			}
		}		
		const int threadindex()
		{			
			#if defined _OPENMP
				return omp_get_thread_num();
			#elif
				return 0;
			#endif
		}
		void flushindex(const int i)
		{
			if (fptr.size() > i) {
				if (fptr[i]) std::fflush(fptr[i]);				
			}
		}		

public:    

	cLogger() {};

	FILE* open(const std::string& logfilename)
	{
		const int i = threadindex();
		if (fptr.size() < i+1)fptr.resize(i+1);
		fptr[i] = fileopen(logfilename, "w");
		if (fptr[i]){
			fprintf(fptr[i],"Logfile opened on %s\n", timestamp().c_str());
		}
		return fptr[i];
	}

	void flush()
	{		
		flushindex(threadindex());
	}
	
	void close()
	{		
		closeindex(threadindex());
	}

	FILE* getfilepointer()
	{
		const int i = threadindex();
		if(fptr.size()>i)return fptr[i];
		else return (FILE*)NULL;
	}
		
	~cLogger()
	{
		for (size_t i = 0; i < fptr.size(); i++) {
			closeindex(i);
		}
	};

	void logmsg(const std::string& msg) {
		if (FILE* fp = getfilepointer()) {
			fprintf(fp, msg.c_str());
		}
		printf(msg.c_str());
		std::fflush(stdout);
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
		if (FILE* fp = getfilepointer()) {
			fprintf(fp, msg.c_str());
		}
		if (mpi_openmp_rank() == rank) {
			printf(msg.c_str());
			std::fflush(stdout);
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
};

#endif

