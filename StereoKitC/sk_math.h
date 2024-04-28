#pragma once

#include "stereokit.h"

#define MATH_PI 3.1415926535898f

namespace sk {

///////////////////////////////////////////

inline int32_t  maxi(int32_t  a, int32_t  b) { return a > b ? a : b; }
inline uint32_t maxi(uint32_t a, uint32_t b) { return a > b ? a : b; }
inline int64_t  maxi(int64_t  a, int64_t  b) { return a > b ? a : b; }
inline uint64_t maxi(uint64_t a, uint64_t b) { return a > b ? a : b; }
inline int32_t  mini(int32_t  a, int32_t  b) { return a < b ? a : b; }
inline uint32_t mini(uint32_t a, uint32_t b) { return a < b ? a : b; }
inline int64_t  mini(int64_t  a, int64_t  b) { return a < b ? a : b; }
inline uint64_t mini(uint64_t a, uint64_t b) { return a < b ? a : b; }

inline float math_lerp    (float a, float b, float t)     { return a + (b - a) * t; }
inline float math_lerp_cl (float a, float b, float t)     { return a + (b - a) * fminf(1,t); }
inline float math_saturate(float x)                       { return fmaxf(0, fminf(1, x)); }
inline float math_clamp   (float x, float min, float max) { return fmaxf(min, fminf(max, x)); }

inline float math_ease_overshoot(float a, float b, float overshoot, float t) { t = 1 - t; return math_lerp(a,b, 1-(t*t * ((overshoot + 1) * t - overshoot))); }
inline float math_ease_hop      (float a, float peak, float t) { return a+(peak-a)*sinf(t*MATH_PI); }
inline float math_ease_smooth   (float a, float b, float t) { t = 1-t; return a + (b-a) * (1-t*t); }

// twist - rotation around the "direction" vector
// swing - rotation around axis that is perpendicular to "direction" vector
void quat_decompose_swing_twist(quat rotation, vec3 direction, quat *out_swing, quat *out_twist);

bool32_t bounds_ray_intersect_dist(bounds_t bounds, ray_t ray, float* out_distance);
bool32_t bounds_capsule_intersect (bounds_t bounds, vec3 line_start, vec3 line_end, float radius, vec3* out_at);
vec3     bounds_corner            (bounds_t bounds, int32_t index8);
float    bounds_sdf               (bounds_t bounds, vec3 point);
float    bounds_sdf_manhattan     (bounds_t bounds, vec3 sample_point);

float line_closest_point_tdist     (vec3 line_start, vec3 line_end, vec3 point);
void  line_line_closest_point_tdist(vec3 line_start, vec3 line_end, vec3 other_line_start, vec3 other_line_end, vec3* out_line_at, vec3* out_other_line_at);

vec3 math_cubemap_corner(int i);

} // namespace sk