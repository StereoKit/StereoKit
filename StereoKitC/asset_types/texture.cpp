#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"
#include "../systems/platform/platform_utils.h"
#include "../libraries/ferr_hash.h"
#include "../sk_math.h"
#include "../spherical_harmonics.h"
#include "texture.h"

#pragma warning( disable : 26451 6011 6262 6308 6387 28182 )
#define STB_IMAGE_IMPLEMENTATION
#include "../libraries/stb_image.h"
#pragma warning( default : 26451 6011 6262 6308 6387 28182 )

namespace sk {

///////////////////////////////////////////

tex_t tex_create(tex_type_ type, tex_format_ format) {
	tex_t result = (tex_t)assets_allocate(asset_type_texture);
	result->type   = type;
	result->format = format;
	result->address_mode = tex_address_wrap;
	result->sample_mode  = tex_sample_linear;
	result->anisotropy   = 4;
	return result;
}

///////////////////////////////////////////

tex_t tex_add_zbuffer(tex_t texture, tex_format_ format) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err("tex_add_zbuffer can't add a zbuffer to a non-rendertarget texture!");
		return nullptr;
	}

	char id[64];
	assets_unique_name(asset_type_texture, "zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex_create(tex_type_depth, format);
	tex_set_id       (texture->depth_buffer, id);
	tex_set_color_arr(texture->depth_buffer, texture->tex.width, texture->tex.height, nullptr, texture->tex.array_count);
	skg_tex_attach_depth(&texture->tex, &texture->depth_buffer->tex);
	
	return texture->depth_buffer;
}

///////////////////////////////////////////

void tex_set_zbuffer(tex_t texture, tex_t depth_texture) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err("tex_set_zbuffer can't add a zbuffer to a non-rendertarget texture!");
		return;
	}
	if (!(depth_texture->type & tex_type_depth)) {
		log_err("tex_set_zbuffer can't add a non-depth texture as a zbuffer!");
		return;
	}
	assets_addref(depth_texture->header);
	if (texture->depth_buffer != nullptr)
		tex_release(texture->depth_buffer);
	texture->depth_buffer = depth_texture;

	skg_tex_attach_depth(&texture->tex, &depth_texture->tex);
}

///////////////////////////////////////////

void tex_set_surface(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count) {
	if (skg_tex_is_valid(&texture->tex))
		skg_tex_destroy (&texture->tex);

	skg_tex_type_ skg_type = skg_tex_type_image;
	if      (type & tex_type_cubemap     ) skg_type = skg_tex_type_cubemap;
	else if (type & tex_type_depth       ) skg_type = skg_tex_type_depth;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);
	texture->tex    = skg_tex_create_from_existing(native_surface, skg_type, skg_tex_fmt_from_native(native_fmt), width, height, surface_count);
}

///////////////////////////////////////////

void tex_set_surface_layer(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_index) {
	if (skg_tex_is_valid(&texture->tex))
		skg_tex_destroy (&texture->tex);

	skg_tex_type_ skg_type = skg_tex_type_image;
	if      (type & tex_type_cubemap     ) skg_type = skg_tex_type_cubemap;
	else if (type & tex_type_depth       ) skg_type = skg_tex_type_depth;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);
	texture->tex    = skg_tex_create_from_layer(native_surface, skg_type, skg_tex_fmt_from_native(native_fmt), width, height, surface_index);
}

///////////////////////////////////////////

tex_t tex_find(const char *id) {
	tex_t result = (tex_t)assets_find(id, asset_type_texture);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void tex_set_id(tex_t tex, const char *id) {
	assets_set_id(tex->header, id);
}

///////////////////////////////////////////

tex_t tex_create_mem(void *data, size_t data_size, bool32_t srgb_data) {
	bool     is_hdr   = stbi_is_hdr_from_memory((stbi_uc*)data, (int)data_size);
	int      channels = 0;
	int      width    = 0;
	int      height   = 0;
	uint8_t *col_data =  is_hdr ? 
		(uint8_t *)stbi_loadf_from_memory((stbi_uc*)data, (int)data_size, &width, &height, &channels, 4):
		(uint8_t *)stbi_load_from_memory ((stbi_uc*)data, (int)data_size, &width, &height, &channels, 4);

	if (col_data == nullptr) {
		log_warn("Couldn't parse image data!");
		return nullptr;
	}
	tex_format_ format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
	if (is_hdr) format = tex_format_rgba128;

	tex_t result = tex_create(tex_type_image, format);

	tex_set_colors(result, width, height, col_data);
	free(col_data);
	return result;
}

///////////////////////////////////////////

tex_t tex_create_file(const char *file, bool32_t srgb_data) {
	tex_t result = tex_find(file);
	if (result != nullptr)
		return result;

	void  *file_data;
	size_t file_size;
	if (!platform_read_file(assets_file(file), &file_data, &file_size))
		return nullptr;

	result = tex_create_mem(file_data, file_size, srgb_data);
	free(file_data);

	if (result == nullptr) {
		log_warnf("Issue loading file [%s]", file);
		return nullptr;
	}
	tex_set_id(result, file);
	
	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_file(const char *equirectangular_file, bool32_t srgb_data, spherical_harmonics_t *sh_lighting_info) {
	tex_t result = tex_find(equirectangular_file);
	if (result != nullptr)
		return result;

	const vec3 up   [6] = { -vec3_up, -vec3_up, vec3_forward, -vec3_forward, -vec3_up, -vec3_up };
	const vec3 fwd  [6] = { {1,0,0}, {-1,0,0}, {0,-1,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
	const vec3 right[6] = { {0,0,-1}, {0,0,1}, {1,0,0}, {1,0,0}, {1,0,0}, {-1,0,0} };

	tex_t equirect = tex_create_file(equirectangular_file, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	if (equirect == nullptr)
		return nullptr;
	equirect->header.id = hash_fnv64_string("temp/equirectid");
	material_t convert_material = material_find(default_id_material_equirect);

	material_set_texture( convert_material, "source", equirect );

	tex_t    face    = tex_create(tex_type_rendertarget, equirect->format);
	void    *data[6] = {};
	int      width   = equirect->tex.height / 2;
	int      height  = width;
	size_t   size    = (size_t)width*(size_t)height*tex_format_size(equirect->format);
	tex_set_colors(face, width, height, nullptr);
	for (size_t i = 0; i < 6; i++) {
		material_set_vector(convert_material, "up",      { up   [i].x, up   [i].y, up   [i].z, 0 });
		material_set_vector(convert_material, "right",   { right[i].x, right[i].y, right[i].z, 0 });
		material_set_vector(convert_material, "forward", { fwd  [i].x, fwd  [i].y, fwd  [i].z, 0 });

		render_blit (face, convert_material);
		data[i] = malloc(size);
		tex_get_data(face, data[i], size);
	}

	result = tex_create(tex_type_image | tex_type_cubemap, equirect->format);
	tex_set_color_arr(result, width, height, (void**)&data, 6, sh_lighting_info);
	tex_set_id       (result, equirectangular_file);

	material_release(convert_material);
	tex_release(equirect);
	tex_release(face);

	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_files(const char **cube_face_file_xxyyzz, bool32_t srgb_data, spherical_harmonics_t *sh_lighting_info) {
	tex_t result = tex_find(cube_face_file_xxyyzz[0]);
	if (result != nullptr)
		return result;

	// Load all 6 faces
	uint8_t *data[6] = {};
	int  final_width  = 0;
	int  final_height = 0;
	bool loaded       = true;
	for (size_t i = 0; i < 6; i++) {
		int channels = 0;
		int width    = 0;
		int height   = 0;
		data[i] = stbi_load(assets_file(cube_face_file_xxyyzz[i]), &width, &height, &channels, 4);

		// Check if there were issues, or one of the images is the wrong size!
		if (data[i] == nullptr || 
			(final_width  != 0 && final_width  != width ) ||
			(final_height != 0 && final_height != height)) {
			loaded = false;
			log_errf("Issue loading cubemap image '%s', file not found, invalid image format, or faces of different sizes?", cube_face_file_xxyyzz[i]);
			break;
		}
		final_width  = width;
		final_height = height;
	}

	// free memory if we failed
	if (!loaded) {
		for (size_t i = 0; i < 6; i++) {
			free(data[i]);
		}
		return nullptr;
	}

	// Create with the data we have
	result = tex_create(tex_type_image | tex_type_cubemap, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	tex_set_color_arr(result, final_width, final_height, (void**)&data, 6, sh_lighting_info);
	tex_set_id       (result, cube_face_file_xxyyzz[0]);
	for (size_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

void tex_release(tex_t texture) {
	if (texture == nullptr)
		return;
	assets_releaseref(texture->header);
}

///////////////////////////////////////////

void tex_destroy(tex_t tex) {
	skg_tex_destroy(&tex->tex);
	if (tex->depth_buffer != nullptr) tex_release(tex->depth_buffer);
	
	*tex = {};
}

///////////////////////////////////////////

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info) {
	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->tex.width != width || texture->tex.height != height || texture->tex.array_count != data_count;
	if (!different_size && (data == nullptr || *data == nullptr))
		return;
	if (!skg_tex_is_valid(&texture->tex) || different_size || (!different_size && !dynamic)) {
		skg_tex_destroy(&texture->tex);

		if (!different_size && !dynamic)
			texture->type &= tex_type_dynamic;

		skg_tex_fmt_  format   = (skg_tex_fmt_)texture->format;
		skg_use_      use      = texture->type & tex_type_dynamic ? skg_use_dynamic  : skg_use_static;
		skg_mip_      use_mips = texture->type & tex_type_mips    ? skg_mip_generate : skg_mip_none;
		skg_tex_type_ type     = skg_tex_type_image;
		if      (texture->type & tex_type_cubemap)      type = skg_tex_type_cubemap;
		else if (texture->type & tex_type_depth)        type = skg_tex_type_depth;
		else if (texture->type & tex_type_rendertarget) type = skg_tex_type_rendertarget;
		texture->tex = skg_tex_create(type, use, format, use_mips);

		skg_tex_set_contents_arr(&texture->tex, (const void**)data, data_count, width, height);
		if (texture->depth_buffer != nullptr)
			tex_set_colors(texture->depth_buffer, width, height, nullptr);
	} else if (dynamic) {
		skg_tex_set_contents_arr(&texture->tex, (const void**)data, data_count, width, height);
	} else {
		log_warn("Attempting additional writes to a non-dynamic texture!");
	}
}

///////////////////////////////////////////

void tex_set_colors(tex_t texture, int32_t width, int32_t height, void *data) {
	void *data_arr[1] = { data };
	tex_set_color_arr(texture, width, height, data_arr, 1);
}

///////////////////////////////////////////

void tex_set_options(tex_t texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
	texture->address_mode = address_mode;
	texture->anisotropy   = anisotropy_level;
	texture->sample_mode  = sample;

	skg_tex_address_ skg_addr;
	switch (address_mode) {
	case tex_address_clamp:  skg_addr = skg_tex_address_clamp;  break;
	case tex_address_wrap:   skg_addr = skg_tex_address_repeat; break;
	case tex_address_mirror: skg_addr = skg_tex_address_mirror; break;
	default: skg_addr = skg_tex_address_repeat;
	}

	skg_tex_sample_ skg_sample;
	switch (sample) {
	case tex_sample_linear:     skg_sample = skg_tex_sample_linear;      break; // Technically trilinear
	case tex_sample_point:      skg_sample = skg_tex_sample_point;       break;
	case tex_sample_anisotropic:skg_sample = skg_tex_sample_anisotropic; break;
	default: skg_sample = skg_tex_sample_linear;
	}

	skg_tex_settings(&texture->tex, skg_addr, skg_sample, anisotropy_level);
}

///////////////////////////////////////////

tex_format_ tex_get_format(tex_t texture) {
	return texture->format;
}

///////////////////////////////////////////

int32_t tex_get_width(tex_t texture) {
	return texture->tex.width;
}

///////////////////////////////////////////

int32_t tex_get_height(tex_t texture) {
	return texture->tex.height;
}

///////////////////////////////////////////

void tex_set_sample(tex_t texture, tex_sample_ sample) {
	texture->sample_mode = sample;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

tex_sample_ tex_get_sample(tex_t texture) {
	return texture->sample_mode;
}

///////////////////////////////////////////

void tex_set_address(tex_t texture, tex_address_ address_mode) {
	texture->address_mode = address_mode;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

tex_address_ tex_get_address(tex_t texture) {
	return texture->address_mode;
}

///////////////////////////////////////////

void tex_set_anisotropy(tex_t texture, int32_t anisotropy_level) {
	texture->anisotropy = anisotropy_level;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->anisotropy);
}

///////////////////////////////////////////

int32_t tex_get_anisotropy(tex_t texture) {
	return texture->anisotropy;
}

///////////////////////////////////////////

size_t tex_format_size(tex_format_ format) {
	switch (format) {
	case tex_format_depth32:
	case tex_format_depthstencil:
	case tex_format_r32:
	case tex_format_rgba32:
	case tex_format_rgba32_linear: return sizeof(color32);
	case tex_format_rgba64:        return sizeof(uint16_t)*4;
	case tex_format_rgba128:       return sizeof(color128);
	case tex_format_r16:
	case tex_format_depth16:       return sizeof(uint16_t);
	case tex_format_r8:            return sizeof(uint8_t);
	default: return sizeof(color32);
	}
}

///////////////////////////////////////////

tex_format_ tex_get_tex_format(int64_t native_fmt) {
	skg_tex_fmt_ skg_fmt = skg_tex_fmt_from_native(native_fmt);

	// tex_format_ should be kept to match skg_tex_fmt_, so this should 
	// always be valid.
	return (tex_format_)skg_fmt;
}

///////////////////////////////////////////

void tex_get_data(tex_t texture, void *out_data, size_t out_data_size) {
	memset(out_data, 0, out_data_size);
	if (!skg_tex_get_contents(&texture->tex, out_data, out_data_size))
		log_warn("Couldn't get texture contents!");
}

///////////////////////////////////////////

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution, spherical_harmonics_t* sh_lighting_info) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba128);
	if (result == nullptr) {
		return nullptr;
	}
	gradient_dir = vec3_normalize(gradient_dir);

	int32_t size  = resolution;
	// make size a power of two
	int32_t power = (int32_t)logf((float)size);
	if (pow(2, power) < size)
		power += 1;
	size = (int32_t)pow(2, power);

	float    half_px = 0.5f / size;
	int32_t  size2 = size * size;
	color128*data[6];
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

			float pct = (vec3_dot(pt, gradient_dir)+1)*0.5f;
			data[i][x + y * size] = gradient_get(gradient_bot_to_top, pct);
		}
		}
	}

	tex_set_color_arr(result, (int32_t)size, (int32_t)size, (void**)data, 6, sh_lighting_info);
	for (int32_t i = 0; i < 6; i++) {
		free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap_sh(const spherical_harmonics_t& lookup, int32_t face_size) {
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

} // namespace sk