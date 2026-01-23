// load_hdr.h - Radiance HDR image loader with SIMD optimization
//
// Loads .hdr (RGBE) files from memory and converts to RGB9E5 or RG11B10 format.
// Supports both RLE-compressed and uncompressed scanlines.
// Automatically uses SSE4.1 (x64) or NEON (ARM64), with scalar fallback.
//
// Usage:
//   // Query dimensions without decoding
//   hdr_header_t header = hdr_parse_header(data, size);
//   if (header.valid) printf("%dx%d\n", header.width, header.height);
//
//   // Decode pixels using header (choose format)
//   hdr_image_t img = hdr_decode_pixels_rgb9e5(data, size, &header);
//   hdr_image_t img = hdr_decode_pixels_rg11b10(data, size, &header);
//
//   // Or load in one step
//   hdr_image_t img = hdr_load_from_memory_rgb9e5(data, size);
//   hdr_image_t img = hdr_load_from_memory_rg11b10(data, size);
//
//   // Cleanup
//   hdr_image_free(&img);

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__SSE4_1__) || (defined(_MSC_VER) && defined(_M_X64))
#define HDR_SIMD_SSE4 1
#include <smmintrin.h>
#elif defined(__ARM_NEON) || defined(__aarch64__)
#define HDR_SIMD_NEON 1
#include <arm_neon.h>
#endif

typedef struct {
	int32_t width;
	int32_t height;
	int32_t data_offset;  // Byte offset where pixel data begins
	int32_t valid;        // 1 if header parsed successfully
} hdr_header_t;

typedef struct {
	uint32_t *pixels;  // RGB9E5 packed
	int32_t   width;
	int32_t   height;
} hdr_image_t;

///////////////////////////////////////////////////////////////////////////////
// Private - Header parsing
///////////////////////////////////////////////////////////////////////////////

static inline const uint8_t *_hdr_find_line_end(const uint8_t *ptr, const uint8_t *end) {
	while (ptr < end && *ptr != '\n') ptr++;
	return ptr < end ? ptr + 1 : NULL;
}

static inline const char *_hdr_parse_int(const char *str, int32_t *out) {
	int32_t val  = 0;
	int32_t sign = 1;
	if (*str == '-') { sign = -1; str++; }
	if (*str < '0' || *str > '9') return NULL;
	while (*str >= '0' && *str <= '9') val = val * 10 + (*str++ - '0');
	*out = val * sign;
	return str;
}

///////////////////////////////////////////////////////////////////////////////
// Private - RGBE to RGB9E5 conversion
///////////////////////////////////////////////////////////////////////////////

static inline uint32_t _rgbe_to_rgb9e5_scalar(uint8_t r, uint8_t g, uint8_t b, uint8_t e) {
	int32_t exp_new = e - 113;                          // rebias: RGBE bias 128 -> RGB9E5 bias 15
	int32_t under   = exp_new >> 31;                    // -1 if underflow, 0 otherwise
	int32_t over    = (31 - exp_new) >> 31;             // -1 if overflow,  0 otherwise
	exp_new = (exp_new & ~under & ~over) | (31 & over); // clamp to [0, 31]

	uint32_t mask = ~under;                             // 0 if underflow, ~0 otherwise
	uint32_t r9 = ((uint32_t)r << 1) & mask;            // 8-bit -> 9-bit mantissa, zero if underflow
	uint32_t g9 = ((uint32_t)g << 1) & mask;
	uint32_t b9 = ((uint32_t)b << 1) & mask;

	r9 |= over & 511;                                   // saturate to 511 if overflow
	g9 |= over & 511;
	b9 |= over & 511;

	return ((uint32_t)exp_new << 27) | (b9 << 18) | (g9 << 9) | r9;
}

///////////////////////////////////////////////////////////////////////////////
// Private - RGBE to RG11B10 conversion (direct integer math, no float)
///////////////////////////////////////////////////////////////////////////////

// Portable CLZ (count leading zeros) for 8-bit values
static inline int32_t _clz8(uint8_t x) {
	if (x == 0) return 8;
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_clz((uint32_t)x) - 24;
#else
	int32_t n = 0;
	if (x <= 0x0F) { n += 4; x <<= 4; }
	if (x <= 0x3F) { n += 2; x <<= 2; }
	if (x <= 0x7F) { n += 1; }
	return n;
#endif
}

// Convert single RGBE channel to 11-bit unsigned float (5-bit exp, 6-bit mantissa)
// Uses pure integer math: value = m * 2^(e-136)
// R11 format: value = (1 + man/64) * 2^(exp-15) for normalized
static inline uint32_t _rgbe_to_uf11(uint8_t m, int32_t e) {
	if (m == 0) return 0;

	// Find MSB position (7 = bit 7, 0 = bit 0)
	int32_t msb = 7 - _clz8(m);

	// New exponent: msb + e - 136 + 15 = msb + e - 121
	int32_t exp11 = msb + e - 121;

	if (exp11 >= 31) return 0x7BF;  // Max: exp=30, mantissa=63
	if (exp11 <= 0) {
		// Denormalized or underflow
		if (exp11 < -5) return 0;
		// Denorm: shift mantissa right, no implicit 1
		int32_t shift = 7 - 6 - exp11;  // = 1 - exp11
		if (shift >= 8) return 0;
		return (uint32_t)m >> shift;
	}

	// Extract 6-bit mantissa from bits below MSB
	// Clear MSB (implicit 1), then align remaining bits to 6-bit field
	uint32_t frac = m ^ (1u << msb);
	uint32_t man6 = (msb >= 6) ? (frac >> (msb - 6)) : (frac << (6 - msb));

	return ((uint32_t)exp11 << 6) | (man6 & 0x3F);
}

// Convert single RGBE channel to 10-bit unsigned float (5-bit exp, 5-bit mantissa)
static inline uint32_t _rgbe_to_uf10(uint8_t m, int32_t e) {
	if (m == 0) return 0;

	int32_t msb = 7 - _clz8(m);
	int32_t exp10 = msb + e - 121;

	if (exp10 >= 31) return 0x3DF;  // Max: exp=30, mantissa=31
	if (exp10 <= 0) {
		if (exp10 < -4) return 0;
		int32_t shift = 7 - 5 - exp10;  // = 2 - exp10
		if (shift >= 8) return 0;
		return (uint32_t)m >> shift;
	}

	uint32_t frac = m ^ (1u << msb);
	uint32_t man5 = (msb >= 5) ? (frac >> (msb - 5)) : (frac << (5 - msb));

	return ((uint32_t)exp10 << 5) | (man5 & 0x1F);
}

static inline uint32_t _rgbe_to_rg11b10_scalar(uint8_t r, uint8_t g, uint8_t b, uint8_t e) {
	if (e == 0) return 0;

	uint32_t r11 = _rgbe_to_uf11(r, (int32_t)e);
	uint32_t g11 = _rgbe_to_uf11(g, (int32_t)e);
	uint32_t b10 = _rgbe_to_uf10(b, (int32_t)e);

	return r11 | (g11 << 11) | (b10 << 22);
}

#ifdef HDR_SIMD_SSE4

// SSE4.1: Process 4 pixels at once
static inline void _rgbe_to_rgb9e5_sse4(
	const uint8_t *r_in, const uint8_t *g_in, const uint8_t *b_in, const uint8_t *e_in,
	uint32_t *out, int32_t count)
{
	const __m128i bias    = _mm_set1_epi32(113);
	const __m128i zero    = _mm_setzero_si128();
	const __m128i max_exp = _mm_set1_epi32(31);
	const __m128i max_man = _mm_set1_epi32(511);

	int32_t i = 0;
	for (; i + 4 <= count; i += 4) {
		// Load 4 bytes, zero-extend to 32-bit
		__m128i r = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(r_in + i)));
		__m128i g = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(g_in + i)));
		__m128i b = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(b_in + i)));
		__m128i e = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(e_in + i)));

		// exp_new = clamp(e - 113, 0, 31)
		__m128i exp_new = _mm_sub_epi32(e, bias);
		__m128i clamped = _mm_max_epi32(zero, _mm_min_epi32(max_exp, exp_new));

		// Masks for underflow and overflow
		__m128i under = _mm_cmplt_epi32(exp_new, zero);
		__m128i over  = _mm_cmpgt_epi32(exp_new, max_exp);

		// r9 = (r << 1), zero if underflow, saturate if overflow
		__m128i r9 = _mm_slli_epi32(r, 1);
		__m128i g9 = _mm_slli_epi32(g, 1);
		__m128i b9 = _mm_slli_epi32(b, 1);

		r9 = _mm_andnot_si128(under, r9);
		g9 = _mm_andnot_si128(under, g9);
		b9 = _mm_andnot_si128(under, b9);

		r9 = _mm_or_si128(r9, _mm_and_si128(over, max_man));
		g9 = _mm_or_si128(g9, _mm_and_si128(over, max_man));
		b9 = _mm_or_si128(b9, _mm_and_si128(over, max_man));

		// Pack result
		__m128i result = _mm_or_si128(
			_mm_or_si128(_mm_slli_epi32(clamped, 27), _mm_slli_epi32(b9, 18)),
			_mm_or_si128(_mm_slli_epi32(g9, 9), r9)
		);

		_mm_storeu_si128((__m128i*)(out + i), result);
	}

	// Scalar tail
	for (; i < count; i++) {
		out[i] = _rgbe_to_rgb9e5_scalar(r_in[i], g_in[i], b_in[i], e_in[i]);
	}
}

#define _rgbe_to_rgb9e5_simd _rgbe_to_rgb9e5_sse4

// SSE4.1: Process 4 pixels at once (RG11B10)
// Uses int->float conversion to get MSB position (FPU normalization), then integer math
static inline void _rgbe_to_rg11b10_sse4(
	const uint8_t *r_in, const uint8_t *g_in, const uint8_t *b_in, const uint8_t *e_in,
	uint32_t *out, int32_t count)
{
	const __m128i bias_121  = _mm_set1_epi32(121);  // 136 - 15 (RGBE bias - R11 bias)
	const __m128i bias_127  = _mm_set1_epi32(127);
	const __m128i zero      = _mm_setzero_si128();
	const __m128i max_exp   = _mm_set1_epi32(30);
	const __m128i max_r11   = _mm_set1_epi32(0x7BF);   // exp=30, man=63
	const __m128i max_b10   = _mm_set1_epi32(0x3DF);   // exp=30, man=31
	const __m128i mask_man6 = _mm_set1_epi32(0x3F);
	const __m128i mask_man5 = _mm_set1_epi32(0x1F);

	int32_t i = 0;
	for (; i + 4 <= count; i += 4) {
		// Load 4 bytes, zero-extend to 32-bit
		__m128i ri = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(r_in + i)));
		__m128i gi = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(g_in + i)));
		__m128i bi = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(b_in + i)));
		__m128i ei = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(const int32_t*)(e_in + i)));

		// Convert mantissas to float to get normalized form (FPU finds MSB for us)
		__m128 rf = _mm_cvtepi32_ps(ri);
		__m128 gf = _mm_cvtepi32_ps(gi);
		__m128 bf = _mm_cvtepi32_ps(bi);

		// Extract exponent from float bits (this is MSB position + 127)
		__m128i rf_bits = _mm_castps_si128(rf);
		__m128i gf_bits = _mm_castps_si128(gf);
		__m128i bf_bits = _mm_castps_si128(bf);

		__m128i r_msb = _mm_sub_epi32(_mm_srli_epi32(rf_bits, 23), bias_127);  // MSB position
		__m128i g_msb = _mm_sub_epi32(_mm_srli_epi32(gf_bits, 23), bias_127);
		__m128i b_msb = _mm_sub_epi32(_mm_srli_epi32(bf_bits, 23), bias_127);

		// Final exponent: msb + e - 121
		__m128i r_exp = _mm_sub_epi32(_mm_add_epi32(r_msb, ei), bias_121);
		__m128i g_exp = _mm_sub_epi32(_mm_add_epi32(g_msb, ei), bias_121);
		__m128i b_exp = _mm_sub_epi32(_mm_add_epi32(b_msb, ei), bias_121);

		// Extract mantissa from float bits (already normalized, just need to align)
		// Float mantissa is 23 bits, we want 6 bits for R/G, 5 bits for B
		__m128i r_man = _mm_and_si128(_mm_srli_epi32(rf_bits, 17), mask_man6);
		__m128i g_man = _mm_and_si128(_mm_srli_epi32(gf_bits, 17), mask_man6);
		__m128i b_man = _mm_and_si128(_mm_srli_epi32(bf_bits, 18), mask_man5);

		// Handle overflow (exp > 30) -> max value
		__m128i r_over = _mm_cmpgt_epi32(r_exp, max_exp);
		__m128i g_over = _mm_cmpgt_epi32(g_exp, max_exp);
		__m128i b_over = _mm_cmpgt_epi32(b_exp, max_exp);

		// Clamp exponent to [0, 30]
		r_exp = _mm_max_epi32(zero, _mm_min_epi32(max_exp, r_exp));
		g_exp = _mm_max_epi32(zero, _mm_min_epi32(max_exp, g_exp));
		b_exp = _mm_max_epi32(zero, _mm_min_epi32(max_exp, b_exp));

		// Zero mantissa if exponent underflowed (exp was < 0)
		// Note: simplified handling - denormals become 0
		__m128i r_valid = _mm_cmpgt_epi32(_mm_add_epi32(r_msb, ei), bias_121);
		__m128i g_valid = _mm_cmpgt_epi32(_mm_add_epi32(g_msb, ei), bias_121);
		__m128i b_valid = _mm_cmpgt_epi32(_mm_add_epi32(b_msb, ei), bias_121);

		r_man = _mm_and_si128(r_man, r_valid);
		g_man = _mm_and_si128(g_man, g_valid);
		b_man = _mm_and_si128(b_man, b_valid);
		r_exp = _mm_and_si128(r_exp, r_valid);
		g_exp = _mm_and_si128(g_exp, g_valid);
		b_exp = _mm_and_si128(b_exp, b_valid);

		// Pack: R11 = (exp << 6) | man, G11 = (exp << 6) | man, B10 = (exp << 5) | man
		__m128i r11 = _mm_or_si128(_mm_slli_epi32(r_exp, 6), r_man);
		__m128i g11 = _mm_or_si128(_mm_slli_epi32(g_exp, 6), g_man);
		__m128i b10 = _mm_or_si128(_mm_slli_epi32(b_exp, 5), b_man);

		// Apply overflow saturation
		r11 = _mm_or_si128(_mm_andnot_si128(r_over, r11), _mm_and_si128(r_over, max_r11));
		g11 = _mm_or_si128(_mm_andnot_si128(g_over, g11), _mm_and_si128(g_over, max_r11));
		b10 = _mm_or_si128(_mm_andnot_si128(b_over, b10), _mm_and_si128(b_over, max_b10));

		// Zero check: if original mantissa was 0, result should be 0
		__m128i r_zero = _mm_cmpeq_epi32(ri, zero);
		__m128i g_zero = _mm_cmpeq_epi32(gi, zero);
		__m128i b_zero = _mm_cmpeq_epi32(bi, zero);
		r11 = _mm_andnot_si128(r_zero, r11);
		g11 = _mm_andnot_si128(g_zero, g11);
		b10 = _mm_andnot_si128(b_zero, b10);

		// Final pack: R[0:10] | G[11:21] | B[22:31]
		__m128i result = _mm_or_si128(
			_mm_or_si128(r11, _mm_slli_epi32(g11, 11)),
			_mm_slli_epi32(b10, 22)
		);

		_mm_storeu_si128((__m128i*)(out + i), result);
	}

	// Scalar tail
	for (; i < count; i++) {
		out[i] = _rgbe_to_rg11b10_scalar(r_in[i], g_in[i], b_in[i], e_in[i]);
	}
}

#define _rgbe_to_rg11b10_simd _rgbe_to_rg11b10_sse4

#elif defined(HDR_SIMD_NEON)

// NEON: Process 4 pixels at once
static inline void _rgbe_to_rgb9e5_neon(
	const uint8_t *r_in, const uint8_t *g_in, const uint8_t *b_in, const uint8_t *e_in,
	uint32_t *out, int32_t count)
{
	const int32x4_t bias    = vdupq_n_s32(113);
	const int32x4_t zero    = vdupq_n_s32(0);
	const int32x4_t max_exp = vdupq_n_s32(31);
	const uint32x4_t max_man = vdupq_n_u32(511);

	int32_t i = 0;
	for (; i + 4 <= count; i += 4) {
		// Load 4 bytes, zero-extend to 32-bit
		uint8x8_t r8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(r_in + i)));
		uint8x8_t g8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(g_in + i)));
		uint8x8_t b8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(b_in + i)));
		uint8x8_t e8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(e_in + i)));

		uint32x4_t r = vmovl_u16(vget_low_u16(vmovl_u8(r8)));
		uint32x4_t g = vmovl_u16(vget_low_u16(vmovl_u8(g8)));
		uint32x4_t b = vmovl_u16(vget_low_u16(vmovl_u8(b8)));
		int32x4_t  e = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(e8))));

		// exp_new = clamp(e - 113, 0, 31)
		int32x4_t exp_new = vsubq_s32(e, bias);
		int32x4_t clamped = vmaxq_s32(zero, vminq_s32(max_exp, exp_new));

		// Masks for underflow and overflow
		uint32x4_t under = vcltq_s32(exp_new, zero);  // exp < 0
		uint32x4_t over  = vcgtq_s32(exp_new, max_exp);  // exp > 31

		// r9 = (r << 1), zero if underflow, saturate if overflow
		uint32x4_t r9 = vshlq_n_u32(r, 1);
		uint32x4_t g9 = vshlq_n_u32(g, 1);
		uint32x4_t b9 = vshlq_n_u32(b, 1);

		r9 = vbicq_u32(r9, under);  // zero if underflow
		g9 = vbicq_u32(g9, under);
		b9 = vbicq_u32(b9, under);

		r9 = vorrq_u32(r9, vandq_u32(over, max_man));  // saturate if overflow
		g9 = vorrq_u32(g9, vandq_u32(over, max_man));
		b9 = vorrq_u32(b9, vandq_u32(over, max_man));

		// Pack: (exp << 27) | (b << 18) | (g << 9) | r
		uint32x4_t result = vorrq_u32(
			vorrq_u32(vshlq_n_u32(vreinterpretq_u32_s32(clamped), 27), vshlq_n_u32(b9, 18)),
			vorrq_u32(vshlq_n_u32(g9, 9), r9)
		);

		vst1q_u32(out + i, result);
	}

	// Scalar tail
	for (; i < count; i++) {
		out[i] = _rgbe_to_rgb9e5_scalar(r_in[i], g_in[i], b_in[i], e_in[i]);
	}
}

#define _rgbe_to_rgb9e5_simd _rgbe_to_rgb9e5_neon

// NEON: Process 4 pixels at once (RG11B10)
// Uses int->float conversion to get MSB position (FPU normalization), then integer math
static inline void _rgbe_to_rg11b10_neon(
	const uint8_t *r_in, const uint8_t *g_in, const uint8_t *b_in, const uint8_t *e_in,
	uint32_t *out, int32_t count)
{
	const int32x4_t bias_121  = vdupq_n_s32(121);
	const int32x4_t bias_127  = vdupq_n_s32(127);
	const int32x4_t zero      = vdupq_n_s32(0);
	const int32x4_t max_exp   = vdupq_n_s32(30);
	const uint32x4_t max_r11  = vdupq_n_u32(0x7BF);
	const uint32x4_t max_b10  = vdupq_n_u32(0x3DF);
	const uint32x4_t mask_man6 = vdupq_n_u32(0x3F);
	const uint32x4_t mask_man5 = vdupq_n_u32(0x1F);

	int32_t i = 0;
	for (; i + 4 <= count; i += 4) {
		// Load 4 bytes, zero-extend to 32-bit
		uint8x8_t r8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(r_in + i)));
		uint8x8_t g8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(g_in + i)));
		uint8x8_t b8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(b_in + i)));
		uint8x8_t e8 = vreinterpret_u8_u32(vdup_n_u32(*(const uint32_t*)(e_in + i)));

		uint32x4_t ri = vmovl_u16(vget_low_u16(vmovl_u8(r8)));
		uint32x4_t gi = vmovl_u16(vget_low_u16(vmovl_u8(g8)));
		uint32x4_t bi = vmovl_u16(vget_low_u16(vmovl_u8(b8)));
		int32x4_t  ei = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(vmovl_u8(e8))));

		// Convert mantissas to float to get normalized form (FPU finds MSB for us)
		float32x4_t rf = vcvtq_f32_u32(ri);
		float32x4_t gf = vcvtq_f32_u32(gi);
		float32x4_t bf = vcvtq_f32_u32(bi);

		// Extract exponent from float bits (this is MSB position + 127)
		uint32x4_t rf_bits = vreinterpretq_u32_f32(rf);
		uint32x4_t gf_bits = vreinterpretq_u32_f32(gf);
		uint32x4_t bf_bits = vreinterpretq_u32_f32(bf);

		int32x4_t r_msb = vsubq_s32(vreinterpretq_s32_u32(vshrq_n_u32(rf_bits, 23)), bias_127);
		int32x4_t g_msb = vsubq_s32(vreinterpretq_s32_u32(vshrq_n_u32(gf_bits, 23)), bias_127);
		int32x4_t b_msb = vsubq_s32(vreinterpretq_s32_u32(vshrq_n_u32(bf_bits, 23)), bias_127);

		// Final exponent: msb + e - 121
		int32x4_t r_exp = vsubq_s32(vaddq_s32(r_msb, ei), bias_121);
		int32x4_t g_exp = vsubq_s32(vaddq_s32(g_msb, ei), bias_121);
		int32x4_t b_exp = vsubq_s32(vaddq_s32(b_msb, ei), bias_121);

		// Extract mantissa from float bits (already normalized)
		uint32x4_t r_man = vandq_u32(vshrq_n_u32(rf_bits, 17), mask_man6);
		uint32x4_t g_man = vandq_u32(vshrq_n_u32(gf_bits, 17), mask_man6);
		uint32x4_t b_man = vandq_u32(vshrq_n_u32(bf_bits, 18), mask_man5);

		// Handle overflow (exp > 30)
		uint32x4_t r_over = vcgtq_s32(r_exp, max_exp);
		uint32x4_t g_over = vcgtq_s32(g_exp, max_exp);
		uint32x4_t b_over = vcgtq_s32(b_exp, max_exp);

		// Clamp exponent to [0, 30]
		r_exp = vmaxq_s32(zero, vminq_s32(max_exp, r_exp));
		g_exp = vmaxq_s32(zero, vminq_s32(max_exp, g_exp));
		b_exp = vmaxq_s32(zero, vminq_s32(max_exp, b_exp));

		// Zero mantissa if exponent underflowed
		uint32x4_t r_valid = vcgtq_s32(vaddq_s32(r_msb, ei), bias_121);
		uint32x4_t g_valid = vcgtq_s32(vaddq_s32(g_msb, ei), bias_121);
		uint32x4_t b_valid = vcgtq_s32(vaddq_s32(b_msb, ei), bias_121);

		r_man = vandq_u32(r_man, r_valid);
		g_man = vandq_u32(g_man, g_valid);
		b_man = vandq_u32(b_man, b_valid);
		r_exp = vandq_s32(r_exp, vreinterpretq_s32_u32(r_valid));
		g_exp = vandq_s32(g_exp, vreinterpretq_s32_u32(g_valid));
		b_exp = vandq_s32(b_exp, vreinterpretq_s32_u32(b_valid));

		// Pack: R11 = (exp << 6) | man, etc.
		uint32x4_t r11 = vorrq_u32(vshlq_n_u32(vreinterpretq_u32_s32(r_exp), 6), r_man);
		uint32x4_t g11 = vorrq_u32(vshlq_n_u32(vreinterpretq_u32_s32(g_exp), 6), g_man);
		uint32x4_t b10 = vorrq_u32(vshlq_n_u32(vreinterpretq_u32_s32(b_exp), 5), b_man);

		// Apply overflow saturation
		r11 = vorrq_u32(vbicq_u32(r11, r_over), vandq_u32(r_over, max_r11));
		g11 = vorrq_u32(vbicq_u32(g11, g_over), vandq_u32(g_over, max_r11));
		b10 = vorrq_u32(vbicq_u32(b10, b_over), vandq_u32(b_over, max_b10));

		// Zero check: if original mantissa was 0, result should be 0
		uint32x4_t r_zero = vceqq_u32(ri, vdupq_n_u32(0));
		uint32x4_t g_zero = vceqq_u32(gi, vdupq_n_u32(0));
		uint32x4_t b_zero = vceqq_u32(bi, vdupq_n_u32(0));
		r11 = vbicq_u32(r11, r_zero);
		g11 = vbicq_u32(g11, g_zero);
		b10 = vbicq_u32(b10, b_zero);

		// Final pack
		uint32x4_t result = vorrq_u32(
			vorrq_u32(r11, vshlq_n_u32(g11, 11)),
			vshlq_n_u32(b10, 22)
		);

		vst1q_u32(out + i, result);
	}

	// Scalar tail
	for (; i < count; i++) {
		out[i] = _rgbe_to_rg11b10_scalar(r_in[i], g_in[i], b_in[i], e_in[i]);
	}
}

#define _rgbe_to_rg11b10_simd _rgbe_to_rg11b10_neon

#endif

///////////////////////////////////////////////////////////////////////////////
// Private - RLE decoding (planar output for SIMD conversion)
///////////////////////////////////////////////////////////////////////////////

// Decode RLE directly to planar channel buffers
static inline int32_t _hdr_decode_scanline_planar(
	const uint8_t **ptr, const uint8_t *end,
	uint8_t *r_out, uint8_t *g_out, uint8_t *b_out, uint8_t *e_out,
	int32_t width)
{
	const uint8_t *p = *ptr;

	if (p + 4 > end) return 0;
	uint8_t h0 = *p++, h1 = *p++, h2 = *p++, h3 = *p++;

	if (h0 != 2 || h1 != 2 || (h2 & 0x80)) {
		// Uncompressed - deinterleave
		r_out[0] = h0; g_out[0] = h1; b_out[0] = h2; e_out[0] = h3;
		int32_t remaining = (width - 1) * 4;
		if (p + remaining > end) return 0;
		for (int32_t i = 1; i < width; i++) {
			r_out[i] = *p++;
			g_out[i] = *p++;
			b_out[i] = *p++;
			e_out[i] = *p++;
		}
		*ptr = p;
		return 1;
	}

	int32_t encoded_width = (h2 << 8) | h3;
	if (encoded_width != width) return 0;

	// Decode directly into output buffers (HDR RLE is already planar!)
	uint8_t *channels[4] = { r_out, g_out, b_out, e_out };
	for (int32_t ch = 0; ch < 4; ch++) {
		int32_t i = 0;
		while (i < width) {
			if (p >= end) return 0;
			uint8_t c = *p++;
			if (c > 128) {
				// RLE run
				if (p >= end) return 0;
				uint8_t val = *p++;
				int32_t run = c - 128;
				int32_t to_fill = (i + run <= width) ? run : (width - i);
				memset(channels[ch] + i, val, to_fill);
				i += to_fill;
			} else {
				// Literal bytes
				int32_t count = c;
				if (p + count > end) return 0;
				int32_t to_copy = (i + count <= width) ? count : (width - i);
				memcpy(channels[ch] + i, p, to_copy);
				p += count;
				i += to_copy;
			}
		}
	}

	*ptr = p;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Public
///////////////////////////////////////////////////////////////////////////////

static inline hdr_header_t hdr_parse_header(const void *data, size_t size) {
	hdr_header_t   result = {0};
	const uint8_t *ptr    = (const uint8_t *)data;
	const uint8_t *end    = ptr + size;

	if (size < 11) return result;
	if      (memcmp(ptr, "#?RADIANCE", 10) == 0) ptr += 10;
	else if (memcmp(ptr, "#?RGBE",     6 ) == 0) ptr += 6;
	else return result;

	ptr = _hdr_find_line_end(ptr, end);
	if (!ptr) return result;

	while (ptr < end) {
		if (*ptr == '\n') { ptr++; break; }
		ptr = _hdr_find_line_end(ptr, end);
		if (!ptr) return result;
	}

	const uint8_t *line_start = ptr;
	ptr = _hdr_find_line_end(ptr, end);
	if (!ptr) return result;

	int32_t line_len = (int32_t)(ptr - line_start - 1);
	if (line_len <= 0 || line_len > 64) return result;

	char line[65];
	memcpy(line, line_start, line_len);
	line[line_len] = '\0';

	const char *p = line;
	if      (p[0] == '-' && p[1] == 'Y' && p[2] == ' ') p += 3;
	else if (p[0] == '+' && p[1] == 'Y' && p[2] == ' ') p += 3;
	else return result;
	p = _hdr_parse_int(p, &result.height);
	if (!p || !(p[0] == ' ' && p[1] == '+' && p[2] == 'X' && p[3] == ' ')) return result;
	p = _hdr_parse_int(p + 4, &result.width);
	if (!p) return result;

	if (result.width <= 0 || result.height <= 0 || result.width > 32768 || result.height > 32768) {
		result.width = result.height = 0;
		return result;
	}

	result.data_offset = (int32_t)(ptr - (const uint8_t *)data);
	result.valid       = 1;
	return result;
}

static inline hdr_image_t hdr_decode_pixels_rgb9e5(const void *data, size_t size, const hdr_header_t *header) {
	hdr_image_t result = {0};
	if (!header->valid) return result;

	const uint8_t *ptr = (const uint8_t *)data + header->data_offset;
	const uint8_t *end = (const uint8_t *)data + size;
	int32_t width  = header->width;
	int32_t height = header->height;

	result.width  = width;
	result.height = height;
	result.pixels = (uint32_t *)malloc(width * height * sizeof(uint32_t));
	if (!result.pixels) { hdr_image_t empty = {0}; return empty; }

#if defined(HDR_SIMD_SSE4) || defined(HDR_SIMD_NEON)
	uint8_t *r_buf = (uint8_t *)malloc(width);
	uint8_t *g_buf = (uint8_t *)malloc(width);
	uint8_t *b_buf = (uint8_t *)malloc(width);
	uint8_t *e_buf = (uint8_t *)malloc(width);

	if (!r_buf || !g_buf || !b_buf || !e_buf) {
		free(r_buf); free(g_buf); free(b_buf); free(e_buf);
		free(result.pixels);
		hdr_image_t empty = {0}; return empty;
	}

	for (int32_t y = 0; y < height; y++) {
		if (!_hdr_decode_scanline_planar(&ptr, end, r_buf, g_buf, b_buf, e_buf, width)) {
			free(r_buf); free(g_buf); free(b_buf); free(e_buf);
			free(result.pixels);
			hdr_image_t empty = {0}; return empty;
		}
		_rgbe_to_rgb9e5_simd(r_buf, g_buf, b_buf, e_buf, result.pixels + y * width, width);
	}

	free(r_buf); free(g_buf); free(b_buf); free(e_buf);
#else
	uint8_t *scanline = (uint8_t *)malloc(width * 4);
	if (!scanline) {
		free(result.pixels);
		hdr_image_t empty = {0}; return empty;
	}

	for (int32_t y = 0; y < height; y++) {
		const uint8_t *p = ptr;
		if (p + 4 > end) goto fail_rgb9e5;
		uint8_t h0 = *p++, h1 = *p++, h2 = *p++, h3 = *p++;

		if (h0 != 2 || h1 != 2 || (h2 & 0x80)) {
			scanline[0] = h0; scanline[1] = h1; scanline[2] = h2; scanline[3] = h3;
			int32_t remaining = (width - 1) * 4;
			if (p + remaining > end) goto fail_rgb9e5;
			memcpy(scanline + 4, p, remaining);
			ptr = p + remaining;
		} else {
			int32_t encoded_width = (h2 << 8) | h3;
			if (encoded_width != width) goto fail_rgb9e5;

			uint8_t *channels[4];
			for (int32_t ch = 0; ch < 4; ch++) {
				channels[ch] = (uint8_t *)malloc(width);
				int32_t i = 0;
				while (i < width) {
					if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rgb9e5; }
					uint8_t c = *p++;
					if (c > 128) {
						if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rgb9e5; }
						uint8_t val = *p++;
						int32_t run = c - 128;
						while (run-- && i < width) channels[ch][i++] = val;
					} else {
						int32_t count = c;
						if (p + count > end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rgb9e5; }
						while (count-- && i < width) channels[ch][i++] = *p++;
					}
				}
			}
			for (int32_t i = 0; i < width; i++) {
				scanline[i*4+0] = channels[0][i];
				scanline[i*4+1] = channels[1][i];
				scanline[i*4+2] = channels[2][i];
				scanline[i*4+3] = channels[3][i];
			}
			for (int32_t ch = 0; ch < 4; ch++) free(channels[ch]);
			ptr = p;
		}

		uint32_t *row = result.pixels + y * width;
		for (int32_t x = 0; x < width; x++) {
			uint8_t *px = scanline + x * 4;
			row[x] = _rgbe_to_rgb9e5_scalar(px[0], px[1], px[2], px[3]);
		}
	}

	free(scanline);
	goto done_rgb9e5;
fail_rgb9e5:
	free(scanline);
	free(result.pixels);
	hdr_image_t empty = {0}; return empty;
done_rgb9e5:;
#endif

	return result;
}

static inline hdr_image_t hdr_decode_pixels_rg11b10(const void *data, size_t size, const hdr_header_t *header) {
	hdr_image_t result = {0};
	if (!header->valid) return result;

	const uint8_t *ptr = (const uint8_t *)data + header->data_offset;
	const uint8_t *end = (const uint8_t *)data + size;
	int32_t width  = header->width;
	int32_t height = header->height;

	result.width  = width;
	result.height = height;
	result.pixels = (uint32_t *)malloc(width * height * sizeof(uint32_t));
	if (!result.pixels) { hdr_image_t empty = {0}; return empty; }

#if defined(HDR_SIMD_SSE4) || defined(HDR_SIMD_NEON)
	uint8_t *r_buf = (uint8_t *)malloc(width);
	uint8_t *g_buf = (uint8_t *)malloc(width);
	uint8_t *b_buf = (uint8_t *)malloc(width);
	uint8_t *e_buf = (uint8_t *)malloc(width);

	if (!r_buf || !g_buf || !b_buf || !e_buf) {
		free(r_buf); free(g_buf); free(b_buf); free(e_buf);
		free(result.pixels);
		hdr_image_t empty = {0}; return empty;
	}

	for (int32_t y = 0; y < height; y++) {
		if (!_hdr_decode_scanline_planar(&ptr, end, r_buf, g_buf, b_buf, e_buf, width)) {
			free(r_buf); free(g_buf); free(b_buf); free(e_buf);
			free(result.pixels);
			hdr_image_t empty = {0}; return empty;
		}
		_rgbe_to_rg11b10_simd(r_buf, g_buf, b_buf, e_buf, result.pixels + y * width, width);
	}

	free(r_buf); free(g_buf); free(b_buf); free(e_buf);
#else
	uint8_t *scanline = (uint8_t *)malloc(width * 4);
	if (!scanline) {
		free(result.pixels);
		hdr_image_t empty = {0}; return empty;
	}

	for (int32_t y = 0; y < height; y++) {
		const uint8_t *p = ptr;
		if (p + 4 > end) goto fail_rg11b10;
		uint8_t h0 = *p++, h1 = *p++, h2 = *p++, h3 = *p++;

		if (h0 != 2 || h1 != 2 || (h2 & 0x80)) {
			scanline[0] = h0; scanline[1] = h1; scanline[2] = h2; scanline[3] = h3;
			int32_t remaining = (width - 1) * 4;
			if (p + remaining > end) goto fail_rg11b10;
			memcpy(scanline + 4, p, remaining);
			ptr = p + remaining;
		} else {
			int32_t encoded_width = (h2 << 8) | h3;
			if (encoded_width != width) goto fail_rg11b10;

			uint8_t *channels[4];
			for (int32_t ch = 0; ch < 4; ch++) {
				channels[ch] = (uint8_t *)malloc(width);
				int32_t i = 0;
				while (i < width) {
					if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rg11b10; }
					uint8_t c = *p++;
					if (c > 128) {
						if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rg11b10; }
						uint8_t val = *p++;
						int32_t run = c - 128;
						while (run-- && i < width) channels[ch][i++] = val;
					} else {
						int32_t count = c;
						if (p + count > end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_rg11b10; }
						while (count-- && i < width) channels[ch][i++] = *p++;
					}
				}
			}
			for (int32_t i = 0; i < width; i++) {
				scanline[i*4+0] = channels[0][i];
				scanline[i*4+1] = channels[1][i];
				scanline[i*4+2] = channels[2][i];
				scanline[i*4+3] = channels[3][i];
			}
			for (int32_t ch = 0; ch < 4; ch++) free(channels[ch]);
			ptr = p;
		}

		uint32_t *row = result.pixels + y * width;
		for (int32_t x = 0; x < width; x++) {
			uint8_t *px = scanline + x * 4;
			row[x] = _rgbe_to_rg11b10_scalar(px[0], px[1], px[2], px[3]);
		}
	}

	free(scanline);
	goto done_rg11b10;
fail_rg11b10:
	free(scanline);
	free(result.pixels);
	hdr_image_t empty = {0}; return empty;
done_rg11b10:;
#endif

	return result;
}

static inline hdr_image_t hdr_load_from_memory_rgb9e5(const void *data, size_t size) {
	hdr_header_t header = hdr_parse_header(data, size);
	return hdr_decode_pixels_rgb9e5(data, size, &header);
}

static inline hdr_image_t hdr_load_from_memory_rg11b10(const void *data, size_t size) {
	hdr_header_t header = hdr_parse_header(data, size);
	return hdr_decode_pixels_rg11b10(data, size, &header);
}

static inline void hdr_image_free(hdr_image_t *img) {
	if (img && img->pixels) {
		free(img->pixels);
		img->pixels = NULL;
	}
}
