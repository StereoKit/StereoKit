#include "spherical_harmonics.h"
#include "math.h"
#include "asset_types/texture.h"

namespace sk {


vec3 to_color_128(uint8_t *color) { return *(vec3 *)color; }
vec3 to_color_32 (uint8_t *color) { return vec3{color[0]/255.f,color[1]/255.f,color[2]/255.f}; }

///////////////////////////////////////////

void sh_add(spherical_harmonics_t &to, vec3 light_dir, color128 light_color) {
	vec3 col = { light_color.r, light_color.g, light_color.b };

	// From here:
	// https://graphics.stanford.edu/papers/envmap/envmap.pdf
	to.coefficients[0] += col * 0.282095f; // Y00
	to.coefficients[1] += col * 0.488603f *  light_dir.x; // Y11
	to.coefficients[2] += col * 0.488603f *  light_dir.z; // Y10
	to.coefficients[3] += col * 0.488603f *  light_dir.y; // Y1_1
	to.coefficients[4] += col * 1.092548f * (light_dir.x*light_dir.z); // Y21
	to.coefficients[5] += col * 1.092548f * (light_dir.y*light_dir.z); // Y2_1
	to.coefficients[6] += col * 1.092548f * (light_dir.y*light_dir.x); // Y2_2
	to.coefficients[7] += col * 0.946176f * (light_dir.z * light_dir.z - 0.315392f); // Y20
	to.coefficients[8] += col * 0.546274f * (light_dir.x*light_dir.x - light_dir.y*light_dir.y); // Y22
}

///////////////////////////////////////////

spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size) {
	spherical_harmonics_t result = {};
	size_t   col_size            = tex_format_size(format);
	vec3   (*convert)(uint8_t *) = format == tex_format_rgba128 ?
		to_color_128 :
		to_color_32;

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

				// From here:
				// https://graphics.stanford.edu/papers/envmap/envmap.pdf
				result.coefficients[0] += color * 0.282095f; // Y00
				result.coefficients[1] += color * 0.488603f *  pt.x; // Y11
				result.coefficients[2] += color * 0.488603f *  pt.z; // Y10
				result.coefficients[3] += color * 0.488603f *  pt.y; // Y1_1
				result.coefficients[4] += color * 1.092548f * (pt.x*pt.z); // Y21
				result.coefficients[5] += color * 1.092548f * (pt.y*pt.z); // Y2_1
				result.coefficients[6] += color * 1.092548f * (pt.y*pt.x); // Y2_2
				result.coefficients[7] += color * 0.946176f * (pt.z * pt.z - 0.315392f); // Y20
				result.coefficients[8] += color * 0.546274f * (pt.x*pt.x - pt.y*pt.y); // Y22
			}
		}
	}

	float count = face_size * face_size * 6;
	for (size_t i = 0; i < 9; i++)
		result.coefficients[i] /= count;

	return result;
}

///////////////////////////////////////////

color128 sh_lookup(const spherical_harmonics_t &lookup, vec3 normal) {
	vec3 result = {};

	static const float Pi = 3.141592654f;
	static const float CosineA0 = Pi;
	static const float CosineA1 = (2.0f * Pi) / 3.0f;
	static const float CosineA2 = Pi * 0.25f;

	// Band 0
	result += lookup.coefficients[0] * (0.282095f * CosineA0);

	// Band 1
	result += lookup.coefficients[1] * (0.488603f * normal.y * CosineA1);
	result += lookup.coefficients[2] * (0.488603f * normal.z * CosineA1);
	result += lookup.coefficients[3] * (0.488603f * normal.x * CosineA1);

	// Band 2
	result += lookup.coefficients[4] * (1.092548f * normal.x * normal.y * CosineA2);
	result += lookup.coefficients[5] * (1.092548f * normal.y * normal.z * CosineA2);
	result += lookup.coefficients[6] * (0.315392f * (3.0f * normal.z * normal.z - 1.0f) * CosineA2);
	result += lookup.coefficients[7] * (1.092548f * normal.x * normal.z * CosineA2);
	result += lookup.coefficients[8] * (0.546274f * (normal.x * normal.x - normal.y * normal.y) * CosineA2);

	return { result.x, result.y, result.z, 1 };
}

///////////////////////////////////////////

tex_t sh_to_tex(const spherical_harmonics_t &lookup, int32_t face_size) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba128);
	if (result == nullptr) {
		return nullptr;
	}

	int32_t size  = face_size;
	// make size a power of two
	int32_t power = (int32_t)logf((float)size);
	if (pow(2, power) < size)
		power += 1;
	size = (int32_t)pow(2, power);

	float    half_px = 0.5f / size;
	int32_t  size2 = size * size;
	color128 *data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = (color128 *)malloc(size2 * sizeof(color128));
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4+1);
		vec3 p3 = math_cubemap_corner(i * 4+2);
		vec3 p4 = math_cubemap_corner(i * 4+3); 

		for (int32_t y = 0; y < size; y++) {
			float py = 1 - (y / (float)size + half_px);

			// Top face is flipped on both axes
			if (i == 2) {
				py = 1 - py;
			}
			for (int32_t x = 0; x < size; x++) {
				float px = x / (float)size + half_px;

				// Top face is flipped on both axes
				if (i == 2) {
					px = 1 - px;
				}

				vec3 pl = vec3_lerp(p1, p4, py);
				vec3 pr = vec3_lerp(p2, p3, py);
				vec3 pt = vec3_lerp(pl, pr, px);
				pt = vec3_normalize(pt);

				data[i][x + y * size] = sh_lookup(lookup, pt);
			}
		}
	}

	tex_set_color_arr(result, (int32_t)size, (int32_t)size, (void**)data, 6);
	for (int32_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

inline vec4 to_vec4(const vec3& vec) { return { vec.x, vec.y, vec.z, 0 }; }

void sh_to_fast(const spherical_harmonics_t& lookup, vec4* fast_9) {
	static const float CosineA0 = 3.141592654;
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