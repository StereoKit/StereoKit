#include "stereokit.h"
#include "sk_math.h"
#include "sk_math_dx.h"

#include "systems/render.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;

namespace sk {

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

///////////////////////////////////////////
// vec3                                  //
///////////////////////////////////////////

vec3 vec3_cross(const vec3 &a, const vec3 &b) {
	return math_fast_to_vec3( XMVector3Cross(math_vec3_to_fast(a), math_vec3_to_fast(b)) );
}

///////////////////////////////////////////
// quat                                  //
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

void quat_to_axis_angle(quat a, vec3* out_axis, float* out_rotation_deg) {
	float    angle;
	XMVECTOR axis;
	XMQuaternionToAxisAngle(&axis, &angle, math_quat_to_fast(a));

	if (out_axis)         *out_axis         = math_fast_to_vec3(axis);
	if (out_rotation_deg) *out_rotation_deg = angle * rad2deg;
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
// pose_t                                //
///////////////////////////////////////////

void pose_matrix_out(const pose_t &pose, matrix &out_result, vec3 scale) {
	matrix_trs_out(out_result, pose.position, pose.orientation, scale);
}

///////////////////////////////////////////

matrix pose_matrix(const pose_t &pose, vec3 scale) {
	return matrix_trs(pose.position, pose.orientation, scale);
}

///////////////////////////////////////////
// ray_t                                 //
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

vec3 ray_point_closest(ray_t ray, vec3 pt) {
	float t = vec3_dot(pt - ray.pos, ray.dir);
	return ray.pos + ray.dir * t;
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
// matrix                                //
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

void matrix_mul(matrix a, matrix b, DirectX::XMMATRIX &out_matrix) {
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


///////////////////////////////////////////

vec3 matrix_mul_direction(const XMMATRIX &transform, const vec3 &direction) {
	return math_fast_to_vec3( XMVector3TransformNormal(math_vec3_to_fast(direction), transform) );
}
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
// plane_t                               //
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

bool32_t plane_ray_intersect  (plane_t plane, ray_t ray, vec3 *out_pt) {
	// t = -(Pi . N + d) / (V . N)
	// Pf = Pi + tV
	float t = 
		-(vec3_dot(ray.pos, plane.normal) + plane.d) / 
		 vec3_dot(ray.dir, plane.normal);
	*out_pt = ray.pos + ray.dir * t;
	return t >= 0;
}

///////////////////////////////////////////

bool32_t plane_line_intersect (plane_t plane, vec3 p1, vec3 p2, vec3 *out_pt) {
	vec3 dir = p2 - p1;
	float t = 
		-(vec3_dot(p1,  plane.normal) + plane.d) / 
		 vec3_dot(dir, plane.normal);
	*out_pt = p1 + dir * t;
	return t >= 0 && t <= 1;
}

///////////////////////////////////////////

vec3 plane_point_closest(plane_t plane, vec3 pt) {
	float dist = vec3_dot(plane.normal, pt) + plane.d;
	return pt - plane.normal * dist;
}

///////////////////////////////////////////
// sphere_t                              //
///////////////////////////////////////////

// From here: http://www.iquilezles.org/www/articles/intersectors/intersectors.htm
bool32_t sphere_ray_intersect(sphere_t sphere, ray_t ray, vec3 *out_pt) {
	*out_pt = {};

	vec3 oc = ray.pos - sphere.center;
	float b = vec3_dot(oc, ray.dir);
	float c = vec3_dot(oc, oc) - sphere.radius * sphere.radius;
	float h = b * b - c;
	if (h < 0.0) return false; // no intersection
	h = sqrtf(h);
	*out_pt = ray.pos + ray.dir * (-b - h);
	return true;
	//return vec2(-b - h, -b + h);
}

///////////////////////////////////////////

bool32_t sphere_point_contains(sphere_t sphere, vec3 pt) {
	vec3 diff = pt - sphere.center; 
	return vec3_dot(diff, diff) <= (sphere.radius * sphere.radius);
}

///////////////////////////////////////////
// bounds_t                              //
///////////////////////////////////////////

vec3 bounds_corner(bounds_t bounds, int32_t index8) {
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
		(float)((index8 / 4) * 2 - 1),
		(float)(((index8 / 2) % 2) * 2 - 1),
		(float)((index8 % 2) * 2 - 1) };
	vec3 half = bounds.dimensions / 2;
	return bounds.center + half * sign;
}

///////////////////////////////////////////

float bounds_sdf_manhattan(bounds_t bounds, vec3 sample_point) {
	vec3 rel         = sample_point - bounds.center;
	vec3 quadrant_pt = vec3{ fabsf(rel.x), fabsf(rel.y), fabsf(rel.z) };
	vec3 dist        = quadrant_pt - bounds.dimensions / 2;
	return fmaxf(fmaxf(dist.x, dist.y), dist.z);
}

///////////////////////////////////////////

float bounds_sdf(bounds_t bounds, vec3 point) {
	vec3  q      = vec3_abs(point-bounds.center) - bounds.dimensions/2;
	float inside = fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0);
	return vec3_magnitude(vec3{fmaxf(q.x,0), fmaxf(q.y,0), fmaxf(q.z,0)}) + inside;
}

///////////////////////////////////////////

bool32_t bounds_point_contains(bounds_t bounds, vec3 pt) {
	pt = vec3_abs(pt - bounds.center) * 2;
	return
		pt.x <= bounds.dimensions.x &&
		pt.y <= bounds.dimensions.y &&
		pt.z <= bounds.dimensions.z;
}

///////////////////////////////////////////

// From here: http://www.iquilezles.org/www/articles/intersectors/intersectors.htm
bool32_t bounds_ray_intersect_dist(bounds_t bounds, ray_t ray, float* out_distance) {
	vec3 rayRelative = ray.pos - bounds.center;
	vec3 m = { 1.f / ray.dir.x, 1.f / ray.dir.y, 1.f / ray.dir.z };
	vec3 n = m * rayRelative; 
	vec3 k = vec3_abs(m) * bounds.dimensions/2;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = fmaxf(fmaxf(t1.x, t1.y), t1.z);
	float tF = fminf(fminf(t2.x, t2.y), t2.z);
	if (tN > tF || tF < 0.0) {
		if (out_distance) *out_distance = 0;
		return false;
	}

	if (out_distance) *out_distance = tN;
	return true;
}

///////////////////////////////////////////

bool32_t bounds_ray_intersect(bounds_t bounds, ray_t ray, vec3* out_pt) {
	float    dist   = 0;
	bool32_t result = bounds_ray_intersect_dist(bounds, ray, &dist);
	if (out_pt)
		*out_pt = ray.pos + ray.dir * dist;
	return result;
}

///////////////////////////////////////////

bool32_t bounds_capsule_intersect(bounds_t bounds, vec3 line_start, vec3 line_end, float radius, vec3 *out_at) {
	float d   = 0;
	ray_t ray = { line_start, line_end - line_start };

	// If the direction is too small, we can get inf/NaN's, so we treat it as a point.
	if (vec3_magnitude_sq(ray.dir) < 0.0001f) {
		*out_at = ray.pos;
		return bounds_point_contains({ bounds.center, bounds.dimensions + vec3_one*(radius*2) }, ray.pos);
	} 
	
	if (bounds_ray_intersect_dist(bounds, ray, &d) && d <= 1) {
		*out_at = line_start + (line_end - line_start) * d;
		return true;
	}

	// Make this all relative to the center of the bounds
	ray.pos    -= bounds.center;
	line_start -= bounds.center;
	line_end   -= bounds.center;
	vec3 size = bounds.dimensions / 2;

	float closest_sq = FLT_MAX;
	vec3  closest_pt = vec3{0,0,0};

	vec3 edges[] = {
		{-1,-1,-1}, { 1,-1,-1},
		{-1, 1,-1}, { 1, 1,-1},
		{-1,-1, 1}, { 1,-1, 1},
		{-1, 1, 1}, { 1, 1, 1},
		{-1,-1,-1}, {-1, 1,-1},
		{ 1,-1,-1}, { 1, 1,-1},
		{-1,-1, 1}, {-1, 1, 1},
		{ 1,-1, 1}, { 1, 1, 1},
		{-1,-1,-1}, {-1,-1, 1},
		{ 1,-1,-1}, { 1,-1, 1},
		{-1, 1,-1}, {-1, 1, 1},
		{ 1, 1,-1}, { 1, 1, 1}
	};
	// check all edges
	for (int32_t i = 0; i < 12; i += 1) {
		vec3 a, b;
		line_line_closest_point_tdist(edges[i*2]*size, edges[i*2+1]*size, line_start, line_end, &a, &b);
		float dist_sq = vec3_distance_sq(a, b);
		if (closest_sq > dist_sq) {
			closest_sq = dist_sq;
			closest_pt = b;
		}
	}

	if (closest_sq <= radius * radius) {
		*out_at = closest_pt + bounds.center;
		return true;
	}
	return false;
}

///////////////////////////////////////////

bool32_t bounds_line_contains(bounds_t bounds, vec3 pt1, vec3 pt2) {
	vec3  delta = pt2 - pt1;
	// If this is just a point, then we can just do a simple contains check
	if (vec3_dot(delta, delta) <= 0.0001f)
		return bounds_point_contains(bounds, pt1);

	// If any of these is 0, then m becomes inf, and that axis is then
	// ignored from the calculation. This is a bit of a hack, and I should
	// probably find a better way to fix this? But it shouldn't be too
	// terrible
	if (delta.x == 0) delta.x = 0.000000000001f;
	if (delta.y == 0) delta.y = 0.000000000001f;
	if (delta.z == 0) delta.z = 0.000000000001f;

	vec3  ray_origin = pt1 - bounds.center;
	vec3  m  = { 1.f / delta.x, 1.f / delta.y, 1.f / delta.z };
	vec3  n  = m * ray_origin;
	vec3  k  = vec3_abs(m) * (bounds.dimensions / 2);
	vec3  t1 = -n - k;
	vec3  t2 = -n + k;
	float tN = fmaxf(fmaxf(t1.x, t1.y), t1.z);
	float tF = fminf(fminf(t2.x, t2.y), t2.z);
	return tF >= 0 && tN < tF && (tF <= 1 || tN <= 1);
}

///////////////////////////////////////////

bool32_t bounds_capsule_contains(bounds_t bounds, vec3 pt1, vec3 pt2, float radius) {
	// A simple hack with some slight error at the corners, is to just
	// inflate the bounds by the radius of the capsule!
	return bounds_line_contains(bounds_t{ bounds.center, bounds.dimensions + vec3{radius*2,radius*2,radius*2} }, pt1, pt2);
}

///////////////////////////////////////////

bounds_t bounds_grow_to_fit_pt(bounds_t bounds, vec3 pt) {
	vec3 half = bounds.dimensions / 2;
	vec3 min  = bounds.center     - half;
	vec3 max  = bounds.center     + half;
	if      (pt.x > max.x) max.x = pt.x;
	else if (pt.x < min.x) min.x = pt.x;
	if      (pt.y > max.y) max.y = pt.y;
	else if (pt.y < min.y) min.y = pt.y;
	if      (pt.z > max.z) max.z = pt.z;
	else if (pt.z < min.z) min.z = pt.z;
	
	bounds_t result;
	result.center     = vec3_lerp(min, max, 0.5f);
	result.dimensions = max - min;
	return result;
}

///////////////////////////////////////////

bounds_t bounds_grow_to_fit_box_opt(bounds_t *opt_bounds, bounds_t box, const matrix* opt_transform) {
	XMVECTOR min, max;
	int32_t start;
	if (opt_bounds) {
		vec3 half = opt_bounds->dimensions / 2;
		vec3 minf = opt_bounds->center - half;
		vec3 maxf = opt_bounds->center + half;
		start = 0;
		min   = XMLoadFloat3((XMFLOAT3*)&minf);
		max   = XMLoadFloat3((XMFLOAT3*)&maxf);
	} else {
		vec3 corner = opt_transform
			? matrix_transform_pt(*opt_transform, bounds_corner(box, 0))
			: bounds_corner(box, 0);
		min   = XMLoadFloat3((XMFLOAT3*)&corner);
		max   = min;
		start = 1;
	}

	for (int32_t i = 1; i < 8; i++) {
		vec3 pt = opt_transform 
			? matrix_transform_pt(*opt_transform, bounds_corner(box, i))
			: bounds_corner(box, i);

		XMVECTOR xpt = XMLoadFloat3((XMFLOAT3*)&pt);
		min = XMVectorMin(min, xpt);
		max = XMVectorMax(max, xpt);
	}
	XMVECTOR center     = XMVectorMultiplyAdd(min, g_XMOneHalf, XMVectorMultiply(max, g_XMOneHalf));
	XMVECTOR dimensions = XMVectorSubtract   (max, min);

	bounds_t result;
	XMStoreFloat3((XMFLOAT3*)&result.center,     center);
	XMStoreFloat3((XMFLOAT3*)&result.dimensions, dimensions);
	return result;
}

///////////////////////////////////////////

bounds_t bounds_grow_to_fit_box(bounds_t bounds, bounds_t box, const matrix* opt_transform) {
	return bounds_grow_to_fit_box_opt(&bounds, box, opt_transform);
}

///////////////////////////////////////////

bounds_t bounds_transform(bounds_t bounds, matrix transform) {
	return bounds_grow_to_fit_box_opt(nullptr, bounds, &transform);
}

///////////////////////////////////////////
// lines                                 //
///////////////////////////////////////////

float line_closest_point_tdist(vec3 line_start, vec3 line_end, vec3 point) {
	vec3 dir = line_end - line_start;
	return vec3_dot(point - line_start, dir) / vec3_dot(dir, dir);
}

///////////////////////////////////////////

void line_line_closest_point_tdist(vec3 line_start, vec3 line_end, vec3 other_line_start, vec3 other_line_end, vec3* out_line_at, vec3* out_other_line_at) {
	vec3 V1  = line_end         - line_start;
	vec3 V2  = other_line_end   - other_line_start;
	vec3 V21 = other_line_start - line_start;

	float v22   = vec3_dot(V2,  V2);
	float v11   = vec3_dot(V1,  V1);
	float v21   = vec3_dot(V2,  V1);
	float v21_1 = vec3_dot(V21, V1);
	float v21_2 = vec3_dot(V21, V2);
	float denom = v21 * v21 - v22 * v11;

	float s, t;
	if (fabsf(denom) < 1e-6f) {
		s = 0.;
		t = (v11 * s - v21_1) / (v21 != 0 ? v21 : 1e-6f); // Avoid division by zero
	} else {
		s = ( v21_2 * v21 - v22 * v21_1) / denom;
		t = (-v21_1 * v21 + v11 * v21_2) / denom;
	}
	*out_line_at       = line_start       + V1 * fmaxf(fminf(s, 1.f), 0.f);
	*out_other_line_at = other_line_start + V2 * fmaxf(fminf(t, 1.f), 0.f);
}

} // namespace sk