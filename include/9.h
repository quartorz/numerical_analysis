#pragma once

#include <utility>
#include <type_traits>

namespace numerical_analysis{
	template <typename Arg, typename Ret, typename Func, typename Func2, typename OnError>
	// ニュートン法(導関数有)
	// x0: 初期値
	// f: 関数
	// df: 導関数
	// n: 計算回数
	// epsilon: 精度
	// onerror: 計算不可の時に呼び出す関数
	inline typename std::remove_cv<Ret>::type newton_method(
		typename std::remove_cv<Arg>::type x0,
		Func f,
		Func2 df,
		OnError onerror,
		const unsigned int n = 1000,
		const typename std::remove_cv<Ret>::type epsilon = 10E-12l
		)
	{
		typedef typename std::remove_cv<Arg>::type arg_type;
		typedef typename std::remove_cv<Ret>::type ret_type;

		arg_type x;

		for(unsigned int i = 0; i < n; ++i){
			x = x0 - f(x0) / df(x0);

			if(static_cast<ret_type>(std::abs(x - x0)) < epsilon)
				return std::move(x);

			x0 = std::move(x);
		}

		return std::move(onerror());
	}

	template <unsigned k, typename Type>
	class legendre_polynomial{
	public:
		inline Type operator()(const Type &x) const
		{
			return ((2 * k - 1) * x * legendre_polynomial<k - 1, double>()(x) - (k - 1) * legendre_polynomial<k - 2, double>()(x)) / k;
		}
		Type differentiate(const Type &x) const
		{
			return k * (x * operator()(x) - legendre_polynomial<k - 1, Type>()(x)) / (x * x - 1);
		}
	};

	template <typename Type>
	class legendre_polynomial<0, Type>{
	public:
		inline Type operator()(const Type &x) const
		{
			return Type(1.0);
		}
		Type differentiate(const Type &x) const 
		{
			return Type(0.0);
		}
	};

	template <typename Type>
	class legendre_polynomial<1, Type>{
	public:
		inline Type operator()(const Type &x) const
		{
			return x;
		}
		Type differentiate(const Type &x) const
		{
			return Type(1.0);
		}
	};

	template <class T>
	class empty_handler{
	public:
		inline T operator()(...) const
		{
			return T();
		}
	};
}
