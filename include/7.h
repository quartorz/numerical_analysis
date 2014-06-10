#pragma once

#include <cassert>

namespace numerical_analysis{
	template <typename Type, class Func>
	inline Type multiple_segment_trapezoidal_rule(Func f, Type a, Type b, unsigned m)
	{
		Type h = (b - a) / m;
		Type ans = f(a) + f(b);

		for(unsigned i = 1; i < m; ++i)
			ans += 2 * f(a + h * i);
		ans *= h;
		ans /= 2;

		return ans;
	}

	template <typename Type, class Func>
	inline Type multiple_segment_simpsons_rule(Func f, Type a, Type b, unsigned m)
	{
		assert(m % 2 == 0);

		Type h = (b - a) / m;
		Type ans = f(a) + f(b);

		for(unsigned i = 1; i < m; ++i)
			ans += (2 << (i & 1)) * f(a + h * i);
		ans *= h;
		ans /= 3;

		return ans;
	}
}
