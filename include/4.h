#pragma once

#define _USE_MATH_DEFINES

#include <Windows.h>
#include <d2d1.h>
#include <vector>
#include <cmath>

namespace numerical_analysis{
	bool plot_init();
	void plot_uninit();
	struct SubplotData;
	class Plot{
		friend bool plot_init();
		static LRESULT CALLBACK WindowProc_Static(HWND, UINT, WPARAM, LPARAM);
		LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
		bool CreateResource();
		void DestroyResource();
		void OnPaint();
		CRITICAL_SECTION cs;
		HWND hwnd;
		ID2D1HwndRenderTarget *target;
		std::vector<SubplotData> plotdata;
		unsigned row, column, sub;
		int mode;
	public:
		Plot(const wchar_t *windo_title);
		~Plot();
		void set_window_title(const wchar_t *title);
		void create_subplot(unsigned row, unsigned column);
		void set_subplot(unsigned row, unsigned column);
		void set_subplot(unsigned subplot);
		enum PlotMode{
			Line = 0, Point = 1
		};
		void set_plot_mode(PlotMode mode);
		void plot(std::vector<double> &x, std::vector<double> &y);
		void plot(std::vector<std::pair<double, double>> &pt);
		void set_plot_title(const wchar_t *title);
		enum Position{
			Left, Center, Right,
			Top = 0,      Bottom = 2
		};
		void set_plot_title_pos(Position x, Position y);
		void set_x_range(double min, double max);
		void set_y_range(double min, double max);
		void clear();
		void clear_all();
	};

	// [min, max]をn等分する
	inline void generate_numbers(std::vector<double> &out, double min, double max, unsigned n)
	{
		out.clear();
		double d = max - min;
		for(unsigned i = 0; i <= n; i++)
			out.push_back(min + d * i / n);
	}
	// [min, max]をn等分する
	inline std::vector<double> generate_numbers(double min, double max, unsigned n)
	{
		std::vector<double> out(n + 1);
		double d = max - min;
		for(unsigned i = 0; i <= n; i++)
			out[i] = min + d * i / n;
		return out;
	}

	// 区間[-1, 1]をk分割するチェビシェフ点を返す
	inline std::vector<double> generate_chebyshev_points(unsigned k)
	{
		std::vector<double> ret(k);
		for(unsigned i = 0; i < k; i++)
			ret[i] = std::cos((2 * i + 1) * M_PI / (2 * k));
		return ret;
	}

	class Interpolation{
		std::vector<double> x, c;
	public:
		Interpolation(std::vector<double> &x, std::vector<double> &y)
			: x(x), c(y)
		{
			for(unsigned i = 0; i < x.size(); i++)
				for(unsigned j = 0; j < x.size(); j++)
					if(i != j)
						c[i] /= x[i] - x[j];
		}
		double operator()(double x)
		{
			double ret = 0;
			for(unsigned i = 0; i < this->x.size(); i++){
				double a = 1;
				for(unsigned j = 0; j < this->x.size(); j++)
					if(i != j)
						a *= x - this->x[j];
				a *= c[i];
				ret += a;
			}
			return ret;
		}
	};
}
