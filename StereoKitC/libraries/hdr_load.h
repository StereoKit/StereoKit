// load_hdr.h - Radiance HDR image loader with SIMD optimization
//
// Loads .hdr (RGBE) files from memory and converts to RGB9E5 format.
// Supports both RLE-compressed and uncompressed scanlines.
// Automatically uses SSE4.1 (x64) or NEON (ARM64), with scalar fallback.
//
// Usage:
//   // Query dimensions without decoding
//   hdr_header_t header = hdr_parse_header(data, size);
//   if (header.valid) printf("%dx%d\n", header.width, header.height);
//
//   // Decode pixels using header
//   hdr_image_t img = hdr_decode_pixels(data, size, &header);
//
//   // Or load in one step
//   hdr_image_t img = hdr_load_from_memory(data, size);
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

static inline hdr_header_t hdr_parse_header(const void *data, int32_t size) {
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

static inline hdr_image_t hdr_decode_pixels(const void *data, int32_t size, const hdr_header_t *header) {
	hdr_image_t result = {0};
	if (!header->valid) return result;

	const uint8_t *ptr = (const uint8_t *)data + header->data_offset;
	const uint8_t *end = (const uint8_t *)data + size;
	int32_t width  = header->width;
	int32_t height = header->height;

	result.width  = width;
	result.height = height;
	result.pixels = (uint32_t *)malloc(width * height * sizeof(uint32_t));
	if (!result.pixels) return (hdr_image_t){0};

#if defined(HDR_SIMD_SSE4) || defined(HDR_SIMD_NEON)
	// Allocate channel buffers once for entire image
	uint8_t *r_buf = (uint8_t *)malloc(width);
	uint8_t *g_buf = (uint8_t *)malloc(width);
	uint8_t *b_buf = (uint8_t *)malloc(width);
	uint8_t *e_buf = (uint8_t *)malloc(width);

	if (!r_buf || !g_buf || !b_buf || !e_buf) {
		free(r_buf); free(g_buf); free(b_buf); free(e_buf);
		free(result.pixels);
		return (hdr_image_t){0};
	}

	for (int32_t y = 0; y < height; y++) {
		if (!_hdr_decode_scanline_planar(&ptr, end, r_buf, g_buf, b_buf, e_buf, width)) {
			free(r_buf); free(g_buf); free(b_buf); free(e_buf);
			free(result.pixels);
			return (hdr_image_t){0};
		}
		_rgbe_to_rgb9e5_simd(r_buf, g_buf, b_buf, e_buf, result.pixels + y * width, width);
	}

	free(r_buf); free(g_buf); free(b_buf); free(e_buf);
#else
	// Scalar fallback
	uint8_t *scanline = (uint8_t *)malloc(width * 4);
	if (!scanline) {
		free(result.pixels);
		return (hdr_image_t){0};
	}

	for (int32_t y = 0; y < height; y++) {
		// Need interleaved decode for scalar path
		const uint8_t *p = ptr;
		if (p + 4 > end) goto fail_scalar;
		uint8_t h0 = *p++, h1 = *p++, h2 = *p++, h3 = *p++;

		if (h0 != 2 || h1 != 2 || (h2 & 0x80)) {
			scanline[0] = h0; scanline[1] = h1; scanline[2] = h2; scanline[3] = h3;
			int32_t remaining = (width - 1) * 4;
			if (p + remaining > end) goto fail_scalar;
			memcpy(scanline + 4, p, remaining);
			ptr = p + remaining;
		} else {
			int32_t encoded_width = (h2 << 8) | h3;
			if (encoded_width != width) goto fail_scalar;

			uint8_t *channels[4];
			for (int32_t ch = 0; ch < 4; ch++) {
				channels[ch] = (uint8_t *)malloc(width);
				int32_t i = 0;
				while (i < width) {
					if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_scalar; }
					uint8_t c = *p++;
					if (c > 128) {
						if (p >= end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_scalar; }
						uint8_t val = *p++;
						int32_t run = c - 128;
						while (run-- && i < width) channels[ch][i++] = val;
					} else {
						int32_t count = c;
						if (p + count > end) { for (int32_t j = 0; j <= ch; j++) free(channels[j]); goto fail_scalar; }
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
	goto done_scalar;
fail_scalar:
	free(scanline);
	free(result.pixels);
	return (hdr_image_t){0};
done_scalar:;
#endif

	return result;
}

static inline hdr_image_t hdr_load_from_memory(const void *data, int32_t size) {
	hdr_header_t header = hdr_parse_header(data, size);
	return hdr_decode_pixels(data, size, &header);
}

static inline void hdr_image_free(hdr_image_t *img) {
	if (img && img->pixels) {
		free(img->pixels);
		img->pixels = NULL;
	}
}
