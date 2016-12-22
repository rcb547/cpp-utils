/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _cgal_utils_H
#define _cgal_utils_H

#include <vector>

bool line_data_alpha_shape_polygon_ch(
	const std::vector<unsigned int>& line_index_start,
	const std::vector<unsigned int>& line_index_count,
	const std::vector<double>& x,
	const std::vector<double>& y,
	const double nullx,
	const double nully,
	size_t maxvertices,
	std::vector<double>& px,
	std::vector<double>& py);
#endif


