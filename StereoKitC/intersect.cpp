#include "stereokit.h"

#include "math.h"

#include <DirectXMath.h>
using namespace DirectX;

namespace sk {

bool32_t plane_ray_intersect  (plane_t plane, ray_t ray, vec3 *out_pt) {
	// t = -(Pi . N + d) / (V . N)
	// Pf = Pi + tV
	float t = 
		(vec3_dot(ray.pos, plane.normal) + plane.d) / 
		 vec3_dot(ray.dir, plane.normal);
	*out_pt = ray.pos + ray.dir * t;
	return t >= 0;
}

///////////////////////////////////////////

bool32_t plane_line_intersect (plane_t plane, vec3 p1, vec3 p2, vec3 *out_pt) {
	vec3 dir = p2 - p1;
	float t = 
		(vec3_dot(p1,  plane.normal) + plane.d) / 
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

bool32_t sphere_ray_intersect(sphere_t sphere, ray_t ray, vec3 *out_pt) {
	vec3  diff = sphere.center - ray.pos;
	float dot  = vec3_dot(diff, ray.dir);

	float dist2   = (dot*dot) - vec3_dot(diff,diff);
	float radius2 = sphere.radius * sphere.radius;
	if ( dot < 0.0 || dist2 > radius2) return false;

	float dist = sqrtf( radius2 - dist2 );

	//*out_pt = dot - dist;

	return true;
}

///////////////////////////////////////////

bool32_t sphere_point_contains(sphere_t sphere, vec3 pt) {
	vec3 diff = pt - sphere.center; 
	return vec3_dot(diff, diff) < (sphere.radius * sphere.radius);
}

///////////////////////////////////////////

bool32_t bounds_ray_intersect(bounds_t bounds, ray_t ray, vec3* out_pt) {
return false; 
}

///////////////////////////////////////////

bool32_t bounds_point_contains(bounds_t bounds, vec3 pt) {
	pt = vec3_abs(pt - bounds.center) * 2;
	return
		pt.x < bounds.dimensions.x &&
		pt.y < bounds.dimensions.y &&
		pt.z < bounds.dimensions.z;
}

}