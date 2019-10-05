#include "stereokit.h"
#include "math.h"

using namespace DirectX;

///////////////////////////////////////////

vec3 quat_mul_vec(const quat &a, const vec3 &b) {
	XMVECTOR rotation = XMVector3Rotate(XMLoadFloat3((XMFLOAT3*)&b), XMLoadFloat4((XMFLOAT4*)&a));
	return math_fast_to_vec3(rotation);
}

///////////////////////////////////////////

quat quat_mul(const quat &a, const quat &b) {
	XMVECTOR rotation = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4*)&a),XMLoadFloat4((XMFLOAT4*)&b));
	return math_fast_to_quat(rotation);
}

///////////////////////////////////////////

quat quat_difference(const quat &a, const quat &b) {
	XMVECTOR inv        = XMQuaternionInverse (XMLoadFloat4((XMFLOAT4 *)& a));
	XMVECTOR difference = XMQuaternionMultiply(XMLoadFloat4((XMFLOAT4 *)& b), inv);
	return math_fast_to_quat(difference);
}

///////////////////////////////////////////

quat quat_lookat(const vec3 &from, const vec3 &at) {
	XMMATRIX mat = XMMatrixLookAtRH(math_vec3_to_fast(from), math_vec3_to_fast(at), XMVectorSet(0, 1, 0, 0));
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_euler(const vec3 &euler_degrees) {
	XMVECTOR result = XMQuaternionRotationRollPitchYaw(euler_degrees.x * -deg2rad, euler_degrees.y * -deg2rad, euler_degrees.z * -deg2rad);
	return math_fast_to_quat(result);
}

///////////////////////////////////////////

quat quat_lerp(const quat &a, const quat &b, float t) {
	XMVECTOR blend = XMQuaternionSlerp(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b), t);
	return math_fast_to_quat(blend);
}

///////////////////////////////////////////

quat quat_normalize(const quat &a) {
	return math_fast_to_quat(XMQuaternionNormalize(math_quat_to_fast(a)));
}

///////////////////////////////////////////

void pose_matrix_out(const pose_t &pose, matrix &out_result) {
	matrix_trs_out(out_result, pose.position, pose.orientation);
}

///////////////////////////////////////////

matrix pose_matrix(const pose_t &pose) {
	return matrix_trs(pose.position, pose.orientation);
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
	XMMATRIX mat_a, mat_b;
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

///////////////////////////////////////////

void matrix_trs_out(matrix &out_result, const vec3 &position, const quat &orientation, const vec3 &scale) {
	XMMATRIX mat = XMMatrixAffineTransformation(
		XMLoadFloat3((XMFLOAT3 *)& scale), DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4 *)& orientation),
		XMLoadFloat3((XMFLOAT3 *)& position));
	math_fast_to_matrix(mat, &out_result);
}