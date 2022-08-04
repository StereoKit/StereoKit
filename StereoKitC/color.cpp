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
vec3 color_to_hsv(const color128 &col) {
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
vec3 color_to_lab(const color128 &color) {
	vec3 lab;
	color128 col = color;

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

void gradient_release(gradient_t gradient) {
	sk_free(gradient->keys);
	*gradient = {};
	sk_free(gradient);
}

}