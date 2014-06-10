#pragma once

namespace numerical_analysis{
	template<typename Ret>
	// 計算不可時のハンドラ
	struct empty_handler{
		typename std::remove_cv<Ret>::type operator()(void)
		{
			return typename std::remove_cv<Ret>::type();
		}
	};
}
