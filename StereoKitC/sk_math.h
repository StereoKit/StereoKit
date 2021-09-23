#pragma once

#include "stereokit.h"

#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif

#define MATH_PI 3.1415926535898f

namespace sk {
	
void matrix_mul(const matrix &a, const matrix &b, DirectX::XMMATRIX &out_matrix);
void matrix_mul(const matrix &a, const DirectX::XMMATRIX &b, DirectX::XMMATRIX &out_matrix);
vec3 matrix_mul_direction(const DirectX::XMMATRIX &transform, const vec3 &direction);

///////////////////////////////////////////

inline DirectX::XMVECTOR math_vec3_to_fast(const vec3 &vector) {
	return DirectX::XMLoadFloat3((DirectX::XMFLOAT3 *)&vector); 
}

///////////////////////////////////////////

inline DirectX::XMVECTOR math_vec4_to_fast(const vec4 &vector) {
	return DirectX::XMLoadFloat4((DirectX::XMFLOAT4 *)&vector); 
}

///////////////////////////////////////////

inline DirectX::XMVECTOR math_quat_to_fast(const quat &quaternion) {
	return DirectX::XMLoadFloat4((DirectX::XMFLOAT4 *)&quaternion); 
}

///////////////////////////////////////////

inline vec3 math_fast_to_vec3(const DirectX::XMVECTOR &a) {
	vec3 result; 
	DirectX::XMStoreFloat3((DirectX::XMFLOAT3 *)&result, a);
	return result;
}

///////////////////////////////////////////

inline vec4 math_fast_to_vec4(const DirectX::XMVECTOR &a) {
	vec4 result; 
	DirectX::XMStoreFloat4((DirectX::XMFLOAT4 *)&result, a);
	return result;
}

///////////////////////////////////////////

inline quat math_fast_to_quat(const DirectX::XMVECTOR &a) {
	quat result; 
	DirectX::XMStoreFloat4((DirectX::XMFLOAT4 *)&result, a);
	return result;
}

///////////////////////////////////////////

inline void math_matrix_to_fast(const matrix &mat, DirectX::XMMATRIX *out_matrix) {
	*out_matrix = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4 *)&mat.row);
}

///////////////////////////////////////////

inline void math_fast_to_matrix(const DirectX::XMMATRIX &mat, matrix *out_matrix) {
	DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)out_matrix, mat);
}

///////////////////////////////////////////

inline vec3 matrix_mul_point(const DirectX::XMMATRIX &transform, vec3 point){
	return math_fast_to_vec3( XMVector3Transform(math_vec3_to_fast(point), transform) );
}

///////////////////////////////////////////

inline int32_t  maxi(int32_t  a, int32_t  b) { return a > b ? a : b; }
inline uint32_t maxi(uint32_t a, uint32_t b) { return a > b ? a : b; }
inline int64_t  maxi(int64_t  a, int64_t  b) { return a > b ? a : b; }
inline uint64_t maxi(uint64_t a, uint64_t b) { return a > b ? a : b; }
inline int32_t  mini(int32_t  a, int32_t  b) { return a < b ? a : b; }
inline uint32_t mini(uint32_t a, uint32_t b) { return a < b ? a : b; }
inline int64_t  mini(int64_t  a, int64_t  b) { return a < b ? a : b; }
inline uint64_t mini(uint64_t a, uint64_t b) { return a < b ? a : b; }

inline float math_lerp    (float a, float b, float t) { return a + (b - a) * t; }
inline float math_lerp_cl (float a, float b, float t) { return a + (b - a) * fminf(1,t); }
inline float math_saturate(float x)                   { return fmaxf(0, fminf(1, x)); }

inline float math_ease_overshoot(float a, float b, float overshoot, float t) { t = 1 - t; return math_lerp(a,b, 1-(t*t * ((overshoot + 1) * t - overshoot))); }
inline float math_ease_hop      (float a, float peak, float t) { return a+(peak-a)*sinf(t*MATH_PI); }

vec3 bounds_corner (const bounds_t &bounds, int32_t index8);
vec3 math_cubemap_corner(int i);

} // namespace sk