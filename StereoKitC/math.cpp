#include "stereokit.h"
#include "math.h"

using namespace DirectX;

///////////////////////////////////////////

inline XMVECTOR to_fast3  (const vec3 &vec)   { return XMLoadFloat3((XMFLOAT3 *)& vec); }
inline vec3     from_fast3(const XMVECTOR &a) { vec3 result; XMStoreFloat3((XMFLOAT3 *)& result, a); return result; }
inline quat     from_fastq(const XMVECTOR &a) { quat result; XMStoreFloat4((XMFLOAT4 *)& result, a); return result; }

///////////////////////////////////////////

vec3 operator*(const quat &a, const vec3 &b) {
	XMVECTOR rotation = XMVector3Rotate(XMLoadFloat3((XMFLOAT3*)&b), XMLoadFloat4((XMFLOAT4*)&a));
	vec3 result;
	XMStoreFloat3((XMFLOAT3 *)&result, rotation);
	return result;
}

///////////////////////////////////////////

quat operator*(const quat &a, const quat &b) {
	XMVECTOR rotation = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4*)&a),XMLoadFloat4((XMFLOAT4*)&b));
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, rotation);
	return result;
}

///////////////////////////////////////////

quat quat_difference(const quat &a, const quat &b) {
	XMVECTOR inv = XMQuaternionInverse(XMLoadFloat4((XMFLOAT4 *)& a));
	XMVECTOR difference = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4 *)& b), inv);
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, difference);
	return result;
}

///////////////////////////////////////////

quat quat_lookat(const vec3 &from, const vec3 &at) {
	XMMATRIX mat = XMMatrixLookAtRH(to_fast3(from), to_fast3(at), XMVectorSet(0, 1, 0, 0));
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_lerp(const quat &a, const quat &b, float t) {
	XMVECTOR blend = XMQuaternionSlerp(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b), t);
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, blend);
	return result;
}

///////////////////////////////////////////

quat quat_mul(const quat &a, const quat &b) {
	XMVECTOR mul = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b));
	quat result;
	XMStoreFloat4((XMFLOAT4 *)&result, mul);
	return result;
}

///////////////////////////////////////////

void pose_matrix(const pose_t &pose, matrix &out_result) {
	XMMATRIX mat = XMMatrixAffineTransformation(
		DirectX::g_XMOne, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4*)&pose.orientation),
		XMLoadFloat3((XMFLOAT3*)&pose.position));
}

///////////////////////////////////////////

bool32_t ray_intersect_plane(ray_t ray, vec3 plane_pt, vec3 plane_normal, float &out_t) {
	float denom = vec3_dot(plane_normal, ray.dir); 
	if (denom > 1e-6) { 
		vec3 difference = plane_pt - ray.pos; 
		out_t = vec3_dot(difference, plane_normal) / denom; 
		return (out_t >  1e-6); 
	}
	return false; 
}

///////////////////////////////////////////

void matrix_inverse(const matrix &a, matrix &out_matrix) {
	XMMATRIX mat_a, mat_result;
	math_matrix_to_fast(a, &mat_a);
	mat_result = XMMatrixInverse(nullptr, mat_a);
	math_fast_to_matrix(mat_result, &out_matrix);
}

///////////////////////////////////////////

void matrix_mul(const matrix &a, const matrix &b, matrix &out_matrix) {
	XMMATRIX mat_a, mat_b, mat_result;
	math_matrix_to_fast(a, &mat_a);
	math_matrix_to_fast(b, &mat_b);
	mat_result = mat_a * mat_b;
	math_fast_to_matrix(mat_result, &out_matrix);
}

///////////////////////////////////////////

void matrix_mul(const matrix &a, const matrix &b, DirectX::XMMATRIX &out_matrix) {
	XMMATRIX mat_a, mat_b, mat_result;
	math_matrix_to_fast(a, &mat_a);
	math_matrix_to_fast(b, &mat_b);
	out_matrix = mat_a * mat_b;
}

///////////////////////////////////////////

vec3 matrix_mul_point(const matrix &transform, const vec3 &point) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3Transform(math_vec3_to_fast(point), mat) );
}

///////////////////////////////////////////

vec3 matrix_mul_direction(const matrix &transform, const vec3 &direction) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(direction), mat) );
}

///////////////////////////////////////////

matrix matrix_trs(const vec3 &position, const quat &orientation, const vec3 &scale) {
	XMMATRIX mat = XMMatrixAffineTransformation(
		XMLoadFloat3((XMFLOAT3 *)& scale), DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4 *)& orientation),
		XMLoadFloat3((XMFLOAT3 *)& position));
	matrix result;
	math_fast_to_matrix(mat, &result);
	return result;
}