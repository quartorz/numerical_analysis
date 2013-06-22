#include "4.h"
#include <conio.h>
#include <algorithm>
#include <iostream>

using namespace numerical_analysis;

int main()
{
	plot_init();
	{
		const wchar_t *titles[] = {
			L"ラグランジュ補間 4次",
			L"ラグランジュ補間 6次",
			L"チェビシェフ補間 4次",
			L"チェビシェフ補間 6次",
		};
		int n[] = {4, 6, 4, 6};

		Plot plt(L"数値解析");
		plt.create_subplot(2, 2);
		plt.set_plot_title_pos(Plot::Left, Plot::Top);
		std::vector<double> x, y;

		auto draw_func = [&](){
			std::vector<double> x = generate_numbers(-1, 1, 200), y(x.size());
			std::transform(x.begin(), x.end(), y.begin(), [](double x){
				return 1 / (1 + 25 * x * x);
			});
			plt.plot(x, y);
		};
		auto draw_lagrange_interpolation = [&](int n){
			std::vector<double> x = generate_numbers(-1, 1, n), y(x.size());
			std::transform(x.begin(), x.end(), y.begin(), [](double x){
				return 1 / (1 + 25 * x * x);
			});
			plt.set_plot_mode(Plot::Point);
			plt.plot(x, y);
			plt.set_plot_mode(Plot::Line);
			Interpolation interpolation(x, y);
			generate_numbers(x, -1, 1, 200);
			y.resize(x.size());
			std::transform(x.begin(), x.end(), y.begin(), [&](double x){
				return interpolation(x);
			});
			plt.plot(x, y);
		};
		auto draw_chebyshev_interpolation = [&](int n){
			std::vector<double> x = generate_chebyshev_points(n + 1), y(x.size());
			std::transform(x.begin(), x.end(), y.begin(), [](double x){
				return 1 / (1 + 25 * x * x);
			});
			plt.set_plot_mode(Plot::Point);
			plt.plot(x, y);
			plt.set_plot_mode(Plot::Line);
			Interpolation interpolation(x, y);
			generate_numbers(x, -1, 1, 200);
			y.resize(x.size());
			std::transform(x.begin(), x.end(), y.begin(), [&](double x){
				return interpolation(x);
			});
			plt.plot(x, y);
		};

		for(unsigned i = 0; i < 2; i++){
			plt.set_subplot(i);
			plt.set_x_range(-1.3, 1.3);
			plt.set_y_range(-0.5, 1.3);
			plt.set_plot_title_pos(Plot::Left, Plot::Top);
			plt.set_plot_title(titles[i]);
			draw_func();
			draw_lagrange_interpolation(n[i]);
		}

		for(unsigned i = 2; i < 4; i++){
			plt.set_subplot(i);
			plt.set_x_range(-1.3, 1.3);
			plt.set_y_range(-0.5, 1.3);
			plt.set_plot_title_pos(Plot::Left, Plot::Top);
			plt.set_plot_title(titles[i]);
			draw_func();
			draw_chebyshev_interpolation(n[i]);
		}

		std::cout << "Press any key..." << std::endl;
		::getch();
	}
	plot_uninit();
}
