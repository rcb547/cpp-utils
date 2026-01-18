#pragma once
#include <type_traits>
#include <complex>
#include <limits>

namespace CppUtils {
	template <typename T>
	static bool nearly_equal(const T a, const T b, const T rel_th = 128 * std::numeric_limits<T>::epsilon(), T abs_th = std::numeric_limits<T>::epsilon())
	{
		// Adapted from an answer in https://stackoverflow.com/questions/4915462/how-should-i-do-floating-point-comparison
		assert(std::numeric_limits<T>::epsilon() <= rel_th);
		assert(rel_th < (T)1);

		if (a == b) return true;

		auto diff = std::abs(a - b);
		auto norm = std::min((std::abs(a) + std::abs(b)), std::numeric_limits<T>::max());
		return diff < std::max(abs_th, rel_th * norm);
	};

	template <class T>
	std::enable_if_t<std::numeric_limits<T>::is_integer == false, bool>
		equal_within_ulps(const T x, const T y, const std::size_t n) {
		// From https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
		const T m = std::min(std::fabs(x), std::fabs(y));
		const int exp = m < std::numeric_limits<T>::min()
			? std::numeric_limits<T>::min_exponent - 1
			: std::ilogb(m);
		//T a = std::fabs(x - y);
		//T b = n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
		return std::fabs(x - y) <= n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
	}

	template <typename T>
	static bool nearly_equal_ulps(const T x, const T y)
	{
		return equal_within_ulps<T>(x, y, 0);
	};

	template <typename T>
	static T pct_diff(const T refval, const T val) {
		return (T)100.0 * (val - refval) / refval;
	};

	template <typename T>
	static std::complex<T> pct_diff(const std::complex<T> refval, const std::complex<T> val) {
		return std::complex<T>(pct_diff(refval.real(), val.real()), pct_diff(refval.imag(), val.imag()));
	};

	double pct_diff_ex(const double& a, const double& b, const double tol = std::numeric_limits<double>::epsilon()) {
		double r = 100.0 * (b - a) / a;
		//constexpr double eps = std::numeric_limits<double>::epsilon();
		//constexpr double eps = 1e-14;
		// Amend for closeness within numerical precision	
		if (nearly_equal_ulps(a, b)) r = 0.0;
		else if (a == 0.0 && b == 0.0) r = 0.0;
		else if (std::abs(b - a) <= tol) r = 0.0;
		else if (std::abs(b - a) <= tol && std::abs(a) <= tol) r = 0.0;
		return r;
	};

	std::complex<double> pct_diff_ex(const std::complex<double>& a, const std::complex<double>& b, const double tol = std::numeric_limits<double>::epsilon()) {
		std::complex<double> r(pct_diff_ex(a.real(), b.real(), tol), pct_diff_ex(a.imag(), b.imag(), tol));
		return r;
	};


	template <typename T>
	T stable_tanh(const T& x) {
		//tanh(x) = (1.0 - exp(-4x))/(1.0 + 2.0*exp(-2x) + exp(-4x));
		const T e2 = exp((T)-2.0 * x);
		const T e4 = e2 * e2;
		const T tanhx = ((T)1.0 - e4) / ((T)1.0 + (T)2.0 * e2 + e4);
		return tanhx;
	}

	// The limit b is adjusted to lie on a node for the given limit a and density given floating point precision
	// Returns n and the spacing
	template <typename T>
	static void spacing_ab_density1(const T& a, T& b, const T& density, size_t& n, T& spacing) {
		spacing = (T)1.0 / density;
		n = 1 + std::ceil((b - a) / spacing);
		b = a + (T)(n - 1) * spacing;
	};

	template <typename T>
	static void spacing_ab_n(const T& a, T& b, const size_t& n, T& spacing) {
		spacing = (b - a) / (T)(n - 1);
		b = a + (T)(n - 1) * spacing;
	};

	template <typename T>
	T smaller_even_divisor(const T& n, const T& start) {
		T d = std::min(n, start);
		while (n % d != 0) {
			d--;
		}
		return d;
	}

	/*
	template <typename T>
	int test_ulps() {
		T x, y;
		std::cout << std::setprecision(16);

		x = 1.0; y = 1.0;
		std::cout << "x=" << x << " y=" << y << " : " << nearly_equal_ulps(x, y) << std::endl;

		x = 1.0; y = 1.0 + 0.5 * std::numeric_limits<T>::epsilon();
		std::cout << "x=" << x << " y=" << y << " : " << nearly_equal_ulps(x, y) << std::endl;

		x = 1.0; y = 1.0 + std::numeric_limits<T>::epsilon();
		std::cout << "x=" << x << " y=" << y << " : " << nearly_equal_ulps(x, y) << std::endl;

		x = 1.0; y = 1.0 + 2.0*std::numeric_limits<T>::epsilon();
		std::cout << "x=" << x << " y=" << y << " : " << nearly_equal_ulps(x, y) << std::endl;

		x = 3000.0; y = 3000.0 + std::numeric_limits<T>::epsilon();
		std::cout << "x=" << x << " y=" << y << " : " << nearly_equal_ulps(x, y) << std::endl;

		return 0;
	};

	template <typename T>
	void test_tanh() {
		const int n = 400;
		std::vector<T> x = linspace<T>(-100, 100, n);
		for (int i = 0; i < n; i++) {
			T t0 = std::tanh(x[i]);
			T t1 = stable_tanh(x[i]);
			std::cout << std::fixed << std::setprecision(16)
				<< x[i] << " "
				<< t0 << " "
				<< t1 << std::endl;
		}
	}*/
};