#include "stereokit.h"
#include "sk_math.h"
#include "sk_math_dx.h"

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
	return math_fast_to_quat(difference);
}

///////////////////////////////////////////

quat quat_lookat(const vec3 &from, const vec3 &at) {
	if (from.x == at.x && from.y == at.y && from.z == at.z)
		return quat_identity;

	XMMATRIX mat = XMMatrixLookAtRH(math_vec3_to_fast(from), math_vec3_to_fast(at), XMVectorSet(0, 1, 0, 0));
	return math_fast_to_quat(XMQuaternionRotationMatrix(XMMatrixTranspose(mat)));
}

///////////////////////////////////////////

quat quat_lookat_up(const vec3 &from, const vec3 &at, const vec3 &up) {
	if (from.x == at.x && from.y == at.y && from.z == at.z)
		return quat_identity;

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

vec4 quat_to_axis_angle(quat a) {
	float    angle;
	XMVECTOR xaxis;
	XMQuaternionToAxisAngle(&xaxis, &angle, math_quat_to_fast(a));

	vec4 result = math_fast_to_vec4(xaxis);
	result.w = angle * rad2deg;
	return result;
}

///////////////////////////////////////////

quat quat_axis_angle(vec3 axis, float angle_deg) {
	return math_fast_to_quat(XMQuaternionRotationAxis(math_vec3_to_fast(axis), angle_deg*deg2rad));
}

///////////////////////////////////////////

quat quat_normal_angle(vec3 normalized_axis, float angle_deg) {
	return math_fast_to_quat(XMQuaternionRotationNormal(math_vec3_to_fast(normalized_axis), angle_deg*deg2rad));
}

///////////////////////////////////////////

// See: https://stackoverflow.com/a/22401169/10813424
// and its reference: http://www.euclideanspace.com/maths/geometry/rotations/for/decomposition/
void quat_decompose_swing_twist(quat rotation, vec3 direction, quat *out_swing, quat *out_twist) {
	vec3 axis  = { rotation.x, rotation.y, rotation.z }; // rotation axis
	vec3 p     = vec3_project  ( axis, direction ); // return projection v1 on to v2  (parallel component)
	*out_twist = quat_normalize( { p.x, p.y, p.z, rotation.w } );
	*out_swing = rotation * quat{-out_twist->x, -out_twist->y, -out_twist->z, out_twist->w}; // twist.conjugated();
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
	if (render_get_projection() == projection_perspective) {
		out_ray.pos = input_head()->position;
		out_ray.dir = vec3{screen_pixel_pos.x, screen_pixel_pos.y, 1.0f};
		out_ray.dir = render_unproject_pt(out_ray.dir) - out_ray.pos;
		out_ray.dir = vec3_normalize(out_ray.dir);
	} else {
		out_ray.pos = render_unproject_pt(vec3{screen_pixel_pos.x, screen_pixel_pos.y, 0});
		out_ray.dir = input_head()->orientation * vec3_forward;
	}
	return true;
}

///////////////////////////////////////////

void matrix_inverse(const matrix &a, matrix &out_matrix) {
	XMMATRIX mat_a, mat_result;
	math_matrix_to_fast(a, &mat_a);
	mat_result = XMMatrixInverse(nullptr, mat_a);
	math_fast_to_matrix(mat_result, &out_matrix);
}

///////////////////////////////////////////

matrix matrix_invert(const matrix &a) {
	XMMATRIX mat_a, mat_result;
	math_matrix_to_fast(a, &mat_a);
	mat_result = XMMatrixInverse(nullptr, mat_a);
	matrix result;
	math_fast_to_matrix(mat_result, &result);
	return result;
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

vec4 matrix_mul_point4(const matrix &transform, const vec4 &point) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec4( XMVector4Transform(math_vec4_to_fast(point), mat) );
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
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);
	XMVECTOR orient = math_quat_to_fast(orientation);
	return math_fast_to_quat(XMQuaternionMultiply(orient, rot));
}

///////////////////////////////////////////

pose_t matrix_mul_pose(const matrix& transform, const pose_t& pose) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);
	XMVECTOR orient = math_quat_to_fast(pose.orientation);
	return pose_t{
		math_fast_to_vec3(XMVector3Transform(math_vec3_to_fast(pose.position), mat)),
		math_fast_to_quat(XMQuaternionMultiply(orient, rot)) };
}

///////////////////////////////////////////

vec3 matrix_transform_pt(matrix transform, vec3 point) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3Transform(math_vec3_to_fast(point), mat) );
}

///////////////////////////////////////////

vec4 matrix_transform_pt4(matrix transform, vec4 point) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec4( XMVector4Transform(math_vec4_to_fast(point), mat) );
}

///////////////////////////////////////////

vec3 matrix_transform_dir (matrix transform, vec3 direction) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(direction), mat) );
}

///////////////////////////////////////////

ray_t matrix_transform_ray (matrix transform, ray_t ray) {
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	return ray_t {
		math_fast_to_vec3( XMVector3Transform      (math_vec3_to_fast(ray.pos), mat) ),
		math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(ray.dir), mat) )
	};
}

///////////////////////////////////////////

quat matrix_transform_quat(matrix transform, quat rotation) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);
	XMVECTOR orient = math_quat_to_fast(rotation);
	return math_fast_to_quat(XMQuaternionMultiply(orient, rot));
}

///////////////////////////////////////////

pose_t matrix_transform_pose(matrix transform, pose_t pose) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);
	XMVECTOR orient = math_quat_to_fast(pose.orientation);
	return pose_t{
		math_fast_to_vec3(XMVector3Transform(math_vec3_to_fast(pose.position), mat)),
		math_fast_to_quat(XMQuaternionMultiply(orient, rot)) };
}

///////////////////////////////////////////

matrix matrix_transpose(matrix transform) {
	return matrix{
		vec4{transform.row[0].x, transform.row[1].x, transform.row[2].x, transform.row[3].x},
		vec4{transform.row[0].y, transform.row[1].y, transform.row[2].y, transform.row[3].y},
		vec4{transform.row[0].z, transform.row[1].z, transform.row[2].z, transform.row[3].z},
		vec4{transform.row[0].w, transform.row[1].w, transform.row[2].w, transform.row[3].w} };
}

///////////////////////////////////////////

vec3 matrix_to_angles(const matrix &transform) {
	// see: https://stackoverflow.com/questions/1996957/conversion-euler-to-matrix-and-matrix-to-euler
	vec3 result;
	result.x = asinf(-transform.m[9]); // _32
	if (cosf(result.x) > 0.0001f) {
		result.y = atan2f(transform.m[8], transform.m[10]); // _31, _33
		result.z = atan2f(transform.m[1], transform.m[5] ); // _12, _22
	} else {
		result.y = 0.0f;
		result.z = atan2f(-transform.m[4], transform.m[0]); // _21, _11
	}
	return result * rad2deg;
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

matrix matrix_t(vec3 position) {
	return {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		position.x, position.y, position.z, 1 };
}

///////////////////////////////////////////

matrix matrix_r(quat orientation) {
	XMMATRIX mat = XMMatrixRotationQuaternion(XMLoadFloat4((XMFLOAT4*)&orientation));
	matrix result;
	math_fast_to_matrix(mat, &result);
	return result;
}

///////////////////////////////////////////

matrix matrix_s(vec3 scale) {
	return {
		scale.x,0,0,0,
		0,scale.y,0,0,
		0,0,scale.z,0,
		0,0,0,1 };
}

///////////////////////////////////////////

matrix matrix_ts(vec3 position, vec3 scale) {
	return {
		scale.x,0,0,0,
		0,scale.y,0,0,
		0,0,scale.z,0,
		position.x, position.y, position.z, 1 };
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

matrix matrix_perspective(float fov_degrees, float aspect_ratio, float near_clip, float far_clip) {
	matrix result;
	math_fast_to_matrix(XMMatrixPerspectiveFovRH(
		fov_degrees * deg2rad, aspect_ratio,
		near_clip, far_clip), &result);
	return result;
}

///////////////////////////////////////////

matrix matrix_orthographic(float width, float height, float near_clip, float far_clip) {
	matrix result;
	math_fast_to_matrix(XMMatrixOrthographicRH(
		width, height,
		near_clip, far_clip), &result);
	return result;
}

///////////////////////////////////////////

bool32_t matrix_decompose(const matrix &transform, vec3 &out_position, vec3 &out_scale, quat &out_orientation) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	bool result = XMMatrixDecompose(&scale, &rot, &pos, mat);

	out_position    = math_fast_to_vec3(pos);
	out_scale       = math_fast_to_vec3(scale);
	out_orientation = math_fast_to_quat(rot);
	return result;
}

///////////////////////////////////////////

vec3 matrix_extract_translation(const matrix &transform) {
	// Last row of the matrix
	return vec3{ transform.m[12], transform.m[13], transform.m[14] }; 
}

///////////////////////////////////////////

vec3 matrix_extract_scale(const matrix &transform) {
	return vec3{
		XMVectorGetX(XMVector3Length(XMLoadFloat4((DirectX::XMFLOAT4 *)&transform.row[0]))),
		XMVectorGetX(XMVector3Length(XMLoadFloat4((DirectX::XMFLOAT4 *)&transform.row[1]))),
		XMVectorGetX(XMVector3Length(XMLoadFloat4((DirectX::XMFLOAT4 *)&transform.row[2]))) };
}

///////////////////////////////////////////

quat matrix_extract_rotation(const matrix &transform) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);

	return math_fast_to_quat(rot);
}

///////////////////////////////////////////

pose_t matrix_extract_pose(const matrix &transform) {
	XMVECTOR pos, scale, rot;
	XMMATRIX mat;
	math_matrix_to_fast(transform, &mat);
	XMMatrixDecompose(&scale, &rot, &pos, mat);

	return pose_t {
		math_fast_to_vec3(pos),
		math_fast_to_quat(rot) };
}

///////////////////////////////////////////

plane_t plane_from_points(vec3 p1, vec3 p2, vec3 p3) {
	vec3 dir1   = p2 - p1;
	vec3 dir2   = p2 - p3;
	vec3 normal = vec3_cross(dir1, dir2);
	return { normal, -vec3_dot(p2, normal) };
}

///////////////////////////////////////////

plane_t plane_from_ray(ray_t ray) {
	return { ray.dir, -vec3_dot(ray.pos, ray.dir) };
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