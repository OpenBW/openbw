
template<typename utype>
struct xy_t {
	utype x {};
	utype y {};
	xy_t() = default;
	xy_t(utype x, utype y) : x(x), y(y) {}
	explicit xy_t(std::array<utype, 2> arr) : x(arr[0]), y(arr[1]) {}
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
		xy r(*this);
		return r += n;
	}
	xy_t& operator+=(const xy_t& n) {
		x += n.x;
		y += n.y;
		return *this;
	}
	xy_t operator -() const {
		return xy(-x, -y);
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
	xy_t operator/(utype d) const {
		return xy_t(*this) /= d;
	}
	xy_t&operator/=(utype d) {
		x /= d;
		y /= d;
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
	xy_t operator*(utype d) const {
		return xy_t(*this) *= d;
	}
	xy_t& operator*=(utype d) {
		x *= d;
		y *= d;
		return *this;
	}
};

using xy = xy_t<int>;

template<typename T>
struct rect_t {
	T from;
	T to;
	bool operator==(const rect_t& n) const {
		return from == n.from && to == n.to;
	}
};

using rect = rect_t<xy>;

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
auto make_reverse_range(cont_T&&cont) {
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

	template<typename iterator_T, typename transform_F>
	transform_iterator(iterator_T&& ptr, transform_F&& f) : ptr(std::forward<iterator_T>(ptr)), f(std::forward<transform_F>(f)) {}

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

template<typename cont_T, typename score_F>
auto get_best_score(cont_T&& cont, score_F&& score) {
	std::remove_const<std::remove_reference<std::result_of<score_F(std::remove_reference<cont_T>::type::value_type)>::type>::type>::type best_score {};
	std::remove_reference<cont_T>::type::value_type best {};
	if (cont.empty()) return best;
	auto i = cont.begin();
	auto e = cont.end();
	auto& front = *i;
	best = front;
	best_score = score(front);
	++i;
	for (; i != e; ++i) {
		auto& v = *i;
		auto s = score(v);
		if (s < best_score) {
			best = v;
			best_score = s;
		}
	}
	return best;
}

