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

inline float math_lerp    (float a, float b, float t) { return a + (b - a) * t; }
inline float math_lerp_cl (float a, float b, float t) { return a + (b - a) * fminf(1,t); }
inline float math_saturate(float x)                   { return fmaxf(0, fminf(1, x)); }

inline float math_ease_overshoot(float a, float b, float overshoot, float t) { t = 1 - t; return math_lerp(a,b, 1-(t*t * ((overshoot + 1) * t - overshoot))); }
inline float math_ease_hop      (float a, float peak, float t) { return a+(peak-a)*sinf(t*MATH_PI); }

// twist - rotation around the "direction" vector
// swing - rotation around axis that is perpendicular to "direction" vector
void quat_decompose_swing_twist(quat rotation, vec3 direction, quat *out_swing, quat *out_twist);

vec3 bounds_corner (const bounds_t &bounds, int32_t index8);
vec3 math_cubemap_corner(int i);

} // namespace sk