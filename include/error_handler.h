#pragma once

namespace numerical_analysis{
	template<typename Ret>
	// �v�Z�s���̃n���h��
	struct empty_handler{
		typename std::remove_cv<Ret>::type operator()(void)
		{
			return typename std::remove_cv<Ret>::type();
		}
	};
}
