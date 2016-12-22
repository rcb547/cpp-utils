/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _rollpitchyaw_H
#define _rollpitchyaw_H

#include "matrixvector23.h"
////////////////////////////////////////////////////////////////////////////////
Matrix33<double> rollpitchyaw_matrix(const double& roll, const double& pitch, const double& yaw);
Matrix33<double> inverse_rollpitchyaw_matrix(const double& roll, const double& pitch, const double& yaw);
Matrix33<double> yaw_matrix(const double& yaw);
Matrix33<double> pitch_matrix(const double& pitch);
Matrix33<double> roll_matrix(const double& roll);
////////////////////////////////////////////////////////////////////////////////
#endif
