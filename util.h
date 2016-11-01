#ifndef BWGAME_UTIL_H
#define BWGAME_UTIL_H

#include <cstdint>
#include <iterator>
#include <limits>

namespace bwgame {

template<typename utype>
struct xy_t {
	utype x {};
	utype y {};
	xy_t() = default;
	xy_t(utype x, utype y) : x(x), y(y) {}
	bool operator<(const xy_t& n) const {
		if (y == n.y) return x < n.x;
		return y < n.y;
	}
	bool operator>(const xy_t& n) const {
		if (y == n.y) return x > n.x;
		return y > n.y;
	}
	bool operator<=(const xy_t& n) const {
		if (y == n.y) return x <= n.x;
		return y <= n.y;
	}
	bool operator>=(const xy_t& n) const {
		if (y == n.y) return x >= n.x;
		return y >= n.y;
	}
	bool operator==(const xy_t& n) const {
		return x == n.x && y == n.y;
	}
	bool operator!=(const xy_t& n) const {
		return x != n.x || y != n.y;
	}
	xy_t operator-(const xy_t& n) const {
		xy_t r(*this);
		return r -= n;
	}
	xy_t& operator-=(const xy_t& n) {
		x -= n.x;
		y -= n.y;
		return *this;
	}
	xy_t operator+(const xy_t& n) const {
		xy_t r(*this);
		return r += n;
	}
	xy_t& operator+=(const xy_t& n) {
		x += n.x;
		y += n.y;
		return *this;
	}
	xy_t operator -() const {
		return xy_t(-x, -y);
	}
	xy_t operator/(const xy_t& n) const {
		xy_t r(*this);
		return r /= n;
	}
	xy_t&operator/=(const xy_t& n) {
		x /= n.x;
		y /= n.y;
		return *this;
	}
	template<typename T>
	xy_t operator/(T&& v) const {
		return xy_t(*this) /= v;
	}
	template<typename T>
	xy_t&operator/=(T&& v) {
		x /= v;
		y /= v;
		return *this;
	}
	xy_t operator*(const xy_t& n) const {
		xy_t r(*this);
		return r *= n;
	}
	xy_t& operator*=(const xy_t& n) {
		x *= n.x;
		y *= n.y;
		return *this;
	}
	template<typename T>
	xy_t operator*(T&& v) const {
		return xy_t(*this) *= v;
	}
	template<typename T>
	xy_t& operator*=(T&& v) {
		x *= v;
		y *= v;
		return *this;
	}
};

template<typename T>
struct rect_t {
	T from;
	T to;
	rect_t() = default;
	rect_t(T from, T to) : from(from), to(to) {}
	bool operator==(const rect_t& n) const {
		return from == n.from && to == n.to;
	}

	rect_t operator+(const rect_t& n) const {
		return { from + n.from, to + n.to };
	}
};

template<typename iter_T>
struct iterators_range {
	iter_T begin_it;
	iter_T end_it;
	iterators_range(iter_T begin_it, iter_T end_it) : begin_it(begin_it), end_it(end_it) {}

	typedef iter_T iterator;

	typedef typename iter_T::value_type value_type;

	iterator begin() {
		return begin_it;
	}
	iterator end() {
		return end_it;
	}

};


template<typename iter_T>
iterators_range<iter_T> make_iterators_range(iter_T begin, iter_T end) {
	return iterators_range<iter_T>(begin, end);
}

template<typename cont_T>
auto make_reverse_range(cont_T&& cont) {
	return make_iterators_range(cont.rbegin(), cont.rend());
}

template<typename iterator_T, typename transform_F>
struct transform_iterator {
private:
	typedef transform_iterator this_t;
	iterator_T ptr;
	transform_F f;
public:
	using iterator_category = typename std::iterator_traits<iterator_T>::iterator_category;
	using reference = typename std::result_of<transform_F(typename std::iterator_traits<iterator_T>::reference)>::type;
	using value_type = typename std::remove_cv<typename std::remove_const<reference>::type>::type;
	using difference_type = typename std::iterator_traits<iterator_T>::difference_type;
	using pointer = reference*;

	template<typename arg_iterator_T, typename arg_transform_F>
	transform_iterator(arg_iterator_T&& ptr, arg_transform_F&& f) : ptr(std::forward<arg_iterator_T>(ptr)), f(std::forward<arg_transform_F>(f)) {}

	reference operator*() const {
		return f(*ptr);
	}
	this_t& operator++() {
		++ptr;
		return *this;
	}
	this_t operator++(int) {
		auto r = *this;
		++ptr;
		return r;
	}
	this_t& operator--() {
		--ptr;
		return *this;
	}
	this_t operator--(int) {
		auto r = *this;
		--ptr;
		return r;
	}
	this_t& operator+=(difference_type diff) {
		ptr += diff;
		return *this;
	}
	this_t operator+(difference_type diff) const {
		auto r = *this;
		return r += diff;
	}
	this_t& operator-=(difference_type diff) {
		ptr -= diff;
		return *this;
	}
	this_t operator-(difference_type diff) const {
		auto r = *this;
		return r -= diff;
	}
	difference_type operator-(const this_t& other) const {
		return ptr - other.ptr;
	}
	bool operator==(const this_t& rhs) const {
		return ptr == rhs.ptr;
	}
	bool operator!=(const this_t& rhs) const {
		return !(*this == rhs);
	}
	bool operator<(const this_t& rhs) const {
		return ptr < rhs.ptr;
	}
	bool operator<=(const this_t& rhs) const {
		return ptr <= rhs.ptr;
	}
	bool operator>(const this_t& rhs) const {
		return ptr > rhs.ptr;
	}
	bool operator>=(const this_t& rhs) const {
		return ptr >= rhs.ptr;
	}
};

template<typename iterator_T, typename transform_F>
auto make_transform_iterator(iterator_T&& c, transform_F&& f) {
	return transform_iterator<iterator_T, transform_F>(std::forward<iterator_T>(c), std::forward<transform_F>(f));
}

template<typename range_T>
auto ptr(range_T&& r) {
	auto f = [](auto& ref) {
		return &ref;
	};
	return make_iterators_range(make_transform_iterator(r.begin(), f), make_transform_iterator(r.end(), f));
}

template<typename range_T>
auto reverse(range_T&& r) {
	return make_iterators_range(std::make_reverse_iterator(r.end()), std::make_reverse_iterator(r.begin()));
}


struct identity {
	template<typename T>
	decltype(auto) operator()(T&& v) const {
		return std::forward<T>(v);
	}
};

template<typename iterator_T, typename score_F>
auto get_best_score(iterator_T begin, iterator_T end, score_F&& score) {
	if (begin == end) return end;
	auto i = begin;
	auto best = i;
	auto best_score = score(*i);
	++i;
	for (; i != end; ++i) {
		auto s = score(*i);
		if (s < best_score) {
			best = i;
			best_score = s;
		}
	}
	return best;
}

template<typename cont_T, typename score_F>
auto get_best_score(cont_T&& cont, score_F&& score) {
	return get_best_score(cont.begin(), cont.end(), std::forward<score_F>(score));
}


template<size_t bits>
using int_fastn_t = typename std::conditional<bits <= 8, std::int_fast8_t,
	typename std::conditional<bits <= 16, int_fast16_t,
	typename std::conditional<bits <= 32, int_fast32_t,
	typename std::enable_if<bits <= 64, int_fast64_t>::type>::type>::type>::type;
template<size_t bits>
using uint_fastn_t = typename std::conditional<bits <= 8, std::uint_fast8_t,
	typename std::conditional<bits <= 16, uint_fast16_t,
	typename std::conditional<bits <= 32, uint_fast32_t,
	typename std::enable_if<bits <= 64, uint_fast64_t>::type>::type>::type>::type;

template<typename T, std::enable_if<std::is_integral<T>::value && std::numeric_limits<T>::radix == 2>* = nullptr>
using int_bits = std::integral_constant<size_t, std::numeric_limits<T>::digits + std::is_signed<T>::value>;

template<typename T>
using is_native_fast_int = std::integral_constant<bool, std::is_integral<T>::value && std::is_literal_type<T>::value && sizeof(T) <= sizeof(void*)>;

template<size_t t_integer_bits, size_t t_fractional_bits, bool t_is_signed, bool t_exact_integer_bits = false>
struct fixed_point {
	static const bool is_signed = t_is_signed;
	static const bool exact_integer_bits = t_exact_integer_bits;
	static const size_t integer_bits = t_integer_bits;
	static const size_t fractional_bits = t_fractional_bits;
	static const size_t total_bits = integer_bits + fractional_bits;
	using raw_unsigned_type = uint_fastn_t<total_bits>;
	using raw_signed_type = int_fastn_t<total_bits>;
	using raw_type = typename std::conditional<is_signed, raw_signed_type, raw_unsigned_type>::type;
	raw_type raw_value;

	using double_size_fixed_point = fixed_point<total_bits * 2 - fractional_bits, fractional_bits, is_signed>;

	void wrap() {
		if (!exact_integer_bits) return;
		raw_value <<= int_bits<raw_type>::value - total_bits;
		raw_value >>= int_bits<raw_type>::value - total_bits;
	}

	static fixed_point from_raw(raw_type raw_value) {
		fixed_point r;
		r.raw_value = raw_value;
		r.wrap();
		return r;
	}

	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	static fixed_point integer(T integer_value) {
		return from_raw((raw_type)integer_value << fractional_bits);
	}

	static fixed_point zero() {
		return integer(0);
	}
	static fixed_point one() {
		return integer(1);
	}

	raw_type integer_part() const {
		return raw_value >> fractional_bits;
	}
	raw_type fractional_part() const {
		return raw_value & (((raw_type)1 << fractional_bits) - 1);
	}

	template<size_t n_integer_bits, size_t n_fractional_bits, bool n_exact_integer_bits, size_t result_integer_bits = integer_bits, size_t result_fractional_bits = fractional_bits, typename std::enable_if<((result_integer_bits < n_integer_bits || result_fractional_bits < n_fractional_bits) && result_fractional_bits <= n_fractional_bits)>::type* = nullptr>
	static auto truncate(const fixed_point<n_integer_bits, n_fractional_bits, is_signed, n_exact_integer_bits>& n) {
		using result_type = fixed_point<result_integer_bits, result_fractional_bits, is_signed, exact_integer_bits>;
		typename result_type::raw_type raw_value = (typename result_type::raw_type)(n.raw_value >> (n_fractional_bits - result_fractional_bits));
		return result_type::from_raw(raw_value);
	}

	template<size_t n_integer_bits, size_t n_fractional_bits, bool n_exact_integer_bits, size_t result_integer_bits = integer_bits, size_t result_fractional_bits = fractional_bits, typename std::enable_if<((result_integer_bits > n_integer_bits || result_fractional_bits > n_fractional_bits) && result_fractional_bits >= n_fractional_bits)>::type* = nullptr>
	static auto extend(const fixed_point<n_integer_bits, n_fractional_bits, is_signed, n_exact_integer_bits>& n) {
		using result_type = fixed_point<result_integer_bits, result_fractional_bits, is_signed, exact_integer_bits>;
		typename result_type::raw_type raw_value = n.raw_value;
		raw_value <<= result_fractional_bits - n_fractional_bits;
		return result_type::from_raw(raw_value);
	}

	fixed_point floor() const {
		return integer(integer_part());
	}
	fixed_point ceil() const {
		return (*this + integer(1) - from_raw(1)).floor();
	}
	fixed_point abs() const {
		if (*this >= zero()) return *this;
		else return from_raw(-raw_value);
	}

	auto as_signed() const {
		return fixed_point<integer_bits, fractional_bits, true, exact_integer_bits>::from_raw(raw_value);
	}
	auto as_unsigned() const {
		return fixed_point<integer_bits, fractional_bits, false, exact_integer_bits>::from_raw(raw_value);
	}

	bool operator==(const fixed_point& n) const {
		return raw_value == n.raw_value;
	}
	bool operator!=(const fixed_point& n) const {
		return raw_value != n.raw_value;
	}
	bool operator<(const fixed_point& n) const {
		return raw_value < n.raw_value;
	}
	bool operator<=(const fixed_point& n) const {
		return raw_value <= n.raw_value;
	}
	bool operator>(const fixed_point& n) const {
		return raw_value > n.raw_value;
	}
	bool operator>=(const fixed_point& n) const {
		return raw_value >= n.raw_value;
	}

	fixed_point operator-() const {
		static_assert(is_signed, "fixed_point: cannot negate an unsigned number");
		return from_raw(-raw_value);
	}

	fixed_point& operator+=(const fixed_point& n) {
		raw_value += n.raw_value;
		wrap();
		return *this;
	}
	fixed_point operator+(const fixed_point& n) const {
		return from_raw(raw_value + n.raw_value);
	}
	fixed_point& operator-=(const fixed_point& n) {
		raw_value -= n.raw_value;
		wrap();
		return *this;
	}
	fixed_point operator-(const fixed_point& n) const {
		return from_raw(raw_value - n.raw_value);
	}

// 	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
// 	fixed_point& operator+=(T integer_value) {
// 		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in addition");
// 		*this += integer(integer_value);
// 		wrap();
// 		return *this;
// 	}
// 	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
// 	fixed_point operator+(T integer_value) const {
// 		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in addition");
// 		return *this + integer(integer_value);
// 	}
// 	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
// 	fixed_point& operator-=(T integer_value) {
// 		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in subtraction");
// 		*this -= integer(integer_value);
// 		wrap();
// 		return *this;
// 	}
// 	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
// 	fixed_point operator-(T integer_value) const {
// 		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in subtraction");
// 		return *this - integer(integer_value);
// 	}

	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	fixed_point& operator/=(T integer_value) {
		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in division");
		raw_value /= integer_value;
		wrap();
		return *this;
	}
	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	fixed_point operator/(T integer_value) const {
		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in division");
		return from_raw(raw_value / integer_value);
	}
	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	fixed_point& operator*=(T integer_value) {
		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in multiplication");
		raw_value *= integer_value;
		wrap();
		return *this;
	}
	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	auto operator*(T integer_value) const {
		static_assert(std::is_signed<T>::value == is_signed, "fixed_point: cannot mix signed/unsigned in multiplication");
		return from_raw(raw_value * integer_value);
	}

	template<size_t n_integer_bits>
	fixed_point& operator*=(const fixed_point<n_integer_bits, fractional_bits, is_signed, exact_integer_bits>& n) {
		return *this = *this * n;
	}

	template<size_t n_integer_bits>
	fixed_point& operator/=(const fixed_point<n_integer_bits, fractional_bits, is_signed, exact_integer_bits>& n) {
		return *this = *this / n;
	}

	// rounds towards negative infinity
	template<size_t n_integer_bits, size_t n_fractional_bits>
	auto operator*(const fixed_point<n_integer_bits, n_fractional_bits, is_signed, exact_integer_bits>& n) {
		using result_type = fixed_point<(integer_bits > n_integer_bits ? integer_bits : n_integer_bits), (fractional_bits > n_fractional_bits ? fractional_bits : n_fractional_bits), is_signed, exact_integer_bits>;
		using tmp_t = typename fixed_point<result_type::integer_bits, fractional_bits + n_fractional_bits, is_signed>::raw_type;
		tmp_t tmp = (tmp_t)raw_value * (tmp_t)n.raw_value >> n_fractional_bits;
		return result_type::from_raw((typename result_type::raw_type)tmp);
	}

	// rounds towards 0
	template<size_t n_integer_bits, size_t n_fractional_bits>
	auto operator/(const fixed_point<n_integer_bits, n_fractional_bits, is_signed, exact_integer_bits>& n) const {
		using result_type = fixed_point<(integer_bits > n_integer_bits ? integer_bits : n_integer_bits), (fractional_bits > n_fractional_bits ? fractional_bits : n_fractional_bits), is_signed, exact_integer_bits>;
		using tmp_t = typename fixed_point<result_type::integer_bits, fractional_bits + n_fractional_bits, is_signed>::raw_type;
		tmp_t tmp = ((tmp_t)raw_value << n_fractional_bits) / (tmp_t)n.raw_value;
		return result_type::from_raw((typename result_type::raw_type)tmp);
	}

	// returns a * b / c
	// rounds towards 0
	static fixed_point multiply_divide(fixed_point a, fixed_point b, fixed_point c) {
		constexpr raw_type max_value_no_overflow = std::numeric_limits<raw_type>::max() >> (int_bits<raw_type>::value / 2);
		using tmp_t = typename fixed_point<integer_bits, fractional_bits + fractional_bits, is_signed>::raw_type;
		if (!is_native_fast_int<tmp_t>::value && a.raw_value <= max_value_no_overflow && b.raw_value <= max_value_no_overflow) {
			return from_raw(a.raw_value * b.raw_value / c.raw_value);
		} else {
			return from_raw((tmp_t)a.raw_value * b.raw_value / c.raw_value);
		}
	}

	// returns a / b * c
	// rounds towards 0
	static fixed_point divide_multiply(fixed_point a, fixed_point b, fixed_point c) {
		return from_raw(a.raw_value / b.raw_value * c.raw_value);
	}

};

using fp8 = fixed_point<24, 8, true>;
using ufp8 = fixed_point<24, 8, false>;
using direction_t = fixed_point<0, 8, true, true>;

using xy = xy_t<int>;
using xy_fp8 = xy_t<fp8>;

using rect = rect_t<xy>;

}

#endif
