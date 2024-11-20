/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <chrono>
#include "general_utils.hpp"


class cStopWatch{	

	std::chrono::high_resolution_clock::time_point t0;
	std::chrono::high_resolution_clock::time_point t1;
	
public:

	cStopWatch(){ reset(); }
	void start(){ t0 = std::chrono::high_resolution_clock::now(); }
	void stop(){ t1 = std::chrono::high_resolution_clock::now(); }
	void reset(){
		t1 = std::chrono::high_resolution_clock::now();
		t0 = std::chrono::high_resolution_clock::now();
	}

	double etime(){
		using namespace std::chrono;
		duration<double> time_span = duration_cast<std::chrono::microseconds>(t1 - t0);		
		return time_span.count();
	}

	double etimenow(){
		using namespace std::chrono;
		t1 = high_resolution_clock::now();
		duration<double> time_span = duration_cast<std::chrono::microseconds>(t1 - t0);				
		return time_span.count();
	}

	double reportnow(){
		double e = etimenow();
		std::cout << "Elapsed time = " << e << std::endl;
		return e;
	}

};

