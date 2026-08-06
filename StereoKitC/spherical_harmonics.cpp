#include "spherical_harmonics.h"
#include "sk_math.h"

#include <float.h>

namespace sk {

///////////////////////////////////////////

// Reference here:
// https://github.com/kayru/Probulator/blob/master/Source/Probulator/SphericalHarmonics.h#L317
void sh_windowing(spherical_harmonics_t &harmonics, float window_width) {
	int i = 0;
	for(int band = 0; band <= 2; band++) {
		float s = 1.0f / (1.0f + window_width * band * band * (band + 1.0f) * (band + 1.0f));
		for(int m = -band; m <= band; m++) {
			harmonics.coefficients[i++] *= s;
		}
	}
}

///////////////////////////////////////////

// Minimum of the irradiance reconstruction over 26 probe directions.
static float _sh_min_lookup(const spherical_harmonics_t &harmonics) {
	float lowest = FLT_MAX;
	for (int32_t i = 0; i < 27; i++) {
		int32_t x = i % 3 - 1, y = (i / 3) % 3 - 1, z = i / 9 - 1;
		if (x == 0 && y == 0 && z == 0) continue;
		vec3     dir = vec3_normalize(vec3{ (float)x, (float)y, (float)z });
		color128 c   = sh_lookup(harmonics, dir);
		lowest = fminf(lowest, fminf(c.r, fminf(c.g, c.b)));
	}
	return lowest;
}

// Dering adaptively: widen the window only until the irradiance
// reconstruction stops going negative, so well-behaved environments keep
// their full directionality. Mirrors sh_window_fit in the sh_compute shader.
void sh_window_fit(spherical_harmonics_t &harmonics) {
	if (_sh_min_lookup(harmonics) >= 0) return;
	float lo = 0, hi = 4.0f;
	for (int32_t i = 0; i < 10; i++) {
		float mid = (lo + hi) * 0.5f;
		spherical_harmonics_t test = harmonics;
		sh_windowing(test, mid);
		if (_sh_min_lookup(test) >= 0) hi = mid;
		else                           lo = mid;
	}
	sh_windowing(harmonics, hi);
}

///////////////////////////////////////////

spherical_harmonics_t sh_create(const sh_light_t* lights, int32_t light_count) {
	spherical_harmonics_t result = {};
	for (int32_t i = 0; i < light_count; i++) {
		sh_add(result, vec3_normalize(lights[i].dir_to), { lights[i].color.r, lights[i].color.g, lights[i].color.b });
	}
	// Lights average rather than sum, so a scene's overall brightness stays
	// stable as lights are added.
	for (int32_t i = 0; i < 9; i++) {
		result.coefficients[i] /= (float)light_count;
	}

	// Apply windowing to prevent overshooting
	sh_window_fit(result);

	return result;
}

///////////////////////////////////////////

void sh_brightness(spherical_harmonics_t &harmonics, float scale) {
	for (int32_t i = 0; i < 9; i++)
		harmonics.coefficients[i] *= scale;
}

///////////////////////////////////////////

// Projection normalization, calibrated so a uniform radiance-1 environment
// reads exactly 1 from sh_lookup. DirectXMath's directional-light constant
// pi/0.75 read 4.7% hot here. Must match fRet in
// shader_builtin_sh_compute.hlsl.
static const float SH_PROJECT_NORM = 4.0f;

void sh_add(spherical_harmonics_t &to, vec3 light_dir, vec3 light_color) {
	light_dir = { -light_dir.x, -light_dir.y, light_dir.z };

	// From DirectXMath's XMSHEvalDirectionalLight. See:
	// https://github.com/microsoft/DirectXMath/blob/master/SHMath/DirectXSH.cpp#L4476
	light_color = light_color * SH_PROJECT_NORM;

	// The rest is a mix of XMSHEvalDirectionalLight, XMSHEvalDirection, and
	// sh_eval_basis_2
	const float z2 = light_dir.z*light_dir.z;
	const float s1 = light_dir.y;
	const float c1 = light_dir.x;
	const float s2 = light_dir.x*s1 + light_dir.y*c1;
	const float c2 = light_dir.x*c1 - light_dir.y*s1;

	to.coefficients[0] += light_color * 0.282094791773878140f;
	to.coefficients[2] += light_color * 0.488602511902919920f*light_dir.z;
	to.coefficients[6] += light_color * (0.946174695757560080f*z2 + -0.315391565252520050f);
	to.coefficients[1] += light_color * -0.488602511902919920f*s1;
	to.coefficients[3] += light_color * -0.488602511902919920f*c1;
	const float p_2_1 = -1.092548430592079200f*light_dir.z;
	to.coefficients[5] += light_color * p_2_1*s1;
	to.coefficients[7] += light_color * p_2_1*c1;
	to.coefficients[4] += light_color * 0.546274215296039590f*s2;
	to.coefficients[8] += light_color * 0.546274215296039590f*c2;
}

///////////////////////////////////////////

color128 sh_lookup(const spherical_harmonics_t &harmonics, vec3 normal) {
	vec3 result = {};

	static const float Pi = 3.141592654f;
	static const float CosineA0 = Pi;
	static const float CosineA1 = (2.0f * Pi) / 3.0f;
	static const float CosineA2 = Pi * 0.25f;

	// Band 0
	result += harmonics.coefficients[0] * (0.282095f * CosineA0);

	// Band 1
	result += harmonics.coefficients[1] * (0.488603f * normal.y * CosineA1);
	result += harmonics.coefficients[2] * (0.488603f * normal.z * CosineA1);
	result += harmonics.coefficients[3] * (0.488603f * normal.x * CosineA1);

	// Band 2
	result += harmonics.coefficients[4] * (1.092548f * normal.x * normal.y * CosineA2);
	result += harmonics.coefficients[5] * (1.092548f * normal.y * normal.z * CosineA2);
	result += harmonics.coefficients[6] * (0.315392f * (3.0f * normal.z * normal.z - 1.0f) * CosineA2);
	result += harmonics.coefficients[7] * (1.092548f * normal.x * normal.z * CosineA2);
	result += harmonics.coefficients[8] * (0.546274f * (normal.x * normal.x - normal.y * normal.y) * CosineA2);

	return { result.x, result.y, result.z, 1 };
}

///////////////////////////////////////////

// Evaluates the SH as radiance in a direction, no cosine kernel: this
// reconstructs the environment itself, where sh_lookup lights a surface.
color128 sh_lookup_radiance(const spherical_harmonics_t &harmonics, vec3 dir) {
	vec3 result = {};

	result += harmonics.coefficients[0] *  0.282095f;
	result += harmonics.coefficients[1] * (0.488603f * dir.y);
	result += harmonics.coefficients[2] * (0.488603f * dir.z);
	result += harmonics.coefficients[3] * (0.488603f * dir.x);
	result += harmonics.coefficients[4] * (1.092548f * dir.x * dir.y);
	result += harmonics.coefficients[5] * (1.092548f * dir.y * dir.z);
	result += harmonics.coefficients[6] * (0.315392f * (3.0f * dir.z * dir.z - 1.0f));
	result += harmonics.coefficients[7] * (1.092548f * dir.x * dir.z);
	result += harmonics.coefficients[8] * (0.546274f * (dir.x * dir.x - dir.y * dir.y));

	// Undo the projection's directional-light normalization.
	result = result * ((4.0f * MATH_PI) / SH_PROJECT_NORM);
	return { result.x, result.y, result.z, 1 };
}

///////////////////////////////////////////

vec3 sh_dominant_dir(const sk_ref(spherical_harmonics_t) harmonics) {
	// Reference from here:
	// https://seblagarde.wordpress.com/2011/10/09/dive-in-sh-buffer-idea/
	vec3 dir = {
		harmonics.coefficients[3].x * 0.3f + harmonics.coefficients[3].y * 0.59f + harmonics.coefficients[3].z * 0.11f,
		harmonics.coefficients[1].x * 0.3f + harmonics.coefficients[1].y * 0.59f + harmonics.coefficients[1].z * 0.11f,
		harmonics.coefficients[2].x * 0.3f + harmonics.coefficients[2].y * 0.59f + harmonics.coefficients[2].z * 0.11f };

	// If no lighting data, default to light traveling down from above
	if (vec3_magnitude_sq(dir) < 0.0001f)
		return { 0, -1, 0 };

	return -vec3_normalize(dir);
}

///////////////////////////////////////////

void sh_to_fast(const spherical_harmonics_t& lookup, vec4* fast_7) {
	static const float CosineA0 = 3.141592654f;
	static const float CosineA1 = (2.0f * CosineA0) / 3.0f;
	static const float CosineA2 = CosineA0 * 0.25f;

	// Pre-bake SH basis constants into coefficients
	vec3 sh0 = lookup.coefficients[0] * (0.282095f * CosineA0);
	vec3 sh1 = lookup.coefficients[1] * (0.488603f * CosineA1);
	vec3 sh2 = lookup.coefficients[2] * (0.488603f * CosineA1);
	vec3 sh3 = lookup.coefficients[3] * (0.488603f * CosineA1);
	vec3 sh4 = lookup.coefficients[4] * (1.092548f * CosineA2);
	vec3 sh5 = lookup.coefficients[5] * (1.092548f * CosineA2);
	vec3 sh6 = lookup.coefficients[6] * (0.315392f * CosineA2);
	vec3 sh7 = lookup.coefficients[7] * (1.092548f * CosineA2);
	vec3 sh8 = lookup.coefficients[8] * (0.546274f * CosineA2);

	// Pack into dot-product form for efficient GPU evaluation.
	// Shader evaluates as:
	//   vA = float4(normal, 1)
	//   vB = float4(n.x*n.y, n.y*n.z, n.z*n.z, n.z*n.x)
	//   vC = n.x*n.x - n.y*n.y
	//   result.c = dot(shA[c], vA) + dot(shB[c], vB) + shC.c * vC
	// The (3*nz^2-1) term is split: 3*sh6 goes into shB.z, -sh6 folds
	// into shA.w alongside the band 0 constant.

	// Band 0+1 per channel: dot(shA, float4(normal, 1))
	fast_7[0] = { sh3.x, sh1.x, sh2.x, sh0.x - sh6.x };
	fast_7[1] = { sh3.y, sh1.y, sh2.y, sh0.y - sh6.y };
	fast_7[2] = { sh3.z, sh1.z, sh2.z, sh0.z - sh6.z };

	// Band 2 per channel: dot(shB, float4(n.x*n.y, n.y*n.z, n.z*n.z, n.z*n.x))
	fast_7[3] = { sh4.x, sh5.x, 3.0f*sh6.x, sh7.x };
	fast_7[4] = { sh4.y, sh5.y, 3.0f*sh6.y, sh7.y };
	fast_7[5] = { sh4.z, sh5.z, 3.0f*sh6.z, sh7.z };

	// Last band 2 term: (n.x^2 - n.y^2)
	fast_7[6] = { sh8.x, sh8.y, sh8.z, 0 };
}

}