/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <Eigen/Dense>

//The returnd 3x3 rotation matrices are position vector rotations about the origin
//u = R  * v : rotates a position vector v about the origin and gives position vector u in the original coordinate system
//u = R' * v : rotates the axes about the origin and gives u in the new rotated coordinate system
//Be careful about this distinction
//The order of rotations matters

template<class T>
Eigen::Matrix<T, 3, 3> rollpitchyaw_matrix(const T& roll, const T& pitch, const T& yaw)
{			
	//Roll then pitch then yaw order - specify angles in radians	
	//Same as yaw_matrix(yaw) * pitch_matrix(pitch) * roll_matrix(roll)
	const T cosr = cos(roll);
	const T cosp = cos(pitch);
	const T cosy = cos(yaw);

	const T sinr = sin(roll);
	const T sinp = sin(pitch);
	const T siny = sin(yaw);

	Eigen::Matrix<T, 3, 3> m;
	m(0,0) = cosp * cosy;
	m(0,1) = cosy * sinp * sinr - cosr * siny;
	m(0,2) = sinr * siny + cosr * cosy * sinp;
	m(1,0) = cosp * siny;
	m(1,1) = cosr * cosy + sinp * sinr * siny;
	m(1,2) = cosr * sinp * siny - cosy * sinr;
	m(2,0) = -sinp;
	m(2,1) = cosp * sinr;
	m(2,2) = cosp * cosr;	
	return m;
}

template<class T>
Eigen::Matrix<T, 3, 3> yawpitchroll_matrix(const T& roll, const T& pitch, const T& yaw)
{	
	//Yaw then pitch then roll order - specify angles in radians	
	//Same as roll_matrix(roll) * pitch_matrix(pitch) * yaw_matrix(yaw)
	//I think this is the aviation convention order
	const T cosr = cos(roll);
	const T cosp = cos(pitch);
	const T cosy = cos(yaw);

	const T sinr = sin(roll);
	const T sinp = sin(pitch);
	const T siny = sin(yaw);

	Eigen::Matrix<T, 3, 3> m;
	m(0,0) = cosp * cosy;
	m(0,1) = -cosp * siny;
	m(0,2) = sinp;
	m(1,0) = cosr * siny + cosy * sinp * sinr;
	m(1,1) = cosr * cosy - sinp * sinr * siny;
	m(1,2) = -cosp * sinr;
	m(2,0) = sinr * siny - cosr * cosy * sinp;
	m(2,1) = cosy * sinr + cosr * sinp * siny;
	m(2,2) = cosp * cosr;
	return m;
}

template<class T>
Eigen::Matrix<T, 3, 3> roll_matrix(const T& roll)
{
	//Specify angles in radians
	const T cosr = cos(roll);
	const T sinr = sin(roll);
	Eigen::Matrix<T,3,3> m;
	m(0,0) = 1.0; 	m(0,1) = 0.0; 	m(0,2) = 0.0;
	m(1,0) = 0.0; 	m(1,1) = cosr; 	m(1,2) = -sinr;
	m(2,0) = 0.0; 	m(2,1) = sinr;  m(2,2) = cosr;
	return m;
}

template<class T>
Eigen::Matrix<T, 3, 3> pitch_matrix(const T& pitch)
{
	//Specify angles in radians
	const T cosp = cos(pitch);
	const T sinp = sin(pitch);
	Eigen::Matrix<T, 3, 3> m;
	m(0,0) = cosp;  m(0,1) = 0.0; m(0,2) = sinp;
	m(1,0) = 0.0;   m(1,1) = 1.0; m(1,2) = 0.0;
	m(2,0) = -sinp; m(2,1) = 0.0; m(2,2) = cosp;
	return m;
}

template<class T>
Eigen::Matrix<T, 3, 3> yaw_matrix(const T& yaw)
{
	//Specify angles in radians
	const T cosy = cos(yaw);
	const T siny = sin(yaw);
	Eigen::Matrix<T, 3, 3> m;
	m(0,0) = cosy;  m(0,1) = -siny; m(0,2) = 0.0;
	m(1,0) = siny;  m(1,1) =  cosy; m(1,2) = 0.0;
	m(2,0) = 0.0; 	m(2,1) =  0.0;  m(2,2) = 1.0;
	return m;
}

