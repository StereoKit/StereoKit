#include "stereokit.h"
#include "math.h"

#include "systems/render.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;

namespace sk {

///////////////////////////////////////////

vec3 vec3_cross(const vec3 &a, const vec3 &b) {
	return math_fast_to_vec3( XMVector3Cross(math_vec3_to_fast(a), math_vec3_to_fast(b)) );
}

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
	XMVECTOR difference = XMQuaternionMultiply(inv, XMLoadFloat4((XMFLOAT4 *)& b));
	//difference = XMQuaternionNormalize(difference);
	return math_fast_to_quat(difference);
}

///////////////////////////////////////////

quat quat_lookat(const vec3 &from, const vec3 &at) {
	XMMATRIX mat = XMMatrixLookAtRH(math_vec3_to_fast(from), math_vec3_to_fast(at), XMVectorSet(0, 1, 0, 0));
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_lookat_up(const vec3 &from, const vec3 &at, const vec3 &up) {
	XMMATRIX mat = XMMatrixLookAtRH(math_vec3_to_fast(from), math_vec3_to_fast(at), math_vec3_to_fast(up));
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_from_angles(float pitch_x_deg, float yaw_y_deg, float roll_z_deg) {
	XMVECTOR result = XMQuaternionRotationRollPitchYaw(pitch_x_deg * deg2rad, yaw_y_deg * deg2rad, roll_z_deg * deg2rad);
	return math_fast_to_quat(result);
}

///////////////////////////////////////////

quat quat_slerp(const quat &a, const quat &b, float t) {
	XMVECTOR blend = XMQuaternionSlerp(XMLoadFloat4((XMFLOAT4 *)& a), XMLoadFloat4((XMFLOAT4 *)& b), t);
	return math_fast_to_quat(blend);
}

///////////////////////////////////////////

quat quat_normalize(const quat &a) {
	return math_fast_to_quat(XMQuaternionNormalize(math_quat_to_fast(a)));
}

///////////////////////////////////////////

quat quat_inverse(const quat &a) {
	return math_fast_to_quat(XMQuaternionInverse(math_quat_to_fast(a)));
}

///////////////////////////////////////////

void pose_matrix_out(const pose_t &pose, matrix &out_result, vec3 scale) {
	matrix_trs_out(out_result, pose.position, pose.orientation, scale);
}

///////////////////////////////////////////

matrix pose_matrix(const pose_t &pose, vec3 scale) {
	return matrix_trs(pose.position, pose.orientation, scale);
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

bool32_t ray_from_mouse(vec2 screen_pixel_pos, ray_t &out_ray) {
	float x = (((screen_pixel_pos.x / (float)sk_system_info().display_width ) - 0.5f) *  2.f);
	float y = (((screen_pixel_pos.y / (float)sk_system_info().display_height) - 0.5f) * -2.f);
	if (x >= -1 && y >= -1 && x <= 1 && y <= 1) {
		out_ray.pos = input_head()->position;
		out_ray.dir = vec3{ x, y, 1.0f };
		out_ray.dir = render_unproject_pt(out_ray.dir);
		out_ray.dir = vec3_normalize(out_ray.dir);

		return true;
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
	mat_result = XMMatrixMultiply(mat_a, mat_b);
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

void matrix_mul(const matrix &a, const DirectX::XMMATRIX &b, DirectX::XMMATRIX &out_matrix) {
	XMMATRIX mat_a;
	math_matrix_to_fast(a, &mat_a);
	out_matrix = mat_a * b;
}

///////////////////////////////////////////

vec3 matrix_mul_point(const matrix &transform, const vec3 &point) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3Transform(math_vec3_to_fast(point), mat) );
}

///////////////////////////////////////////

vec3 matrix_mul_point(const XMMATRIX &transform, const vec3 &point) {
	return math_fast_to_vec3( XMVector3Transform(math_vec3_to_fast(point), transform) );
}

///////////////////////////////////////////

vec3 matrix_mul_direction(const matrix &transform, const vec3 &direction) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(direction), mat) );
}

///////////////////////////////////////////

vec3 matrix_mul_direction(const XMMATRIX &transform, const vec3 &direction) {
	return math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(direction), transform) );
}

///////////////////////////////////////////

quat matrix_mul_rotation(const matrix& transform, const quat& orientation) {
	XMMATRIX mat = XMMatrixRotationQuaternion(math_quat_to_fast(orientation));
	XMMATRIX fast_transform;
	math_matrix_to_fast(transform, &fast_transform);
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixMultiply(mat, fast_transform)));
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

///////////////////////////////////////////

vec3 bounds_corner(const bounds_t &bounds, int32_t index8) {
	// Sign generates a pattern like this:
	// -1, -1, -1
	// -1, -1,  1
	// -1,  1, -1
	// -1,  1,  1
	//  1, -1, -1
	//  1, -1,  1
	//  1,  1, -1
	//  1,  1,  1
	vec3 sign = {
		(float)( (index8 / 4) * 2 - 1),
		(float)(((index8 / 2) % 2) * 2 - 1),
		(float)( (index8 % 2) * 2 - 1) };
	vec3 half = bounds.dimensions / 2;
	return bounds.center + half * sign;
}

///////////////////////////////////////////

vec3 math_cubemap_corner(int i) {
	float neg = (float)((i / 4) % 2 ? -1 : 1);
	int nx  = ((i+24) / 16) % 2;
	int ny  = (i / 8)       % 2;
	int nz  = (i / 16)      % 2;
	int u   = ((i+1) / 2)   % 2; // U: 0,1,1,0
	int v   = (i / 2)       % 2; // V: 0,0,1,1

	return {
		(nx ? neg : ny ? (u?-1:1)*neg : (u?1:-1)*neg),
		(nx || nz ? (v?1:-1) : neg),
		(nx ? (u?-1:1)*neg : ny ? (v?1:-1) : neg)
	};
}

} // namespace sk