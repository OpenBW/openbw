
template<typename utype>
struct xy_typed {
	utype x, y;
	typedef xy_typed<utype> xy;
	xy() : x(0), y(0) {}
	xy(utype x, utype y) : x(x), y(y) {}
	//xy(Position pos) : x(pos.x), y(pos.y) {}
	bool operator<(const xy&n) const {
		if (y == n.y) return x < n.x;
		return y < n.y;
	}
	bool operator>(const xy&n) const {
		if (y == n.y) return x > n.x;
		return y > n.y;
	}
	bool operator<=(const xy&n) const {
		if (y == n.y) return x <= n.x;
		return y <= n.y;
	}
	bool operator>=(const xy&n) const {
		if (y == n.y) return x >= n.x;
		return y >= n.y;
	}
	bool operator==(const xy&n) const {
		return x == n.x && y == n.y;
	}
	bool operator!=(const xy&n) const {
		return x != n.x || y != n.y;
	}
	xy operator-(const xy&n) const {
		xy r(*this);
		return r -= n;
	}
	xy&operator-=(const xy&n) {
		x -= n.x;
		y -= n.y;
		return *this;
	}
	xy operator+(const xy&n) const {
		xy r(*this);
		return r += n;
	}
	xy&operator+=(const xy&n) {
		x += n.x;
		y += n.y;
		return *this;
	}
	double length() const {
		return sqrt(x*x + y*y);
	}
	xy operator -() const {
		return xy(-x, -y);
	}
	double angle() const {
		return atan2(y, x);
	}
	xy operator/(const xy&n) const {
		xy r(*this);
		return r /= n;
	}
	xy&operator/=(const xy&n) {
		x /= n.x;
		y /= n.y;
		return *this;
	}
	xy operator/(utype d) const {
		xy r(*this);
		return r /= d;
	}
	xy&operator/=(utype d) {
		x /= d;
		y /= d;
		return *this;
	}
	xy operator*(const xy&n) const {
		xy r(*this);
		return r *= n;
	}
	xy&operator*=(const xy&n) {
		x *= n.x;
		y *= n.y;
		return *this;
	}
	xy operator*(utype d) const {
		xy r(*this);
		return r *= d;
	}
	xy&operator*=(utype d) {
		x *= d;
		y *= d;
		return *this;
	}
};

typedef xy_typed<int> xy;

struct rect {
	xy from;
	xy to;
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
auto make_reverse_range(cont_T&&cont) {
	return make_iterators_range(cont.rbegin(), cont.rend());
}

