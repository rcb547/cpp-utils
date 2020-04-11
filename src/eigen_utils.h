/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _eigen_utils_H
#define _eigen_utils_H

#include <vector>
#include <Eigen/Dense>
typedef Eigen::VectorXd VectorDouble;
typedef Eigen::MatrixXd MatrixDouble;

#include "random_utils.h"

template<typename T>
void print(const Eigen::Matrix<T, -1, -1>& A, const std::string& name)
{
	std::cout << name << std::endl;
	for (auto i = 0; i < A.rows(); i++) {
		for (auto j = 0; j < A.cols(); j++) {
			std::cout << A(i, j) << " ";
		}
		std::cout << std::endl;
	}
};

template<typename T>
void writetofile(const Eigen::Matrix<T,-1,-1>& A, const std::string& path)
{
	std::ofstream ofs(path, std::ofstream::out);
	for (auto i = 0; i < A.rows(); i++) {
		for (auto j = 0; j < A.cols(); j++) {
			ofs << i + 1 << "\t" << j + 1 << "\t" << A(i, j) << std::endl;;
		}
	}
};

template<typename T>
void writetofile(const Eigen::Matrix<T,-1,1>& x, const std::string& path)
{
	std::ofstream ofs(path, std::ofstream::out);
	for (auto i = 0; i < x.rows(); i++) {
		ofs << x[i] << std::endl;
	}
};

template<typename VecType>
void writetofile(const VecType& x, const std::string& path)
{
	std::ofstream ofs(path, std::ofstream::out);
	for (size_t i = 0; i < x.size(); i++) {		
		ofs << x[i] << std::endl;
	}	
};

//template<typename T>
//std::vector<T> operator*(const Eigen::Matrix<T, -1, -1>& A, const std::vector<T>& x)
//{
//	size_t M = A.rows();
//	size_t N = A.cols();
//	std::vector<double> b(M);
//	for (auto i = 0; i < M; i++) {
//		b[i] = 0.0;
//		for (auto j = 0; j < N; j++) {
//			b[i] += A(i, j) * x[j];
//		}
//	}
//	return b;
//};

template<typename T>
double mtDm(const Eigen::Matrix<T, -1, 1>& m, const Eigen::Matrix<T, -1, -1>& D)
{
	//D must be diagonal
	size_t n = m.rows();
	double sum = 0.0;
	for (size_t i = 0; i < n; i++)sum += (m[i] * m[i] * D(i, i));
	return sum;
};

template<typename T>
double mtDm(const std::vector<double>& m, const Eigen::Matrix<T, -1, -1>& D)
{
	//D must be diagonal
	size_t n = (size_t)m.size();
	double sum = 0.0;
	for (size_t i = 0; i < n; i++) {
		sum += (m[i] * m[i] * D(i, i));
	}
	return sum;
};

template<typename T>
T mtAm(const Eigen::Matrix<T, -1, 1>& m, const Eigen::Matrix<T, -1, -1>& A)
{
	return m.transpose() * (A * m);	
};

template<typename T>
T mtAm(const std::vector<double>& m, const Eigen::Matrix<T, -1, -1>& A)
{
	std::vector<T> a = A * m;
	T sum = 0.0;
	size_t n = (size_t)m.size();
	for (size_t i = 0; i < n; i++) {
		sum += m[i] * a[i];
	}
	return sum;
};

template<typename MatType>
MatType pseudoInverse(const MatType& A, double epsilon = std::numeric_limits<double>::epsilon())
{
	//See https://eigen.tuxfamily.org/bz/show_bug.cgi?id=257
	assert(A.rows() >= A.cols());
	Eigen::JacobiSVD<MatType> svd(A, Eigen::ComputeThinU | Eigen::ComputeThinV);
	double tolerance = epsilon*std::max(A.cols(),A.rows())*svd.singularValues().array().abs()(0);
	return svd.matrixV() * (svd.singularValues().array().abs() > tolerance).select(svd.singularValues().array().inverse(),0).matrix().asDiagonal()*svd.matrixU().adjoint();
}

template<typename T>
Eigen::Matrix<T, -1, 1> get_nrand(size_t n, const T& mean, const T& stddev)
{
	Eigen::Matrix<T, -1, 1> x(n);
	std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
	std::normal_distribution<T> dist(mean, stddev);
	for (size_t i = 0; i < x.rows(); i++) {
		x[i] = dist(gen);
	}
	return x;
};

template<typename T>
std::vector<T> mvnrand_lowercholesky(const Eigen::Matrix<T, -1, -1>& L)
{
	//L - lower diagonal of the Cholesky decomposition
	size_t n = L.rows();
	VectorDouble u = get_nrand<double>(n, 0.0, 1.0);	
	VectorDouble v = L * u;
	std::vector<double> x(n);
	for (size_t i = 0; i < n; i++) {
		x[i] = v[i];
	}
	return x;
};

template<typename T>
std::vector<T> mvnrand_covariance(const Eigen::Matrix<T, -1, -1>& C)
{
	Eigen::LLT<const Eigen::Matrix<T, -1, -1>&> lu(C);
	return mvnrand_lowercholesky(lu.matrixL());
};

template<typename T>
T mvgaussian_pdf(const VectorDouble& m0, const Eigen::Matrix<T, -1, -1>& C, const VectorDouble& m)
{
	size_t k = m0.rows();
	MatrixDouble I = MatrixDouble::Identity(k, k);

	Eigen::FullPivLU<MatrixDouble> lu(C);
	MatrixDouble invC = lu.solve(I);
	double detC = lu.determinant();

	VectorDouble dm = m - m0;
	double a = -0.5 * mtAm(dm, invC);
	double pdf = exp(a) / sqrt(pow(TWOPI, k) * detC);
	return pdf;
};

#endif


