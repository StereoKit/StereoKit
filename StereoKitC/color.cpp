#include "stereokit.h"

#include <math.h>
#include "math.h"

namespace sk {

///////////////////////////////////////////

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
color128 color_hsv(float hue, float saturation, float value, float transparency) {
	const vec4 K = vec4{ 1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f };
	vec3 p = {
		fabsf(((hue + K.x) - floor(hue + K.x)) * 6.0 - K.w),
		fabsf(((hue + K.y) - floor(hue + K.y)) * 6.0 - K.w),
		fabsf(((hue + K.z) - floor(hue + K.z)) * 6.0 - K.w) };
	return {
		math_lerp(K.x, fmaxf(0,fminf(p.x - K.x, 1.0f)), saturation) * value,
		math_lerp(K.x, fmaxf(0,fminf(p.y - K.x, 1.0f)), saturation) * value,
		math_lerp(K.x, fmaxf(0,fminf(p.z - K.x, 1.0f)), saturation) * value,
		transparency };
}

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 color_to_hsv(color128 col) {
	const vec4 K = vec4{ 0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f };
	vec4 p = col.g < col.b ? vec4{ col.b, col.g, K.w, K.z } : vec4{ col.g, col.b, K.x, K.y };
	vec4 q = col.r < p.x   ? vec4{ p.x, p.y, p.w, col.r }   : vec4{ col.r, p.y, p.z, p.x };

	float d = q.x - fminf(q.w, q.y);
	float e = 1.0e-10;
	return vec3{ fabsf(q.z + (q.w - q.y) / (6.0f * d + e)), d / (q.x + e), q.x };
}

///////////////////////////////////////////
}