#include "6.h"

#include <iostream>
#include <cmath>

using namespace numerical_analysis;

int main(void)
{
	auto func_y = [&](double t){
		return 2 * std::exp(t * t - 1);
	};
	auto func_y_prime = [](double t, double y){
		return 2 * t * y;
	};
	auto process = [&](const double &t, const double &y, const double &h){
		std::cout << t << ' ' << y << ' ' << std::abs(y - func_y(t)) << ' ' << func_y(t) << std::endl;
	};

	std::cout << "Euler's method" << std::endl;
	initial_value_problem(
		euler_method(),
		func_y_prime,
		1,
		2,
		2,
		0.01,
		process
		);

	std::cout << "quadratic Runge-Kutta method" << std::endl;
	initial_value_problem(
		quadratic_runge_kutta_method(),
		func_y_prime,
		1,
		2,
		2,
		0.01,
		process
		);

	std::cout << "quartic Runge-Kutta method" << std::endl;
	initial_value_problem(
		quartic_runge_kutta_method(),
		func_y_prime,
		1,
		2,
		2,
		0.01,
		process
		);
/*	initial_value_problem(
		euler_method(),
		[](double t, double y){
			return 2 * t * y;
		},
		1,
		2,
		2,
		0.01
		);
	initial_value_problem(
		euler_method(),
		[](double t, double y){
			return 2 * t * y;
		},
		1,
		2,
		2,
		0.01
		);*/
}
