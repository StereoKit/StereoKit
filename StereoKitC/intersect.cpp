#include "stereokit.h"

#include "sk_math.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace sk {

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

vec3     plane_point_closest(plane_t plane, vec3 pt) {
	float dist = vec3_dot(plane.normal, pt) + plane.d;
	return pt - plane.normal * dist;
}

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

// From here: http://www.iquilezles.org/www/articles/intersectors/intersectors.htm
bool32_t bounds_ray_intersect(bounds_t bounds, ray_t ray, vec3* out_pt) {
	*out_pt = {};

	vec3 rayRelative = ray.pos - bounds.center;
	vec3 m = { 1.f / ray.dir.x, 1.f / ray.dir.y, 1.f / ray.dir.z };
	vec3 n = m * rayRelative; 
	vec3 k = vec3_abs(m) * bounds.dimensions/2;
	vec3 t1 = -n - k;
	vec3 t2 = -n + k;
	float tN = fmaxf(fmaxf(t1.x, t1.y), t1.z);
	float tF = fminf(fminf(t2.x, t2.y), t2.z);
	if (tN > tF || tF < 0.0) return false;
	
	*out_pt = ray.pos + ray.dir * tN;
	return true;
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

bool32_t bounds_line_contains(bounds_t bounds, vec3 pt1, vec3 pt2) {
	vec3  delta = pt2 - pt1;
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
		vec3 corner = bounds_corner(box, 0);
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

vec3 ray_point_closest(ray_t ray, vec3 pt) {
	float t = vec3_dot(pt - ray.pos, ray.dir);
	return ray.pos + ray.dir * t;
}

}