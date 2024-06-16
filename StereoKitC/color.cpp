#include "stereokit.h"

#include <string.h>
#include <math.h>
#include "sk_math.h"
#include "sk_memory.h"

namespace sk {

struct _gradient_t {
	gradient_key_t *keys;
	int32_t         count;
	int32_t         capacity;
};

///////////////////////////////////////////

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
color128 color_hsv(float hue, float saturation, float value, float transparency) {
	const vec4 K = vec4{ 1.0f, 2.0f / 3.0f, 1.0f / 3.0f, 3.0f };
	vec3 p = {
		fabsf(((hue + K.x) - floorf(hue + K.x)) * 6.0f - K.w),
		fabsf(((hue + K.y) - floorf(hue + K.y)) * 6.0f - K.w),
		fabsf(((hue + K.z) - floorf(hue + K.z)) * 6.0f - K.w) };
	return {
		math_lerp(K.x, fmaxf(0,fminf(p.x - K.x, 1.0f)), saturation) * value,
		math_lerp(K.x, fmaxf(0,fminf(p.y - K.x, 1.0f)), saturation) * value,
		math_lerp(K.x, fmaxf(0,fminf(p.z - K.x, 1.0f)), saturation) * value,
		transparency };
}

///////////////////////////////////////////

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 color_to_hsv(color128 col) {
	const vec4 K = vec4{ 0.0f, -1.0f / 3.0f, 2.0f / 3.0f, -1.0f };
	vec4 p = col.g < col.b ? vec4{ col.b, col.g, K.w, K.z } : vec4{ col.g, col.b, K.x, K.y };
	vec4 q = col.r < p.x   ? vec4{ p.x, p.y, p.w, col.r }   : vec4{ col.r, p.y, p.z, p.x };

	float d = q.x - fminf(q.w, q.y);
	float e = 1.0e-10f;
	return vec3{ fabsf(q.z + (q.w - q.y) / (6.0f * d + e)), d / (q.x + e), q.x };
}

///////////////////////////////////////////

// from https://www.easyrgb.com/en/math.php, Conversion to XYZ, then RGB
// LAB is usually in 0-100,-200-200,-200-200 range, but we convert it
// all to 0-1 for ease of use.
color128 color_lab(float l, float a, float b, float transparency) {
	l = l * 100;
	a = a * 400 - 200;
	b = b * 400 - 200;
	float
		y = (l + 16.f) / 116.f,
		x = (a / 500.f) + y,
		z = y - (b / 200.f);
	color128 col = { 0,0,0,transparency };

	x = 0.95047f * ((x * x * x > 0.008856f) ? x * x * x : (x - 16/116.f) / 7.787f);
	y = 1.00000f * ((y * y * y > 0.008856f) ? y * y * y : (y - 16/116.f) / 7.787f);
	z = 1.08883f * ((z * z * z > 0.008856f) ? z * z * z : (z - 16/116.f) / 7.787f);

	col.r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	col.g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	col.b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

	col.r = (col.r > 0.0031308f) ? (1.055f * powf(col.r, 1/2.4f) - 0.055f) : 12.92f * col.r;
	col.g = (col.g > 0.0031308f) ? (1.055f * powf(col.g, 1/2.4f) - 0.055f) : 12.92f * col.g;
	col.b = (col.b > 0.0031308f) ? (1.055f * powf(col.b, 1/2.4f) - 0.055f) : 12.92f * col.b;

	return { 
		fmaxf(0, fminf(1, col.r)),
		fmaxf(0, fminf(1, col.g)),
		fmaxf(0, fminf(1, col.b)), transparency };
}

///////////////////////////////////////////

// https://www.easyrgb.com/en/math.php, conversion to XYZ, then LAB
// LAB is usually in 0-100,-200-200,-200-200 range, but we convert it
// all to 0-1 for ease of use.
vec3 color_to_lab(color128 linear_color) {
	vec3 lab;
	color128 col = linear_color;

	col.r = (col.r > 0.04045f) ? powf((col.r + 0.055f) / 1.055f, 2.4f) : col.r / 12.92f;
	col.g = (col.g > 0.04045f) ? powf((col.g + 0.055f) / 1.055f, 2.4f) : col.g / 12.92f;
	col.b = (col.b > 0.04045f) ? powf((col.b + 0.055f) / 1.055f, 2.4f) : col.b / 12.92f;

	// D65, Daylight, sRGB, aRGB
	lab.x = (col.r * 0.4124f + col.g * 0.3576f + col.b * 0.1805f) / 0.95047f;
	lab.y = (col.r * 0.2126f + col.g * 0.7152f + col.b * 0.0722f) / 1.00000f;
	lab.z = (col.r * 0.0193f + col.g * 0.1192f + col.b * 0.9505f) / 1.08883f;

	lab.x = (lab.x > 0.008856f) ? powf(lab.x, 1/3.f) : (7.787f * lab.x) + 16/116.f;
	lab.y = (lab.y > 0.008856f) ? powf(lab.y, 1/3.f) : (7.787f * lab.y) + 16/116.f;
	lab.z = (lab.z > 0.008856f) ? powf(lab.z, 1/3.f) : (7.787f * lab.z) + 16/116.f;

	return {
		(1.16f * lab.y) - .16f,
		1.25f * (lab.x - lab.y) + 0.5f,
		0.5f * (lab.y - lab.z) + 0.5f};
}

///////////////////////////////////////////

// Oklab core functions are from here:
// https://bottosson.github.io/posts/oklab/

color128 color_oklab(float lightness, float a, float b, float transparency) {
	float l_ = lightness + 0.3963377774f * a + 0.2158037573f * b;
	float m_ = lightness - 0.1055613458f * a - 0.0638541728f * b;
	float s_ = lightness - 0.0894841775f * a - 1.2914855480f * b;

	float l = l_ * l_ * l_;
	float m = m_ * m_ * m_;
	float s = s_ * s_ * s_;

	return {
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
		transparency };
}

///////////////////////////////////////////

vec3 color_to_oklab(color128 linear_color) {
	vec3 c = { linear_color.r, linear_color.g, linear_color.b };
	vec3 lms = vec3{
		vec3_dot(c, vec3{0.4122214708f, 0.5363325363f, 0.0514459929f}),
		vec3_dot(c, vec3{0.2119034982f, 0.6806995451f, 0.1073969566f}),
		vec3_dot(c, vec3{0.0883024619f, 0.2817188376f, 0.6299787005f}),
	};
	vec3 crlms = { cbrtf(lms.x), cbrtf(lms.y), cbrtf(lms.z) };

	return {
		vec3_dot(crlms, {0.2104542553f, +0.7936177850f, -0.0040720468f}),
		vec3_dot(crlms, {1.9779984951f, -2.4285922050f, +0.4505937099f}),
		vec3_dot(crlms, {0.0259040371f, +0.7827717662f, -0.8086757660f}),
	};
}

///////////////////////////////////////////

// Oklab HSV functions are from here:
// https://bottosson.github.io/posts/colorpicker/

float ok_toe(float x) {
	const float k_1 = 0.206f;
	const float k_2 = 0.03f;
	const float k_3 = (1.f + k_1) / (1.f + k_2);
	return 0.5f * (k_3 * x - k_1 + sqrtf((k_3 * x - k_1) * (k_3 * x - k_1) + 4 * k_2 * k_3 * x));
}

///////////////////////////////////////////

float ok_toe_inv(float x) {
	const float k_1 = 0.206f;
	const float k_2 = 0.03f;
	const float k_3 = (1.f + k_1) / (1.f + k_2);
	return (x * x + k_1 * x) / (k_3 * (x + k_2));
}

///////////////////////////////////////////

vec2 ok_to_ST(vec2 cusp) {
	float L = cusp.x;
	float C = cusp.y;
	return { C / L, C / (1 - L) };
}

///////////////////////////////////////////

float ok_compute_max_saturation(float a, float b) {
	// Max saturation will be when one of r, g or b goes below zero.

	// Select different coefficients depending on which component goes below zero first
	float k0, k1, k2, k3, k4, wl, wm, ws;

	if (-1.88170328f * a - 0.80936493f * b > 1)
	{
		// Red component
		k0 = +1.19086277f;   k1 = +1.76576728f;   k2 = +0.59662641f; k3 = +0.75515197f; k4 = +0.56771245f;
		wl = +4.0767416621f; wm = -3.3077115913f; ws = +0.2309699292f;
	}
	else if (1.81444104f * a - 1.19445276f * b > 1)
	{
		// Green component
		k0 = +0.73956515f;   k1 = -0.45954404f;   k2 = +0.08285427f; k3 = +0.12541070f; k4 = +0.14503204f;
		wl = -1.2684380046f; wm = +2.6097574011f; ws = -0.3413193965f;
	}
	else
	{
		// Blue component
		k0 = +1.35733652f;   k1 = -0.00915799f;   k2 = -1.15130210f; k3 = -0.50559606f; k4 = +0.00692167f;
		wl = -0.0041960863f; wm = -0.7034186147f; ws = +1.7076147010f;
	}

	// Approximate max saturation using a polynomial:
	float S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b;

	// Do one step Halley's method to get closer
	// this gives an error less than 10e6, except for some blue hues where the dS/dh is close to infinite
	// this should be sufficient for most applications, otherwise do two/three steps 
	float k_l = +0.3963377774f * a + 0.2158037573f * b;
	float k_m = -0.1055613458f * a - 0.0638541728f * b;
	float k_s = -0.0894841775f * a - 1.2914855480f * b;

	{
		float l_ = 1.f + S * k_l;
		float m_ = 1.f + S * k_m;
		float s_ = 1.f + S * k_s;

		float l = l_ * l_ * l_;
		float m = m_ * m_ * m_;
		float s = s_ * s_ * s_;

		float l_dS = 3.f * k_l * l_ * l_;
		float m_dS = 3.f * k_m * m_ * m_;
		float s_dS = 3.f * k_s * s_ * s_;

		float l_dS2 = 6.f * k_l * k_l * l_;
		float m_dS2 = 6.f * k_m * k_m * m_;
		float s_dS2 = 6.f * k_s * k_s * s_;

		float f  = wl * l     + wm * m     + ws * s;
		float f1 = wl * l_dS  + wm * m_dS  + ws * s_dS;
		float f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2;

		S = S - f * f1 / (f1*f1 - 0.5f * f * f2);
	}

	return S;
}

///////////////////////////////////////////

vec2 ok_find_cusp(float a, float b)
{
	// First, find the maximum saturation (saturation S = C/L)
	float S_cusp = ok_compute_max_saturation(a, b);

	// Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
	color128 rgb_at_max = color_oklab(1, S_cusp * a, S_cusp * b, 1);
	float L_cusp = cbrtf(1.f / fmaxf(fmaxf(rgb_at_max.r, rgb_at_max.g), rgb_at_max.b));
	float C_cusp = L_cusp * S_cusp;

	return { L_cusp , C_cusp };
}

color128 color_okhsv(float hue, float saturation, float value, float transparency) {
	float a_ = cosf(2.f * MATH_PI * hue);
	float b_ = sinf(2.f * MATH_PI * hue);
	
	vec2 cusp   = ok_find_cusp(a_, b_);
	vec2 ST_max = ok_to_ST(cusp);
	float S_max = ST_max.x;
	float T_max = ST_max.y;
	float S_0 = 0.5f;
	float k = 1 - S_0 / S_max;

	// first we compute L and V as if the gamut is a perfect triangle:

	// L, C when v==1:
	float L_v = 1 - saturation     * S_0 / (S_0 + T_max - T_max * k * saturation);
	float C_v = saturation * T_max * S_0 / (S_0 + T_max - T_max * k * saturation);

	float L = value * L_v;
	float C = value * C_v;

	// then we compensate for both toe and the curved top part of the triangle:
	float L_vt = ok_toe_inv(L_v);
	float C_vt = C_v * L_vt / L_v;

	float L_new = ok_toe_inv(L);
	C = C * L_new / L;
	L = L_new;

	color128 rgb_scale = color_oklab(L_vt, a_ * C_vt, b_ * C_vt, transparency);
	float scale_L = cbrtf(1.f / fmaxf(fmaxf(rgb_scale.r, rgb_scale.g), fmaxf(rgb_scale.b, 0.f)));

	L = L * scale_L;
	C = C * scale_L;

	color128 rgb = color_oklab(L, C * a_, C * b_, transparency);
	return color_to_gamma(rgb);
}

///////////////////////////////////////////

vec3 color_to_okhsv(color128 srgb_gamma_correct) {
	vec3 lab = color_to_oklab(color_to_linear(srgb_gamma_correct));

	float C = sqrtf(lab.y * lab.y + lab.z * lab.z);
	float a_ = lab.y / C;
	float b_ = lab.z / C;

	float L = lab.x;
	float h = 0.5f + 0.5f * atan2f(-lab.z, -lab.y) / MATH_PI;

	vec2 cusp = ok_find_cusp(a_, b_);
	vec2 ST_max = ok_to_ST(cusp);
	float S_max = ST_max.x;
	float T_max = ST_max.y;
	float S_0 = 0.5f;
	float k = 1 - S_0 / S_max;

	// first we find L_v, C_v, L_vt and C_vt

	float t = T_max / (C + L * T_max);
	float L_v = t * L;
	float C_v = t * C;

	float L_vt = ok_toe_inv(L_v);
	float C_vt = C_v * L_vt / L_v;

	// we can then use these to invert the step that compensates for the toe and the curved top part of the triangle:
	color128 rgb_scale = color_oklab(L_vt, a_ * C_vt, b_ * C_vt, 1);
	float scale_L = cbrtf(1.f / fmaxf(fmaxf(rgb_scale.r, rgb_scale.g), fmaxf(rgb_scale.b, 0.f)));

	L = L / scale_L;
	C = C / scale_L;

	C = C * ok_toe(L) / L;
	L = ok_toe(L);

	// we can now compute v and s:

	float v = L / L_v;
	float s = (S_0 + T_max) * C_v / ((T_max * S_0) + T_max * k * C_v);

	return { h, s, v };
}

///////////////////////////////////////////

color128 color_to_linear(color128 srgb_gamma_correct) {
	return {
		powf(srgb_gamma_correct.r, 2.2f),
		powf(srgb_gamma_correct.g, 2.2f),
		powf(srgb_gamma_correct.b, 2.2f),
		srgb_gamma_correct.a };
}

///////////////////////////////////////////

color128 color_to_gamma(color128 srgb_linear) {
	return {
		powf(srgb_linear.r, 1/2.2f),
		powf(srgb_linear.g, 1/2.2f),
		powf(srgb_linear.b, 1/2.2f),
		srgb_linear.a };
}

///////////////////////////////////////////

gradient_t gradient_create() {
	gradient_t result = sk_malloc_t(_gradient_t, 1);
	*result = {};
	return result;
}

///////////////////////////////////////////

gradient_t gradient_create_keys(const gradient_key_t *keys, int32_t count) {
	gradient_t result = gradient_create();
	result->capacity = count;
	result->keys     = sk_malloc_t(gradient_key_t, count);
	for (int32_t i = 0; i < count; i++) {
		gradient_add(result, keys[i].color, keys[i].position);
	}
	return result;
}

///////////////////////////////////////////

void gradient_add(gradient_t gradient, color128 color, float position) {
	if (gradient->count + 1 >= gradient->capacity) {
		gradient->capacity += 1;
		gradient->keys = sk_realloc_t(gradient_key_t, gradient->keys, gradient->capacity);
	}

	int index = gradient->count;
	for (int32_t i = 0; i < gradient->count; i++) {
		if (gradient->keys[i].position > position) {
			index = i;
			break;
		}
	}
	for (int32_t i = gradient->count; i > index; i--) {
		gradient->keys[i] = gradient->keys[i - 1];
	}
	gradient->keys[index] = { color,position };
	gradient->count += 1;
}

///////////////////////////////////////////

void       gradient_set(gradient_t gradient, int32_t index, color128 color, float position) {
	gradient->keys[index] = { color, position };
}

///////////////////////////////////////////

void       gradient_remove(gradient_t gradient, int32_t index) {
	memmove(&gradient->keys[index], &gradient->keys[index + 1], sizeof(gradient_key_t) * (gradient->count - (index + 1)));
	gradient->capacity -= 1;
	
}

///////////////////////////////////////////

int32_t    gradient_count(gradient_t gradient) {
	return gradient->count;
}

///////////////////////////////////////////

color128 gradient_get(gradient_t gradient, float at) {
	if (gradient->count == 0)
		return { 1,0,0,1 };
	if (at < gradient->keys[0].position)
		return gradient->keys[0].color;
	if (at >= gradient->keys[gradient->count - 1].position)
		return gradient->keys[gradient->count - 1].color;

	for (int32_t i = 1; i < gradient->count; i++) {
		if (gradient->keys[i].position >= at) {
			float pct = (at - gradient->keys[i-1].position) / (gradient->keys[i].position - gradient->keys[i-1].position);
			return color_lerp(gradient->keys[i-1].color, gradient->keys[i].color, pct);
		}
	}
	return { 1,0,0,1 };
}

///////////////////////////////////////////

color32 gradient_get32(gradient_t gradient, float at) {
	color128 result = gradient_get(gradient, at);
	return { 
		(uint8_t)(result.r * 255.f), 
		(uint8_t)(result.g * 255.f), 
		(uint8_t)(result.b * 255.f), 
		(uint8_t)(result.a * 255.f) };
}

///////////////////////////////////////////

void gradient_destroy(gradient_t gradient) {
	sk_free(gradient->keys);
	*gradient = {};
	sk_free(gradient);
}

}