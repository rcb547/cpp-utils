/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include "eigen_utils.hpp"

// Angles are specified in RADIANS unledd its an "_degrees" function
// The returnd 3x3 rotation matrices are position vector rotations about the origin
// u = R  * v : rotates a position vector v about the origin and gives position vector u in the original coordinate system
// u = R' * v : rotates the axes about the origin and gives u in the new rotated coordinate system
// Be careful about this distinction
// The order of rotations matters

namespace CppUtils {
	template<class T>
	Eigen::Matrix<T, 3, 3> rollpitchyaw_matrix(const T& roll_radians, const T& pitch_radians, const T& yaw_radians) {
		//Roll then pitch then yaw order - specify angles in radians	
		//Same as yaw_matrix(yaw) * pitch_matrix(pitch) * roll_matrix(roll)
		Eigen::AngleAxis<double> AR(roll_radians, Eigen::Vector3d::UnitX());
		Eigen::AngleAxis<double> AP(pitch_radians, Eigen::Vector3d::UnitY());
		Eigen::AngleAxis<double> AY(yaw_radians, Eigen::Vector3d::UnitZ());
		Eigen::Quaterniond Q(AY * AP * AR); Q.normalize();
		return Q.toRotationMatrix();
	};

	template<class T>
	Eigen::Matrix<T, 3, 3> yawpitchroll_matrix(const T& roll_radians, const T& pitch_radians, const T& yaw_radians) {
		//Yaw then pitch then roll order - specify angles in radians	
		//Same as roll_matrix(roll) * pitch_matrix(pitch) * yaw_matrix(yaw)
		//I think this is the aviation convention order
		Eigen::AngleAxis<double> AR(roll_radians, Eigen::Vector3d::UnitX());
		Eigen::AngleAxis<double> AP(pitch_radians, Eigen::Vector3d::UnitY());
		Eigen::AngleAxis<double> AY(yaw_radians, Eigen::Vector3d::UnitZ());
		Eigen::Quaterniond Q(AR * AP * AY); Q.normalize();
		return Q.toRotationMatrix();
	};

	template<class T>
	Eigen::Matrix<T, 3, 3> roll_matrix(const T& roll_radians) {
		//Specify angles in radians
		const T cosr = cos(roll_radians);
		const T sinr = sin(roll_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = 1.0; 	m(0, 1) = 0.0; 	m(0, 2) = 0.0;
		m(1, 0) = 0.0; 	m(1, 1) = cosr; m(1, 2) = -sinr;
		m(2, 0) = 0.0; 	m(2, 1) = sinr; m(2, 2) = cosr;
		return m;
	};

	template<class T>
	Eigen::Matrix<T, 3, 3> pitch_matrix(const T& pitch_radians) {
		//Specify angles in radians
		const T cosp = cos(pitch_radians);
		const T sinp = sin(pitch_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = cosp;  m(0, 1) = 0.0; m(0, 2) = sinp;
		m(1, 0) = 0.0;   m(1, 1) = 1.0; m(1, 2) = 0.0;
		m(2, 0) = -sinp; m(2, 1) = 0.0; m(2, 2) = cosp;
		return m;
	}

	template<class T>
	Eigen::Matrix<T, 3, 3> yaw_matrix(const T& yaw_radians) {
		//Specify angles in radians
		const T cosy = cos(yaw_radians);
		const T siny = sin(yaw_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = cosy;  m(0, 1) = -siny; m(0, 2) = 0.0;
		m(1, 0) = siny;  m(1, 1) = cosy; m(1, 2) = 0.0;
		m(2, 0) = 0.0; 	m(2, 1) = 0.0;  m(2, 2) = 1.0;
		return m;
	}

	template<class T>
	Eigen::Matrix<T, 3, 3> roll_matrix_derivative(const T& roll_radians) {
		//Specify angles in radians
		const T cosr = cos(roll_radians);
		const T sinr = sin(roll_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = 0.0; 	m(0, 1) = 0.0; m(0, 2) = 0.0;
		m(1, 0) = 0.0; 	m(1, 1) = -sinr; m(1, 2) = -cosr;
		m(2, 0) = 0.0; 	m(2, 1) = cosr; m(2, 2) = -sinr;
		return m;
	};

	template<class T>
	Eigen::Matrix<T, 3, 3> pitch_matrix_derivative(const T& pitch_radians) {
		//Specify angles in radians
		const T cosp = cos(pitch_radians);
		const T sinp = sin(pitch_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = -sinp; m(0, 1) = 0.0; m(0, 2) = cosp;
		m(1, 0) = 0.0; m(1, 1) = 0.0; m(1, 2) = 0.0;
		m(2, 0) = -cosp; m(2, 1) = 0.0; m(2, 2) = -sinp;
		return m;
	}

	template<class T>
	Eigen::Matrix<T, 3, 3> yaw_matrix_derivative(const T& yaw_radians) {
		//Specify angles in radians
		const T cosy = cos(yaw_radians);
		const T siny = sin(yaw_radians);
		Eigen::Matrix<T, 3, 3> m;
		m(0, 0) = -siny;  m(0, 1) = -cosy; m(0, 2) = 0.0;
		m(1, 0) = cosy;  m(1, 1) = -siny; m(1, 2) = 0.0;
		m(2, 0) = 0.0;  m(2, 1) = 0.0; m(2, 2) = 0.0;
		return m;
	};

	// Input in degrees
	inline static Mat3d roll_matrix_degrees(const double& roll_degrees) {
		return roll_matrix(roll_degrees * D2R<double>);
	};

	inline static Mat3d pitch_matrix_degrees(const double& pitch_degrees) {
		return pitch_matrix(pitch_degrees * D2R<double>);
	};

	inline static Mat3d yaw_matrix_degrees(const double& yaw_degrees) {
		return yaw_matrix(yaw_degrees * D2R<double>);
	};

	inline static Mat3d roll_matrix_derivative_degrees(const double& roll_degrees) {
		Mat3d m = roll_matrix_derivative(D2R<double>*roll_degrees);// derivative w.r.t radians
		m *= D2R<double>;
		return m;
	};

	inline static Mat3d pitch_matrix_derivative_degrees(const double& pitch_degrees) {
		Mat3d m = pitch_matrix_derivative(D2R<double>*pitch_degrees);// derivative w.r.t radians
		m *= D2R<double>;
		return m;
	};

	inline static Mat3d yaw_matrix_derivative_degrees(const double& yaw_degrees) {
		Mat3d m = yaw_matrix_derivative(D2R<double>*yaw_degrees);// derivative w.r.t radians
		m *= D2R<double>;
		return m;
	};


	void test_rpy() {
		Vec3d v1(-1, 2, -3);

		Mat3d R = roll_matrix(0.1);
		Mat3d P = pitch_matrix(0.1);
		Mat3d Y = yaw_matrix(0.1);

		Mat3d dR = roll_matrix_derivative(0.1);
		Mat3d dP = pitch_matrix_derivative(0.1);
		Mat3d dY = yaw_matrix_derivative(0.1);

		double del = 0.000001;
		Mat3d R1 = yawpitchroll_matrix(0.1, 0.0, 0.0);
		Mat3d P1 = yawpitchroll_matrix(0.0, 0.1, 0.0);
		Mat3d Y1 = yawpitchroll_matrix(0.0, 0.0, 0.1);
		Mat3d R2 = yawpitchroll_matrix(0.1 + del, 0.0, 0.0);
		Mat3d P2 = yawpitchroll_matrix(0.0, 0.1 + del, 0.0);
		Mat3d Y2 = yawpitchroll_matrix(0.0, 0.0, 0.1 + del);


		std::cout << R << std::endl;
		std::cout << R1 << std::endl;
		std::cout << std::endl;

		std::cout << P << std::endl;
		std::cout << P1 << std::endl;
		std::cout << std::endl;

		std::cout << Y << std::endl;
		std::cout << Y1 << std::endl;

		Vec3d dr1 = ((R2 * v1) - (R1 * v1)) / del;
		Vec3d dr2 = dR * v1;
		Vec3d dp1 = ((P2 * v1) - (P1 * v1)) / del;
		Vec3d dp2 = dP * v1;
		Vec3d dy1 = ((Y2 * v1) - (Y1 * v1)) / del;
		Vec3d dy2 = dY * v1;

		std::cout << std::endl;
		std::cout << dr1 << std::endl;
		std::cout << dr2 << std::endl;
		std::cout << std::endl;

		std::cout << dp1 << std::endl;
		std::cout << dp2 << std::endl;
		std::cout << std::endl;

		std::cout << dy1 << std::endl;
		std::cout << dy2 << std::endl;
		std::cout << std::endl;
	};
};
