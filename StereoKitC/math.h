#pragma once

#include "stereokit.h"
#include <DirectXMath.h>

namespace sk {

void matrix_mul(const matrix &a, const matrix &b, DirectX::XMMATRIX &out_matrix);

///////////////////////////////////////////

inline DirectX::XMVECTOR math_vec3_to_fast(const vec3 &vector) {
	return DirectX::XMLoadFloat3((DirectX::XMFLOAT3 *)&vector); 
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

} // namespace sk