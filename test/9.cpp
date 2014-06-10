#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <functional>
#include "9.h"

int main()
{
	using namespace numerical_analysis;
	std::cout.setf(std::ios::scientific);
	std::cout.precision(17);

	const unsigned n = 5;
	typedef legendre_polynomial<n, double> legendre_polynomial;

	for(unsigned i = 1; i <= n; ++i){
		double x = std::sin(((int)n + 1 - 2 * (int)i) * M_PI / (2 * (int)n + 1));
		std::cout << i << ' ';
		std::cout << newton_method<double, double>(
			x,
			legendre_polynomial(),
			std::bind(&legendre_polynomial::differentiate, legendre_polynomial(), std::placeholders::_1),
			empty_handler<double>());
		std::cout << std::endl;
	}
}
