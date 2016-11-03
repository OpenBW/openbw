#ifndef STRF_H
#define STRF_H

#include <cstring>
#include <string>
#include <algorithm>

namespace bwgame {

namespace strf {


	namespace strf_detail {
		struct descriptor {
			bool end;
			bool flag_left_justify, flag_sign, flag_space, flag_hash, flag_zero;
			unsigned int width, precision;
			char c;
			descriptor() : end(true) {}
		};
		template<typename T> struct unsigned_type;
		template<> struct unsigned_type<char> { typedef unsigned char type; };
		template<> struct unsigned_type<signed char> { typedef unsigned char type; };
		template<> struct unsigned_type<unsigned char> { typedef unsigned char type; };
		template<> struct unsigned_type<short> { typedef unsigned short type; };
		template<> struct unsigned_type<unsigned short> { typedef unsigned short type; };
		template<> struct unsigned_type<int> { typedef unsigned int type; };
		template<> struct unsigned_type<unsigned int> { typedef unsigned int type; };
		template<> struct unsigned_type<long> { typedef unsigned long type; };
		template<> struct unsigned_type<unsigned long> { typedef unsigned long type; };
		template<> struct unsigned_type<long long> { typedef unsigned long long type; };
		template<> struct unsigned_type<unsigned long long> { typedef unsigned long long type; };
		template<typename dst_T>
		struct builder {
			dst_T& dst;
			const char* fmt;
			size_t pos;
			descriptor desc;
			builder(dst_T&dst, const char*fmt) : dst(dst), fmt(fmt), pos(0) {}
			template<typename T> char* reserve_impl(T& dst, size_t n) {
				size_t size = dst.size();
				if (size < pos + n) {
					size_t ns = dst.size() + std::max(dst.size() / 2, (size_t)16);
					if (ns > pos + n) dst.resize(ns);
					else dst.resize(pos + n);
				}
				return (char*)dst.data();
			}
			char* reserve(size_t n) {
				return reserve_impl(dst, n);
			}
			void bad(const char* str) {
				size_t n = strlen(str);
				char* dst = reserve(1 + n + 1);
				dst[pos++] = '(';
				memcpy(dst + pos, str, n);
				pos += n;
				dst[pos++] = ')';
			}
			descriptor next() {
				descriptor r;
				const char* c = fmt;
				auto flush = [&]() {
					if (c == this->fmt) return;
					size_t n = c - this->fmt;
					char* str = this->reserve(n);
					memcpy(&str[this->pos], this->fmt, n);
					this->pos += n;
				};
				auto testflag = [&]() -> bool {
					switch (*c) {
					case '-': r.flag_left_justify = true; break;
					case '+': r.flag_sign = true; break;
					case ' ': r.flag_space = true; break;
					case '#': r.flag_hash = true; break;
					case '0': r.flag_zero = true; break;
					default: return false;
					}
					c++;
					return true;
				};
				auto num = [&](unsigned int&dstv) {
					if (*c == '*') {
						dstv = ~1;
						c++;
						return;
					}
					const char*e = c;
					unsigned int m = 1;
					if (*e >= '0'&&*e <= '9') e++;
					while (*e >= '0'&&*e <= '9') { e++; m *= 10; };
					if (e == c) return;
					unsigned rv = 0;
					for (; c != e; c++) {
						rv += (*c - '0')*m;
						m /= 10;
					}
					dstv = rv;
				};
				while (*c) {
					if (*c == '%') {
						flush();
						if (*++c == '%') {
							char* str = reserve(1);
							str[pos++] = '%';
							++c;
							fmt = c;
						} else {
							r.end = false;
							r.flag_left_justify = false;
							r.flag_sign = false;
							r.flag_space = false;
							r.flag_hash = false;
							r.flag_zero = false;
							while (testflag());
							r.width = ~0;
							r.precision = ~0;
							num(r.width);
							if (*c == '.') ++c, num(r.precision);
							if (!*c) {
								bad("bad format string");
								r.end = true;
								return r;
							}
							r.c = *c++;
							fmt = c;
							return r;
						}
					}
					c++;
				}
				flush();
				r.end = true;
				return r;
			}
			template<typename T, int base, bool caps>
			void do_num(T v) {
				char buf[sizeof(v) * 4];
				bool negative = std::is_signed<T>::value ? (typename std::make_signed<T>::type)v < 0 : false;
				char* c = &buf[sizeof(buf)];
				bool is_zero = v == 0;
				if (is_zero) {
					if (desc.precision != 0) *--c = '0';
				} else {
					if (negative) v = 0 - v;
					typename unsigned_type<T>::type& uv = (typename unsigned_type<T>::type&)v;
					while (uv) {
						char n = uv%base;
						uv /= base;
						char d;
						if (base > 10 && n > 9) d = n - 10 + (caps ? 'A' : 'a');
						else d = '0' + n;
						*--c = d;
					}
				}
				size_t len = &buf[sizeof(buf)] - c;
				const char* num = c;
				char prefix[4];
				c = &prefix[0];
				if (desc.flag_hash && !is_zero) {
					if (base == 8) *c++ = '0';
					else if (base == 16) {
						*c++ = '0';
						if (caps) *c++ = 'X';
						else *c++ = 'x';
					}
				}
				if (negative) {
					*c++ = '-';
				} else {
					if (desc.flag_sign) *c++ = '+';
					else if (desc.flag_space) *c++ = ' ';
				}
				size_t prefix_len = c - &prefix[0];
				size_t outlen = prefix_len + len;
				if (desc.precision != (unsigned)-1 && desc.precision > len) outlen += (desc.precision - len);
				size_t numlen = outlen;
				if (desc.width != (unsigned)-1 && desc.width > outlen) outlen = desc.width;
				char*str = reserve(outlen);
				if (!desc.flag_zero && !desc.flag_left_justify) {
					for (size_t i = 0; i < outlen - numlen; i++) str[pos++] = ' ';
				}
				if (prefix_len) {
					memcpy(&str[pos], prefix, prefix_len);
					pos += prefix_len;
				}
				if (desc.flag_zero) {
					for (size_t i = 0; i < outlen - numlen; i++) str[pos++] = '0';
				}
				if (desc.precision != (unsigned)-1 && desc.precision>len) {
					for (size_t i = 0; i < desc.precision - len; i++) str[pos++] = '0';
				}
				memcpy(&str[pos], num, len);
				pos += len;
				if (desc.flag_left_justify) {
					if (desc.flag_zero) {
						bad("zero flag and left justify flag cannot be specified together");
						return;
					}
					for (size_t i = 0; i<outlen - numlen; i++) str[pos++] = ' ';
				}
			}
			void do_string(const char* s, size_t len) {
				if (desc.flag_zero || desc.flag_hash || desc.flag_sign || desc.flag_space) {
					bad("bad flags for string");
					return;
				}
				if (!s) {
					s = "(null)";
					len = 6;
				}
				size_t outlen = len;
				if (desc.precision != (unsigned)-1 && outlen>desc.precision) outlen = desc.precision;
				if (outlen<len) len = outlen;
				if (desc.width != (unsigned)-1 && desc.width>outlen) outlen = desc.width;
				char* str = reserve(outlen);
				if (!desc.flag_left_justify) {
					for (size_t i = 0; i < outlen - len; i++) str[pos++] = ' ';
				}
				memcpy(&str[pos], s, len);
				pos += len;
				if (desc.flag_left_justify) {
					for (size_t i = 0; i < outlen - len; i++) str[pos++] = ' ';
				}
			}


			template<typename T>
			unsigned int to_uint(T&& v) {
				bad("argument can not be converted to unsigned int");
				return 0;
			}
			unsigned int to_uint(char v) { return v; }
			unsigned int to_uint(unsigned char v) { return v; }
			unsigned int to_uint(short v) { return v; }
			unsigned int to_uint(unsigned short v) { return v; }
			unsigned int to_uint(int v) { return v; }
			unsigned int to_uint(unsigned int v) { return v; }
			unsigned int to_uint(long v) { return v; }
			unsigned int to_uint(unsigned long v) { return v; }
			template<int base, bool caps, typename T>
			void do_signed_int(T&& v) {
				bad("argument is not numeric");
			}
			template<int base, bool caps> void do_signed_int(bool v) { do_num<int, base, caps>(v ? 1 : 0); }
			template<int base, bool caps> void do_signed_int(char v) { do_num<signed char, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(signed char v) { do_num<signed char, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(unsigned char v) { do_num<signed char, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(short v) { do_num<short, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(unsigned short v) { do_num<short, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(int v) { do_num<int, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(unsigned int v) { do_num<int, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(long v) { do_num<long, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(unsigned long v) { do_num<long, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(long long v) { do_num<long long, base, caps>(v); }
			template<int base, bool caps> void do_signed_int(unsigned long long v) { do_num<long long, base, caps>(v); }
			template<int base, bool caps, typename T>
			void do_unsigned_int(T&& v) {
				bad("argument is not numeric");
			}
			template<int base, bool caps> void do_unsigned_int(bool v) { do_num<unsigned int, base, caps>(v ? 1 : 0); }
			template<int base, bool caps> void do_unsigned_int(char v) { do_num<unsigned char, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(signed char v) { do_num<unsigned char, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(unsigned char v) { do_num<unsigned char, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(short v) { do_num<unsigned short, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(unsigned short v) { do_num<unsigned short, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(int v) { do_num<unsigned int, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(unsigned int v) { do_num<unsigned int, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(long v) { do_num<unsigned long, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(unsigned long v) { do_num<unsigned long, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(long long v) { do_num<unsigned long long, base, caps>(v); }
			template<int base, bool caps> void do_unsigned_int(unsigned long long v) { do_num<unsigned long long, base, caps>(v); }
			template<typename T>
			void do_string(T&& v) {
				bad("argument is not a string");
			}
			void do_string(char*str) { do_string(str, str ? strlen(str) : 0); }
			void do_string(const char*str) { do_string(str, str ? strlen(str) : 0); }
			template<typename t, typename traits, typename allocator>
			void do_string(std::basic_string<t, traits, allocator>&&s) { do_string(s.c_str(), s.size()); }
			template<typename t, typename traits, typename allocator>
			void do_string(const std::basic_string<t, traits, allocator>&&s) { do_string(s.c_str(), s.size()); }
			template<typename t, typename traits, typename allocator>
			void do_string(std::basic_string<t, traits, allocator>&s) { do_string(s.c_str(), s.size()); }
			template<typename t, typename traits, typename allocator>
			void do_string(const std::basic_string<t, traits, allocator>&s) { do_string(s.c_str(), s.size()); }
			template<typename T>
			void do_pointer(T&& v) {
				bad("argument is not a pointer");
			}
			void do_pointer(void* v) {
				desc.flag_hash = true;
				do_unsigned_int<16, false>((uintptr_t)v);
			}
			template<typename T> void do_pointer(T*v) {
				do_pointer((void*)v);
			}
			template<typename T>
			void do_float(T&& v) {
				bad("argument is not floating-point");
			}
			void do_float(double v) {
				char fstr[0x10];
				char* c = &fstr[0];
				*c++ = '%';
				if (desc.flag_hash) *c++ = '#';
				if (desc.flag_left_justify) *c++ = '-';
				if (desc.flag_sign) *c++ = '+';
				if (desc.flag_space) *c++ = ' ';
				if (desc.flag_zero) *c++ = '0';
				if (desc.width != (unsigned)-1) *c++ = '*';
				if (desc.precision != (unsigned)-1) { *c++ = '.'; *c++ = '*'; }
				*c++ = desc.c;
				*c = 0;
				size_t len = 0x100;
				if (desc.precision != (unsigned)-1) len += desc.precision;
				if (desc.width != (unsigned)-1 && desc.width >= len) len += desc.width;
				char* str = reserve(len);
				str += pos;
				memset(str, 0, len);
				if (desc.width != (unsigned)-1 && desc.precision != (unsigned)-1) sprintf(str, fstr, (int)desc.width, (int)desc.precision, v);
				else if (desc.width != (unsigned)-1) sprintf(str, fstr, (int)desc.width, v);
				else if (desc.precision != (unsigned)-1) sprintf(str, fstr, (int)desc.precision, v);
				else sprintf(str, fstr, v);
				pos += strlen(str);
			}
			void do_float(float v) {
				do_float((double)v);
			}
			template<typename T>
			void do_char(T&& v) {
				bad("argument is not convertible to a character");
			}
			void do_char(char c) {
				if (desc.flag_zero || desc.flag_hash || desc.flag_sign || desc.flag_space) {
					bad("bad flags for character");
					return;
				}
				size_t outlen = 1;
				if (desc.width != (unsigned)-1 && desc.width > outlen) outlen = desc.width;
				char* str = reserve(outlen);
				if (!desc.flag_left_justify) {
					for (size_t i = 0; i < outlen - 1; i++) str[pos++] = ' ';
				}
				str[pos++] = c;
				if (desc.flag_left_justify) {
					for (size_t i = 0; i < outlen - 1; i++) str[pos++] = ' ';
				}
			}
			void do_char(signed char c) { do_char((char)c); }
			void do_char(unsigned char c) { do_char((char)c); }
			void do_char(short c) { do_char((char)c); }
			void do_char(unsigned short c) { do_char((char)c); }
			void do_char(int c) { do_char((char)c); }
			void do_char(unsigned int c) { do_char((char)c); }
			void do_char(long c) { do_char((char)c); }
			void do_char(unsigned long c) { do_char((char)c); }
			void do_char(long long c) { do_char((char)c); }
			void do_char(unsigned long long c) { do_char((char)c); }
			template<typename T>
			void advance(T&& v) {
				if (desc.end) desc = next();
				if (desc.end) {
					bad("too many arguments for the specified format string");
					return;
				}
				if (desc.width == ~(unsigned)1) {
					desc.width = to_uint(std::forward<T>(v));
					return;
				}
				if (desc.precision == ~(unsigned)1) {
					desc.precision = to_uint(std::forward<T>(v));
					return;
				}
				switch (desc.c) {
				case 'd':
				case 'i':
					do_signed_int<10, false>(std::forward<T>(v));
					break;
				case 'u':
					do_unsigned_int<10, false>(std::forward<T>(v));
					break;
				case 'x':
					do_unsigned_int<16, false>(std::forward<T>(v));
					break;
				case 'X':
					do_unsigned_int<16, true>(std::forward<T>(v));
					break;
				case 'o':
					do_unsigned_int<8, false>(std::forward<T>(v));
					break;
				case 's':
					do_string(std::forward<T>(v));
					break;
				case 'c':
					do_char(std::forward<T>(v));
					break;
				case 'p':
					do_pointer(std::forward<T>(v));
					break;
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					do_float(std::forward<T>(v));
					break;
				default: bad("unrecognized specifier in format string");
				}
				desc.end = true;
			}
			const char* finish() {
				if (!desc.end) {
					bad("too few arguments for the specified format string");
					dst.resize(pos);
					return dst.c_str();
				}
				auto f = next();
				if (!f.end) {
					bad("too few arguments for the specified format string");
					dst.resize(pos);
					return dst.c_str();
				}
				dst.resize(pos);
				return dst.c_str();
			}
		};

		template<typename builder_T>
		void advance(builder_T& b) {
		}
		template<typename builder_T, typename A1, typename ...Ax>
		void advance(builder_T& b, A1&& a1, Ax&&... ax) {
			b.advance(std::forward<A1>(a1));
			strf_detail::advance(b, std::forward<Ax>(ax)...);
		}
	}


	template<typename dst_T, typename ...A>
	const char* format(dst_T& dst, const char* fmt, A&&... args) {
		strf_detail::builder<dst_T> b(dst, fmt);
		strf_detail::advance(b, std::forward<A>(args)...);
		return b.finish();
	}

}

}

#endif
