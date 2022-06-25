#include "spherical_harmonics.h"
#include "sk_math.h"
#include "asset_types/texture_.h"

namespace sk {

void sh_add      (spherical_harmonics_t &to, vec3 light_dir, vec3 light_color);
void sh_windowing(spherical_harmonics_t &harmonics, float window_width);

///////////////////////////////////////////

vec3 to_color_128(uint8_t *color) { return *(vec3 *)color; }
vec3 to_color_32 (uint8_t *color) { return vec3{color[0]/255.f,color[1]/255.f,color[2]/255.f}; }
vec3 to_color_32_linear(uint8_t *color) { return vec3{ powf(color[0] / 255.f,2.2f),powf(color[1] / 255.f,2.2f),powf(color[2] / 255.f,2.2f) }; }

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

spherical_harmonics_t sh_create(const sh_light_t* lights, int32_t light_count) {
	spherical_harmonics_t result = {};
	for (int32_t i = 0; i < light_count; i++) {
		sh_add(result, vec3_normalize(lights[i].dir_to), { lights[i].color.r, lights[i].color.g, lights[i].color.b });
	}
	for (int32_t i = 0; i < 9; i++) {
		result.coefficients[i] /= (float)light_count;
	}

	// Apply windowing to prevent overshooting
	sh_windowing(result, .01f);

	return result;
}

///////////////////////////////////////////

void sh_brightness(spherical_harmonics_t &harmonics, float scale) {
	for (int32_t i = 0; i < 9; i++)
		harmonics.coefficients[i] *= scale;
}

///////////////////////////////////////////

void sh_add(spherical_harmonics_t &to, vec3 light_dir, vec3 light_color) {
	light_dir = { -light_dir.x, -light_dir.y, light_dir.z };

	// From DirectXMath's XMSHEvalDirectionalLight. See:
	// https://github.com/microsoft/DirectXMath/blob/master/SHMath/DirectXSH.cpp#L4476

	// CosWtInt
	const float fCW0 = 0.25f;
	const float fCW1 = 0.5f;
	const float fRet = MATH_PI / (fCW0 + fCW1);
	light_color = light_color * fRet;

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

spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size) {
	spherical_harmonics_t result   = {};
	size_t                col_size = tex_format_size(format);
	vec3     (*convert)(uint8_t *) = nullptr;
	switch (format) {
	case tex_format_rgba128:       convert = to_color_128;       break;
	case tex_format_rgba32:        convert = to_color_32_linear; break;
	case tex_format_rgba32_linear: convert = to_color_32;        break;
	default: return {};
	}

	float half_px = 0.5f / face_size;
	for (int32_t i = 0; i < 6; i++) {
		uint8_t *data = (uint8_t*)env_map_data[i];
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4+1);
		vec3 p3 = math_cubemap_corner(i * 4+2);
		vec3 p4 = math_cubemap_corner(i * 4+3); 

		for (int32_t y = 0; y < face_size; y++) {
			float py = 1 - (y / (float)face_size + half_px);

			// Top face is flipped on both axes
			if (i == 2) {
				py = 1 - py;
			}
			for (int32_t x = 0; x < face_size; x++) {
				float px = x / (float)face_size + half_px;

				// Top face is flipped on both axes
				if (i == 2) {
					px = 1 - px;
				}

				vec3 pl = vec3_lerp(p1, p4, py);
				vec3 pr = vec3_lerp(p2, p3, py);
				vec3 pt = vec3_lerp(pl, pr, px);
				pt = vec3_normalize(pt);

				vec3 color = convert(&data[(x + y * face_size) * col_size]);
				sh_add(result, pt, color);
			}
		}
	}

	float count = face_size * face_size * 6.f;
	for (int32_t i = 0; i < 9; i++)
		result.coefficients[i] /= count;

	// Apply windowing to prevent overshooting
	sh_windowing(result, .01f);

	return result;
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

vec3 sh_dominant_dir(const sk_ref(spherical_harmonics_t) harmonics) {
	// Reference from here:
	// https://seblagarde.wordpress.com/2011/10/09/dive-in-sh-buffer-idea/
	vec3 dir = vec3_normalize({
		harmonics.coefficients[3].x * 0.3f + harmonics.coefficients[3].y * 0.59f + harmonics.coefficients[3].z,
		harmonics.coefficients[1].x * 0.3f + harmonics.coefficients[1].y * 0.59f + harmonics.coefficients[1].z,
		harmonics.coefficients[2].x * 0.3f + harmonics.coefficients[2].y * 0.59f + harmonics.coefficients[2].z });
	return -dir;
}

///////////////////////////////////////////

inline vec4 to_vec4(const vec3& vec) { return { vec.x, vec.y, vec.z, 0 }; }

void sh_to_fast(const spherical_harmonics_t& lookup, vec4* fast_9) {
	static const float CosineA0 = 3.141592654f;
	static const float CosineA1 = (2.0f * CosineA0) / 3.0f;
	static const float CosineA2 = CosineA0 * 0.25f;

	fast_9[0] = to_vec4(lookup.coefficients[0] * (0.282095f * CosineA0));

	fast_9[1] = to_vec4(lookup.coefficients[1] * (0.488603f * CosineA1));
	fast_9[2] = to_vec4(lookup.coefficients[2] * (0.488603f * CosineA1));
	fast_9[3] = to_vec4(lookup.coefficients[3] * (0.488603f * CosineA1));

	fast_9[4] = to_vec4(lookup.coefficients[4] * (1.092548f * CosineA2));
	fast_9[5] = to_vec4(lookup.coefficients[5] * (1.092548f * CosineA2));
	fast_9[6] = to_vec4(lookup.coefficients[6] * (0.315392f * CosineA2));
	fast_9[7] = to_vec4(lookup.coefficients[7] * (1.092548f * CosineA2));
	fast_9[8] = to_vec4(lookup.coefficients[8] * (0.546274f * CosineA2));
}

}