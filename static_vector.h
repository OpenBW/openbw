#ifndef BWGAME_STATIC_VECTOR_H
#define BWGAME_STATIC_VECTOR_H

#include <cstddef>
#include <iterator>
#include <array>

namespace bwgame {

template<typename T, size_t max_elements>
struct static_vector {
	struct t_iterator;
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	struct const_iterator {
		friend static_vector;
	private:
		typedef const_iterator this_t;
		typename static_vector::const_pointer ptr;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename static_vector::value_type value_type;
		typedef typename static_vector::difference_type difference_type;
		typedef typename static_vector::const_pointer pointer;
		typedef typename static_vector::const_reference reference;
		const_iterator() = default;
		const_iterator(const const_iterator&) = default;
		explicit const_iterator(typename static_vector::const_pointer ptr) : ptr(ptr) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		reference operator[](difference_type index) const {
			return *ptr[index];
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
		this_t& operator+(difference_type diff) const {
			auto r = *this;
			return r += diff;
		}
		this_t& operator-=(difference_type diff) {
			ptr -= diff;
			return *this;
		}
		this_t& operator-(difference_type diff) const {
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
	struct iterator {
		friend static_vector;
	private:
		typedef iterator this_t;
		typename static_vector::pointer ptr;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef typename static_vector::value_type value_type;
		typedef typename static_vector::difference_type difference_type;
		typedef typename static_vector::pointer pointer;
		typedef typename static_vector::reference reference;
		iterator() = default;
		iterator(const iterator&) = default;
		explicit iterator(typename static_vector::pointer ptr) : ptr(ptr) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		reference operator[](difference_type index) const {
			return *ptr[index];
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
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
private:
	std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, max_elements> m_data;
	pointer m_end = ptr_begin();
	template<typename... args_T>
	void m_resize(size_type count, args_T&&... args) {
		if (count > capacity()) throw std::length_error("static_vector resized beyond capacity");
		pointer e = ptr_begin() + count;
		if (e > ptr_end()) {
			for (pointer i = ptr_end(); i != e; ++i) {
				try {
					new (i) value_type(std::forward<T>(args)...);
				} catch (...) {
					for (pointer i2 = i; i2 > ptr_end(); --i2) {
						m_destroy(i2 - 1);
					}
					throw;
				}
			}
		} else {
			for (pointer i = ptr_end(); i != e; --i) {
				m_destroy(i - 1);
			}
		}
		m_end = e;
	}
	void m_clear() {
		pointer e = ptr_begin();
		for (pointer i = ptr_end(); i != e; --i) {
			m_destroy(i - 1);
		}
		m_end = e;
	}
	template<typename VT = value_type, typename std::enable_if<std::is_copy_constructible<VT>::value && std::is_nothrow_copy_assignable<VT>::value, int>::type = 0>
	void m_assign(const static_vector& other) {
		auto src_i = other.ptr_begin();
		pointer dst_i = ptr_begin();
		for (; dst_i != ptr_end() && src_i != other.ptr_end(); ++src_i, ++dst_i) {
			*dst_i = *src_i;
		}
		auto e = ptr_begin() + other.size();
		for (; src_i != other.ptr_end(); ++src_i, ++dst_i) {
			try {
				new (dst_i) value_type(*src_i);
			} catch (...) {
				for (pointer i2 = dst_i; i2 > ptr_end(); --i2) {
					m_destroy(i2 - 1);
				}
				throw;
			}
		}
		m_end = e;
	}
	template<typename VT = value_type, typename std::enable_if<std::is_copy_constructible<VT>::value && !std::is_nothrow_copy_assignable<VT>::value, int>::type = 0>
	void m_assign(const static_vector& other) {
		m_clear();
		auto src_i = other.ptr_begin();
		pointer dst_i = ptr_begin();
		auto e = ptr_begin() + other.size();
		for (; src_i != other.ptr_end(); ++src_i, ++dst_i) {
			try {
				new (dst_i) value_type(*src_i);
			} catch (...) {
				for (pointer i2 = dst_i; i2 > ptr_end(); --i2) {
					m_destroy(i2 - 1);
				}
				throw;
			}
		}
		m_end = e;
	}
	template<typename VT = value_type, typename std::enable_if<std::is_nothrow_move_constructible<VT>::value && std::is_nothrow_move_assignable<VT>::value, int>::type = 0>
	void m_assign(static_vector&& other) {
		auto src_i = other.ptr_begin();
		pointer dst_i = ptr_begin();
		for (; dst_i != ptr_end() && src_i != other.ptr_end(); ++src_i, ++dst_i) {
			*dst_i = std::move(*src_i);
		}
		auto e = ptr_begin() + other.size();
		for (; src_i != other.ptr_end(); ++src_i, ++dst_i) {
			new (dst_i) value_type(std::move(*src_i));
		}
		m_end = e;
	}
	template<typename VT = value_type, typename std::enable_if<std::is_nothrow_move_constructible<VT>::value && !std::is_nothrow_move_assignable<VT>::value, int>::type = 0>
	void m_assign(static_vector&& other) {
		m_clear();
		auto src_i = other.ptr_begin();
		pointer dst_i = ptr_begin();
		auto e = ptr_begin() + other.size();
		for (; src_i != other.ptr_end(); ++src_i, ++dst_i) {
			new (dst_i) value_type(std::move(*src_i));
		}
		m_end = e;
	}
	template<typename VT = value_type, typename std::enable_if<std::is_scalar<VT>::value, int>::type = 0>
	void m_destroy(pointer p) {}
	template<typename VT = value_type, typename std::enable_if<!std::is_scalar<VT>::value, int>::type = 0>
	void m_destroy(pointer p) {
		p->~value_type();
	}
	pointer ptr_begin() {
		return (pointer)m_data.data();
	}
	pointer ptr_end() {
		return m_end;
	}
	pointer ptr_cap_end() {
		return (pointer)(m_data.data() + max_elements);
	}
	const_pointer ptr_begin() const {
		return (pointer)m_data.data();
	}
	const_pointer ptr_end() const {
		return m_end;
	}
	const pointer ptr_cap_end() const {
		return (pointer)(m_data.data() + max_elements);
	}
public:
	static_vector() {}
	explicit static_vector(size_type count) {
		resize(count);
	}
	explicit static_vector(size_type count, const value_type& value) {
		resize(count, value);
	}
	static_vector(const static_vector& other) {
		m_assign(other);
	}
	static_vector(static_vector&& other) {
		m_assign(std::move(other));
	}
	~static_vector() {
		for (auto i = ptr_begin(); i != ptr_end(); ++i) {
			m_destroy(i);
		}
	}
	void resize(size_type count) {
		m_resize(count);
	}
	void resize(size_type count, const value_type& value) {
		m_resize(count, value);
	}
	static_vector& operator=(const static_vector& other) {
		m_assign(other);
		return *this;
	}
	static_vector& operator=(static_vector&& other) {
		m_assign(std::move(other));
		return *this;
	}
	reference at(size_type pos) {
		if (pos >= size()) throw std::out_of_range("static_vector subscript out of range");
		return *(ptr_begin() + pos);
	}
	const_reference at(size_type pos) const {
		if (pos >= size()) throw std::out_of_range("static_vector subscript out of range");
		return *(ptr_begin() + pos);
	}
	reference operator[](size_type pos) {
		return *(ptr_begin() + pos);
	}
	constexpr const_reference operator[](size_type pos) const {
		return *(ptr_begin() + pos);
	}
	reference front() {
		return *ptr_begin();
	}
	const_reference front() const {
		return *ptr_begin();
	}
	reference back() {
		return *(ptr_end() - 1);
	}
	const_reference back() const {
		return *(ptr_end() - 1);
	}
	T* data() {
		return ptr_begin();
	}
	const T* data() const {
		return ptr_begin();
	}
	iterator begin() {
		return iterator(ptr_begin());
	}
	const_iterator begin() const {
		return const_iterator(ptr_begin());
	}
	const_iterator cbegin() const {
		return iterator(ptr_begin());
	}
	iterator end() {
		return iterator(ptr_end());
	}
	const_iterator end() const {
		return const_iterator(ptr_end());
	}
	const_iterator cend() const {
		return const_iterator(ptr_end());
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
		return ptr_begin() == ptr_end();
	}
	size_type size() const {
		return ptr_end() - ptr_begin();
	}
	constexpr size_type max_size() const {
		return max_elements;
	}
	constexpr size_type capacity() const {
		return max_elements;
	}
	void clear() {
		m_clear();
	}
	void push_back(const T& value) {
		if (size() == capacity()) throw std::length_error("static_vector resized beyond capacity");
		new (ptr_end()) value_type(value);
		m_end = ptr_end() + 1;
	}
	void push_back(T&& value) {
		if (size() == capacity()) throw std::length_error("static_vector resized beyond capacity");
		new (ptr_end()) value_type(std::move(value));
		m_end = ptr_end() + 1;
	}
	template<typename... args_T>
	void emplace_back(args_T&&... args) {
		if (size() == capacity()) throw std::length_error("static_vector resized beyond capacity");
		new (ptr_end()) value_type(std::forward<args_T>(args)...);
		m_end = ptr_end() + 1;
	}
	void pop_back() {
		m_destroy(ptr_end() - 1);
		--m_end;
	}
	iterator erase(const iterator pos) {
		for (pointer i = pos.ptr;;) {
			pointer ni = i + 1;
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
