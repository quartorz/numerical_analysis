#pragma once

namespace numerical_analysis{
	template <class Algorithm, class Func, class Process>
	// ‰Šú’l–â‘è
	inline auto initial_value_problem(
		Algorithm algo,
		Func f,
		decltype(f(0., 0.)) t_init,
		const decltype(f(0., 0.)) &t_end,
		decltype(f(0., 0.)) y,
		const decltype(f(0., 0.)) &h,
		Process process
		) -> decltype(f(0., 0.))
	{
		while(t_init < t_end){
			process(t_init, y, h);
			algo(f, t_init, y, h);
		}

		process(t_init, y, h);

		return y;
	}

	struct euler_method{
		template <class Func>
		inline void operator()(
			const Func &f,
			decltype(f(0., 0.)) &t,
			decltype(f(0., 0.)) &y,
			const decltype(f(0., 0.)) &h
			)
		{
			y += f(t, y) * h;
			t += h;
		}
	};

	struct quadratic_runge_kutta_method{
		template <class Func>
		inline void operator()(
			const Func &f,
			decltype(f(0., 0.)) &t,
			decltype(f(0., 0.)) &y,
			const decltype(f(0., 0.)) &h
			)
		{
			decltype(f(0., 0.)) k1 = f(t, y) * h;
			decltype(f(0., 0.)) k2 = f(t + h, y + k1) * h;
			y += (k1 + k2) / 2;
			t += h;
		}
	};

	struct quartic_runge_kutta_method{
		template <class Func>
		inline void operator()(
			const Func &f,
			decltype(f(0., 0.)) &t,
			decltype(f(0., 0.)) &y,
			const decltype(f(0., 0.)) &h
			)
		{
			decltype(f(0., 0.)) k1 = f(t, y) * h;
			decltype(f(0., 0.)) k2 = f(t + h / 2, y + k1 / 2) * h;
			decltype(f(0., 0.)) k3 = f(t + h / 2, y + k2 / 2) * h;
			decltype(f(0., 0.)) k4 = f(t + h, y + k3) * h;
			y += (k1 + 2 * k2 + 2 * k3 + k4) / 6;
			t += h;
		}
	};
}
