#ifndef BWGAME_DATA_LOADING_H
#define BWGAME_DATA_LOADING_H

#include "util.h"
#include "containers.h"
#include "data_types.h"

#include <type_traits>
#include <array>
#include <cstring>
#include <cstdio>

namespace bwgame {
namespace data_loading {

static_assert(std::is_same<uint8_t, unsigned char>::value || std::is_same<uint8_t, char>::value, "uint8_t must be (unsigned) char (we use it for aliasing)");

template<typename T>
struct is_std_array : std::false_type {};
template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type{};

// todo: Nothing here should throw exceptions. We should instead have a type to report errors,
//       and all functions should set it through a reference or similar (optional/variant?).
//  (allocators can still throw exceptions if they want to)

static inline bool is_native_little_endian() {
	union endian_t {uint32_t a; uint8_t b;};
	return (endian_t{1}).b == 1;
}

template<typename T, bool little_endian, typename std::enable_if<!is_std_array<T>::value>::type* = nullptr>
static inline T value_at(const uint8_t* ptr) {
	static_assert(std::is_integral<T>::value, "can only read integers and arrays of integers");
	union endian_t {uint32_t a; uint8_t b;};
	const bool native_little_endian = (endian_t{1}).b == 1;
	if (little_endian == native_little_endian) {
		if (((uintptr_t)(void*)ptr & (alignof(T) - 1)) == 0) {
			return *(T*)(void*)ptr;
		} else {
			typename std::aligned_storage<sizeof(T), alignof(T)>::type buf;
			memcpy(&buf, ptr, sizeof(T));
			return *(T*)(void*)&buf;
		}
	} else {
		T r = 0;
		for (size_t i = 0; i < sizeof(T); ++i) {
			r |= (T)ptr[i] << ((little_endian ? i : sizeof(T) - 1 - i) * 8);
		}
		return r;
	}
}
template<typename T, bool little_endian, typename std::enable_if<is_std_array<T>::value>::type* = nullptr>
static inline T value_at(const uint8_t* ptr) {
	T r;
	for (auto& v : r) {
		v = value_at<typename std::remove_reference<decltype(v)>::type, little_endian>(ptr);
		ptr += sizeof(v);
	}
	return r;
}

template<bool little_endian, typename T>
static inline void set_value_at(uint8_t* ptr, T value) {
	static_assert(std::is_integral<T>::value, "can only write integers");
	union endian_t {uint32_t a; uint8_t b;};
	const bool native_little_endian = (endian_t{1}).b == 1;
	if (little_endian == native_little_endian) {
		if (((uintptr_t)(void*)ptr & (alignof(T) - 1)) == 0) {
			*(T*)(void*)ptr = value;
		} else {
			memcpy(ptr, &value, sizeof(T));
		}
	} else {
		for (size_t i = 0; i < sizeof(T); ++i) {
			ptr[i] = value >> ((little_endian ? i : sizeof(T) - 1 - i) * 8);
		}
	}
}

template<typename T, bool little_endian, typename self_T>
T get_impl(self_T& self) {
	typename std::aligned_storage<sizeof(T), alignof(T)>::type buf;
	self.get_bytes((uint8_t*)&buf, sizeof(T));
	return value_at<T, little_endian>((uint8_t*)&buf);
}

template<bool default_little_endian = true, bool bounds_checking = true>
struct data_reader {
	const uint8_t* ptr = nullptr;
	const uint8_t* begin = nullptr;
	const uint8_t* end = nullptr;
	data_reader() = default;
	data_reader(const uint8_t* ptr, const uint8_t* end) : ptr(ptr), begin(ptr), end(end) {}
	template<typename T, bool little_endian = default_little_endian>
	T get() {
		if (bounds_checking && left() < sizeof(T)) error("data_reader: attempt to read past end");
		ptr += sizeof(T);
		return value_at<T, little_endian>(ptr - sizeof(T));
	}
	const uint8_t* get_n(size_t n) {
		const uint8_t* r = ptr;
		if (bounds_checking && left() < n) error("data_reader: attempt to read past end");
		ptr += n;
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	a_vector<T> get_vec(size_t n) {
		const uint8_t* data = get_n(n*sizeof(T));
		a_vector<T> r(n);
		for (size_t i = 0; i < n; ++i, data += sizeof(T)) {
			r[i] = value_at<T, little_endian>(data);
		}
		return r;
	}
	void skip(size_t n) {
		if (bounds_checking && left() < n) error("data_reader: attempt to seek past end");
		ptr += n;
	}
	void get_bytes(uint8_t* dst, size_t n) {
		memcpy(dst, get_n(n), n);
	}
	void seek(size_t offset) {
		if (offset > size()) error("data_reader: attempt to seek past end");
		ptr = begin + offset;
	}
	size_t left() const {
		return end - ptr;
	}
	size_t size() const {
		return (size_t)(end - begin);
	}
	size_t tell() const {
		return (size_t)(ptr - begin);
	}
};

using data_reader_le = data_reader<true>;
using data_reader_be = data_reader<false>;


template<typename base_reader_T, size_t page_size = 0x1000, bool default_little_endian = true>
struct paged_reader {
	base_reader_T& reader;
	std::array<uint8_t, page_size> page;
	size_t current_page = ~(size_t)0;
	size_t file_pointer = 0;
	size_t file_size = 0;
	explicit paged_reader(base_reader_T& reader) : reader(reader) {
		file_size = reader.size();
	}
	void get_bytes(uint8_t* dst, size_t n) {
		if (left() < n) error("paged_reader: attempt to read past end");
		if (n > page_size / 2) {
			reader.seek(file_pointer);
			reader.get_bytes(dst, n);
		} else {
			while (n) {
				size_t page_n = file_pointer / page_size;
				size_t page_offset = file_pointer % page_size;
				size_t read_n = std::min(n, page_size - page_offset);
				if (current_page == page_n) {
					memcpy(dst, &page[page_offset], read_n);
				} else {
					if (read_n == page_size - page_offset) {
						reader.seek(file_pointer);
						reader.get_bytes(dst, read_n);
					} else {
						current_page = page_n;
						reader.seek(page_n * page_size);
						reader.get_bytes(page.data(), std::min(page_size, file_size - page_n * page_size));
						memcpy(dst, &page[page_offset], read_n);
					}
				}
				dst += read_n;
				file_pointer += read_n;
				n -= read_n;
			}
		}
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		uint8_t buffer[sizeof(T)];
		get_bytes(buffer, sizeof(T));
		return value_at<T, little_endian>(buffer);
	}

	void seek(size_t offset) {
		if (offset > file_size) error("paged_reader: attempt to seek past end");
		file_pointer = offset;
	}
	
	size_t left() const {
		return file_size - file_pointer;
	}
	size_t size() const {
		return file_size;
	}
	size_t tell() const {
		return file_pointer;
	}

	bool eof() const {
		return file_pointer == file_size;
	}
};

template<bool default_little_endian = true>
struct file_reader {
	a_string filename;
	FILE* f = nullptr;
	file_reader() = default;
	explicit file_reader(a_string filename) {
		open(std::move(filename));
	}
	~file_reader() {
		if (f) fclose(f);
	}
	file_reader(const file_reader&) = delete;
	file_reader(file_reader&& n) {
		f = n.f;
		n.f = nullptr;
	}
	file_reader& operator=(const file_reader&) = delete;
	file_reader& operator=(file_reader&& n) {
		std::swap(f, n.f);
		return *this;
	}

	void open(a_string filename) {
		if (f) fclose(f);
		f = fopen(filename.c_str(), "rb");
		if (!f) error("file_reader: failed to open %s for reading", filename.c_str());
		this->filename = std::move(filename);
	}

	void get_bytes(uint8_t* dst, size_t n) {
		if (!fread(dst, n, 1, f)) {
			if (feof(f)) error("file_reader: %s: attempt to read past end", filename);
			error("file_reader: %s: read error", filename);
		}
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		uint8_t buffer[sizeof(T)];
		get_bytes(buffer, sizeof(T));
		return value_at<T, little_endian>(buffer);
	}

	template<typename T, bool little_endian = default_little_endian, typename std::enable_if<sizeof(T) == 1>::type* = nullptr>
	a_vector<T> get_vec(size_t n) {
		a_vector<T> r(n);
		get_bytes((uint8_t*)(void*)r.data(), n * sizeof(T));
		return r;
	}

	template<typename T, bool little_endian = default_little_endian, typename std::enable_if<(sizeof(T) > 1)>::type* = nullptr>
	a_vector<T> get_vec(size_t n) {
		a_vector<T> r(n);
		for (size_t i = 0; i < n; ++i) {
			r[i] = get<T, little_endian>();
		}
		return r;
	}

	void seek(size_t offset) {
		if ((size_t)(long)offset != offset || fseek(f, (long)offset, SEEK_SET)) error("file_reader: %s: failed to seek to offset %d", filename, offset);
	}

	bool eof() {
		return feof(f);
	}
	
	size_t left() const {
		return size() - tell();
	}
	size_t tell() const {
		return (size_t)ftell(f);
	}

	size_t size() {
		auto prev_pos = ftell(f);
		fseek(f, 0, SEEK_END);
		auto r = ftell(f);
		fseek(f, prev_pos, SEEK_SET);
		return r;
	}

};

using crypt_table_t = std::array<uint32_t, 256 * 5>;
static auto get_crypt_table() {
	uint32_t n = 0x100001;
	crypt_table_t crypt_table;
	auto next = [&]() {
		n = (n * 125 + 3) % 0x2aaaab;
		return n & 0xffff;
	};
	for (size_t i = 0; i != 256; ++i) {
		for (size_t i2 = 0; i2 != 5; ++i2) {
			crypt_table[256 * i2 + i] = next() << 16;
			crypt_table[256 * i2 + i] |= next();
		}
	}
	return crypt_table;
}

template<typename base_reader_T, bool default_little_endian = true>
struct encrypted_reader {
	base_reader_T& reader;
	size_t end_pos = 0;
	uint32_t key;
	uint32_t add_n = 0xeeeeeeee;
	const crypt_table_t& crypt_table;
	uint32_t data;
	size_t data_n = 0;
	size_t pos = 0;
	encrypted_reader(base_reader_T& reader, size_t end_pos, uint32_t key, const crypt_table_t& crypt_table) : reader(reader), end_pos(end_pos), key(key), crypt_table(crypt_table) {
	}

	void next() {
		pos += 4;
		auto d = reader.template get<uint32_t, true>();
		add_n += crypt_table[(key&0xff) + 1024];
		uint32_t xor_n = key + add_n;
		data = d ^ xor_n;
		add_n = add_n * 33 + data + 3;
		key = ((~key << 21) + 0x11111111) | key >> 11;
	}

	void get_bytes(uint8_t* dst, size_t n) {
		if (left() < n) error("encrypted_reader: attempt to read past end");
		size_t i = 0;
		if (data_n) {
			for (;i != std::min(n, data_n); ++i) {
				dst[i] = data >> (8 * (4 - data_n + i));
			}
			data_n -= std::min(n, data_n);
		}
		while (i + 4 <= n) {
			next();
			set_value_at<true>(dst + i, data);
			i += 4;
		}
		if (i != n) {
			if (end_pos - pos < 4) {
				reader.get_bytes(&dst[i], n - i);
				data_n = 0;
				pos += n - i;
			} else {
				next();
				data_n = 4 - (n - i);
				for (size_t i2 = 0; i != n; ++i, ++i2) {
					dst[i] = data >> (8 * i2);
				}
			}
		}
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		return get_impl<T, little_endian>(*this);
	}
	
	size_t left() const {
		return end_pos - pos + data_n;
	}
	size_t size() const {
		return end_pos;
	}
	size_t tell() const {
		return pos - data_n;
	}

};

template<bool little_endian = true, typename base_reader_T>
auto make_encrypted_reader(base_reader_T& reader, size_t end_pos, uint32_t key, const crypt_table_t& crypt_table) {
	return encrypted_reader<base_reader_T, little_endian>(reader, end_pos, key, crypt_table);
}

static uint32_t string_hash(const char* str, int n, const crypt_table_t& crypt_table) {
	uint32_t r = 0x7FED7FED;
	uint32_t add_n = 0xeeeeeeee;
	for (;*str; ++str) {
		char c = *str;
		if (c == '/') c = '\\';
		else if (c >= 'a' && c <= 'z') c += 'A' - 'a';
		r = (r + add_n) ^ crypt_table[256 * n + c];
		add_n = add_n * 33 + c + r + 3;
	}
	return r;
}

template<typename base_reader_T, bool default_little_endian = true>
struct bit_reader {
	base_reader_T& r;
	uint64_t data{};
	size_t bits_n = 0;
	explicit bit_reader(base_reader_T& r) : r(r) {}
	void next() {
		auto left = r.left();
		if (left >= 4) {
			data |= (uint64_t)r.template get<uint32_t, true>() << bits_n;
			bits_n += 32;
			return;
		}
		switch (left) {
		case 0:
		case 1:
			data |= (uint64_t)r.template get<uint8_t>() << bits_n;
			bits_n += 8;
			break;
		case 2:
			data |= (uint64_t)r.template get<uint16_t, true>() << bits_n;
			bits_n += 16;
			break;
		case 3:
			data |= (uint64_t)r.template get<uint16_t, true>() << bits_n;
			bits_n += 16;
			data |= (uint64_t)r.template get<uint8_t>() << bits_n;
			bits_n += 8;
			break;
		}
	}
	template<size_t bits, bool little_endian = default_little_endian>
	auto get_bits() {
		static_assert(bits <= 32, "bit_reader: only up to 32 bit reads are supported");
		using r_t = uint_fastn_t<bits>;
		if (bits_n < bits) {
			next();
		}
		r_t r = data & (((r_t)1 << bits) - 1);
		data >>= bits;
		bits_n -= bits;
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	T get() {
		return get_bits<int_bits<T>::value, little_endian>();
	}
};

template<bool little_endian = true, typename base_reader_T>
auto make_bit_reader(base_reader_T& reader) {
	return bit_reader<base_reader_T, little_endian>(reader);
}

template<bool little_endian = true>
void decompress(uint8_t* input, size_t input_size, uint8_t* output, size_t output_size) {
	data_reader<little_endian> source_r(input, input + input_size);
	auto r = make_bit_reader(source_r);
	int type = r.template get<uint8_t>();
	int distance_bits = r.template get<uint8_t>();

	if (distance_bits != 4 && distance_bits != 5 && distance_bits != 6) error("decompress: invalid distance bits %d", distance_bits);

	auto get_length = [&]() {
		switch (r.template get_bits<2>()) {
		case 3: return 1;
		case 0:
			switch (r.template get_bits<2>()) {
			case 3: return 6;
			case 0:
				switch (r.template get_bits<6>()) {
				case 3: return 22;
				case 7: return 23;
				case 11: return 24;
				case 15: return 25;
				case 19: return 26;
				case 23: return 27;
				case 27: return 28;
				case 31: return 29;
				case 35: return 30;
				case 39: return 31;
				case 43: return 32;
				case 47: return 33;
				case 51: return 34;
				case 55: return 35;
				case 59: return 36;
				case 63: return 37;
				case 0: return 262 + 8 * r.template get_bits<5>();
				case 1: return r.template get_bits<1>() ? 54 : 38;
				case 2: return 70 + 16 * r.template get_bits<2>();
				case 4: return 134 + 8 * r.template get_bits<4>();
				case 5: return r.template get_bits<1>() ? 55 : 39;
				case 6: return 71 + 16 * r.template get_bits<2>();
				case 8: return 263 + 8 * r.template get_bits<5>();
				case 9: return r.template get_bits<1>() ? 56 : 40;
				case 10: return 72 + 16 * r.template get_bits<2>();
				case 12: return 135 + 8 * r.template get_bits<4>();
				case 13: return r.template get_bits<1>() ? 57 : 41;
				case 14: return 73 + 16 * r.template get_bits<2>();
				case 16: return 264 + 8 * r.template get_bits<5>();
				case 17: return r.template get_bits<1>() ? 58 : 42;
				case 18: return 74 + 16 * r.template get_bits<2>();
				case 20: return 136 + 8 * r.template get_bits<4>();
				case 21: return r.template get_bits<1>() ? 59 : 43;
				case 22: return 75 + 16 * r.template get_bits<2>();
				case 24: return 265 + 8 * r.template get_bits<5>();
				case 25: return r.template get_bits<1>() ? 60 : 44;
				case 26: return 76 + 16 * r.template get_bits<2>();
				case 28: return 137 + 8 * r.template get_bits<4>();
				case 29: return r.template get_bits<1>() ? 61 : 45;
				case 30: return 77 + 16 * r.template get_bits<2>();
				case 32: return 266 + 8 * r.template get_bits<5>();
				case 33: return r.template get_bits<1>() ? 62 : 46;
				case 34: return 78 + 16 * r.template get_bits<2>();
				case 36: return 138 + 8 * r.template get_bits<4>();
				case 37: return r.template get_bits<1>() ? 63 : 47;
				case 38: return 79 + 16 * r.template get_bits<2>();
				case 40: return 267 + 8 * r.template get_bits<5>();
				case 41: return r.template get_bits<1>() ? 64 : 48;
				case 42: return 80 + 16 * r.template get_bits<2>();
				case 44: return 139 + 8 * r.template get_bits<4>();
				case 45: return r.template get_bits<1>() ? 65 : 49;
				case 46: return 81 + 16 * r.template get_bits<2>();
				case 48: return 268 + 8 * r.template get_bits<5>();
				case 49: return r.template get_bits<1>() ? 66 : 50;
				case 50: return 82 + 16 * r.template get_bits<2>();
				case 52: return 140 + 8 * r.template get_bits<4>();
				case 53: return r.template get_bits<1>() ? 67 : 51;
				case 54: return 83 + 16 * r.template get_bits<2>();
				case 56: return 269 + 8 * r.template get_bits<5>();
				case 57: return r.template get_bits<1>() ? 68 : 52;
				case 58: return 84 + 16 * r.template get_bits<2>();
				case 60: return 141 + 8 * r.template get_bits<4>();
				case 61: return r.template get_bits<1>() ? 69 : 53;
				case 62: return 85 + 16 * r.template get_bits<2>();
				}
			case 1:
				switch (r.template get_bits<1>()) {
				case 1: return 7;
				case 0: return r.template get_bits<1>() ? 9 : 8;
				}
			case 2:
				switch (r.template get_bits<3>()) {
				case 1: return 10;
				case 3: return 11;
				case 5: return 12;
				case 7: return 13;
				case 0: return r.template get_bits<1>() ? 18 : 14;
				case 2: return r.template get_bits<1>() ? 19 : 15;
				case 4: return r.template get_bits<1>() ? 20 : 16;
				case 6: return r.template get_bits<1>() ? 21 : 17;
				}
			}
		case 1: return r.template get_bits<1>() ? 0 : 2;
		case 2:
			switch (r.template get_bits<1>()) {
			case 1: return 3;
			case 0: return r.template get_bits<1>() ? 4 : 5;
			}
		}
		return -1;
	};

	auto get_distance = [&]() {
		switch (r.template get_bits<2>()) {
		case 3: return 0;
		case 0:
			switch (r.template get_bits<5>()) {
			case 1: return 39;
			case 2: return 47;
			case 3: return 31;
			case 5: return 35;
			case 6: return 43;
			case 7: return 27;
			case 9: return 37;
			case 10: return 45;
			case 11: return 29;
			case 13: return 33;
			case 14: return 41;
			case 15: return 25;
			case 17: return 38;
			case 18: return 46;
			case 19: return 30;
			case 21: return 34;
			case 22: return 42;
			case 23: return 26;
			case 25: return 36;
			case 26: return 44;
			case 27: return 28;
			case 29: return 32;
			case 30: return 40;
			case 31: return 24;
			case 0: return r.template get_bits<1>() ? 62 : 63;
			case 4: return r.template get_bits<1>() ? 54 : 55;
			case 8: return r.template get_bits<1>() ? 58 : 59;
			case 12: return r.template get_bits<1>() ? 50 : 51;
			case 16: return r.template get_bits<1>() ? 60 : 61;
			case 20: return r.template get_bits<1>() ? 52 : 53;
			case 24: return r.template get_bits<1>() ? 56 : 57;
			case 28: return r.template get_bits<1>() ? 48 : 49;
			}
		case 1:
			switch (r.template get_bits<2>()) {
			case 1: return 2;
			case 3: return 1;
			case 0: return r.template get_bits<1>() ? 5 : 6;
			case 2: return r.template get_bits<1>() ? 3 : 4;
			}
		case 2:
			switch (r.template get_bits<4>()) {
			case 1: return 14;
			case 2: return 18;
			case 3: return 10;
			case 4: return 20;
			case 5: return 12;
			case 6: return 16;
			case 7: return 8;
			case 8: return 21;
			case 9: return 13;
			case 10: return 17;
			case 11: return 9;
			case 12: return 19;
			case 13: return 11;
			case 14: return 15;
			case 15: return 7;
			case 0: return r.template get_bits<1>() ? 22 : 23;
			}
		}
		return -1;
	};


	size_t out_pos = 0;

	if (type == 0) {

		while (out_pos != output_size) {
			if (r.template get_bits<1>()) {

				size_t len = 2 + get_length();
				size_t distance = 0;

				if (len == 519) error("decompress: eof marker found too early");

				if (len == 2) {
					distance = get_distance() << 2;
					distance |= r.template get_bits<2>();
				} else {
					distance = get_distance() << distance_bits;
					if (distance_bits == 4) distance |= r.template get_bits<4>();
					else if (distance_bits == 5) distance |= r.template get_bits<5>();
					else distance |= r.template get_bits<6>();
				}
				size_t src_pos = out_pos - 1 - distance;
				if (src_pos > output_size) {
					len = 0;
				}
				if (src_pos + len > output_size) {
					len = output_size - src_pos;
				}
				if (out_pos + len > output_size) {
					len = output_size - out_pos;
				}
				for (size_t i = 0; i != len; ++i) {
					output[out_pos + i] = output[src_pos + i];
				}
				out_pos += len;

			} else {
				output[out_pos] = r.template get<uint8_t>();
				++out_pos;
			}
		}

	} else error("decompress: type %d not supported", type);
}

static const uint8_t huffman_weight_tables[9][256 + 2] = {
	{ 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x00, 0x00 },
	{ 0x54, 0x16, 0x16, 0x0d, 0x0c, 0x08, 0x06, 0x05, 0x06, 0x05, 0x06, 0x03, 0x04, 0x04, 0x03, 0x05,
	0x0e, 0x0b, 0x14, 0x13, 0x13, 0x09, 0x0b, 0x06, 0x05, 0x04, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02,
	0x0d, 0x07, 0x09, 0x06, 0x06, 0x04, 0x03, 0x02, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02,
	0x09, 0x06, 0x04, 0x04, 0x04, 0x04, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x04,
	0x08, 0x03, 0x04, 0x07, 0x09, 0x05, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02,
	0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02,
	0x06, 0x0a, 0x08, 0x08, 0x06, 0x07, 0x04, 0x03, 0x04, 0x04, 0x02, 0x02, 0x04, 0x02, 0x03, 0x03,
	0x04, 0x03, 0x07, 0x07, 0x09, 0x06, 0x04, 0x03, 0x03, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x0a, 0x02, 0x02, 0x03, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x06, 0x03, 0x05, 0x02, 0x03,
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x01, 0x01, 0x01,
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x04, 0x04, 0x04, 0x07, 0x09, 0x08, 0x0c, 0x02,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x03,
	0x04, 0x01, 0x02, 0x04, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
	0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x06, 0x4b,
	0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x27, 0x00, 0x00, 0x23, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x06, 0x0e, 0x10, 0x04,
	0x06, 0x08, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03, 0x01, 0x01, 0x02, 0x01, 0x01,
	0x01, 0x04, 0x02, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x04, 0x01, 0x01, 0x02, 0x03, 0x03, 0x02,
	0x03, 0x01, 0x03, 0x06, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01,
	0x01, 0x29, 0x07, 0x16, 0x12, 0x40, 0x0a, 0x0a, 0x11, 0x25, 0x01, 0x03, 0x17, 0x10, 0x26, 0x2a,
	0x10, 0x01, 0x23, 0x23, 0x2f, 0x10, 0x06, 0x07, 0x02, 0x09, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 },
	{ 0xff, 0x0b, 0x07, 0x05, 0x0b, 0x02, 0x02, 0x02, 0x06, 0x02, 0x02, 0x01, 0x04, 0x02, 0x01, 0x03,
	0x09, 0x01, 0x01, 0x01, 0x03, 0x04, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
	0x05, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01,
	0x0a, 0x04, 0x02, 0x01, 0x06, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01,
	0x05, 0x02, 0x03, 0x04, 0x03, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x03, 0x03,
	0x01, 0x03, 0x01, 0x01, 0x02, 0x05, 0x01, 0x01, 0x04, 0x03, 0x05, 0x01, 0x03, 0x01, 0x03, 0x03,
	0x02, 0x01, 0x04, 0x03, 0x0a, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x01, 0x0a, 0x02, 0x05, 0x01, 0x01, 0x02, 0x07, 0x02, 0x17, 0x01, 0x05, 0x01, 0x01,
	0x0e, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x06, 0x02, 0x01, 0x04, 0x05, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x11,
	0x00, 0x00 },
	{ 0xff, 0xfb, 0x98, 0x9a, 0x84, 0x85, 0x63, 0x64, 0x3e, 0x3e, 0x22, 0x22, 0x13, 0x13, 0x18, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 },
	{ 0xff, 0xf1, 0x9d, 0x9e, 0x9a, 0x9b, 0x9a, 0x97, 0x93, 0x93, 0x8c, 0x8e, 0x86, 0x88, 0x80, 0x82,
	0x7c, 0x7c, 0x72, 0x73, 0x69, 0x6b, 0x5f, 0x60, 0x55, 0x56, 0x4a, 0x4b, 0x40, 0x41, 0x37, 0x37,
	0x2f, 0x2f, 0x27, 0x27, 0x21, 0x21, 0x1b, 0x1c, 0x17, 0x17, 0x13, 0x13, 0x10, 0x10, 0x0d, 0x0d,
	0x0b, 0x0b, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x05, 0x05, 0x04, 0x04, 0x04, 0x19, 0x18,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 },
	{ 0xc3, 0xcb, 0xf5, 0x41, 0xff, 0x7b, 0xf7, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xbf, 0xcc, 0xf2, 0x40, 0xfd, 0x7c, 0xf7, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7a, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 },
	{ 0xc3, 0xd9, 0xef, 0x3d, 0xf9, 0x7c, 0xe9, 0x1e, 0xfd, 0xab, 0xf1, 0x2c, 0xfc, 0x5b, 0xfe, 0x17,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xbd, 0xd9, 0xec, 0x3d, 0xf5, 0x7d, 0xe8, 0x1d, 0xfb, 0xae, 0xf0, 0x2c, 0xfb, 0x5c, 0xff, 0x18,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x70, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 },
	{ 0xba, 0xc5, 0xda, 0x33, 0xe3, 0x6d, 0xd8, 0x18, 0xe5, 0x94, 0xda, 0x23, 0xdf, 0x4a, 0xd1, 0x10,
	0xee, 0xaf, 0xe4, 0x2c, 0xea, 0x5a, 0xde, 0x15, 0xf4, 0x87, 0xe9, 0x21, 0xf6, 0x43, 0xfc, 0x12,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xb0, 0xc7, 0xd8, 0x33, 0xe3, 0x6b, 0xd6, 0x18, 0xe7, 0x95, 0xd8, 0x23, 0xdb, 0x49, 0xd0, 0x11,
	0xe9, 0xb2, 0xe2, 0x2b, 0xe8, 0x5c, 0xdd, 0x15, 0xf1, 0x87, 0xe7, 0x20, 0xf7, 0x44, 0xff, 0x13,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x5f, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00 }
};

template<bool little_endian = true>
size_t decompress_huffman(uint8_t* input, size_t input_size, uint8_t* output, size_t output_size) {
	data_reader<little_endian> source_r(input, input + input_size);
	auto r = make_bit_reader(source_r);
	size_t weights_index = r.template get<uint8_t>();
	if (weights_index >= 9) error("decompress_huffman: invalid weights index %d", weights_index);
	const uint8_t* weights = huffman_weight_tables[weights_index];
	
	struct tree_node;
	using tree_node_iterator = typename a_list<tree_node>::iterator;
	struct tree_node {
		tree_node_iterator left;
		tree_node_iterator parent;
		int weight;
		int symbol;
	};
	a_list<tree_node> all_nodes;
	auto end = all_nodes.end();
	all_nodes.push_back({end, end, 1, 0x101});
	all_nodes.push_back({end, end, 1, 0x100});
	for (int i = 256; i != 0;) {
		--i;
		int w = weights[i];
		if (w == 0) continue;
		all_nodes.push_back({end, end, w, i});
	}
	
	all_nodes.sort([&](auto& a, auto& b) {
		return a.weight < b.weight;
	});
	
	for (auto a = all_nodes.begin(); a != end;) {
		auto b = std::next(a);
		if (b == end) break;
		
		int w = a->weight + b->weight;
		auto it = std::find_if(all_nodes.begin(), all_nodes.end(), [&](auto& v) {
			return v.weight >= w;
		});
		it = all_nodes.insert(it, {a, end, w, -1});
		
		a->parent = it;
		b->parent = it;
		
		a = std::next(b);
	}
	
	auto increment_weight = [&](tree_node_iterator n) {
		for (; n != end; n = n->parent) {
			++n->weight;
			int w = n->weight;
			auto swap_n_next = std::find_if(std::next(n), end, [&](auto& v) {
				return v.weight >= w;
			});
			auto swap_n = std::prev(swap_n_next);
			if (swap_n == n) continue;
			all_nodes.splice(n, all_nodes, swap_n);
			all_nodes.splice(swap_n_next, all_nodes, n);
			
			if (n->parent->left == n) {
				if (swap_n->parent->left == swap_n) swap_n->parent->left = n;
				n->parent->left = swap_n;
			} else if (swap_n->parent->left == swap_n) swap_n->parent->left = n;
			std::swap(n->parent, swap_n->parent);
		}
	};
	
	size_t out_pos = 0;
	
	while (out_pos < output_size) {
		tree_node_iterator n = std::prev(end);
		while (n->symbol == -1) {
			int bit = r.template get_bits<1>();
			if (bit == 0) n = n->left;
			else n = std::next(n->left);
		}
		if (n->symbol == 256) break;
		uint8_t value;
		if (n->symbol == 257) {
			int symbol = r.template get_bits<8>();
			value = symbol;
			
			n = all_nodes.begin();
			int n_symbol = n->symbol;
			
			n->symbol = -1;
			
			all_nodes.push_front({end, n, 1, n_symbol});
			all_nodes.push_front({end, n, 0, symbol});
			n->left = all_nodes.begin();
			n = n->left;
			
			increment_weight(n);
			if (weights_index != 0) increment_weight(n);
			
		} else value = n->symbol;
		output[out_pos] = value;
		++out_pos;
		
		if (weights_index == 0) increment_weight(n);
	}
	return out_pos;
}

static const int adpcm_index_add_table[32] = {
	-1, 0, -1, 4, -1, 2, -1, 6,
	-1, 1, -1, 5, -1, 3, -1, 7,
	-1, 1, -1, 5, -1, 3, -1, 7,
	-1, 2, -1, 4, -1, 6, -1, 8
}; 
static const int32_t adpcm_step_table[89] = { 
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

template<bool little_endian = true>
size_t decompress_adpcm(uint8_t* input, size_t input_size, uint8_t* output, size_t output_size, size_t channels) {
	data_reader<little_endian> r(input, input + input_size);
	
	size_t out_pos = 0;
	
	a_vector<int16_t> previous_sample(channels);
	a_vector<int> step_index(channels, 44);
	
	r.template get<uint8_t>();
	auto shift = r.template get<uint8_t>();
	
	for (size_t i = 0; i != channels; ++i) {
		auto sample = r.template get<int16_t>();
		previous_sample[i] = sample;
		if (out_pos - output_size < 2) error("decompress_adpcm: attempt to write past end");
		set_value_at<little_endian, int16_t>(output + out_pos, sample);
		out_pos += 2;
	}
	
	while (r.left()) {
		for (size_t channel = 0; channel != channels; ++channel) {
			auto in_value = r.template get<uint8_t>();
			if (~in_value & 0x80) {
				int index = step_index[channel];
				int32_t step = adpcm_step_table[index];
				int32_t sample = step >> shift;
				if (in_value & 1) sample += step;
				if (in_value & 2) sample += step >> 1;
				if (in_value & 4) sample += step >> 2;
				if (in_value & 8) sample += step >> 3;
				if (in_value & 0x10) sample += step >> 4;
				if (in_value & 0x20) sample += step >> 5;
				if (in_value & 0x40) {
					sample = previous_sample[channel] - sample;
					if (sample < -32768) sample = -32768;
				} else {
					sample = previous_sample[channel] + sample;
					if (sample > 32767) sample = 32767;
				}
				
				previous_sample[channel] = sample;
				if (out_pos - output_size < 2) error("decompress_adpcm: attempt to write past end");
				set_value_at<true, int16_t>(output + out_pos, sample);
				out_pos += 2;
				
				index += adpcm_index_add_table[in_value & 0x1f];
				if (index < 0) index = 0;
				else if (index > 88) index = 88;
				step_index[channel] = index;
			} else {
				int n = in_value & 0x7f;
				if (n == 0) {
					if (step_index[channel] != 0) --step_index[channel];
					if (out_pos - output_size < 2) error("decompress_adpcm: attempt to write past end");
					set_value_at<little_endian, int16_t>(output + out_pos, previous_sample[channel]);
					out_pos += 2;
				} else if (n == 1) {
					if (step_index[channel] >= 88 - 8) step_index[channel] = 88;
					else step_index[channel] += 8;
					--channel;
				} else if (n == 2) {
					if (step_index[channel] <= 8) step_index[channel] = 0;
					else step_index[channel] -= 8;
					--channel;
				}
			}
		}
	}
	
	return out_pos;
}

struct hash_table_entry {
	uint32_t hash1;
	uint32_t hash2;
	size_t block_index;
};
struct block_table_entry {
	uint32_t data_offset;
	uint32_t compressed_size;
	uint32_t size;
	uint32_t flags;
};

template<typename base_reader_T, bool default_little_endian = true>
struct mpq_archive_file_reader {
	a_string filename;
	base_reader_T& r;
	size_t sector_size;
	block_table_entry be;
	uint32_t key;
	const crypt_table_t& crypt_table;
	a_vector<size_t> compressed_sectors;
	size_t current_sector = ~(size_t)0;
	a_vector<uint8_t> compressed_data;
	a_vector<uint8_t> sector_data;
	size_t file_position = 0;
	mpq_archive_file_reader(a_string arg_filename, base_reader_T& r, size_t sector_size, block_table_entry be, uint32_t key, const crypt_table_t& crypt_table) : filename(std::move(arg_filename)), r(r), sector_size(sector_size), be(be), key(key), crypt_table(crypt_table) {

		if (~be.flags & 0x200 && ~be.flags & 0x100) {
			error("mpq: %s: file is not compressed", filename);
		}

		sector_data.resize(sector_size);
		r.seek(be.data_offset);

		auto read_sectors = [&](auto&& sectors_r) {
			while (true) {
				size_t offset = sectors_r.template get<uint32_t>();
				compressed_sectors.push_back(offset);
				if (offset == be.compressed_size) break;
				if (compressed_sectors.size() > (be.size + sector_size - 1) / sector_size) error("mpq: %s: too many compressed sectors in file", filename);
			}
		};

		if (be.flags & 0x10000) {
			read_sectors(make_encrypted_reader(r, 4 * ((be.size + sector_size - 1) / sector_size + 1), key - 1, crypt_table));
		} else {
			read_sectors(r);
		}
	}
	void read_sector() {
		current_sector = file_position / sector_size;
		if (current_sector >= compressed_sectors.size() - 1) error("mpq: %s: attempt to read past end", filename);

		size_t sector_data_size = compressed_sectors[current_sector + 1] - compressed_sectors[current_sector];
		r.seek(be.data_offset + compressed_sectors[current_sector]);

		int compression_flags;
		auto get_data = [&](auto&& sector_data_r) {
			if (~be.flags & 0x200) {
				compression_flags = 8;
				if (compressed_data.size() < sector_data_size) compressed_data.resize(sector_data_size);
				sector_data_r.get_bytes(compressed_data.data(), sector_data_size);
			} else {
				compression_flags = sector_data_r.template get<uint8_t>();
	
				if (compressed_data.size() < sector_data_size) compressed_data.resize(sector_data_size);
				sector_data_r.get_bytes(compressed_data.data(), sector_data_size - 1);
			}
		};

		size_t current_sector_size = sector_data.size();
		if (current_sector == compressed_sectors.size() - 2) current_sector_size = be.size % sector_size;

		if (sector_data_size == current_sector_size && sector_data_size <= sector_size) {
			if (be.flags & 0x10000) make_encrypted_reader(r, sector_data_size, key + (uint32_t)current_sector, crypt_table).get_bytes(sector_data.data(), sector_data_size);
			else r.get_bytes(sector_data.data(), sector_data_size);
		} else {
			if (be.flags & 0x10000) get_data(make_encrypted_reader(r, sector_data_size, key + (uint32_t)current_sector, crypt_table));
			else get_data(r);
			if (compression_flags == 8) decompress(compressed_data.data(), sector_data_size, sector_data.data(), current_sector_size);
			else {
				size_t input_size = sector_data_size;
				auto swap_buffers = [&](size_t new_input_size) {
					input_size = new_input_size;
					std::swap(compressed_data, sector_data);
					if (compressed_data.size() < sector_data_size) compressed_data.resize(sector_data_size);
					if (sector_data.size() < current_sector_size) sector_data.resize(current_sector_size);
				};
				if (compression_flags & 1) {
					compression_flags &= ~1;
					size_t out_size = decompress_huffman(compressed_data.data(), input_size, sector_data.data(), current_sector_size);
					if (compression_flags) swap_buffers(out_size);
				}
				if (compression_flags & 0x40) {
					compression_flags &= ~0x40;
					size_t out_size = decompress_adpcm(compressed_data.data(), input_size, sector_data.data(), current_sector_size, 1);
					if (compression_flags) swap_buffers(out_size);
				}
				if (compression_flags & 0x80) {
					compression_flags &= ~0x80;
					size_t out_size = decompress_adpcm(compressed_data.data(), input_size, sector_data.data(), current_sector_size, 2);
					if (compression_flags) swap_buffers(out_size);
				}
				if (compression_flags != 0) error("mpq: %s: unsupported compression flags %d", filename, compression_flags);
			}
		}

	}

	void get_bytes(uint8_t* dst, size_t n) {
		if (file_position + n > be.size) error("mpq: %s: attempt to read past end", filename);
		if (file_position / sector_size != current_sector) read_sector();
		size_t sector_offset = file_position % sector_size;
		while (sector_size - sector_offset < n) {
			size_t n_read = sector_size - sector_offset;
			memcpy(dst, &sector_data[sector_offset], n_read);
			dst += n_read;
			n -= n_read;
			file_position += n_read;
			read_sector();
			sector_offset = 0;
		}
		if (n && sector_size - sector_offset >= n) {
			memcpy(dst, &sector_data[sector_offset], n);
			file_position += n;
		}
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		return get_impl<T, little_endian>(*this);
	}

	size_t size() const {
		return be.size;
	}
};

template<typename base_reader_T, bool default_little_endian = true>
struct mpq_archive_reader {
	crypt_table_t crypt_table = get_crypt_table();
	base_reader_T& r;

	size_t sector_size;
	a_vector<hash_table_entry> hash_table;
	a_vector<block_table_entry> block_table;
	explicit mpq_archive_reader(base_reader_T& r) : r(r) {
		auto mpq_signature = r.template get<uint32_t>();
		if (mpq_signature != 0x1a51504d) error("signature mismatch; file is not an mpq archive");

		r.template get<uint32_t>();
		r.template get<uint32_t>();
		r.template get<uint16_t>(); // version

		auto block_size = r.template get<uint16_t>();

		sector_size = (size_t)512 * 1<<block_size;

		size_t hash_table_offset = (size_t)r.template get<uint32_t>();
		size_t block_table_offset = (size_t)r.template get<uint32_t>();

		size_t hash_table_size = (size_t)(r.template get<uint32_t>() * 16 / 16);
		size_t block_table_size = (size_t)(r.template get<uint32_t>() * 16 / 16);

		r.seek(hash_table_offset);

		auto hash_table_r = make_encrypted_reader(r, 16 * hash_table_size, string_hash("(hash table)", 3, crypt_table), crypt_table);

		for (size_t i = 0; i != hash_table_size; ++i) {
			auto hash1 = hash_table_r.template get<uint32_t>();
			auto hash2 = hash_table_r.template get<uint32_t>();

			hash_table_r.template get<uint16_t>(); // locale
			hash_table_r.template get<uint16_t>(); // platform
			auto block_index = hash_table_r.template get<uint32_t>();

			hash_table.push_back({hash1, hash2, (size_t)block_index});
		}

		auto block_table_r = make_encrypted_reader(r, 16 * block_table_size, string_hash("(block table)", 3, crypt_table), crypt_table);

		for (size_t i = 0; i != block_table_size; ++i) {
			r.seek(block_table_offset + i * 16);
			auto data_offset = block_table_r.template get<uint32_t>();
			auto compressed_size = block_table_r.template get<uint32_t>();
			auto size = block_table_r.template get<uint32_t>();
			auto flags = block_table_r.template get<uint32_t>();
			block_table.push_back({data_offset, compressed_size, size, flags});
		}
	}

	const hash_table_entry* find_hash_table_entry(const a_string& filename) const {
		auto hash0 = string_hash(filename.c_str(), 0, crypt_table);
		auto hash1 = string_hash(filename.c_str(), 1, crypt_table);
		auto hash2 = string_hash(filename.c_str(), 2, crypt_table);

		bool found = false;
		size_t initial_index = hash0 % hash_table.size();
		size_t index = initial_index;
		if (!hash_table.empty()) {
			do {
				auto& he = hash_table[index];
				if (he.block_index == 0xffffffff) break;
				if (he.hash1 == hash1 && he.hash2 == hash2) {
					found = true;
					break;
				}
				++index;
				if (index == hash_table.size()) index = 0;
			} while (index != initial_index);
		}
		return found ? &hash_table[index] : nullptr;
	}

	bool file_exists(const a_string& filename) {
		return find_hash_table_entry(filename) ? true : false;
	}

	auto open(a_string filename) {
		auto* he = find_hash_table_entry(filename);
		if (!he) error("mpq: %s: no such file", filename);

		const char* c = filename.data() + filename.size();
		while (c != filename.data()) {
			auto pc = *(c - 1);
			if (pc == '/' || pc == '\\') break;
			--c;
		}

		auto& be = block_table.at(he->block_index);

		uint32_t file_key = string_hash(c, 3, crypt_table);
		if (be.flags & 0x20000) {
			file_key = (file_key + be.data_offset) ^ be.size;
		}

		return mpq_archive_file_reader<base_reader_T, default_little_endian>(std::move(filename), r, sector_size, be, file_key, crypt_table);
	}

};

template<typename base_reader_T>
auto make_mpq_archive_reader(base_reader_T& reader) {
	return mpq_archive_reader<base_reader_T>(reader);
}

struct mpq_data {
	data_reader<> r;
	mpq_archive_reader<data_reader<>> mpq;
	explicit mpq_data(uint8_t* data, size_t data_size) : r(data, data + data_size), mpq(r) {}
	void operator()(a_vector<uint8_t>& dst, a_string filename) {
		auto file_r = mpq.open(std::move(filename));
		size_t len = file_r.size();
		dst.resize(len);
		file_r.get_bytes(dst.data(), len);
		return;
	}
};

template<typename file_reader_T = file_reader<>>
struct mpq_file {
	file_reader_T file;
	paged_reader<file_reader_T> paged;
	mpq_archive_reader<paged_reader<file_reader_T>> mpq;
	explicit mpq_file(a_string filename) : file(std::move(filename)), paged(file), mpq(paged) {}
	void operator()(a_vector<uint8_t>& dst, a_string filename) {
		auto file_r = mpq.open(std::move(filename));
		size_t len = file_r.size();
		dst.resize(len);
		file_r.get_bytes(dst.data(), len);
		return;
	}
};

template<typename mpq_file_T = mpq_file<>>
struct data_files_loader {
	a_list<mpq_file_T> mpqs;

	void add_mpq_file(a_string filename) {
		mpqs.emplace_back(std::move(filename));
	}

	void operator()(a_vector<uint8_t>& dst, a_string filename) {
		for (auto& v : mpqs) {
			if (v.mpq.file_exists(filename)) {
				v(dst, std::move(filename));
				return;
			}
		}
		error("data_files_loader: %s: file not found", filename);
	}
};

template<typename data_files_loader_T = data_files_loader<>>
data_files_loader_T data_files_directory(a_string path) {
	if (!path.empty() && path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') path += '/';
	data_files_loader_T r;
	r.add_mpq_file(path + "Patch_rt.mpq");
	r.add_mpq_file(path + "BrooDat.mpq");
	r.add_mpq_file(path + "StarDat.mpq");
	return r;
}

template<typename to_T, typename from_T>
struct data_type_cast_helper {
	to_T operator()(from_T v) {
		static_assert(std::is_integral<from_T>::value, "from_T must be integral");
		to_T r = (to_T)v;
		if ((from_T)(intmax_t)r != v) error("value 0x%x of type %s does not fit in type %s", v, typeid(from_T).name(), typeid(to_T).name());
		return r;
	}
};
template<typename to_T, typename from_T>
to_T data_type_cast(from_T v) {
	return data_type_cast_helper<to_T, from_T>()(v);
}
template<typename from_T>
struct data_type_cast_helper<bool, from_T> {
	bool operator()(from_T v) {
		static_assert(std::is_integral<from_T>::value, "from_T must be integral");
		if (v != 0 && v != 1) error("value 0x%x is not a boolean", v);
		return v != 0;
	}
};
template<>
struct data_type_cast_helper<fp8, int32_t> {
	fp8 operator()(int32_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<direction_t, int8_t> {
	direction_t operator()(int8_t v) {
		return direction_t::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, int8_t> {
	fp8 operator()(int8_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, uint8_t> {
	fp8 operator()(uint8_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, int16_t> {
	fp8 operator()(int16_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp1, uint8_t> {
	fp1 operator()(uint8_t v) {
		return fp1::from_raw(v);
	}
};
template<typename T, typename from_T>
struct data_type_cast_helper<type_id<T>, from_T> {
	type_id<T> operator()(from_T v) {
		return type_id<T>(data_type_cast<decltype(std::declval<T>().id), from_T>(v));
	}
};

template<typename load_T, typename field_T>
struct read_data {
	static const size_t read_n = 1;
	template<typename reader_T>
	static field_T read(reader_T& r) {
		return data_type_cast<field_T>(r.template get<load_T>());
	}
};
template<typename load_T>
struct read_data<load_T, xy> {
	static const size_t read_n = 2;
	template<typename reader_T>
	static xy read(reader_T& r) {
		int x = data_type_cast<int>(r.template get<load_T>());
		int y = data_type_cast<int>(r.template get<load_T>());
		return xy(x, y);
	}
};
template<typename load_T>
struct read_data<load_T, rect> {
	static const size_t read_n = 4;
	template<typename reader_T>
	static rect read(reader_T& r) {
		int x0 = data_type_cast<int>(r.template get<load_T>());
		int y0 = data_type_cast<int>(r.template get<load_T>());
		int x1 = data_type_cast<int>(r.template get<load_T>());
		int y1 = data_type_cast<int>(r.template get<load_T>());
		return { {x0, y0}, {x1, y1} };
	}
};

template<typename load_T, typename reader_T, typename ptr_F>
void read_array(reader_T& r, size_t num, ptr_F&& ptr_f) {
	using read_data_t = read_data<load_T, typename std::remove_reference<decltype(*ptr_f(0))>::type>;
	size_t total_size = sizeof(load_T) * read_data_t::read_n * num;
	const uint8_t* ptr = r.get_n(total_size);
	for (size_t i = 0; i < num; ++i) {
		data_reader<true, false> r2(ptr, nullptr);
		auto* field = ptr_f(i);
		*field = read_data_t::read(r2);
		ptr += sizeof(load_T) * read_data_t::read_n;
	}
}

#define rawr(load_type, name, num) read_array<load_type>(r, num, [&](size_t index) {return &arr[index].name;})
#define rawro(load_type, name, num, offset) read_array<load_type>(r, num, [&](size_t index) {return &arr[offset + index].name;})

template<typename data_T>
unit_types_t load_units_dat(const data_T& data) {
	static const size_t total_count = 228;
	static const size_t units_count = 106;
	static const size_t buildings_count = 96;

	unit_types_t unit_types;
	unit_types.vec.resize(total_count);
	for (size_t i = 0; i < total_count; ++i) {
		auto& v = unit_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (UnitTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = unit_types.vec;

	rawr(uint8_t, flingy, total_count);
	rawr(uint16_t, turret_unit_type, total_count);
	rawr(uint16_t, subunit2, total_count);
	rawro(uint16_t, infestation_unit, buildings_count, units_count);
	rawr(uint32_t, construction_animation, total_count);
	rawr(uint8_t, unit_direction, total_count);
	rawr(uint8_t, has_shield, total_count);
	rawr(uint16_t, shield_points, total_count);
	rawr(int32_t, hitpoints, total_count);
	rawr(uint8_t, elevation_level, total_count);
	rawr(uint8_t, unknown1, total_count);
	rawr(uint8_t, sublabel, total_count);
	rawr(uint8_t, computer_ai_idle, total_count);
	rawr(uint8_t, human_ai_idle, total_count);
	rawr(uint8_t, return_to_idle, total_count);
	rawr(uint8_t, attack_unit, total_count);
	rawr(uint8_t, attack_move, total_count);
	rawr(uint8_t, ground_weapon, total_count);
	rawr(uint8_t, max_ground_hits, total_count);
	rawr(uint8_t, air_weapon, total_count);
	rawr(uint8_t, max_air_hits, total_count);
	rawr(uint8_t, ai_internal, total_count);
	rawr(uint32_t, flags, total_count);
	rawr(uint8_t, target_acquisition_range, total_count);
	rawr(uint8_t, sight_range, total_count);
	rawr(uint8_t, armor_upgrade, total_count);
	rawr(uint8_t, unit_size, total_count);
	rawr(uint8_t, armor, total_count);
	rawr(uint8_t, right_click_action, total_count);
	rawr(uint16_t, ready_sound, units_count);
	rawr(uint16_t, first_what_sound, total_count);
	rawr(uint16_t, last_what_sound, total_count);
	rawr(uint16_t, first_pissed_sound, units_count);
	rawr(uint16_t, last_pissed_sound, units_count);
	rawr(uint16_t, first_yes_sound, units_count);
	rawr(uint16_t, last_yes_sound, units_count);
	rawr(int16_t, placement_size, total_count);
	rawro(int16_t, addon_position, buildings_count, units_count);
	rawr(int16_t, dimensions, total_count);
	rawr(uint16_t, portrait, total_count);
	rawr(uint16_t, mineral_cost, total_count);
	rawr(uint16_t, gas_cost, total_count);
	rawr(uint16_t, build_time, total_count);
	rawr(uint16_t, unknown2, total_count);
	rawr(uint8_t, group_flags, total_count);
	rawr(uint8_t, supply_provided, total_count);
	rawr(uint8_t, supply_required, total_count);
	rawr(uint8_t, space_required, total_count);
	rawr(uint8_t, space_provided, total_count);
	rawr(uint16_t, build_score, total_count);
	rawr(uint16_t, destroy_score, total_count);
	rawr(uint16_t, unit_map_string_index, total_count);
	rawr(uint8_t, is_broodwar, total_count);
	rawr(uint16_t, staredit_availability_flags, total_count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "units.dat", r.left());

	return unit_types;
}

template<typename data_T>
weapon_types_t load_weapons_dat(const data_T& data) {
	static const size_t count = 130;

	weapon_types_t weapon_types;
	weapon_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = weapon_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (WeaponTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = weapon_types.vec;

	rawr(uint16_t, label, count);
	rawr(uint32_t, flingy, count);
	rawr(uint8_t, unused, count);
	rawr(uint16_t, target_flags, count);
	rawr(uint32_t, min_range, count);
	rawr(uint32_t, max_range, count);
	rawr(uint8_t, damage_upgrade, count);
	rawr(uint8_t, damage_type, count);
	rawr(uint8_t, bullet_type, count);
	rawr(uint8_t, lifetime, count);
	rawr(uint8_t, hit_type, count);
	rawr(uint16_t, inner_splash_radius, count);
	rawr(uint16_t, medium_splash_radius, count);
	rawr(uint16_t, outer_splash_radius, count);
	rawr(uint16_t, damage_amount, count);
	rawr(uint16_t, damage_bonus, count);
	rawr(uint8_t, cooldown, count);
	rawr(uint8_t, bullet_count, count);
	rawr(uint8_t, attack_angle, count);
	rawr(int8_t, bullet_heading_offset, count);
	rawr(int8_t, forward_offset, count);
	rawr(int8_t, upward_offset, count);
	rawr(uint16_t, target_error_message, count);
	rawr(uint16_t, icon, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "weapons.dat", r.left());

	return weapon_types;
}

template<typename data_T>
upgrade_types_t load_upgrades_dat(const data_T& data) {
	static const size_t count = 61;

	upgrade_types_t upgrade_types;
	upgrade_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = upgrade_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (UpgradeTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = upgrade_types.vec;

	rawr(uint16_t, mineral_cost_base, count);
	rawr(uint16_t, mineral_cost_factor, count);
	rawr(uint16_t, gas_cost_base, count);
	rawr(uint16_t, gas_cost_factor, count);
	rawr(uint16_t, time_cost_base, count);
	rawr(uint16_t, time_cost_factor, count);
	rawr(uint16_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint8_t, max_level, count);
	rawr(uint8_t, is_broodwar, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "upgrades.dat", r.left());

	return upgrade_types;
}

template<typename data_T>
tech_types_t load_techdata_dat(const data_T& data) {
	static const size_t count = 44;

	tech_types_t tech_types;
	tech_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = tech_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (TechTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = tech_types.vec;

	rawr(uint16_t, mineral_cost, count);
	rawr(uint16_t, gas_cost, count);
	rawr(uint16_t, research_time, count);
	rawr(uint16_t, energy_cost, count);
	rawr(uint32_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint16_t, flags, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "techdata.dat", r.left());

	return tech_types;
}

template<typename data_T>
flingy_types_t load_flingy_dat(const data_T& data) {
	static const size_t count = 209;

	flingy_types_t flingy_types;
	flingy_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = flingy_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (FlingyTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = flingy_types.vec;

	rawr(uint16_t, sprite, count);
	rawr(int32_t, top_speed, count);
	rawr(int16_t, acceleration, count);
	rawr(int32_t, halt_distance, count);
	rawr(uint8_t, turn_rate, count);
	rawr(uint8_t, unused, count);
	rawr(uint8_t, movement_type, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "flingy.dat", r.left());

	return flingy_types;
}

template<typename data_T>
sprite_types_t load_sprites_dat(const data_T& data) {
	static const size_t count = 517;
	static const size_t selectable_count = 387;
	static const size_t non_selectable_count = 130;

	sprite_types_t sprite_types;
	sprite_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = sprite_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (SpriteTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = sprite_types.vec;

	rawr(uint16_t, image, count);
	rawro(uint8_t, health_bar_size, selectable_count, non_selectable_count);
	rawr(uint8_t, unk0, count);
	rawr(uint8_t, visible, count);
	rawro(uint8_t, selection_circle, selectable_count, non_selectable_count);
	rawro(uint8_t, selection_circle_vpos, selectable_count, non_selectable_count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "sprites.dat", r.left());

	return sprite_types;
}

template<typename data_T>
image_types_t load_images_dat(const data_T& data) {
	static const size_t count = 999;

	image_types_t image_types;
	image_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = image_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (ImageTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = image_types.vec;

	rawr(uint32_t, grp_filename_index, count);
	rawr(uint8_t, has_directional_frames, count);
	rawr(uint8_t, is_clickable, count);
	rawr(uint8_t, has_iscript_animations, count);
	rawr(uint8_t, always_visible, count);
	rawr(uint8_t, modifier, count);
	rawr(uint8_t, color_shift, count);
	rawr(uint32_t, iscript_id, count);
	rawr(uint32_t, shield_filename_index, count);
	rawr(uint32_t, attack_filename_index, count);
	rawr(uint32_t, damage_filename_index, count);
	rawr(uint32_t, special_filename_index, count);
	rawr(uint32_t, landing_dust_filename_index, count);
	rawr(uint32_t, lift_off_filename_index, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "images.dat", r.left());

	return image_types;
}

template<typename data_T>
order_types_t load_orders_dat(const data_T& data) {
	static const size_t count = 189;

	order_types_t order_types;
	order_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = order_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (Orders)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = order_types.vec;

	rawr(uint16_t, label, count);
	rawr(uint8_t, targets_enemies, count);
	rawr(uint8_t, background, count);
	rawr(uint8_t, unused3, count);
	rawr(uint8_t, valid_for_turret, count);
	rawr(uint8_t, unused5, count);
	rawr(uint8_t, can_be_interrupted, count);
	rawr(uint8_t, unk7, count);
	rawr(uint8_t, can_be_queued, count);
	rawr(uint8_t, unk9, count);
	rawr(uint8_t, can_be_obstructed, count);
	rawr(uint8_t, unk11, count);
	rawr(uint8_t, unused12, count);
	rawr(uint8_t, weapon, count);
	rawr(uint8_t, tech_type, count);
	rawr(uint8_t, animation, count);
	rawr(int16_t, highlight, count);
	rawr(uint16_t, dep_index, count);
	rawr(uint8_t, target_order, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "orders.dat", r.left());

	return order_types;
}

template<typename data_T>
sound_types_t load_sfxdata_dat(const data_T& data) {
	static const size_t count = 1144;

	sound_types_t sound_types;
	sound_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = sound_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (Sounds)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = sound_types.vec;

	rawr(uint32_t, filename_index, count);
	rawr(uint8_t, priority, count);
	rawr(uint8_t, flags, count);
	rawr(uint16_t, race, count);
	rawr(uint8_t, min_volume, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "sfxdata.dat", r.left());

	return sound_types;
}

#undef rawro
#undef rawr

}
}

#endif
