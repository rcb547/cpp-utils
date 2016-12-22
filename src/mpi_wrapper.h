/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _mpi_wrapper_H
#define _mpi_wrapper_H

#include <vector>
#include <mpi.h>
#include <general_utils.h>


class cMpiComm;
class cMpiEnv;

class cMpiEnv{

public:
	bool startandstop;
	int size;
	int rank;
	std::string pname;

	cMpiEnv(){
		startandstop = false;		
	};

	cMpiEnv(int argc, char** argv, bool _startandstop=true){
		startandstop = _startandstop;
		if (startandstop)start(argc, argv);
		setsizerankpname();
	};

	~cMpiEnv(){
		if (startandstop)stop();
	};

	void setsizerankpname(){
		MPI_Comm_size(MPI_COMM_WORLD, &size);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		pname = getpname();		
		return;
	};

	void start(int argc, char** argv){
		MPI_Init(&argc, &argv);		
		setsizerankpname();
		rootmessage("MPI Started Processes=%d\tRank=%d\tProcessor name = %s\n", size, rank, pname.c_str());
		return;
	};

	void stop(){
		rootmessage("Finalizing MPI\n");
		MPI_Finalize();
	}

	std::string getpname(){
		int len;
		char pname[MPI_MAX_PROCESSOR_NAME + 1];
		MPI_Get_processor_name(pname, &len);		
		return std::string(pname);		
	};

	static std::string errorstring(int ierr){

		std::string s;

		if (ierr == MPI_SUCCESS) s = "MPI_SUCCESS: No error; MPI routine completed successfully.";
		else if (ierr == MPI_ERR_COMM) s = "MPI_ERR_COMM: Invalid communicator. A common error is to use a null communicator in a call(not even allowed in MPI_Comm_rank).";
		else if (ierr == MPI_ERR_COUNT) s = "MPI_ERR_COUNT: Invalid count argument. Count arguments must be non - negative; a count of zero is often valid.";
		else if (ierr == MPI_ERR_TYPE) s = "MPI_ERR_TYPE: Invalid datatype argument. May be an uncommitted MPI_Datatype(see MPI_Type_commit).";
		else if (ierr == MPI_ERR_BUFFER) s = "MPI_ERR_BUFFER: Invalid buffer pointer. Usually a null buffer where one is not valid.";
		else if (ierr == MPI_ERR_ROOT) s = "MPI_ERR_ROOT: Invalid root. The root must be specified as a rank in the communicator. Ranks must be between zero and the size of the communicator minus one.";
		else{

		}
		return s;
	}

	static bool chkerr(int ierr){
		if (ierr == MPI_SUCCESS)return true;
		else{
			printf("%s\n", errorstring(ierr).c_str());
			return false;
		}
	}

	static MPI_Datatype mpitype(const char& v){ return MPI_CHAR; }
	static MPI_Datatype mpitype(const size_t& v){ return MPI_UINT64_T; }
	static MPI_Datatype mpitype(const int& v){ return MPI_INT; }
	static MPI_Datatype mpitype(const float& v){ return MPI_FLOAT; }
	static MPI_Datatype mpitype(const double& v){ return MPI_DOUBLE; }	
	static MPI_Datatype mpitype(const std::string& s){ return MPI_CHAR; }
	
	template < typename T >
	static MPI_Datatype mpitype(const std::vector<T>& v){
		T dummy;
		return mpitype(dummy);
	};

	static MPI_Comm world_comm(){
		return MPI_COMM_WORLD;		
	}

	static int world_size(){
		int s;
		int ierr = MPI_Comm_size(MPI_COMM_WORLD, &s);
		chkerr(ierr);
		return s;
	}

	static int world_rank(){
		int r;
		int ierr = MPI_Comm_rank(MPI_COMM_WORLD, &r);
		chkerr(ierr);
		return r;
	}

	static void printsizeofs()
	{
		printf("sizeof(MPI_Int) = %lu\n", sizeof(MPI_INT));
		printf("sizeof(bool) = %lu\n", sizeof(bool));
		printf("sizeof(int) = %lu\n", sizeof(int));
		printf("sizeof(int32_t) = %lu\n", sizeof(int32_t));
		printf("sizeof(int64_t) = %lu\n", sizeof(int64_t));
		printf("sizeof(size_t) = %lu\n", sizeof(size_t));		
	};

};

class cMpiComm{

	MPI_Comm comm;

public:

	cMpiComm(){
		comm = MPI_COMM_NULL;
	};

	cMpiComm(const MPI_Comm& _comm){
		set(_comm);
	};

	operator const MPI_Comm& ()
	{		
		return comm;
	}

	void set(const MPI_Comm& _comm){
		comm = _comm;
	}

	const MPI_Comm& get(){
		return comm;
	}

	int size()
	{
		int s;
		int ierr = MPI_Comm_size(comm, &s);
		chkerr(ierr);
		return s; 
	}

	int rank()
	{ 
		int r;		
		int ierr = MPI_Comm_rank(comm, &r);
		chkerr(ierr);
		return r;
	}

	void barrier()
	{
		int ierr = MPI_Barrier(comm);
		chkerr(ierr);
	}

	bool chkerr(int ierr){
		return cMpiEnv::chkerr(ierr);
	}

	void syncprintf(const char* fmt, ...)
	{		
		size_t sz = (size_t)ceil(log10(size()));
		if (sz == 0)sz = 1;
		std::string szfmt = "[%" + strprint("%lu",sz) + "lu] ";
				
		for (size_t i = 0; i < size(); i++){
			if (i == rank()){
				va_list vargs;
				va_start(vargs, fmt);
				std::printf(szfmt.c_str(), rank());
				std::vprintf(fmt, vargs);				
				std::fflush(stdout);
				va_end(vargs);				
			}
			barrier();
		}
		return;		
	}

	template < typename T >
	bool bcast(T& value, int root = 0){
		int ierr = MPI_Bcast(&value, 1, cMpiEnv::mpitype(value), root, comm);
		return chkerr(ierr);
	};

	template < typename T >
	bool bcast(std::vector<T>& v, int root = 0){
		size_t n = v.size();
		bcast(n, root);
		v.resize(n);
		int ierr = MPI_Bcast(v.data(), (int)n, cMpiEnv::mpitype(v), root, comm);
		return chkerr(ierr);
	};

	bool bcast(std::string& s, int root = 0){
		std::vector<char> buf(s.begin(), s.end());		
		bool status = bcast(buf, root);
		if (rank() != root){
			buf.push_back(0);
			s = std::string(buf.data());
		}
		return status;
	};

	template < typename T >
	bool isend(T& value, int destination){				
		MPI_Request request;
		int tag = 0;
		int ierr = MPI_Isend(&value, 1, cMpiEnv::mpitype(value), destination, tag, comm, &request);
		return chkerr(ierr);
	};

	template < typename T >
	bool isend_vec(std::vector<T>& v, int destination){		
		MPI_Request request;
		int tag = 0;
		int ierr = MPI_Isend(v.data(), (int)v.size(), cMpiEnv::mpitype(v), destination, tag, comm, &request);
		return chkerr(ierr);
	};

	bool isend_str(const std::string& s, int destination){				
		MPI_Request request;
		int tag = 0;
		int ierr = MPI_Isend((void*)s.data(), (int)s.size(), cMpiEnv::mpitype(s), destination, tag, comm, &request);
		return chkerr(ierr);
	};
	
	template < typename T >
	bool irecv(T& value, int source){
		MPI_Request request;
		int tag = 0;
		int ierr = MPI_Irecv(&value, 1, cMpiEnv::mpitype(value), source, tag, comm, &request);
		return chkerr(ierr);
	};

	template < typename T >
	bool irecv_vec(std::vector<T>& v, int source){				
		MPI_Status status;
		MPI_Flag flag;
		MPI_Request request;
		int tag = 0;
		int n;
		int ierr;
		
		ierr = MPI_Iprobe(source, tag, comm, &flag, &status);
		if (flag == 0){
			v.resize(0);
			return false;
		}

		ierr = MPI_Get_count(&status, cMpiEnv::mpitype(v), &n);
		v.resize(n);
		ierr = MPI_Irecv(v.data(), n, cMpiEnv::mpitype(v), source, tag, comm, &request);
		return chkerr(ierr);
	};
	
	bool irecv_str(std::string& s, int source){		
		MPI_Status status;
		int flag;
		MPI_Request request;
		int tag = 0;
		int count;
		int ierr;
		ierr = MPI_Iprobe(source, tag, comm, &flag, &status);
		if (flag == 0){
			s.resize(0);
			return false;
		}

		ierr = MPI_Get_count(&status, cMpiEnv::mpitype(s), &count);		
		s.resize(count);
		ierr = MPI_Irecv((void*)s.data(), count, cMpiEnv::mpitype(s), source, tag, comm, &request);
		return chkerr(ierr);
	};

	template < typename T >
	T sum(T& value){		
		T s;
		int ierr = MPI_Allreduce(&value, &s, 1, cMpiEnv::mpitype(value), MPI_SUM, comm);
		chkerr(ierr);
		return s;
	};

	template < typename T >
	double mean(T& value){	
		return (double)sum(value)/size();
	};

};

#endif

