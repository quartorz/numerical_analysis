#include <vector>
#include <deque>

#define GAUSSIAN_ELIMINATION_CHECK_SIZE
#include "3A.h"

#include <iostream>
#include <typeinfo>

class Array{
	double a[11];
public:
	typedef double valuetype;
	int getsize()const{return 11;}
	double &operator[](int i){return a[i];}
};

NUMERICAL_ANALYSIS_REGISTER_ARRAY_CLASS(Array, getsize(), valuetype);

template <class Matrix, class Answer>
void test(Matrix &matrix, Answer &ans)
{
	using namespace numerical_analysis;

	std::cout << "type of matrix: " << typeid(Matrix).name() << std::endl;
	std::cout << "type of answer: " << typeid(Answer).name() << std::endl;

	for(int i = 0; i < 10; i++){
		for(int j = i; j < 10; j++){
			matrix[i][j] = matrix[j][i] = 1 / (j - i + 1);
		}
		matrix[i][10] = 0;
		for(int j = 0; j < 10; j++){
			matrix[i][10] += matrix[i][j];
		}
	}

	gaussian_elimination<10>(matrix, ans, [](){});

	std::cout << "answer: ";
	for(int i = 0; i < 10; i++){
		std::cout << ans[i] << ' ';
	}
	std::cout << std::endl;
}

int main()
{
	{
		double m[10][11], ans[10];
		test(m, ans);
	}
	{
		std::vector<double> m[10];
		std::vector<double> ans(10);
		for(int i = 0; i < 10; i++)
			m[i].resize(11);
		test(m, ans);
	}
	{
		std::vector<double*> m(10);
		std::deque<double> ans(10);
		for(int i = 0; i < 10; i++)
			m[i] = new double[11];
		test(m, ans);
		for(int i = 0; i < 10; i++)
			delete m[i];
	}
	{
		auto m = new std::vector<double>[10];
		std::deque<double> ans(10);
		for(int i = 0; i < 10; i++)
			m[i].resize(11);
		test(m, ans);
		delete[] m;
	}
	{
		std::vector<std::deque<double>> m(10);
		double ans[10];
		for(int i = 0; i < 10; i++)
			m[i].resize(11);
		test(m, ans);
	}
	{
		double **m = new double*[10];
		Array ans;
		for(int i = 0; i < 10; i++)
			m[i] = new double[11];
		test(m, ans);
		for(int i = 0; i < 10; i++)
			delete[] m[i];
		delete[] m;
	}
	{
		Array m[10];
		Array ans;
		test(m, ans);
	}
}

