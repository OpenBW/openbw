#ifndef BWGAME_CIRCULAR_VECTOR_H
#define BWGAME_CIRCULAR_VECTOR_H

#include <cstddef>
#include <iterator>
#include <array>
#include <memory>
#include <type_traits>
#include <initializer_list>

namespace bwgame {

template<typename allocator_T, bool>
struct circular_vector_allocator_container;

template<typename allocator_T>
struct circular_vector_allocator_container<allocator_T, true> {
	allocator_T get_allocator() {
		return allocator_T();
	}
};

template<typename allocator_T>
struct circular_vector_allocator_container<allocator_T, false> {
protected:
	allocator_T allocator;
public:
	allocator_T get_allocator() {
		return allocator;
	}
};

template<typename T, typename allocator_T = std::allocator<T>>
struct circular_vector: circular_vector_allocator_container<allocator_T, std::is_empty<allocator_T>::value> {
	using allocator_traits = std::allocator_traits<allocator_T>;
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename allocator_traits::pointer;
	using const_pointer = typename allocator_traits::const_pointer;
	struct const_iterator {
		friend circular_vector;
	private:
		typedef const_iterator this_t;
		circular_vector* container;
		typename circular_vector::const_pointer ptr;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename circular_vector::value_type value_type;
		typedef typename circular_vector::difference_type difference_type;
		typedef typename circular_vector::const_pointer pointer;
		typedef typename circular_vector::const_reference reference;
		const_iterator() = default;
		const_iterator(const const_iterator&) = default;
		explicit const_iterator(circular_vector* container, typename circular_vector::const_pointer ptr) : container(container), ptr(ptr) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		reference operator[](difference_type index) const {
			return *(ptr + index);
		}
		this_t& operator++() {
			++ptr;
			if (ptr == container->m_data_end) ptr = container->m_data_begin;
			return *this;
		}
		this_t operator++(int) {
			auto r = *this;
			++*this;
			return r;
		}
		this_t& operator--() {
			if (ptr == container->m_data_begin) ptr = container->m_data_end;
			--ptr;
			return *this;
		}
		this_t operator--(int) {
			auto r = *this;
			--*this;
			return r;
		}
		this_t& operator+=(difference_type diff) {
			if (diff < 0) return *this -= -diff;
			difference_type n_left = container->m_data_end - 1 - ptr;
			if (diff > n_left) diff -= container->m_data_end - container->m_data_begin;
			ptr += diff;
			return *this;
		}
		this_t& operator+(difference_type diff) const {
			auto r = *this;
			return r += diff;
		}
		this_t& operator-=(difference_type diff) {
			if (diff < 0) return *this += -diff;
			difference_type n_left = ptr - container->m_data_begin;
			if (diff > n_left) diff -= container->m_data_end - container->m_data_begin;
			ptr += diff;
			return *this;
		}
		this_t& operator-(difference_type diff) const {
			auto r = *this;
			return r -= diff;
		}
		difference_type operator-(const this_t& other) const {
			difference_type lindex = ptr - container->m_begin;
			if (ptr < container->m_begin) lindex += container->m_data_end - container->m_data_begin;
			difference_type rindex = other.ptr - container->m_begin;
			if (other.ptr < container->m_begin) lindex += container->m_data_end - container->m_data_begin;
			return lindex - rindex;
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
	struct iterator {
		friend circular_vector;
	private:
		typedef iterator this_t;
		circular_vector* container;
		typename circular_vector::pointer ptr;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename circular_vector::value_type value_type;
		typedef typename circular_vector::difference_type difference_type;
		typedef typename circular_vector::pointer pointer;
		typedef typename circular_vector::reference reference;
		iterator() = default;
		iterator(const iterator&) = default;
		explicit iterator(circular_vector* container, typename circular_vector::pointer ptr) : container(container), ptr(ptr) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		reference operator[](difference_type index) const {
			return *(ptr + index);
		}
		this_t& operator++() {
			++ptr;
			if (ptr == container->m_data_end) ptr = container->m_data_begin;
			return *this;
		}
		this_t operator++(int) {
			auto r = *this;
			++*this;
			return r;
		}
		this_t& operator--() {
			if (ptr == container->m_data_begin) ptr = container->m_data_end;
			--ptr;
			return *this;
		}
		this_t operator--(int) {
			auto r = *this;
			--*this;
			return r;
		}
		this_t& operator+=(difference_type diff) {
			if (diff < 0) return *this -= -diff;
			difference_type n_left = container->m_data_end - 1 - ptr;
			if (diff > n_left) diff -= container->m_data_end - container->m_data_begin;
			ptr += diff;
			return *this;
		}
		this_t& operator+(difference_type diff) const {
			auto r = *this;
			return r += diff;
		}
		this_t& operator-=(difference_type diff) {
			if (diff < 0) return *this += -diff;
			difference_type n_left = ptr - container->m_data_begin;
			if (diff > n_left) diff -= container->m_data_end - container->m_data_begin;
			ptr += diff;
			return *this;
		}
		this_t& operator-(difference_type diff) const {
			auto r = *this;
			return r -= diff;
		}
		difference_type operator-(const this_t& other) const {
			difference_type lindex = ptr - container->m_begin;
			if (ptr < container->m_begin) lindex += container->m_data_end - container->m_data_begin;
			difference_type rindex = other.ptr - container->m_begin;
			if (other.ptr < container->m_begin) lindex += container->m_data_end - container->m_data_begin;
			return lindex - rindex;
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
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
private:
	pointer m_data_begin = nullptr;
	pointer m_data_end = nullptr;
	pointer m_begin = nullptr;
	pointer m_end = nullptr;
	pointer next(pointer i) const {
		++i;
		if (i == m_data_end) i = m_data_begin;
		return i;
	}
	pointer prev(pointer i) const {
		if (i == m_data_begin) i = m_data_end;
		--i;
		return i;
	}
	pointer increment(pointer i, size_t n) const {
		size_t n_left = m_data_end - i;
		if (n >= n_left) return m_data_begin + (n - n_left);
		else return i + n;
	}
	
	decltype(auto) get_allocator() {
		return circular_vector_allocator_container<allocator_T, std::is_empty<allocator_T>::value>::get_allocator();
	}

	template<typename... args_T, typename VT = T, typename std::enable_if<std::is_nothrow_move_constructible<VT>::value>::type* = nullptr>
	pointer m_reallocate(size_t new_capacity, args_T&&... args) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		pointer new_end = new_data + new_capacity + 1;
		for (pointer src = m_begin; src != m_end; src = next(src), ++dst) {
			new (dst) value_type(std::move(*src));
		}
		try {
			for (; dst != new_end; ++dst) {
				new (dst) value_type(std::forward<T>(args)...);
			}
		} catch (...) {
			for (pointer i = dst; i != new_data;) {
				--i;
				m_destroy(i);
			}
			get_allocator().deallocate(new_data, new_capacity + 1);
			throw;
		}
		return new_data;
	}
	template<typename... args_T, typename VT = T, typename std::enable_if<!std::is_nothrow_move_constructible<VT>::value>::type* = nullptr>
	pointer m_reallocate(size_t new_capacity, args_T&&... args) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		pointer new_end = new_data + new_capacity + 1;
		try {
			for (pointer src = m_begin; src != m_end; src = next(src), ++dst) {
				new (dst) value_type(*src);
			}
			for (; dst != new_end; ++dst) {
				new (dst) value_type(std::forward<T>(args)...);
			}
		} catch (...) {
			for (pointer i = dst; i != new_data;) {
				--i;
				m_destroy(i);
			}
			get_allocator().deallocate(new_data, new_capacity + 1);
			throw;
		}
		return new_data;
	}
	template<typename VT = T, typename std::enable_if<std::is_nothrow_move_constructible<VT>::value>::type* = nullptr>
	pointer m_reallocate_no_construct(size_t new_capacity) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		for (pointer src = m_begin; src != m_end; src = next(src), ++dst) {
			new (dst) value_type(std::move(*src));
		}
		return new_data;
	}
	template<typename VT = T, typename std::enable_if<!std::is_nothrow_move_constructible<VT>::value>::type* = nullptr>
	pointer m_reallocate_no_construct(size_t new_capacity) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		try {
			for (pointer src = m_begin; src != m_end; src = next(src), ++dst) {
				new (dst) value_type(*src);
			}
		} catch (...) {
			for (pointer i = dst; i != new_data;) {
				--i;
				m_destroy(i);
			}
			get_allocator().deallocate(new_data, new_capacity + 1);
			throw;
		}
		return new_data;
	}
	pointer m_reallocate_copy(size_t new_capacity, const circular_vector& other) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		try {
			for (pointer src = other.m_begin; src != other.m_end; src = other.next(src), ++dst) {
				new (dst) value_type(*src);
			}
		} catch (...) {
			for (pointer i = dst; i != new_data;) {
				--i;
				m_destroy(i);
			}
			get_allocator().deallocate(new_data, new_capacity + 1);
			throw;
		}
		return new_data;
	}
	template<typename iterator_T>
	pointer m_reallocate_copy(size_t new_capacity, iterator_T begin, iterator_T end) {
		pointer new_data = get_allocator().allocate(new_capacity + 1);
		pointer dst = new_data;
		try {
			for (auto src = begin; src != end; ++src, ++dst) {
				new (dst) value_type(*src);
			}
		} catch (...) {
			for (pointer i = dst; i != new_data;) {
				--i;
				m_destroy(i);
			}
			get_allocator().deallocate(new_data, new_capacity + 1);
			throw;
		}
		return new_data;
	}
	
	void m_grow() {
		size_t cap = capacity();
		size_t remaining_cap = max_size() - cap;
		size_t increase = sizeof(T) * cap < 0x2000 ? cap : cap / 2;
		if (increase > remaining_cap) increase = remaining_cap;
		if (sizeof(T) * increase < 0x20) {
			increase = (0x20 + sizeof(T) - 1) / sizeof(T);
			if (increase > remaining_cap) throw std::length_error("circular_vector exceeded maximum size");
		}
		size_t new_cap = cap + increase;
		pointer new_data = m_reallocate_no_construct(new_cap);
		pointer new_end = new_data + size();
		m_clear();
		if (m_data_begin) get_allocator().deallocate(m_data_begin, m_data_end - m_data_begin);
		m_data_begin = new_data;
		m_data_end = m_data_begin + new_cap + 1;
		m_begin = new_data;
		m_end = new_end;
	}

	template<typename... args_T>
	void m_expand(size_t new_capacity, args_T&&... args) {
		pointer new_data = m_reallocate(new_capacity);
		pointer new_end = new_data + size();
		if (m_data_begin) get_allocator().deallocate(m_data_begin, m_data_end - m_data_begin);
		m_data_begin = new_data;
		m_data_end = m_data_begin + new_capacity + 1;
		m_begin = new_data;
		m_end = new_end;
	}

	template<typename... args_T>
	void m_resize(size_type count, args_T&&... args) {
		if (count > capacity()) {
			m_expand(count, std::forward<args_T>(args)...);
			return;
		} else if (count > size()) {
			pointer new_end = increment(m_begin, count);
			for (pointer i = m_end; i != new_end; i = next(i)) {
				try {
					new (i) value_type(std::forward<T>(args)...);
				} catch (...) {
					for (pointer i2 = i; i2 != m_end;) {
						i = prev(i2);
						m_destroy(i2);
					}
					throw;
				}
			}
			m_end = new_end;
		} else {
			pointer new_end = increment(m_begin, count);
			for (pointer i = m_end; i != new_end;) {
				i = prev(i);
				m_destroy(i);
			}
			m_end = new_end;
		}
	}

	void m_clear() {
		pointer e = m_begin;
		for (pointer i = m_end; i != e;) {
			i = prev(i);
			m_destroy(i);
		}
		m_end = e;
	}
	
	template<typename iterator_T>
	void m_assign(iterator_T begin, iterator_T end) {
		size_t new_size = std::distance(begin, end);
		if (size() >= new_size) {
			pointer new_end = increment(m_begin, new_size);
			pointer i = m_begin;
			for (auto src = begin; i != new_end; i = next(i), ++src) {
				new (i) value_type(*src);
			}
			for (; i != m_end; i = next(i)) {
				m_destroy(i);
			}
			m_end = new_end;
		} else {
			pointer new_data = m_reallocate_copy(new_size, begin, end);
			m_clear();
			pointer new_end = new_data + new_size;
			if (m_data_begin) get_allocator().deallocate(m_data_begin, m_data_end - m_data_begin);
			m_data_begin = new_data;
			m_data_end = m_data_begin + new_size + 1;
			m_begin = new_data;
			m_end = new_end;
		}
	}

	void m_assign(const circular_vector& other) {
		size_t new_size = other.size();
		if (size() >= new_size) {
			pointer new_end = increment(m_begin, new_size);
			pointer i = m_begin;
			for (pointer src = other.m_begin; i != new_end; i = next(i), src = other.next(src)) {
				new (i) value_type(*src);
			}
			for (; i != m_end; i = next(i)) {
				m_destroy(i);
			}
			m_end = new_end;
		} else {
			pointer new_data = m_reallocate_copy(new_size, other);
			m_clear();
			pointer new_end = new_data + new_size;
			if (m_data_begin) get_allocator().deallocate(m_data_begin, m_data_end - m_data_begin);
			m_data_begin = new_data;
			m_data_end = m_data_begin + new_size + 1;
			m_begin = new_data;
			m_end = new_end;
		}
	}
	void m_assign(circular_vector&& other) {
		std::swap(m_data_begin, other.m_data_begin);
		std::swap(m_data_end, other.m_data_end);
		std::swap(m_begin, other.m_begin);
		std::swap(m_end, other.m_end);
	}
	void m_destroy(pointer p) {
		get_allocator().destroy(p);
	}
public:
	circular_vector() {}
	explicit circular_vector(size_type count) {
		resize(count);
	}
	explicit circular_vector(size_type count, const value_type& value) {
		resize(count, value);
	}
	circular_vector(const circular_vector& other) {
		m_assign(other);
	}
	circular_vector(circular_vector&& other) {
		m_assign(std::move(other));
	}
	~circular_vector() {
		m_clear();
		if (m_data_begin) get_allocator().deallocate(m_data_begin, m_data_end - m_data_begin);
	}
	void resize(size_type count) {
		m_resize(count);
	}
	void resize(size_type count, const value_type& value) {
		m_resize(count, value);
	}
	circular_vector& operator=(const circular_vector& other) {
		m_assign(other);
		return *this;
	}
	circular_vector& operator=(circular_vector&& other) {
		m_assign(std::move(other));
		return *this;
	}
	circular_vector& operator=(std::initializer_list<T> ilist) {
		m_assign(ilist.begin(), ilist.end());
		return *this;
	}
	reference at(size_type pos) {
		if (pos >= size()) throw std::out_of_range("circular_vector subscript out of range");
		return *increment(m_begin, pos);
	}
	const_reference at(size_type pos) const {
		if (pos >= size()) throw std::out_of_range("circular_vector subscript out of range");
		return *increment(m_begin, pos);
	}
	reference operator[](size_type pos) {
		return *increment(m_begin, pos);
	}
	constexpr const_reference operator[](size_type pos) const {
		return *increment(m_begin, pos);
	}
	reference front() {
		return *m_begin;
	}
	const_reference front() const {
		return *m_begin;
	}
	reference back() {
		return *prev(m_end);
	}
	const_reference back() const {
		return *prev(m_end);
	}
	iterator begin() {
		return iterator(this, m_begin);
	}
	const_iterator begin() const {
		return const_iterator(this, m_begin);
	}
	const_iterator cbegin() const {
		return iterator(this, m_begin);
	}
	iterator end() {
		return iterator(this, m_end);
	}
	const_iterator end() const {
		return const_iterator(this, m_end);
	}
	const_iterator cend() const {
		return const_iterator(this, m_end);
	}
	reverse_iterator rbegin() {
		return reverse_iterator(end());
	}
	const_reverse_iterator rbegin() const {
		return reverse_iterator(end());
	}
	const_reverse_iterator crbegin() const {
		return reverse_iterator(end());
	}
	reverse_iterator rend() {
		return reverse_iterator(begin());
	}
	const_reverse_iterator rend() const {
		return reverse_iterator(begin());
	}
	const_reverse_iterator crend() const {
		return reverse_iterator(begin());
	}
	bool empty() const {
		return m_begin == m_end;
	}
	size_type size() const {
		if (m_begin > m_end) return (m_data_end - m_data_begin) - (m_begin - m_end);
		else return m_end - m_begin;
	}
	constexpr size_type max_size() const {
		return (size_type)-1 / sizeof(T) - 1;
	}
	constexpr size_type capacity() const {
		if (!m_data_begin) return 0;
		return m_data_end - m_data_begin - 1;
	}
	void clear() {
		m_clear();
	}
	void push_back(const T& value) {
		if (size() == capacity()) m_grow();
		new (m_end) value_type(value);
		m_end = next(m_end);
	}
	void push_back(T&& value) {
		if (size() == capacity()) m_grow();
		new (m_end) value_type(std::move(value));
		m_end = next(m_end);
	}
	template<typename... args_T>
	void emplace_back(args_T&&... args) {
		if (size() == capacity()) m_grow();
		new (m_end) value_type(std::forward<args_T>(args)...);
		m_end = next(m_end);
	}
	void pop_back() {
		auto new_end = prev(m_end);
		m_destroy(new_end);
		m_end = new_end;
	}
	void push_front(const T& value) {
		if (size() == capacity()) m_grow();
		pointer new_begin = prev(m_begin);
		new (new_begin) value_type(value);
		m_begin = new_begin;
	}
	void push_front(T&& value) {
		if (size() == capacity()) m_grow();
		pointer new_begin = prev(m_begin);
		new (new_begin) value_type(std::move(value));
		m_begin = new_begin;
	}
	template<typename... args_T>
	void emplace_front(args_T&&... args) {
		if (size() == capacity()) m_grow();
		pointer new_begin = prev(m_begin);
		new (new_begin) value_type(std::forward<args_T>(args)...);
		m_begin = new_begin;
	}
	void pop_front() {
		auto new_begin = next(m_begin);
		m_destroy(m_begin);
		m_begin = new_begin;
	}
	iterator erase(const iterator pos) {
		for (pointer i = pos.ptr;;) {
			pointer ni = next(i);
			if (ni == m_end) {
				m_destroy(i);
				m_end = i;
				return pos;
			}
			*i = std::move(*ni);
			i = ni;
		}
	}
};

}

#endif
