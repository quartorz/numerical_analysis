#include "7.h"
#include <iostream>
#include <cmath>

int main()
{
	auto f = [](double x){
		return std::exp(x) / (1 + x);
	};
	std::cout.precision(10);
	std::cout.setf(std::ostream::scientific);
	std::cout << numerical_analysis::multiple_segment_trapezoidal_rule(
		f,
		0.,
		1.,
		20
	) << std::endl;
	std::cout << numerical_analysis::multiple_segment_simpsons_rule(
		f,
		0.,
		1.,
		20
	) << std::endl;
}
