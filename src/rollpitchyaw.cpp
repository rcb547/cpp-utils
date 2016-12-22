/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#include <cmath>
#include "rollpitchyaw.h"

#define D2R 0.0174532925199432957692369076848
#define R2D 57.295779513082320876798154814105

///////////////////////////////////////////////////////////////////////////
Matrix33<double> rollpitchyaw_matrix(const double& roll, const double& pitch, const double& yaw)
{
	double cosr = cos(D2R*roll);
	double cosp = cos(D2R*pitch);
	double cosy = cos(D2R*yaw);

	double sinr = sin(D2R*roll);
	double sinp = sin(D2R*pitch);
	double siny = sin(D2R*yaw);
	
	Matrix33<double> R;
	R.e11 = cosp*cosy; 
	R.e12 = cosp*siny;
	R.e13 = -sinp;

	R.e21 = sinr*sinp*cosy - cosr*siny;
	R.e22 = sinr*sinp*siny + cosr*cosy;
	R.e23 = sinr*cosp;

	R.e31 = cosr*sinp*cosy + sinr*siny;
	R.e32 = cosr*sinp*siny - sinr*cosy;
	R.e33 = cosr*cosp;

	return R;
}
////////////////////////////////////////////////////////////////
Matrix33<double> inverse_rollpitchyaw_matrix(const double& roll, const double& pitch, const double& yaw)
{
	double cosr = cos(D2R*roll);
	double cosp = cos(D2R*pitch);
	double cosy = cos(D2R*yaw);

	double sinr = sin(D2R*roll);
	double sinp = sin(D2R*pitch);
	double siny = sin(D2R*yaw);
	
	Matrix33<double> R;
	R.e11 = cosp*cosy; 
	R.e21 = cosp*siny;
	R.e31 = -sinp;

	R.e12 = sinr*sinp*cosy - cosr*siny;
	R.e22 = sinr*sinp*siny + cosr*cosy;
	R.e32 = sinr*cosp;

	R.e13 = cosr*sinp*cosy + sinr*siny;
	R.e23 = cosr*sinp*siny - sinr*cosy;
	R.e33 = cosr*cosp;

	return R;
}
////////////////////////////////////////////////////////////////////////////
Matrix33<double> yaw_matrix(const double& yaw)
{
	double cosy = cos(yaw);
	double siny = sin(yaw);
	
	Matrix33<double> R;

	R.e11 = cosy; 
	R.e12 = siny;
	R.e13 = 0.0;

	R.e21 = -siny;
	R.e22 = cosy;
	R.e23 = 0.0;

	R.e31 = 0.0;
	R.e32 = 0.0;
	R.e33 = 1.0;

	return R;
}
////////////////////////////////////////////////////////////////////////////
Matrix33<double> pitch_matrix(const double& pitch)
{
	double cosp = cos(pitch);
	double sinp = sin(pitch);
	
	Matrix33<double> R;

	R.e11 = cosp; 
	R.e12 = 0.0;
	R.e13 = -sinp;

	R.e21 = 0.0;
	R.e22 = 1.0;
	R.e23 = 0.0;

	R.e31 = sinp;
	R.e32 = 0.0;
	R.e33 = cosp;

	return R;
}
////////////////////////////////////////////////////////////////////////////
Matrix33<double> roll_matrix(const double& roll)
{
	double cosr = cos(roll);
	double sinr = sin(roll);
	
	Matrix33<double> R;

	R.e11 = 1.0; 
	R.e12 = 0.0;
	R.e13 = 0.0;

	R.e21 = 0.0;
	R.e22 = cosr;
	R.e23 = sinr;

	R.e31 = 0.0;
	R.e32 = -sinr;
	R.e33 = cosr;

	return R;
}
///////////////////////////////////////////////////////////////////////////
