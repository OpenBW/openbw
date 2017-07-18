#ifndef BWGAME_REPLAY_SAVER_H
#define BWGAME_REPLAY_SAVER_H

#include "bwgame.h"
#include "replay.h"

namespace bwgame {

namespace data_loading {

template<bool default_little_endian = true, bool bounds_checking = true>
struct data_writer {
	uint8_t* ptr = nullptr;
	uint8_t* begin = nullptr;
	const uint8_t* end = nullptr;
	data_writer() = default;
	data_writer(uint8_t* ptr, const uint8_t* end) : ptr(ptr), begin(ptr), end(end) {}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "don't know how to write this type");
		size_t n = size();
		skip(sizeof(T));
		data_loading::set_value_at<little_endian>(data() + n, v);
	}
	void skip(size_t n) {
		if (bounds_checking && left() < n) error("data_writer: attempt to write past end");
		ptr += n;
	}
	void put_bytes(const uint8_t* src, size_t n) {
		size_t pos = size();
		skip(n);
		memcpy(data() + pos, src, n);
	}
	size_t size() const {
		return ptr - begin;
	}
	const uint8_t* data() const {
		return begin;
	}
	uint8_t* data() {
		return begin;
	}
	size_t left() const {
		return end - ptr;
	}
	size_t tell() const {
		return ptr - begin;
	}
	void seek(size_t pos) {
		if (pos > end - begin) error("data_writer: attempt to seek beyond end");
		ptr = begin + pos;
	}
};

template<typename dst_T, bool default_little_endian = true>
struct vector_writer {
	dst_T& dst;
	vector_writer(dst_T& dst) : dst(dst) {}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "don't know how to write this type");
		size_t n = dst.size();
		skip(sizeof(T));
		data_loading::set_value_at<little_endian>(data() + n, v);
	}
	void skip(size_t n) {
		if (left() < n) error("data_writer: attempt to write past end");
		dst.resize(dst.size() + n);
	}
	void put_bytes(const uint8_t* src, size_t n) {
		size_t pos = dst.size();
		skip(n);
		memcpy(data() + pos, src, n);
	}
	size_t size() const {
		return dst.capacity();
	}
	const uint8_t* data() const {
		return dst.data();
	}
	uint8_t* data() {
		return dst.data();
	}
	size_t left() const {
		return dst.capacity() - dst.size();
	}
	size_t tell() const {
		return dst.size();
	}
	void seek(size_t pos) {
		dst.resize(pos);
	}
};

template<bool default_little_endian = true, typename dst_T>
auto make_vector_writer(dst_T& dst) {
	return vector_writer<dst_T, default_little_endian>(dst);
}

template<typename buffers_T, bool default_little_endian = true>
struct buffers_writer {
	buffers_T& buffers;
	buffers_writer(buffers_T& buffers) : buffers(buffers) {}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void put_bytes(const uint8_t* src, size_t n) {
		if (buffers.empty()) buffers.emplace_back();
		auto& buf = buffers.back();
		size_t pos = buf.size();
		size_t left = buf.capacity() - pos;
		if (left >= n) {
			buf.resize(pos + n);
			memcpy(buf.data() + pos, src, n);
		} else {
			buf.resize(pos + left);
			memcpy(buf.data() + pos, src, left);
			buffers.emplace_back();
			put_bytes(src + left, n - left);
		}
	}
};

template<bool default_little_endian = true, typename buffers_T>
auto make_buffers_writer(buffers_T& buffers) {
	return buffers_writer<buffers_T, default_little_endian>(buffers);
}


template<typename base_writer_T, bool default_little_endian = true>
struct bit_writer {
	base_writer_T& w;
	size_t bits_n = 0;
	uint8_t data;
	explicit bit_writer(base_writer_T& w) : w(w) {}
	template<size_t bits, bool little_endian = default_little_endian, typename T>
	void put_bits(T v) {
		v &= (((T)1 << bits) - 1);
		size_t n = bits;
		if (bits_n) {
			w.seek(w.tell() - 1);
			data |= (uint8_t)v << (8 - bits_n);
			w.put(data);
			if (n <= bits_n) {
				bits_n -= n;
				n = 0;
				return;
			} else {
				v >>= bits_n;
				n -= bits_n;
				bits_n = 0;
			}
		}
		while (true) {
			if (n > 8) {
				w.template put<uint8_t>((uint8_t)v);
				n -= 8;
				v >>= std::conditional<sizeof(v) <= 1, std::integral_constant<int, 0>, std::integral_constant<int, 8>>::type::value;
			} else {
				data = (uint8_t)v;
				w.template put<uint8_t>((uint8_t)v);
				bits_n = 8 - n;
				return;
			}
		}
	}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		return put_bits<int_bits<T>::value, little_endian>(v);
	}
};

template<bool little_endian = true, typename base_writer_T>
auto make_bit_writer(base_writer_T& writer) {
	return bit_writer<base_writer_T, little_endian>(writer);
}

template<bool little_endian = true, typename writer_T>
void compress(const uint8_t* input, size_t input_size, writer_T& writer) {
	
	auto write_length = [&](auto& w, int v) {
		switch (v) {
		case 0: return w.template put_bits<3>(0x5); case 1: return w.template put_bits<2>(0x3); case 2: return w.template put_bits<3>(0x1); case 3: return w.template put_bits<3>(0x6);
		case 4: return w.template put_bits<4>(0xa); case 5: return w.template put_bits<4>(0x2); case 6: return w.template put_bits<4>(0xc); case 7: return w.template put_bits<5>(0x14);
		case 8: return w.template put_bits<6>(0x4); case 9: return w.template put_bits<6>(0x24); case 10: return w.template put_bits<7>(0x18); case 11: return w.template put_bits<7>(0x38);
		case 12: return w.template put_bits<7>(0x58); case 13: return w.template put_bits<7>(0x78); case 14: return w.template put_bits<8>(0x8); case 15: return w.template put_bits<8>(0x28);
		case 16: return w.template put_bits<8>(0x48); case 17: return w.template put_bits<8>(0x68); case 18: return w.template put_bits<8>(0x88); case 19: return w.template put_bits<8>(0xa8);
		case 20: return w.template put_bits<8>(0xc8); case 21: return w.template put_bits<8>(0xe8); case 22: return w.template put_bits<10>(0x30); case 23: return w.template put_bits<10>(0x70);
		case 24: return w.template put_bits<10>(0xb0); case 25: return w.template put_bits<10>(0xf0); case 26: return w.template put_bits<10>(0x130); case 27: return w.template put_bits<10>(0x170);
		case 28: return w.template put_bits<10>(0x1b0); case 29: return w.template put_bits<10>(0x1f0); case 30: return w.template put_bits<10>(0x230); case 31: return w.template put_bits<10>(0x270);
		case 32: return w.template put_bits<10>(0x2b0); case 33: return w.template put_bits<10>(0x2f0); case 34: return w.template put_bits<10>(0x330); case 35: return w.template put_bits<10>(0x370);
		case 36: return w.template put_bits<10>(0x3b0); case 37: return w.template put_bits<10>(0x3f0); case 38: return w.template put_bits<11>(0x10); case 39: return w.template put_bits<11>(0x50);
		case 40: return w.template put_bits<11>(0x90); case 41: return w.template put_bits<11>(0xd0); case 42: return w.template put_bits<11>(0x110); case 43: return w.template put_bits<11>(0x150);
		case 44: return w.template put_bits<11>(0x190); case 45: return w.template put_bits<11>(0x1d0); case 46: return w.template put_bits<11>(0x210); case 47: return w.template put_bits<11>(0x250);
		case 48: return w.template put_bits<11>(0x290); case 49: return w.template put_bits<11>(0x2d0); case 50: return w.template put_bits<11>(0x310); case 51: return w.template put_bits<11>(0x350);
		case 52: return w.template put_bits<11>(0x390); case 53: return w.template put_bits<11>(0x3d0); case 54: return w.template put_bits<11>(0x410); case 55: return w.template put_bits<11>(0x450);
		case 56: return w.template put_bits<11>(0x490); case 57: return w.template put_bits<11>(0x4d0); case 58: return w.template put_bits<11>(0x510); case 59: return w.template put_bits<11>(0x550);
		case 60: return w.template put_bits<11>(0x590); case 61: return w.template put_bits<11>(0x5d0); case 62: return w.template put_bits<11>(0x610); case 63: return w.template put_bits<11>(0x650);
		case 64: return w.template put_bits<11>(0x690); case 65: return w.template put_bits<11>(0x6d0); case 66: return w.template put_bits<11>(0x710); case 67: return w.template put_bits<11>(0x750);
		case 68: return w.template put_bits<11>(0x790); case 69: return w.template put_bits<11>(0x7d0); case 70: return w.template put_bits<12>(0x20); case 71: return w.template put_bits<12>(0x60);
		case 72: return w.template put_bits<12>(0xa0); case 73: return w.template put_bits<12>(0xe0); case 74: return w.template put_bits<12>(0x120); case 75: return w.template put_bits<12>(0x160);
		case 76: return w.template put_bits<12>(0x1a0); case 77: return w.template put_bits<12>(0x1e0); case 78: return w.template put_bits<12>(0x220); case 79: return w.template put_bits<12>(0x260);
		case 80: return w.template put_bits<12>(0x2a0); case 81: return w.template put_bits<12>(0x2e0); case 82: return w.template put_bits<12>(0x320); case 83: return w.template put_bits<12>(0x360);
		case 84: return w.template put_bits<12>(0x3a0); case 85: return w.template put_bits<12>(0x3e0); case 86: return w.template put_bits<12>(0x420); case 87: return w.template put_bits<12>(0x460);
		case 88: return w.template put_bits<12>(0x4a0); case 89: return w.template put_bits<12>(0x4e0); case 90: return w.template put_bits<12>(0x520); case 91: return w.template put_bits<12>(0x560);
		case 92: return w.template put_bits<12>(0x5a0); case 93: return w.template put_bits<12>(0x5e0); case 94: return w.template put_bits<12>(0x620); case 95: return w.template put_bits<12>(0x660);
		case 96: return w.template put_bits<12>(0x6a0); case 97: return w.template put_bits<12>(0x6e0); case 98: return w.template put_bits<12>(0x720); case 99: return w.template put_bits<12>(0x760);
		case 100: return w.template put_bits<12>(0x7a0); case 101: return w.template put_bits<12>(0x7e0); case 102: return w.template put_bits<12>(0x820); case 103: return w.template put_bits<12>(0x860);
		case 104: return w.template put_bits<12>(0x8a0); case 105: return w.template put_bits<12>(0x8e0); case 106: return w.template put_bits<12>(0x920); case 107: return w.template put_bits<12>(0x960);
		case 108: return w.template put_bits<12>(0x9a0); case 109: return w.template put_bits<12>(0x9e0); case 110: return w.template put_bits<12>(0xa20); case 111: return w.template put_bits<12>(0xa60);
		case 112: return w.template put_bits<12>(0xaa0); case 113: return w.template put_bits<12>(0xae0); case 114: return w.template put_bits<12>(0xb20); case 115: return w.template put_bits<12>(0xb60);
		case 116: return w.template put_bits<12>(0xba0); case 117: return w.template put_bits<12>(0xbe0); case 118: return w.template put_bits<12>(0xc20); case 119: return w.template put_bits<12>(0xc60);
		case 120: return w.template put_bits<12>(0xca0); case 121: return w.template put_bits<12>(0xce0); case 122: return w.template put_bits<12>(0xd20); case 123: return w.template put_bits<12>(0xd60);
		case 124: return w.template put_bits<12>(0xda0); case 125: return w.template put_bits<12>(0xde0); case 126: return w.template put_bits<12>(0xe20); case 127: return w.template put_bits<12>(0xe60);
		case 128: return w.template put_bits<12>(0xea0); case 129: return w.template put_bits<12>(0xee0); case 130: return w.template put_bits<12>(0xf20); case 131: return w.template put_bits<12>(0xf60);
		case 132: return w.template put_bits<12>(0xfa0); case 133: return w.template put_bits<12>(0xfe0); case 134: return w.template put_bits<14>(0x40); case 135: return w.template put_bits<14>(0xc0);
		case 136: return w.template put_bits<14>(0x140); case 137: return w.template put_bits<14>(0x1c0); case 138: return w.template put_bits<14>(0x240); case 139: return w.template put_bits<14>(0x2c0);
		case 140: return w.template put_bits<14>(0x340); case 141: return w.template put_bits<14>(0x3c0); case 142: return w.template put_bits<14>(0x440); case 143: return w.template put_bits<14>(0x4c0);
		case 144: return w.template put_bits<14>(0x540); case 145: return w.template put_bits<14>(0x5c0); case 146: return w.template put_bits<14>(0x640); case 147: return w.template put_bits<14>(0x6c0);
		case 148: return w.template put_bits<14>(0x740); case 149: return w.template put_bits<14>(0x7c0); case 150: return w.template put_bits<14>(0x840); case 151: return w.template put_bits<14>(0x8c0);
		case 152: return w.template put_bits<14>(0x940); case 153: return w.template put_bits<14>(0x9c0); case 154: return w.template put_bits<14>(0xa40); case 155: return w.template put_bits<14>(0xac0);
		case 156: return w.template put_bits<14>(0xb40); case 157: return w.template put_bits<14>(0xbc0); case 158: return w.template put_bits<14>(0xc40); case 159: return w.template put_bits<14>(0xcc0);
		case 160: return w.template put_bits<14>(0xd40); case 161: return w.template put_bits<14>(0xdc0); case 162: return w.template put_bits<14>(0xe40); case 163: return w.template put_bits<14>(0xec0);
		case 164: return w.template put_bits<14>(0xf40); case 165: return w.template put_bits<14>(0xfc0); case 166: return w.template put_bits<14>(0x1040); case 167: return w.template put_bits<14>(0x10c0);
		case 168: return w.template put_bits<14>(0x1140); case 169: return w.template put_bits<14>(0x11c0); case 170: return w.template put_bits<14>(0x1240); case 171: return w.template put_bits<14>(0x12c0);
		case 172: return w.template put_bits<14>(0x1340); case 173: return w.template put_bits<14>(0x13c0); case 174: return w.template put_bits<14>(0x1440); case 175: return w.template put_bits<14>(0x14c0);
		case 176: return w.template put_bits<14>(0x1540); case 177: return w.template put_bits<14>(0x15c0); case 178: return w.template put_bits<14>(0x1640); case 179: return w.template put_bits<14>(0x16c0);
		case 180: return w.template put_bits<14>(0x1740); case 181: return w.template put_bits<14>(0x17c0); case 182: return w.template put_bits<14>(0x1840); case 183: return w.template put_bits<14>(0x18c0);
		case 184: return w.template put_bits<14>(0x1940); case 185: return w.template put_bits<14>(0x19c0); case 186: return w.template put_bits<14>(0x1a40); case 187: return w.template put_bits<14>(0x1ac0);
		case 188: return w.template put_bits<14>(0x1b40); case 189: return w.template put_bits<14>(0x1bc0); case 190: return w.template put_bits<14>(0x1c40); case 191: return w.template put_bits<14>(0x1cc0);
		case 192: return w.template put_bits<14>(0x1d40); case 193: return w.template put_bits<14>(0x1dc0); case 194: return w.template put_bits<14>(0x1e40); case 195: return w.template put_bits<14>(0x1ec0);
		case 196: return w.template put_bits<14>(0x1f40); case 197: return w.template put_bits<14>(0x1fc0); case 198: return w.template put_bits<14>(0x2040); case 199: return w.template put_bits<14>(0x20c0);
		case 200: return w.template put_bits<14>(0x2140); case 201: return w.template put_bits<14>(0x21c0); case 202: return w.template put_bits<14>(0x2240); case 203: return w.template put_bits<14>(0x22c0);
		case 204: return w.template put_bits<14>(0x2340); case 205: return w.template put_bits<14>(0x23c0); case 206: return w.template put_bits<14>(0x2440); case 207: return w.template put_bits<14>(0x24c0);
		case 208: return w.template put_bits<14>(0x2540); case 209: return w.template put_bits<14>(0x25c0); case 210: return w.template put_bits<14>(0x2640); case 211: return w.template put_bits<14>(0x26c0);
		case 212: return w.template put_bits<14>(0x2740); case 213: return w.template put_bits<14>(0x27c0); case 214: return w.template put_bits<14>(0x2840); case 215: return w.template put_bits<14>(0x28c0);
		case 216: return w.template put_bits<14>(0x2940); case 217: return w.template put_bits<14>(0x29c0); case 218: return w.template put_bits<14>(0x2a40); case 219: return w.template put_bits<14>(0x2ac0);
		case 220: return w.template put_bits<14>(0x2b40); case 221: return w.template put_bits<14>(0x2bc0); case 222: return w.template put_bits<14>(0x2c40); case 223: return w.template put_bits<14>(0x2cc0);
		case 224: return w.template put_bits<14>(0x2d40); case 225: return w.template put_bits<14>(0x2dc0); case 226: return w.template put_bits<14>(0x2e40); case 227: return w.template put_bits<14>(0x2ec0);
		case 228: return w.template put_bits<14>(0x2f40); case 229: return w.template put_bits<14>(0x2fc0); case 230: return w.template put_bits<14>(0x3040); case 231: return w.template put_bits<14>(0x30c0);
		case 232: return w.template put_bits<14>(0x3140); case 233: return w.template put_bits<14>(0x31c0); case 234: return w.template put_bits<14>(0x3240); case 235: return w.template put_bits<14>(0x32c0);
		case 236: return w.template put_bits<14>(0x3340); case 237: return w.template put_bits<14>(0x33c0); case 238: return w.template put_bits<14>(0x3440); case 239: return w.template put_bits<14>(0x34c0);
		case 240: return w.template put_bits<14>(0x3540); case 241: return w.template put_bits<14>(0x35c0); case 242: return w.template put_bits<14>(0x3640); case 243: return w.template put_bits<14>(0x36c0);
		case 244: return w.template put_bits<14>(0x3740); case 245: return w.template put_bits<14>(0x37c0); case 246: return w.template put_bits<14>(0x3840); case 247: return w.template put_bits<14>(0x38c0);
		case 248: return w.template put_bits<14>(0x3940); case 249: return w.template put_bits<14>(0x39c0); case 250: return w.template put_bits<14>(0x3a40); case 251: return w.template put_bits<14>(0x3ac0);
		case 252: return w.template put_bits<14>(0x3b40); case 253: return w.template put_bits<14>(0x3bc0); case 254: return w.template put_bits<14>(0x3c40); case 255: return w.template put_bits<14>(0x3cc0);
		case 256: return w.template put_bits<14>(0x3d40); case 257: return w.template put_bits<14>(0x3dc0); case 258: return w.template put_bits<14>(0x3e40); case 259: return w.template put_bits<14>(0x3ec0);
		case 260: return w.template put_bits<14>(0x3f40); case 261: return w.template put_bits<14>(0x3fc0); case 262: return w.template put_bits<15>(0); case 263: return w.template put_bits<15>(0x80);
		case 264: return w.template put_bits<15>(0x100); case 265: return w.template put_bits<15>(0x180); case 266: return w.template put_bits<15>(0x200); case 267: return w.template put_bits<15>(0x280);
		case 268: return w.template put_bits<15>(0x300); case 269: return w.template put_bits<15>(0x380); case 270: return w.template put_bits<15>(0x400); case 271: return w.template put_bits<15>(0x480);
		case 272: return w.template put_bits<15>(0x500); case 273: return w.template put_bits<15>(0x580); case 274: return w.template put_bits<15>(0x600); case 275: return w.template put_bits<15>(0x680);
		case 276: return w.template put_bits<15>(0x700); case 277: return w.template put_bits<15>(0x780); case 278: return w.template put_bits<15>(0x800); case 279: return w.template put_bits<15>(0x880);
		case 280: return w.template put_bits<15>(0x900); case 281: return w.template put_bits<15>(0x980); case 282: return w.template put_bits<15>(0xa00); case 283: return w.template put_bits<15>(0xa80);
		case 284: return w.template put_bits<15>(0xb00); case 285: return w.template put_bits<15>(0xb80); case 286: return w.template put_bits<15>(0xc00); case 287: return w.template put_bits<15>(0xc80);
		case 288: return w.template put_bits<15>(0xd00); case 289: return w.template put_bits<15>(0xd80); case 290: return w.template put_bits<15>(0xe00); case 291: return w.template put_bits<15>(0xe80);
		case 292: return w.template put_bits<15>(0xf00); case 293: return w.template put_bits<15>(0xf80); case 294: return w.template put_bits<15>(0x1000); case 295: return w.template put_bits<15>(0x1080);
		case 296: return w.template put_bits<15>(0x1100); case 297: return w.template put_bits<15>(0x1180); case 298: return w.template put_bits<15>(0x1200); case 299: return w.template put_bits<15>(0x1280);
		case 300: return w.template put_bits<15>(0x1300); case 301: return w.template put_bits<15>(0x1380); case 302: return w.template put_bits<15>(0x1400); case 303: return w.template put_bits<15>(0x1480);
		case 304: return w.template put_bits<15>(0x1500); case 305: return w.template put_bits<15>(0x1580); case 306: return w.template put_bits<15>(0x1600); case 307: return w.template put_bits<15>(0x1680);
		case 308: return w.template put_bits<15>(0x1700); case 309: return w.template put_bits<15>(0x1780); case 310: return w.template put_bits<15>(0x1800); case 311: return w.template put_bits<15>(0x1880);
		case 312: return w.template put_bits<15>(0x1900); case 313: return w.template put_bits<15>(0x1980); case 314: return w.template put_bits<15>(0x1a00); case 315: return w.template put_bits<15>(0x1a80);
		case 316: return w.template put_bits<15>(0x1b00); case 317: return w.template put_bits<15>(0x1b80); case 318: return w.template put_bits<15>(0x1c00); case 319: return w.template put_bits<15>(0x1c80);
		case 320: return w.template put_bits<15>(0x1d00); case 321: return w.template put_bits<15>(0x1d80); case 322: return w.template put_bits<15>(0x1e00); case 323: return w.template put_bits<15>(0x1e80);
		case 324: return w.template put_bits<15>(0x1f00); case 325: return w.template put_bits<15>(0x1f80); case 326: return w.template put_bits<15>(0x2000); case 327: return w.template put_bits<15>(0x2080);
		case 328: return w.template put_bits<15>(0x2100); case 329: return w.template put_bits<15>(0x2180); case 330: return w.template put_bits<15>(0x2200); case 331: return w.template put_bits<15>(0x2280);
		case 332: return w.template put_bits<15>(0x2300); case 333: return w.template put_bits<15>(0x2380); case 334: return w.template put_bits<15>(0x2400); case 335: return w.template put_bits<15>(0x2480);
		case 336: return w.template put_bits<15>(0x2500); case 337: return w.template put_bits<15>(0x2580); case 338: return w.template put_bits<15>(0x2600); case 339: return w.template put_bits<15>(0x2680);
		case 340: return w.template put_bits<15>(0x2700); case 341: return w.template put_bits<15>(0x2780); case 342: return w.template put_bits<15>(0x2800); case 343: return w.template put_bits<15>(0x2880);
		case 344: return w.template put_bits<15>(0x2900); case 345: return w.template put_bits<15>(0x2980); case 346: return w.template put_bits<15>(0x2a00); case 347: return w.template put_bits<15>(0x2a80);
		case 348: return w.template put_bits<15>(0x2b00); case 349: return w.template put_bits<15>(0x2b80); case 350: return w.template put_bits<15>(0x2c00); case 351: return w.template put_bits<15>(0x2c80);
		case 352: return w.template put_bits<15>(0x2d00); case 353: return w.template put_bits<15>(0x2d80); case 354: return w.template put_bits<15>(0x2e00); case 355: return w.template put_bits<15>(0x2e80);
		case 356: return w.template put_bits<15>(0x2f00); case 357: return w.template put_bits<15>(0x2f80); case 358: return w.template put_bits<15>(0x3000); case 359: return w.template put_bits<15>(0x3080);
		case 360: return w.template put_bits<15>(0x3100); case 361: return w.template put_bits<15>(0x3180); case 362: return w.template put_bits<15>(0x3200); case 363: return w.template put_bits<15>(0x3280);
		case 364: return w.template put_bits<15>(0x3300); case 365: return w.template put_bits<15>(0x3380); case 366: return w.template put_bits<15>(0x3400); case 367: return w.template put_bits<15>(0x3480);
		case 368: return w.template put_bits<15>(0x3500); case 369: return w.template put_bits<15>(0x3580); case 370: return w.template put_bits<15>(0x3600); case 371: return w.template put_bits<15>(0x3680);
		case 372: return w.template put_bits<15>(0x3700); case 373: return w.template put_bits<15>(0x3780); case 374: return w.template put_bits<15>(0x3800); case 375: return w.template put_bits<15>(0x3880);
		case 376: return w.template put_bits<15>(0x3900); case 377: return w.template put_bits<15>(0x3980); case 378: return w.template put_bits<15>(0x3a00); case 379: return w.template put_bits<15>(0x3a80);
		case 380: return w.template put_bits<15>(0x3b00); case 381: return w.template put_bits<15>(0x3b80); case 382: return w.template put_bits<15>(0x3c00); case 383: return w.template put_bits<15>(0x3c80);
		case 384: return w.template put_bits<15>(0x3d00); case 385: return w.template put_bits<15>(0x3d80); case 386: return w.template put_bits<15>(0x3e00); case 387: return w.template put_bits<15>(0x3e80);
		case 388: return w.template put_bits<15>(0x3f00); case 389: return w.template put_bits<15>(0x3f80); case 390: return w.template put_bits<15>(0x4000); case 391: return w.template put_bits<15>(0x4080);
		case 392: return w.template put_bits<15>(0x4100); case 393: return w.template put_bits<15>(0x4180); case 394: return w.template put_bits<15>(0x4200); case 395: return w.template put_bits<15>(0x4280);
		case 396: return w.template put_bits<15>(0x4300); case 397: return w.template put_bits<15>(0x4380); case 398: return w.template put_bits<15>(0x4400); case 399: return w.template put_bits<15>(0x4480);
		case 400: return w.template put_bits<15>(0x4500); case 401: return w.template put_bits<15>(0x4580); case 402: return w.template put_bits<15>(0x4600); case 403: return w.template put_bits<15>(0x4680);
		case 404: return w.template put_bits<15>(0x4700); case 405: return w.template put_bits<15>(0x4780); case 406: return w.template put_bits<15>(0x4800); case 407: return w.template put_bits<15>(0x4880);
		case 408: return w.template put_bits<15>(0x4900); case 409: return w.template put_bits<15>(0x4980); case 410: return w.template put_bits<15>(0x4a00); case 411: return w.template put_bits<15>(0x4a80);
		case 412: return w.template put_bits<15>(0x4b00); case 413: return w.template put_bits<15>(0x4b80); case 414: return w.template put_bits<15>(0x4c00); case 415: return w.template put_bits<15>(0x4c80);
		case 416: return w.template put_bits<15>(0x4d00); case 417: return w.template put_bits<15>(0x4d80); case 418: return w.template put_bits<15>(0x4e00); case 419: return w.template put_bits<15>(0x4e80);
		case 420: return w.template put_bits<15>(0x4f00); case 421: return w.template put_bits<15>(0x4f80); case 422: return w.template put_bits<15>(0x5000); case 423: return w.template put_bits<15>(0x5080);
		case 424: return w.template put_bits<15>(0x5100); case 425: return w.template put_bits<15>(0x5180); case 426: return w.template put_bits<15>(0x5200); case 427: return w.template put_bits<15>(0x5280);
		case 428: return w.template put_bits<15>(0x5300); case 429: return w.template put_bits<15>(0x5380); case 430: return w.template put_bits<15>(0x5400); case 431: return w.template put_bits<15>(0x5480);
		case 432: return w.template put_bits<15>(0x5500); case 433: return w.template put_bits<15>(0x5580); case 434: return w.template put_bits<15>(0x5600); case 435: return w.template put_bits<15>(0x5680);
		case 436: return w.template put_bits<15>(0x5700); case 437: return w.template put_bits<15>(0x5780); case 438: return w.template put_bits<15>(0x5800); case 439: return w.template put_bits<15>(0x5880);
		case 440: return w.template put_bits<15>(0x5900); case 441: return w.template put_bits<15>(0x5980); case 442: return w.template put_bits<15>(0x5a00); case 443: return w.template put_bits<15>(0x5a80);
		case 444: return w.template put_bits<15>(0x5b00); case 445: return w.template put_bits<15>(0x5b80); case 446: return w.template put_bits<15>(0x5c00); case 447: return w.template put_bits<15>(0x5c80);
		case 448: return w.template put_bits<15>(0x5d00); case 449: return w.template put_bits<15>(0x5d80); case 450: return w.template put_bits<15>(0x5e00); case 451: return w.template put_bits<15>(0x5e80);
		case 452: return w.template put_bits<15>(0x5f00); case 453: return w.template put_bits<15>(0x5f80); case 454: return w.template put_bits<15>(0x6000); case 455: return w.template put_bits<15>(0x6080);
		case 456: return w.template put_bits<15>(0x6100); case 457: return w.template put_bits<15>(0x6180); case 458: return w.template put_bits<15>(0x6200); case 459: return w.template put_bits<15>(0x6280);
		case 460: return w.template put_bits<15>(0x6300); case 461: return w.template put_bits<15>(0x6380); case 462: return w.template put_bits<15>(0x6400); case 463: return w.template put_bits<15>(0x6480);
		case 464: return w.template put_bits<15>(0x6500); case 465: return w.template put_bits<15>(0x6580); case 466: return w.template put_bits<15>(0x6600); case 467: return w.template put_bits<15>(0x6680);
		case 468: return w.template put_bits<15>(0x6700); case 469: return w.template put_bits<15>(0x6780); case 470: return w.template put_bits<15>(0x6800); case 471: return w.template put_bits<15>(0x6880);
		case 472: return w.template put_bits<15>(0x6900); case 473: return w.template put_bits<15>(0x6980); case 474: return w.template put_bits<15>(0x6a00); case 475: return w.template put_bits<15>(0x6a80);
		case 476: return w.template put_bits<15>(0x6b00); case 477: return w.template put_bits<15>(0x6b80); case 478: return w.template put_bits<15>(0x6c00); case 479: return w.template put_bits<15>(0x6c80);
		case 480: return w.template put_bits<15>(0x6d00); case 481: return w.template put_bits<15>(0x6d80); case 482: return w.template put_bits<15>(0x6e00); case 483: return w.template put_bits<15>(0x6e80);
		case 484: return w.template put_bits<15>(0x6f00); case 485: return w.template put_bits<15>(0x6f80); case 486: return w.template put_bits<15>(0x7000); case 487: return w.template put_bits<15>(0x7080);
		case 488: return w.template put_bits<15>(0x7100); case 489: return w.template put_bits<15>(0x7180); case 490: return w.template put_bits<15>(0x7200); case 491: return w.template put_bits<15>(0x7280);
		case 492: return w.template put_bits<15>(0x7300); case 493: return w.template put_bits<15>(0x7380); case 494: return w.template put_bits<15>(0x7400); case 495: return w.template put_bits<15>(0x7480);
		case 496: return w.template put_bits<15>(0x7500); case 497: return w.template put_bits<15>(0x7580); case 498: return w.template put_bits<15>(0x7600); case 499: return w.template put_bits<15>(0x7680);
		case 500: return w.template put_bits<15>(0x7700); case 501: return w.template put_bits<15>(0x7780); case 502: return w.template put_bits<15>(0x7800); case 503: return w.template put_bits<15>(0x7880);
		case 504: return w.template put_bits<15>(0x7900); case 505: return w.template put_bits<15>(0x7980); case 506: return w.template put_bits<15>(0x7a00); case 507: return w.template put_bits<15>(0x7a80);
		case 508: return w.template put_bits<15>(0x7b00); case 509: return w.template put_bits<15>(0x7b80); case 510: return w.template put_bits<15>(0x7c00); case 511: return w.template put_bits<15>(0x7c80);
		case 512: return w.template put_bits<15>(0x7d00); case 513: return w.template put_bits<15>(0x7d80); case 514: return w.template put_bits<15>(0x7e00); case 515: return w.template put_bits<15>(0x7e80);
		case 516: return w.template put_bits<15>(0x7f00); case 517: return w.template put_bits<15>(0x7f80);
		}
	};
	auto write_distance = [&](auto& w, int v) {
		switch (v) {
		case 0: return w.template put_bits<2>(0x3); case 1: return w.template put_bits<4>(0xd); case 2: return w.template put_bits<4>(0x5); case 3: return w.template put_bits<5>(0x19);
		case 4: return w.template put_bits<5>(0x9); case 5: return w.template put_bits<5>(0x11); case 6: return w.template put_bits<5>(0x1); case 7: return w.template put_bits<6>(0x3e);
		case 8: return w.template put_bits<6>(0x1e); case 9: return w.template put_bits<6>(0x2e); case 10: return w.template put_bits<6>(0xe); case 11: return w.template put_bits<6>(0x36);
		case 12: return w.template put_bits<6>(0x16); case 13: return w.template put_bits<6>(0x26); case 14: return w.template put_bits<6>(0x6); case 15: return w.template put_bits<6>(0x3a);
		case 16: return w.template put_bits<6>(0x1a); case 17: return w.template put_bits<6>(0x2a); case 18: return w.template put_bits<6>(0xa); case 19: return w.template put_bits<6>(0x32);
		case 20: return w.template put_bits<6>(0x12); case 21: return w.template put_bits<6>(0x22); case 22: return w.template put_bits<7>(0x42); case 23: return w.template put_bits<7>(0x2);
		case 24: return w.template put_bits<7>(0x7c); case 25: return w.template put_bits<7>(0x3c); case 26: return w.template put_bits<7>(0x5c); case 27: return w.template put_bits<7>(0x1c);
		case 28: return w.template put_bits<7>(0x6c); case 29: return w.template put_bits<7>(0x2c); case 30: return w.template put_bits<7>(0x4c); case 31: return w.template put_bits<7>(0xc);
		case 32: return w.template put_bits<7>(0x74); case 33: return w.template put_bits<7>(0x34); case 34: return w.template put_bits<7>(0x54); case 35: return w.template put_bits<7>(0x14);
		case 36: return w.template put_bits<7>(0x64); case 37: return w.template put_bits<7>(0x24); case 38: return w.template put_bits<7>(0x44); case 39: return w.template put_bits<7>(0x4);
		case 40: return w.template put_bits<7>(0x78); case 41: return w.template put_bits<7>(0x38); case 42: return w.template put_bits<7>(0x58); case 43: return w.template put_bits<7>(0x18);
		case 44: return w.template put_bits<7>(0x68); case 45: return w.template put_bits<7>(0x28); case 46: return w.template put_bits<7>(0x48); case 47: return w.template put_bits<7>(0x8);
		case 48: return w.template put_bits<8>(0xf0); case 49: return w.template put_bits<8>(0x70); case 50: return w.template put_bits<8>(0xb0); case 51: return w.template put_bits<8>(0x30);
		case 52: return w.template put_bits<8>(0xd0); case 53: return w.template put_bits<8>(0x50); case 54: return w.template put_bits<8>(0x90); case 55: return w.template put_bits<8>(0x10);
		case 56: return w.template put_bits<8>(0xe0); case 57: return w.template put_bits<8>(0x60); case 58: return w.template put_bits<8>(0xa0); case 59: return w.template put_bits<8>(0x20);
		case 60: return w.template put_bits<8>(0xc0); case 61: return w.template put_bits<8>(0x40); case 62: return w.template put_bits<8>(0x80); case 63: return w.template put_bits<8>(0);
		}
	};
	
	const int distance_bits = 6;
	
	const size_t max_distance = (64 << distance_bits) - 1;
	const size_t max_2_distance = (64 << 2) - 1;
	const size_t max_length = 518;
	
	a_vector<a_circular_vector<size_t>> lut(256);
	
	auto w = make_bit_writer(writer);
	
	w.template put<uint8_t>(0);
	w.template put<uint8_t>(distance_bits);
	
	const uint8_t* ptr = input;
	for (size_t pos = 0; pos != input_size; ++pos, ++ptr) {
		uint8_t c = *ptr;
		
		size_t best_length = 0;
		size_t best_distance = 0;
		auto& v = lut[c];
		while (!v.empty() && pos - 1 - v.back() > max_distance) v.pop_back();
		for (auto i = v.begin(); i != v.end(); ++i) {
			size_t npos = *i;
			size_t distance = pos - 1 - npos;
			size_t length = 1;
			++npos;
			for (; pos + length != input_size && ptr[length] == input[npos] && length < max_length; ++length) {
				++npos;
				if (npos == pos) npos = *i;
			}
			if (length > best_length && (length > 2 || distance <= max_2_distance)) {
				best_length = length;
				best_distance = distance;
			}
		}
		v.push_front(pos);
		
		if (best_length < 2) {
			w.template put_bits<1>(0);
			w.put(c);
		} else {
			w.template put_bits<1>(1);
			write_length(w, best_length - 2);
			if (best_length == 2) {
				write_distance(w, best_distance >> 2);
				w.template put_bits<2>(best_distance);
			} else if (distance_bits == 4) {
				write_distance(w, best_distance >> 4);
				w.template put_bits<4>(best_distance);
			} else if (distance_bits == 5) {
				write_distance(w, best_distance >> 5);
				w.template put_bits<5>(best_distance);
			} else {
				write_distance(w, best_distance >> 6);
				w.template put_bits<6>(best_distance);
			}
			pos += best_length - 1;
			ptr += best_length - 1;
		}
		
	}
	
	w.template put_bits<1>(1);
	write_length(w, 519 - 2);
	
}

template<typename base_writer_T, bool default_little_endian = true>
struct replay_file_writer {
	crc32_t crc32;
	base_writer_T& w;
	replay_file_writer(base_writer_T& w) : w(w) {
	}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void put_bytes(const uint8_t* data, size_t size) {
		w.template put<uint32_t>(crc32(data, size));
		size_t segments = (size + 8191) / 8192;
		w.template put<uint32_t>(segments);
		
		a_vector<uint8_t> compressed_data;
		
		size_t output_pos = 0;
		for (size_t i = 0; i != segments; ++i) {
			size_t segment_output_size = size - output_pos;
			if (segment_output_size > 8192) segment_output_size = 8192;
			
			compressed_data.clear();
			compressed_data.reserve(4 + 4 + segment_output_size + (segment_output_size - 1) / 2);
			auto cw = data_loading::make_vector_writer(compressed_data);
			data_loading::compress(data + output_pos, segment_output_size, cw);
			if (compressed_data.size() < segment_output_size) {
				w.template put<uint32_t>(compressed_data.size());
				w.put_bytes(compressed_data.data(), compressed_data.size());
			} else {	
				w.template put<uint32_t>(segment_output_size);
				w.put_bytes(data + output_pos, segment_output_size);
			}
			output_pos += segment_output_size;
		}
		
		if (output_pos != size) error("replay_file_writer: wrote %d bytes, expected %d", output_pos, size);
	}
};

template<typename base_writer_T>
auto make_replay_file_writer(base_writer_T& writer) {
	return replay_file_writer<base_writer_T>(writer);
}

template<bool default_little_endian = true>
struct file_writer {
	a_string filename;
	FILE* f = nullptr;
	file_writer() = default;
	explicit file_writer(a_string filename) {
		open(std::move(filename));
	}
	~file_writer() {
		if (f) fclose(f);
	}
	file_writer(const file_writer&) = delete;
	file_writer(file_writer&& n) {
		f = n.f;
		n.f = nullptr;
	}
	file_writer& operator=(const file_writer&) = delete;
	file_writer& operator=(file_writer&& n) {
		std::swap(f, n.f);
		return *this;
	}
	void open(a_string filename) {
		if (f) fclose(f);
		f = fopen(filename.c_str(), "wb");
		if (!f) error("file_writer: failed to open %s for writing", filename.c_str());
		this->filename = std::move(filename);
	}
	void put_bytes(const uint8_t* src, size_t n) {
		if (!fwrite(src, n, 1, f)) {
			error("file_writer: %s: write error", filename);
		}
	}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void seek(size_t offset) {
		if ((size_t)(long)offset != offset || fseek(f, (long)offset, SEEK_SET)) error("file_writer: %s: failed to seek to offset %d", filename, offset);
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

}

struct replay_saver_state {
	a_deque<static_vector<uint8_t, 0x10000>> history;
	int current_history_frame = -1;
	size_t current_actions_size = 0;
	size_t current_actions_size_index = 0;
	size_t current_actions_size_offset = 0;
	
	const uint8_t* map_data = nullptr;
	size_t map_data_size = 0;
	
	uint32_t random_seed = 42;
	a_string player_name;
	
	size_t map_tile_width = 0;
	size_t map_tile_height = 0;
	int active_player_count = 0;
	int slot_count = 0;
	int game_speed = 3;
	int game_type = 0;
	int tileset = 0;
	
	a_string game_name;
	a_string map_name;
	game_load_functions::setup_info_t setup_info;
	std::array<player_t, 12> players;
	std::array<a_string, 12> player_names;
	
};

struct replay_saver_functions {
	replay_saver_state& replay_saver_st;
	explicit replay_saver_functions(replay_saver_state& replay_saver_st) : replay_saver_st(replay_saver_st) {}
	
	void add_action(int current_frame, int owner, const uint8_t* data, size_t data_size) {
		auto w = data_loading::make_buffers_writer(replay_saver_st.history);
		if (current_frame != replay_saver_st.current_history_frame || replay_saver_st.current_actions_size + 1 + data_size >= 0x100) {
			replay_saver_st.current_history_frame = current_frame;
			replay_saver_st.current_actions_size = 1 + data_size;
			w.template put<uint32_t>(current_frame);
			if (data_size >= 0x100) error("replay_saver_functions::add_action: data_size (%d) > 0x100", data_size);
			w.template put<uint8_t>((uint8_t)(1 + data_size));
			replay_saver_st.current_actions_size_index = replay_saver_st.history.size() - 1;
			replay_saver_st.current_actions_size_offset = replay_saver_st.history.back().size() - 1;
		} else {
			replay_saver_st.current_actions_size += 1 + data_size;
			replay_saver_st.history.at(replay_saver_st.current_actions_size_index).at(replay_saver_st.current_actions_size_offset) = (uint8_t)replay_saver_st.current_actions_size;
		}
		w.template put<uint8_t>(owner);
		w.put_bytes(data, data_size);
	}
	
	template<typename writer_T>
	void save_replay(int current_frame, writer_T& w) {
		std::array<uint8_t, 633> game_info_buffer;
		data_loading::data_writer<> giw(game_info_buffer.data(), game_info_buffer.data() + game_info_buffer.size());
		
		auto put_string = [&](const a_string& str, size_t max_size) {
			size_t i = 0;
			for (; i != str.size() && i != max_size; ++i) giw.put<uint8_t>(str[i]);
			for (; i != max_size; ++i) giw.put<uint8_t>(0);
		};
		
		giw.put<uint8_t>(1); // is broodwar
		giw.put<uint32_t>(current_frame); // frame count
		giw.put<uint16_t>(0); // campaign id
		giw.put<uint8_t>(0); // command byte ?
		giw.put<uint32_t>(replay_saver_st.random_seed);
		for (size_t i = 0; i != 8; ++i) giw.put<uint8_t>(0); // player bytes ?
		giw.put<uint32_t>(0); // ?
		put_string(replay_saver_st.player_name, 24);
		giw.put<uint32_t>(0); // game flags?
		giw.put<uint16_t>((uint16_t)replay_saver_st.map_tile_width); // map width
		giw.put<uint16_t>((uint16_t)replay_saver_st.map_tile_height); // map height
		giw.put<uint8_t>(replay_saver_st.active_player_count); // active player acount
		giw.put<uint8_t>(replay_saver_st.slot_count); // slot count
		giw.put<uint8_t>(replay_saver_st.game_speed); // game speed
		giw.put<uint8_t>(0); // game state ?
		giw.put<uint16_t>(replay_saver_st.game_type); // game type ?
		giw.put<uint16_t>(0); // game sub type ?
		giw.put<uint32_t>(0); // ?
		giw.put<uint16_t>(replay_saver_st.tileset); // tileset
		giw.put<uint8_t>(1); // replay saved
		giw.put<uint8_t>(0); // computer player count?
		put_string(replay_saver_st.game_name, 25);
		put_string(replay_saver_st.map_name, 32);
		giw.put<uint16_t>(replay_saver_st.game_type); // game type ?
		giw.put<uint16_t>(0); // game sub type ?
		giw.put<uint16_t>(0); // sub type display ?
		giw.put<uint16_t>(0); // sub type label ?
		giw.put<uint8_t>(replay_saver_st.setup_info.victory_condition); // victory condition
		giw.put<uint8_t>(replay_saver_st.setup_info.resource_type); // resource type
		giw.put<uint8_t>(1); // use standard unit stats
		giw.put<uint8_t>(2); // fog of war enabled
		giw.put<uint8_t>(replay_saver_st.setup_info.starting_units); // create initial units
		giw.put<uint8_t>(0); // use fixed positions ?
		giw.put<uint8_t>(0); // restriction flags ?
		giw.put<uint8_t>(0); // allies enabled
		giw.put<uint8_t>(0); // teams enabled
		giw.put<uint8_t>(0); // cheats enabled
		giw.put<uint8_t>(replay_saver_st.setup_info.tournament_mode); // tournament mode ?
		giw.put<uint32_t>(0); // victory condition value?
		giw.put<uint32_t>(replay_saver_st.setup_info.starting_minerals); // starting minerals
		giw.put<uint32_t>(0); // starting gas
		giw.put<uint8_t>(0); // ?
		
		for (size_t i = 0; i != 12; ++i) {
			giw.put<uint32_t>(i); // slot ?
			giw.put<uint32_t>(i); // player id
			giw.put<uint8_t>(replay_saver_st.players[i].controller); // controller
			giw.put<uint8_t>((int)replay_saver_st.players[i].race); // race
			giw.put<uint8_t>(replay_saver_st.players[i].force); // force
			put_string(replay_saver_st.player_names[i], 25);
		}
		
		for (size_t i = 0; i != 8; ++i) {
			giw.put<uint32_t>(replay_saver_st.players[i].color);
		}
		for (size_t i = 0; i != 8; ++i) {
			giw.put<uint8_t>(0); // create_melee_units_for_player
		}
		
		auto rw = data_loading::make_replay_file_writer(w);
		
		rw.template put<uint32_t>(0x53526572);
		rw.put_bytes(game_info_buffer.data(), game_info_buffer.size());
		
		size_t history_size = 0;
		for (auto& v : replay_saver_st.history) history_size += v.size();
		rw.template put<uint32_t>(history_size);
		a_vector<uint8_t> tmp_buf;
		tmp_buf.reserve(history_size);
		for (auto& v : replay_saver_st.history) {
			tmp_buf.insert(tmp_buf.end(), v.begin(), v.end());
		}
		rw.put_bytes(tmp_buf.data(), tmp_buf.size());
		
		if (!replay_saver_st.map_data) error("replay_saver_functions::save_replay: replay_saver_state::map_data is null");
		rw.template put<uint32_t>(replay_saver_st.map_data_size);
		rw.put_bytes(replay_saver_st.map_data, replay_saver_st.map_data_size);
	}
	
};


}

#endif
