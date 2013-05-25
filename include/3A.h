#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <type_traits>

#define NUMERICAL_ANALYSIS_REGISTER_ARRAY_CLASS(classname, get_size, value_type)\
	namespace numerical_analysis{\
		template <>\
		struct get_size_class_impl<classname>{\
			typedef std::size_t size_type;\
			inline static size_type size(classname &container, const std::size_t n = 0)\
			{\
				return container.get_size;\
			}\
		};\
		template <>\
		struct element_of_impl<classname>{\
			typedef classname::value_type type;\
		};\
	}

namespace numerical_analysis{
	template <class T>
	T &&declval();
	template <class T>
	class has_size{
		template <typename U>
		static decltype(declval<U>().size(), void(), std::true_type()) check(U);
		static std::false_type check(...);
	public:
		typedef decltype(check(declval<T>())) type;
		static const bool value = type::value;
	};
	template <class Container>
	struct get_size_class_impl{
		static_assert(has_size<Container>::value, "container type must have function 'size()' and type 'value_type' or be registered by NUMERICAL_ANALYSIS_REGISTER_ARRAY_CLASS");
		typedef typename Container::size_type size_type;
		inline static size_type size(Container &container, const std::size_t n = 0 /* unused */)
		{
			return container.size();
		}
	};
	template <typename Array, std::size_t N>
	struct get_size_class_impl<Array[N]>{
		typedef std::size_t size_type;
		inline static size_type size(Array (&array)[N], const std::size_t n = 0 /* unused */)
		{
			return N;
		}
	};
	template <typename PointerTo>
	struct get_size_class_impl<PointerTo*>{
		typedef std::size_t size_type;
		inline static size_type size(PointerTo *&pointer, const std::size_t n)
		{
			return n;
		}
	};
	template <class Type>
	struct get_size_class : public get_size_class_impl<typename std::remove_cv<typename std::remove_reference<Type>::type>::type>{
	};
	template <class Type>
	// コンテナ、配列、ポインタに関わらずサイズを取得する関数
	typename get_size_class<Type>::size_type get_size(Type &object, std::size_t n = 0)
	{
		return get_size_class<Type>::size(object, n);
	}

	template <class Type>
	class has_value_type{
	private:
		template <class U>
		static decltype(declval<typename U::value_type>(), void(), std::true_type()) check(U);
		static std::false_type check(...);
	public:
		typedef decltype(check(declval<Type>())) type;
		static const bool value = type::value;
	};
	template <class Container>
	struct element_of_impl{
		static_assert(has_value_type<Container>::value, "container type must have function 'size()' and type 'value_type' or be registered by NUMERICAL_ANALYSIS_REGISTER_ARRAY_CLASS");
		typedef typename Container::value_type type;
	};
	template <typename Array, int N>
	struct element_of_impl<Array[N]>{
		typedef Array type;
	};
	template <typename PointerTo>
	struct element_of_impl<PointerTo*>{
		typedef PointerTo type;
	};
	template <class Type>
	struct element_of : public element_of_impl<typename std::remove_cv<typename std::remove_reference<Type>::type>::type>{
	};

	template <class Type>
	class has_operator_bracket{
		template <class U>
		static decltype(declval<U>().operator[](0), void(), std::true_type()) check(U);
		static std::false_type check(...);
	public:
		typedef decltype(check(declval<Type>())) type;
		static const bool value = type::value;
	};
	template <class Type>
	struct is_random_accessible : public std::conditional<
			std::is_array<typename std::remove_cv<Type>::type>::value
				|| std::is_pointer<typename std::remove_cv<Type>::type>::value
				|| has_operator_bracket<typename std::remove_cv<Type>::type>::value,
			std::true_type,
			std::false_type
		>::type
	{
	};

	template <std::size_t N, class T1, class T2, class OnError>
	// ガウスの消去法 Ax=b
	// T1 &a: 拡大係数行列(N行(N+1)列)
	// T2 &ans: 答え
	// epsilon: ピボットを0とみなす閾値
	// onerror: 計算不可の時に呼び出す関数
	// 戻り値 計算できたらtrue
	inline typename std::enable_if<
		is_random_accessible<T1>::value
		&& is_random_accessible<typename element_of<T1>::type>::value
		&& is_random_accessible<T2>::value,
		bool
	>::type gaussian_elimination(
		T1 &a,
		T2 &ans,
		OnError onerror,
		const typename element_of<typename element_of<T1>::type>::type epsilon = 10E-12l
		)
	{
#if defined(GAUSSIAN_ELIMINATION_CHECK_SIZE)
		{
			auto a_size = get_size(a, N);
			assert(a_size >= N);
			for(decltype(a_size) i = 0; i < a_size; i++){
				assert(get_size(a[i], N + 1) >= N + 1);
			}
		}
#endif

		// 前進消去
		for(std::size_t i = 0; i < N - 1; i++){
			std::size_t maxidx = i;
			for(std::size_t j = i + 1; j < N; j++)
				if(std::abs(a[maxidx][i]) < std::abs(a[j][i]))
					maxidx = j;

			if(maxidx != i){
				for(std::size_t j = i; j <= N; j++)
					std::swap(a[maxidx][j], a[i][j]);
			}

			// ここまでピボット選択

			if(std::abs(a[i][i]) <= epsilon){
				onerror();
				return false;
			}
			for(std::size_t j = i + 1; j < N; j++){
				auto m = a[j][i] / a[i][i];
				for(std::size_t k = i; k <= N; k++){
					a[j][k] -= a[i][k] * m;
				}
			}
		}

		// 後退代入
		for(int i = N - 1; i >= 0; i--){
			ans[i] = a[i][N];
			for(std::size_t j = i + 1; j < N; j++){
				ans[i] -= ans[j] * a[i][j];
			}
			ans[i] /= a[i][i];
		}

		return true;
	}

	template <std::size_t N, class T1, class T2, class T3, class OnError>
	// ガウスの消去法 Ax=b
	// 係数と右辺が別なバージョン
	// T1 &A: 係数行列(N次正方行列)
	// T2 &b: 右辺(N次元列ベクトル)
	// T3 &ans: 答え
	// epsilon: ピボットを0とみなす閾値
	// onerror: 計算不可の時に呼び出す関数
	// 戻り値 計算できたらtrue
	inline typename std::enable_if<
		is_random_accessible<T1>::value
		&& is_random_accessible<typename element_of<T1>::type>::value
		&& is_random_accessible<T2>::value
		&& is_random_accessible<T3>::value,
		bool
	>::type gaussian_elimination(
		T1 &A,
		T2 &b,
		T3 &ans,
		OnError onerror,
		const typename element_of<typename element_of<T1>::type>::type epsilon = 10E-12l
		)
	{
#if defined(GAUSSIAN_ELIMINATION_CHECK_SIZE)
		{
			auto a_size = get_size(A, N);
			assert(a_size >= N);
			assert(get_size(b, N) >= N);
			for(decltype(a_size) i = 0; i < a_size; i++){
				assert(get_size(A[i], N) >= N);
			}
		}
#endif

		// 前進消去
		for(std::size_t i = 0; i < N - 1; i++){
			std::size_t maxidx = i;
			for(std::size_t j = i + 1; j < N; j++)
				if(std::abs(A[maxidx][i]) < std::abs(A[j][i]))
					maxidx = j;

			if(maxidx != i){
				for(std::size_t j = i; j < N; j++)
					std::swap(A[maxidx][j], A[i][j]);
				std::swap(b[i], b[maxidx]);
			}

			// ここまでピボット選択

			if(std::abs(A[i][i]) <= epsilon){
				onerror();
				return false;
			}
			for(std::size_t j = i + 1; j < N; j++){
				auto m = A[j][i] / A[i][i];
				for(std::size_t k = i; k < N; k++){
					A[j][k] -= A[i][k] * m;
				}
				b[j] -= b[i] * m;
			}
		}

		// 後退代入
		for(int i = N - 1; i >= 0; i--){
			ans[i] = b[i];
			for(std::size_t j = i + 1; j < N; j++){
				ans[i] -= ans[j] * A[i][j];
			}
			ans[i] /= A[i][i];
		}
		
		return true;
	}
}

