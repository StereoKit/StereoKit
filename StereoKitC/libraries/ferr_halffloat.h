#pragma once

// Tools for working with IEEE-754 half precision floating point values, used
// by GPU color formats like r16b16g16a16

// On some compilers this will require enabling some SIMD features
// CMake: add_compile_options(-mavx -mf16c)

#include <stdint.h>

#if defined(_M_AMD64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
	#define FHF_ARCH_X86
	#include <immintrin.h>
#elif (defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)) && defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
	// Note: ARM64 has native f16 support in ARMv8.2+, older ARM requires software
	// conversion, which is why we also guard on FP16_VECTOR_ARITHMETIC
	#define FHF_ARCH_ARM
	#include <arm_neon.h>
#else
	#define FHF_ARCH_SOFTWARE
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined(FHF_ARCH_X86)

///////////////////////////////////////////////////////////////////////////////

inline float fhf_f16_to_f32_x1(uint16_t f) {
	// 1/8th utilization of this SIMD operation, extremely unoptimal!
	__m128i halfs  = _mm_cvtsi32_si128(f);
	__m128  floats = _mm_cvtph_ps(halfs);
	return _mm_cvtss_f32(floats);
}

inline void fhf_f16_to_f32_x4(const uint16_t* f_x4, float* out_f_x4) {
	// 4/8th utilization of this SIMD operation, unoptimal!
	__m128i halfs  = _mm_loadl_epi64((__m128i*)f_x4);
	__m128  floats = _mm_cvtph_ps(halfs);
	_mm_storeu_ps(out_f_x4, floats);
}

inline void fhf_f16_to_f32_x8(const uint16_t* f_x8, float* out_f_x8) {
	// Full utilization
	__m128i halfs  = _mm_loadu_si128((__m128i*)f_x8);
	__m256  floats = _mm256_cvtph_ps(halfs);
	_mm256_storeu_ps(out_f_x8, floats);
}

///////////////////////////////////////////////////////////////////////////////

inline uint16_t fhf_f32_to_f16_x1(float f) {
	// 1/4th utilization of this SIMD operation, unoptimal!
	__m128  floats = _mm_set_ss(f);
	__m128i halfs  = _mm_cvtps_ph(floats, 0);
	return (uint16_t)_mm_cvtsi128_si32(halfs);
}

inline void fhf_f32_to_f16_x4(const float* f_x4, uint16_t* out_f_x4) {
	__m128  floats = _mm_loadu_ps(f_x4);
	__m128i halfs  = _mm_cvtps_ph(floats, 0);
	_mm_storel_epi64((__m128i*)out_f_x4, halfs);
}

inline void fhf_f32_to_f16_x8(const float* f_x8, uint16_t* out_f_x8) {
	__m256  floats = _mm256_loadu_ps(f_x8);
	__m128i halfs  = _mm256_cvtps_ph(floats, 0);
	_mm_storeu_si128((__m128i*)out_f_x8, halfs);
}

///////////////////////////////////////////////////////////////////////////////

#elif defined(FHF_ARCH_ARM)

///////////////////////////////////////////////////////////////////////////////

inline float fhf_f16_to_f32_x1(uint16_t f) {
	// Native hardware support
	__fp16 h = *(__fp16*)&f;
	return (float)h;
}

inline void fhf_f16_to_f32_x4(const uint16_t* f_x4, float* out_f_x4) {
	float16x4_t halfs  = vld1_f16((const __fp16*)f_x4);
	float32x4_t floats = vcvt_f32_f16(halfs);
	vst1q_f32(out_f_x4, floats);
}

inline void fhf_f16_to_f32_x8(const uint16_t* f_x8, float* out_f_x8) {
	float16x8_t halfs    = vld1q_f16((const __fp16*)f_x8);
	float32x4_t floats_0 = vcvt_f32_f16(vget_low_f16(halfs));
	float32x4_t floats_1 = vcvt_f32_f16(vget_high_f16(halfs));
	vst1q_f32(out_f_x8 + 0, floats_0);
	vst1q_f32(out_f_x8 + 4, floats_1);
}

///////////////////////////////////////////////////////////////////////////////

inline uint16_t fhf_f32_to_f16_x1(float f) {
	__fp16 h = (__fp16)f;
	return *(uint16_t*)&h;
}

inline void fhf_f32_to_f16_x4(const float* f_x4, uint16_t* out_f_x4) {
	float32x4_t floats = vld1q_f32(f_x4);
	float16x4_t halfs = vcvt_f16_f32(floats);
	vst1_f16((__fp16*)out_f_x4, halfs);
}

inline void fhf_f32_to_f16_x8(const float* f_x8, uint16_t* out_f_x8) {
	float32x4_t floats_0 = vld1q_f32(f_x8 + 0);
	float32x4_t floats_1 = vld1q_f32(f_x8 + 4);
	float16x4_t halfs_0 = vcvt_f16_f32(floats_0);
	float16x4_t halfs_1 = vcvt_f16_f32(floats_1);
	float16x8_t halfs = vcombine_f16(halfs_0, halfs_1);
	vst1q_f16((__fp16*)out_f_x8, halfs);
}

///////////////////////////////////////////////////////////////////////////////

#elif defined(FHF_ARCH_SOFTWARE) // No native f16 support, use software conversion

///////////////////////////////////////////////////////////////////////////////

inline float fhf_f16_to_f32_x1(uint16_t h) {
	// Software conversion
	union { uint32_t i; float f; } v;
	uint32_t sign     = (h & 0x8000) << 16;
	uint32_t exponent = (h & 0x7C00) >> 10;
	uint32_t mantissa = (h & 0x03FF);

	if      (exponent == 0)  { v.i = sign; } // +- 0
	else if (exponent == 31) { v.i = sign | 0x7F800000 | (mantissa << 13); } // Inf/NaN
	else                     { v.i = sign | ((exponent + 112) << 23) | (mantissa << 13); }
	return v.f;
}

inline void fhf_f16_to_f32_x4(const uint16_t* f_x4, float* out_f_x4) {
	// Unrolled software conversion
	out_f_x4[0] = fhf_f16_to_f32_x1(f_x4[0]);
	out_f_x4[1] = fhf_f16_to_f32_x1(f_x4[1]);
	out_f_x4[2] = fhf_f16_to_f32_x1(f_x4[2]);
	out_f_x4[3] = fhf_f16_to_f32_x1(f_x4[3]);
}

inline void fhf_f16_to_f32_x8(const uint16_t* f_x8, float* out_f_x8) {
	// Unrolled software conversion
	out_f_x8[0] = fhf_f16_to_f32_x1(f_x8[0]);
	out_f_x8[1] = fhf_f16_to_f32_x1(f_x8[1]);
	out_f_x8[2] = fhf_f16_to_f32_x1(f_x8[2]);
	out_f_x8[3] = fhf_f16_to_f32_x1(f_x8[3]);
	out_f_x8[4] = fhf_f16_to_f32_x1(f_x8[4]);
	out_f_x8[5] = fhf_f16_to_f32_x1(f_x8[5]);
	out_f_x8[6] = fhf_f16_to_f32_x1(f_x8[6]);
	out_f_x8[7] = fhf_f16_to_f32_x1(f_x8[7]);
}

///////////////////////////////////////////////////////////////////////////////

inline uint16_t fhf_f32_to_f16_x1(float f) {
	// Software conversion
	union { float f; uint32_t i; } v;
	v.f = f;
	uint32_t sign     =  (v.i >> 16) & 0x8000;
	uint32_t exponent = ((v.i >> 23) & 0xFF);
	uint32_t mantissa = v.i & 0x7FFFFF;

	if      (exponent == 0)   { return sign;                               } // +- 0
	else if (exponent == 255) { return sign | 0x7C00 | (mantissa ? 1 : 0); } // Inf/NaN
	else if (exponent <  113) { return sign;                               } // Underflow to +-0
	else if (exponent >  142) { return sign | 0x7C00;                      } // Overflow to +-Inf
	else { return sign | ((exponent - 112) << 10) | (mantissa >> 13); }
}

inline void fhf_f32_to_f16_x4(const float* f_x4, uint16_t* out_f_x4) {
	// Unrolled software conversion
	out_f_x4[0] = fhf_f32_to_f16_x1(f_x4[0]);
	out_f_x4[1] = fhf_f32_to_f16_x1(f_x4[1]);
	out_f_x4[2] = fhf_f32_to_f16_x1(f_x4[2]);
	out_f_x4[3] = fhf_f32_to_f16_x1(f_x4[3]);
}

inline void fhf_f32_to_f16_x8(const float* f_x8, uint16_t* out_f_x8) {
	// Unrolled software conversion
	out_f_x8[0] = fhf_f32_to_f16_x1(f_x8[0]);
	out_f_x8[1] = fhf_f32_to_f16_x1(f_x8[1]);
	out_f_x8[2] = fhf_f32_to_f16_x1(f_x8[2]);
	out_f_x8[3] = fhf_f32_to_f16_x1(f_x8[3]);
	out_f_x8[4] = fhf_f32_to_f16_x1(f_x8[4]);
	out_f_x8[5] = fhf_f32_to_f16_x1(f_x8[5]);
	out_f_x8[6] = fhf_f32_to_f16_x1(f_x8[6]);
	out_f_x8[7] = fhf_f32_to_f16_x1(f_x8[7]);
}

///////////////////////////////////////////////////////////////////////////////

#endif // Architecture selection