#include <cmath>
#include <iostream>
#include <iomanip>

#include "2.h"

double f(double x)
{
	return std::exp(x) - x * x - 2;
}

double df(double x)
{
	return std::exp(x) - 2 * x;
}

int main()
{
	using namespace numerical_analysis;

	std::cout.setf(std::ios::scientific);
	std::cout.precision(15);
	std::cout << bisection_method<double,double>(1. ,2. ,f) << std::endl;
	std::cout << regula_falsi_method<double,double>(1. ,2. ,f) << std::endl;
	std::cout << secant_method<double,double>(1. ,2. ,f, [](){return 0;}) << std::endl;
	std::cout << newton_method<double,double>(2. ,f, df, [](){return 0;}) << std::endl;
//	std::cout << newton_method<double,double>(2. ,f) << std::endl;

	return 0;
}
