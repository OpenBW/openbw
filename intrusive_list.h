

template<typename T, std::pair<T*, T*> T::*link_ptr>
struct intrusive_list_member_link {
	std::pair<T*, T*>* operator()(T* ptr) {
		return &(ptr->*link_ptr);
	}
};

template<typename T, typename link_T>
struct intrusive_list {
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;

	struct const_iterator {
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef typename intrusive_list::value_type value_type;
		typedef typename intrusive_list::difference_type difference_type;
		typedef typename intrusive_list::const_pointer pointer;
		typedef typename intrusive_list::const_reference reference;
	private:
		typedef const_iterator this_t;
		pointer ptr = nullptr;
	public:
		const_iterator() = default;
		explicit const_iterator(pointer ptr) : ptr(ptr) {}
		explicit const_iterator(reference v) : ptr(&v) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		this_t& operator++() {
			ptr = link_T()(ptr)->second;
			return *this;
		}
		this_t& operator++(int) {
			auto r = *this;
			ptr = link_T()(ptr)->second;
			return r;
		}
		this_t& operator--() {
			ptr = link_T()(ptr)->first;
			return *this;
		}
		this_t& operator--(int) {
			auto r = *this;
			ptr = link_T()(ptr)->first;
			return r;
		}
		bool operator==(const this_t& rhs) const {
			return ptr == rhs.ptr;
		}
		bool operator!=(const this_t& rhs) const {
			return !(*this == rhs);
		}
	};

	struct iterator {
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef typename intrusive_list::value_type value_type;
		typedef typename intrusive_list::difference_type difference_type;
		typedef typename intrusive_list::pointer pointer;
		typedef typename intrusive_list::reference reference;
	private:
		typedef iterator this_t;
		pointer ptr = nullptr;
	public:
		iterator() = default;
		explicit iterator(pointer ptr) : ptr(ptr) {}
		explicit iterator(reference v) : ptr(&v) {}
		reference operator*() const {
			return *ptr;
		}
		pointer operator->() const {
			return ptr;
		}
		this_t& operator++() {
			ptr = link_T()(ptr)->second;
			return *this;
		}
		this_t operator++(int) {
			auto r = *this;
			ptr = link_T()(ptr)->second;
			return r;
		}
		this_t& operator--() {
			ptr = link_T()(ptr)->first;
			return *this;
		}
		this_t& operator--(int) {
			auto r = *this;
			ptr = link_T()(ptr)->first;
			return r;
		}
		bool operator==(const this_t& rhs) const {
			return ptr == rhs.ptr;
		}
		bool operator!=(const this_t& rhs) const {
			return !(*this == rhs);
		}
	};

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
	std::pair<T*, T*> header = { &*end(), &*end() };
	pointer ptr_begin() const {
		return header.second;
	}
	pointer ptr_end() const {
		uint8_t* p = (uint8_t*)&header;
		p -= (intptr_t)link_T()((T*)nullptr);
		return (T*)p;
	}
	pointer ptr_front() const {
		return header.second;
	}
	pointer ptr_back() const {
		return header.first;
	}
	void set_front_back_ptrs() {
		link_T()(ptr_front())->first = ptr_end();
		link_T()(ptr_back())->second = ptr_end();
	}
public:
	intrusive_list() = default;
	intrusive_list(const intrusive_list &) = delete;
	intrusive_list(intrusive_list&& n) noexcept {
		*this = std::move(n);
	}
	intrusive_list& operator=(const intrusive_list& n) = delete;
	intrusive_list& operator=(intrusive_list&& n) noexcept {
		if (n.empty()) clear();
		else {
			header = n.header;
			n.clear();
			set_front_back_ptrs();
		}
		return *this;
	}
	reference front() {
		return *ptr_front();
	}
	const_reference front() const {
		return *ptr_front();
	}
	reference back() {
		return *ptr_back();
	}
	const_reference back() const {
		return *ptr_back();
	}
	iterator begin() {
		return iterator(ptr_begin());
	}
	const_iterator begin() const {
		return iterator(ptr_begin());
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
	size_type size() const = delete;
	constexpr size_type max_size() const = delete;
	void clear() {
		header = { &*end(), &*end() };
	}
	iterator insert(iterator pos, reference v) {
		pointer before = &*pos;
		pointer after = link_T()(before)->first;
		link_T()(before)->first = &v;
		link_T()(after)->second = &v;
		link_T()(&v)->first = after;
		link_T()(&v)->second = before;
		return iterator(v);
	}
	iterator erase(iterator pos) {
		pointer ptr = &*pos;
		pointer prev = link_T()(ptr)->first;
		pointer next = link_T()(ptr)->second;
		link_T()(prev)->second = next;
		link_T()(next)->first = prev;
		return iterator(*next);
	}
	void remove(reference v) {
		erase(iterator(v));
	}
	void push_back(reference v) {
		insert(end(), v);
	}
	void pop_back() {
		erase(back());
	}
	void push_front(reference v) {
		insert(begin(), v);
	}
	void pop_front() {
		erase(iterator(front()));
	}
	void swap(intrusive_list& n) noexcept {
		if (n.empty()) {
			if (empty()) return;
			n.header = header;
			n.set_front_back_ptrs();
			clear();
		} else if (empty()) {
			header = n.header;
			set_front_back_ptrs();
			n.clear();
		} else {
			std::swap(header, n.header);
			set_front_back_ptrs();
			n.set_front_back_ptrs();
		}
	}
	iterator iterator_to(reference v) {
		return iterator(v);
	}
	static iterator s_iterator_to(reference v) {
		return iterator(v);
	}
};

