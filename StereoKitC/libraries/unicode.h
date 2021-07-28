#pragma once

#include <stdint.h>

///////////////////////////////////////////

// Source:
// https://github.com/skeeto/branchless-utf8
// Should switch this to a SIMD implementation later, possibly:
// https://dirtyhandscoding.github.io/posts/utf8lut-vectorized-utf-8-converter-decoding-utf-8.html
inline char32_t utf8_decode_fast(const char *utf8_str, const char **out_next_char) {
	static const uint32_t lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	static const int32_t  masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
	static const uint32_t mins[]   = {4194304, 0, 128, 2048, 65536};
	static const int32_t  shiftc[] = {0, 18, 12, 6, 0};
	static const int32_t  shifte[] = {0, 6, 4, 2, 0};

	const unsigned char *s   = (unsigned char *)utf8_str;
	int32_t              len = lengths[s[0] >> 3];

	// Compute the pointer to the next character early so that the next
	// iteration can start working on the next character. Neither Clang
	// nor GCC figure out this reordering on their own.
	const unsigned char *next = s + len + !len;

	// Assume a four-byte character and load four bytes. Unused bits are
	// shifted out.
	char32_t result = (char32_t)(s[0] & masks[len]) << 18;
	result |= (char32_t)(s[1] & 0x3f) << 12;
	result |= (char32_t)(s[2] & 0x3f) <<  6;
	result |= (char32_t)(s[3] & 0x3f) <<  0;
	result >>= shiftc[len];

	*out_next_char = (const char *)next;
	return result;
}

///////////////////////////////////////////

inline char32_t utf8_decode(const char *utf8_str, const char **out_next_char, int32_t *out_err) {
	static const char lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	static const int32_t  masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
	static const uint32_t mins[]   = {4194304, 0, 128, 2048, 65536};
	static const int32_t  shiftc[] = {0, 18, 12, 6, 0};
	static const int32_t  shifte[] = {0, 6, 4, 2, 0};

	const char *s   = utf8_str;
	int32_t     len = lengths[s[0] >> 3];

	// Compute the pointer to the next character early so that the next
	// iteration can start working on the next character. Neither Clang
	// nor GCC figure out this reordering on their own.
	const char *next = s + len + !len;

	// Assume a four-byte character and load four bytes. Unused bits are
	// shifted out.
	char32_t result = (char32_t)(s[0] & masks[len]) << 18;
	result |= (char32_t)(s[1] & 0x3f) << 12;
	result |= (char32_t)(s[2] & 0x3f) <<  6;
	result |= (char32_t)(s[3] & 0x3f) <<  0;
	result >>= shiftc[len];

	// Accumulate the various error conditions.
	*out_err  = (result < mins[len]) << 6; // non-canonical encoding
	*out_err |= ((result >> 11) == 0x1b) << 7;  // surrogate half?
	*out_err |= (result > 0x10FFFF) << 8;  // out of range?
	*out_err |= (s[1] & 0xc0) >> 2;
	*out_err |= (s[2] & 0xc0) >> 4;
	*out_err |= (s[3]       ) >> 6;
	*out_err ^= 0x2a; // top two bits of each tail byte correct?
	*out_err >>= shifte[len];

	*out_next_char = next;
	return result;
}

///////////////////////////////////////////

inline char32_t utf16_decode_fast(const char16_t *utf16_str, const char16_t **out_next) {
	const char16_t *start = utf16_str;

	if (utf16_str[0] < 0xd800 || utf16_str[0] > 0xdfff) {
		char32_t result = utf16_str[0];
		*out_next = utf16_str + 1;
		return result;

	} else if (utf16_str[0] >= 0xd800 && utf16_str[0] <= 0xdbff &&
	           utf16_str[1] >= 0xdc00 && utf16_str[1] <= 0xdfff) {

		char32_t result = (((utf16_str[0] & 0x03ffL) << 10) | ((utf16_str[1] & 0x03ffL) <<  0)) + 0x00010000L;
		*out_next = utf16_str + 2;
		return result;
	}
	return 0;
}