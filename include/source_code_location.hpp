/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <sstream>

namespace CppUtils {

	class SourceCodeLocation {

	private:
		std::string location;

	public:
		SourceCodeLocation() {};

		SourceCodeLocation(const char* file, const char* function, const int linenumber) {
			std::ostringstream oss;
			oss << "File: " << file << "\t" << "Function: " << function << "\t" << "Line : " << linenumber;
			location = oss.str();
		};

		const char* c_str() const {
			return location.c_str();
		}

		size_t size() const {
			return location.size();
		}

	};
};

#define _SRC_ CppUtils::SourceCodeLocation(__FILE__, __FUNCTION__, __LINE__)
#define _SRC_CSTR_ CppUtils::SourceCodeLocation(__FILE__, __FUNCTION__, __LINE__).c_str()



