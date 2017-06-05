#ifndef BWGAME_UTIL_H
#define BWGAME_UTIL_H

#include <cstdint>
#include <iterator>
#include <limits>
#include <array>
#include <type_traits>

#include "containers.h"
#include "strf.h"

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
	xy_t& operator/=(const xy_t& n) {
		x /= n.x;
		y /= n.y;
		return *this;
	}
	template<typename T>
	xy_t operator/(T&& v) const {
		return xy_t(*this) /= v;
	}
	template<typename T>
	xy_t& operator/=(T&& v) {
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
private:
	iter_T begin_it;
	iter_T end_it;
public:
	iterators_range(iter_T begin_it, iter_T end_it) : begin_it(begin_it), end_it(end_it) {}

	using iterator = iter_T;
	
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using pointer = typename std::iterator_traits<iterator>::pointer;
	using reference = typename std::iterator_traits<iterator>::reference;

	iterator begin() {
		return begin_it;
	}
	iterator end() {
		return end_it;
	}
	
	bool empty() const {
		return begin_it == end_it;
	}
	
	reference front() {
		return *begin_it;
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
	using pointer = typename std::remove_reference<reference>::type*;

	template<typename arg_iterator_T, typename arg_transform_F>
	transform_iterator(arg_iterator_T&& ptr, arg_transform_F&& f) : ptr(std::forward<arg_iterator_T>(ptr)), f(std::forward<arg_transform_F>(f)) {}

	reference operator*() const {
		return f(*ptr);
	}
	reference operator*() {
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
		return ptr != rhs.ptr;
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

template<typename range_T, typename transform_F>
auto make_transform_range(range_T&& r, transform_F&& f) {
	auto begin = make_transform_iterator(r.begin(), std::forward<transform_F>(f));
	return make_iterators_range(begin, make_transform_iterator(r.end(), std::forward<transform_F>(f)));
}

template<typename iterator_T, typename predicate_F>
struct filter_iterator {
private:
	typedef filter_iterator this_t;
	iterator_T ptr;
	iterator_T end_ptr;
	predicate_F f;
public:
	using iterator_category = std::forward_iterator_tag;
	using reference = typename std::iterator_traits<iterator_T>::reference;
	using value_type = typename std::iterator_traits<iterator_T>::value_type;
	using difference_type = typename std::iterator_traits<iterator_T>::difference_type;
	using pointer = value_type*;

	template<typename arg_iterator_T, typename arg_predicate_F>
	filter_iterator(arg_iterator_T&& ptr, arg_iterator_T&& end_ptr, arg_predicate_F&& f) : ptr(std::forward<arg_iterator_T>(ptr)), end_ptr(std::forward<arg_iterator_T>(end_ptr)), f(std::forward<arg_predicate_F>(f)) {
		if (ptr != end_ptr && !f(*ptr)) ++*this;
	}

	reference operator*() const {
		return *ptr;
	}
	this_t& operator++() {
		do {
			++ptr;
		} while (ptr != end_ptr && !f(*ptr));
		return *this;
	}
	this_t operator++(int) {
		auto r = *this;
		++*this;
		return r;
	}
	bool operator==(const this_t& rhs) const {
		return ptr == rhs.ptr;
	}
	bool operator!=(const this_t& rhs) const {
		return ptr != rhs.ptr;
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

template<typename iterator_T, typename predicate_F>
auto make_filter_iterator(iterator_T&& c, iterator_T&& end, predicate_F&& f) {
	return filter_iterator<iterator_T, predicate_F>(std::forward<iterator_T>(c), std::forward<iterator_T>(end), std::forward<predicate_F>(f));
}

template<typename range_T, typename predicate_F>
auto make_filter_range(range_T&& r, predicate_F&& f) {
	auto begin = make_filter_iterator(r.begin(), r.end(), std::forward<predicate_F>(f));
	return make_iterators_range(begin, make_filter_iterator(r.end(), r.end(), std::forward<predicate_F>(f)));
}

template<typename range_T>
auto ptr(range_T&& r) {
	return make_transform_range(r, [](auto& ref) {
		return &ref;
	});
}

template<typename range_T>
auto reverse(range_T&& r) {
	return make_iterators_range(std::make_reverse_iterator(r.end()), std::make_reverse_iterator(r.begin()));
}

template<typename range_T>
auto range_size(range_T&& r) {
	auto rv = std::distance(r.begin(), r.end());
	return (typename std::make_unsigned<decltype(rv)>::type)rv;
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

struct nullopt_t {
	struct init {
		constexpr init() {}
	};
	constexpr explicit nullopt_t(init) {};
};
static constexpr nullopt_t nullopt{nullopt_t::init{}};

struct in_place_tag {
	struct init {};
	constexpr explicit in_place_tag(init) {};
};
static inline in_place_tag in_place() {
	std::terminate();
}
using in_place_t = in_place_tag(&)();

template<typename T>
struct optional {
private:
	typename std::aligned_storage<sizeof(T), alignof(T)>::type buf;
	bool has_obj = false;
	T* ptr() {
		return (T*)&buf;
	}
	T& obj() {
		return *(T*)&buf;
	}
	const T& obj() const {
		return *(T*)&buf;
	}
	void destroy() {
		obj().~value_type();
		has_obj = false;
	}
public:
	using value_type = T;
	optional() = default;
	optional(nullopt_t) noexcept {}
	template<typename NT = T, typename std::enable_if<std::is_copy_constructible<NT>::value>::type* = nullptr>
	optional(const optional& n) {
		if (n.has_obj) {
			has_obj = true;
			new (ptr()) value_type(n.obj());
		}
	}
	optional(optional&& n) noexcept(std::is_nothrow_move_constructible<value_type>::value) {
		if (n.has_obj) {
			new (ptr()) value_type(std::move(n.obj()));
			has_obj = true;
		}
	}
	template<typename... args_T>
	optional(in_place_t, args_T&&... args) {
		has_obj = true;
		new (ptr()) value_type(std::forward<args_T>(args)...);
	}
	~optional() {
		if (has_obj) destroy();
	}

	optional& operator=(nullopt_t) noexcept {
		if (has_obj) destroy();
		return *this;
	}
	template<typename NT = T, typename std::enable_if<std::is_copy_assignable<NT>::value>::type* = nullptr>
	optional& operator=(const optional& n) noexcept(std::is_nothrow_move_assignable<value_type>::value && std::is_nothrow_move_constructible<value_type>::value) {
		if (!n.has_obj) *this = nullopt;
		else {
			if (has_obj) obj() = n.obj();
			else {
				has_obj = true;
				new (ptr()) value_type(n.obj());
			}
		}
		return *this;
	}
	optional& operator=(optional&& n) {
		if (has_obj) {
			if (n.has_obj) obj() = std::move(n.obj());
			else destroy();
		} else {
			if (n.has_obj) {
				new (ptr()) value_type(std::move(n.obj()));
				has_obj = true;
			}
		}
		return *this;
	}
	template<typename n_T, typename std::enable_if<std::is_same<std::decay_t<n_T>, value_type>::value>::type* = nullptr>
	optional& operator=(n_T&& n) {
		if (has_obj) {
			obj() = std::forward<n_T>(n);
		} else {
			new (ptr()) value_type(std::forward<n_T>(n));
			has_obj = true;
		}
		return *this;
	}
	const value_type* operator->() const {
		return ptr();
	}
	value_type* operator->() {
		return ptr();
	}
	const value_type& operator*() const& {
		return obj();
	}
	value_type& operator*() & {
		return obj();
	}
	const value_type&& operator*() const&& {
		return std::move(obj());
	}
	value_type&& operator*() && {
		return std::move(obj());
	}
	explicit operator bool() const {
		return has_obj;
	}
	bool has_value() const {
		return has_obj;
	}
	void reset() {
		if (has_obj) destroy();
	}
	template<typename... args_T>
	void emplace(args_T&&... args) {
		if (has_obj) obj().~value_type();
		else has_obj = true;
		new (ptr()) value_type(std::forward<args_T>(args)...);
	}
};

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

	static constexpr fixed_point from_raw(raw_type raw_value) {
		return fixed_point{exact_integer_bits ? (raw_type)((raw_type)(raw_value << (int_bits<raw_type>::value - total_bits)) >> (int_bits<raw_type>::value - total_bits)) : raw_value};
	}

	template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
	static fixed_point integer(T integer_value) {
		return from_raw((raw_type)((raw_unsigned_type)integer_value << fractional_bits));
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

using fp1 = fixed_point<31, 1, true>;
using fp8 = fixed_point<24, 8, true>;
using ufp8 = fixed_point<24, 8, false>;
using fp16 = fixed_point<16, 16, true>;
using ufp16 = fixed_point<16, 16, false>;
using direction_t = fixed_point<0, 8, true, true>;

using xy = xy_t<int>;
using xy_fp8 = xy_t<fp8>;

using rect = rect_t<xy>;

static inline constexpr fp8 operator ""_fp8(unsigned long long int value) {
	return fp8::from_raw((fp8::raw_type)value);
}

static inline constexpr direction_t operator ""_dir(unsigned long long int value) {
	return direction_t::from_raw((direction_t::raw_type)value);
}

template<typename T, typename index_T, size_t N = (size_t)index_T::None>
struct type_indexed_array {
private:
	using arr_T = std::array<T, N>;
	arr_T arr;
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = typename arr_T::iterator;
	using const_iterator = typename arr_T::const_iterator;
	using reverse_iterator = typename arr_T::reverse_iterator;
	using const_reverse_iterator = typename arr_T::const_reverse_iterator;
	
	reference at(index_T pos) {
		return arr.at((size_t)pos);
	}
	const_reference at(index_T pos) const {
		return arr.at((size_t)pos);
	}
	reference operator[](index_T pos) {
		return arr[(size_t)pos];
	}
	const_reference operator[](index_T pos) const {
		return arr[(size_t)pos];
	}
	reference front() {
		return arr.front();
	}
	const_reference front() const {
		return arr.front();
	}
	reference back() {
		return arr.back();
	}
	const_reference back() const {
		return arr.back();
	}
	pointer data() noexcept {
		return arr.data();
	}
	const_pointer data() const noexcept {
		return arr.data();
	}
	iterator begin() noexcept {
		return arr.begin();
	}
	const_iterator begin() const noexcept {
		return arr.begin();
	}
	const_iterator cbegin() const noexcept {
		return arr.cbegin();
	}
	iterator end() noexcept {
		return arr.end();
	} 
	const_iterator end() const noexcept {
		return arr.end();
	}
	const_iterator cend() const noexcept {
		return arr.cend();
	}
	bool empty() const noexcept {
		return arr.empty();
	}
	constexpr size_type size() const noexcept {
		return arr.size();
	}
	constexpr size_type max_size() const noexcept {
		return arr.max_size();
	}
	void fill(const_reference value) {
		arr.fill(value);
	}
	void swap(type_indexed_array& n) {
		arr.swap(n.arr);
	}
};

template<typename T, typename std::enable_if<std::is_unsigned<T>::value>::type* = nullptr>
T isqrt(T n) {
	T r = 0;
	T p = (T)1 << (8 * sizeof(T) - 2);
	while (p > n) p /= 4u;
	while (p) {
		if (n >= r + p) {
			n -= r + p;
			r += 2u * p;
		}
		r /= 2u;
		p /= 4u;
	}
	return r;
}

template<typename...T>
a_string format(const char*fmt, T&&... args) {
	bwgame::a_string str;
	bwgame::strf::format(str, fmt, std::forward<T>(args)...);
	return str;
}

struct exception : std::runtime_error {
	exception(const a_string& str) : std::runtime_error(str.c_str()) {}
};

template<typename...T>
void error(const char* fmt, T&&... args) {
	throw exception(format(fmt, std::forward<T>(args)...));
}

}

#endif
