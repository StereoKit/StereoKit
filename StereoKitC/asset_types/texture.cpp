// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../platforms/platform.h"
#include "../libraries/qoi.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_halffloat.h"
#include "../libraries/profiler.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../spherical_harmonics.h"
#include "texture.h"
#include "texture_.h"
#include "texture_compression.h"

#define STBI_NO_STDIO
#include "../libraries/stb_image.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

namespace sk {

bool   tex_load_image_data(void* data, size_t data_size, bool32_t srgb_data, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data_arr);
bool   tex_load_image_info(void* data, size_t data_size, bool32_t srgb_data, tex_type_* ref_image_type, tex_format_* out_format, int32_t *out_width, int32_t *out_height, int32_t* out_array_count, int32_t* out_mip_count);
void   tex_update_label   (tex_t texture);
size_t tex_format_pitch   (tex_format_ format, int32_t width);
void  _tex_set_options    (skg_tex_t* texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level);
bool  _tex_valid_lighting_source(const tex_t cubemap_texture, bool log);
bool  _tex_get_cubemap_lighting (const tex_t cubemap_texture, spherical_harmonics_t* out_lighting);

const char *tex_msg_load_failed           = "Texture file failed to load: %s";
const char *tex_msg_invalid_fmt           = "Texture invalid format: %s";
const char *tex_msg_invalid_cubemap       = "Texture not recognized as valid cubemap format: %s";
const char *tex_msg_nested_arrays         = "Texture array cannot contain nested array texture: %s";
const char *tex_msg_mismatched_images     = "Texture array mismatched format or size: %s";
const char *tex_msg_inconsistent_parse    = "Texture data doesn't match earlier parse: %s";
const char *tex_msg_requires_rendertarget = "Zbuffer can only be attached to a rendertarget!";
const char *tex_msg_requires_depth        = "Zbuffer must be a depth texture!";

tex_t tex_error_texture   = nullptr;
tex_t tex_loading_texture = nullptr;

///////////////////////////////////////////
// Texture loading stages                //
///////////////////////////////////////////

struct tex_load_t {
	bool32_t    is_srgb;
	char      **file_names;
	int32_t     file_count;

	void      **file_data;
	size_t     *file_sizes;

	void      **color_data;
	int32_t     color_width;
	int32_t     color_height;
	int32_t     color_array_count;
	int32_t     color_mip_count;
	tex_format_ color_format;
};

///////////////////////////////////////////

void tex_load_free(asset_header_t *, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;

	for (int32_t i = 0; i < data->file_count; i++) {
		if (data->file_names != nullptr) sk_free(data->file_names[i]);
		if (data->file_data  != nullptr) sk_free(data->file_data [i]);
		if (data->color_data != nullptr) sk_free(data->color_data[i]);
	}
	sk_free(data->file_names);
	sk_free(data->file_sizes);
	sk_free(data->file_data);
	sk_free(data->color_data);
	sk_free(data);
}

///////////////////////////////////////////

bool32_t tex_load_arr_files_shared(asset_task_t* task, asset_header_t* asset, void* job_data) {
	profiler_zone();

	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	data->file_data  = sk_malloc_t(void *, data->file_count);
	data->file_sizes = sk_malloc_t(size_t, data->file_count);

	int32_t     final_width       = 0;
	int32_t     final_height      = 0;
	int32_t     final_array_count = 0;
	int32_t     final_mip_count   = 0;
	tex_format_ final_format      = tex_format_none;

	// Load all files
	for (int32_t i = 0; i < data->file_count; i++) {
		// Read from file
		bool32_t loaded = platform_read_file(data->file_names[i], &data->file_data[i], &data->file_sizes[i]);
		if (!loaded) {
			log_warnf(tex_msg_load_failed, data->file_names[i]);
			tex->header.state = asset_state_error_not_found;
			return false;
		}

		// Grab the image metadata
		int32_t     curr_width       = 0;
		int32_t     curr_height      = 0;
		int32_t     curr_array_count = 0;
		int32_t     curr_mip_count   = 0;
		tex_format_ curr_format      = tex_format_none;
		if (!tex_load_image_info(data->file_data[i], data->file_sizes[i], data->is_srgb, &tex->type, &curr_format, &curr_width, &curr_height, &curr_array_count, &curr_mip_count)) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}

		// For multiple images, they should all be the same size/format/layout.
		if ((final_width       != 0          && final_width       != curr_width      ) ||
			(final_height      != 0          && final_height      != curr_height     ) ||
			(final_format != tex_format_none && final_format      != curr_format     ) ||
			(final_array_count != 0          && final_array_count != curr_array_count) ||
			(final_mip_count   != 0          && final_mip_count   != curr_mip_count  ) ) {
			log_warnf(tex_msg_mismatched_images, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}
		// If the user has specified multiple image files, those files cannot
		// be array textures themselves. Or rather, they could, but we haven't
		// implemented that scenario here yet.
		if (data->file_count > 1 && curr_array_count > 1) {
			log_warnf(tex_msg_nested_arrays, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}
		final_width       = curr_width;
		final_height      = curr_height;
		final_array_count = curr_array_count;
		final_mip_count   = curr_mip_count;
		final_format      = curr_format;
	}
	data->color_width       = final_width;
	data->color_height      = final_height;
	data->color_array_count = final_array_count * data->file_count;
	data->color_mip_count   = final_mip_count;
	data->color_format      = final_format;
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_arr_files(asset_task_t *task, asset_header_t *asset, void *job_data) {
	bool32_t result = tex_load_arr_files_shared(task, asset, job_data);

	if (!result)
		return false;

	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	tex_set_meta(tex, data->color_width, data->color_height, data->color_format);
	assets_task_set_complexity(task, data->color_width * data->color_height * data->color_array_count);
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_arr_parse(asset_task_t *, asset_header_t *asset, void *job_data) {
	profiler_zone();

	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	data->color_data = sk_malloc_t(void*, data->file_count * data->color_array_count);

	// Parse all files
	int32_t array_index = 0;
	for (int32_t i = 0; i < data->file_count; i++) {
		int32_t     width       = 0;
		int32_t     height      = 0;
		int32_t     array_count = 0;
		int32_t     mip_count   = 0;
		tex_format_ format      = tex_format_none;
		if (!tex_load_image_data(data->file_data[i], data->file_sizes[i], data->is_srgb, &tex->type, &format, &width, &height, &array_count, &mip_count, &data->color_data[array_index])) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			goto end;
		}
		array_index += array_count;

		// Make sure the data in this image matches what we extracted in
		// earlier phases of texture creation. If it doesn't, then something
		// has gone very wrong!
		if (tex->format           != format ||
			data->color_width     != width  ||
			data->color_height    != height ||
			data->color_mip_count != mip_count) {
			log_warnf(tex_msg_inconsistent_parse, data->file_names[i]);
			tex->header.state = asset_state_error;
			goto end;
		}
	}

	if (data->color_array_count != array_index) {
		log_warnf(tex_msg_inconsistent_parse, data->file_names[0]);
		tex->header.state = asset_state_error;
		goto end;
	}

end:

	// Release file memory now that we're done with it
	for (int32_t i = 0; i < data->file_count; i++)
		sk_free(data->file_data[i]);

	if (tex->header.state >= asset_state_none) {
		tex->header.state = asset_state_loaded_meta;
		return true;
	} else {
		tex_set_fallback(tex, tex_error_texture);
		return false;
	}
}

///////////////////////////////////////////

bool32_t tex_load_arr_upload(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	// Create with the data we have
	tex_set_color_arr_mips(tex, tex->width, tex->height, data->color_data, data->color_array_count, data->color_mip_count);

	return true;
}

///////////////////////////////////////////

void tex_load_on_failure(asset_header_t *asset, void *) {
	tex_set_fallback((tex_t)asset, tex_error_texture);
}

///////////////////////////////////////////

bool tex_load_image_info(void *data, size_t data_size, bool32_t srgb_data, tex_type_* ref_image_type, tex_format_* out_format, int32_t *out_width, int32_t *out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	// Check STB image formats
	int32_t comp;
	bool success = stbi_info_from_memory((const stbi_uc*)data, (int)data_size, out_width, out_height, &comp) == 1;
	if (success) {
		*out_mip_count   = 1;
		*out_array_count = 1;
		if (stbi_is_hdr_from_memory((stbi_uc *)data, (int)data_size)) *out_format = tex_format_rg11b10;
		else                                                          *out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		return true;
	}

	// Check QOI
	qoi_desc q_desc = {};
	success = qoi_info(data, (int)data_size, &q_desc);
	if (success) {
		*out_width       = q_desc.width;
		*out_height      = q_desc.height;
		*out_mip_count   = 1;
		*out_array_count = 1;
		if (q_desc.colorspace == QOI_LINEAR) *out_format = tex_format_rgba32_linear;
		else                                 *out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		return true;
	}

	if (ktx2_info(data, data_size, ref_image_type, out_format, out_width, out_height, out_array_count, out_mip_count))
		return true;

	if (basisu_info(data, data_size, out_format, out_width, out_height, out_array_count, out_mip_count))
		return true;

	return false;
}

///////////////////////////////////////////

bool tex_load_image_data(void *data, size_t data_size, bool32_t srgb_data, tex_type_* ref_image_type, tex_format_ *out_format, int32_t *out_width, int32_t *out_height, int32_t *out_array_count, int32_t *out_mip_count, void **out_data_arr) {
	int32_t channels = 0;

	// Check for an stbi HDR image
	if (stbi_is_hdr_from_memory((stbi_uc*)data, (int)data_size)) {
		*out_format      = tex_format_rg11b10;
		*out_array_count = 1;
		*out_mip_count   = 1;
		float* full = stbi_loadf_from_memory((stbi_uc*)data, (int)data_size, out_width, out_height, &channels, 4);

		if (full == nullptr) return false;

		// stb loads HDR as full float, which is pretty intense! We can pretty
		// safely drop it to packed floats and save ourselves 75% memory and a
		// pile of bandwidth/performance.
		int32_t   ct     = *out_width * *out_height;
		uint32_t* packed = sk_malloc_t(uint32_t, ct);
		for (int32_t i=0; i < ct; i += 1) {
			packed[i] = fhf_f32_to_r11g11ba10f(&full[i * 4]);
		}
		*out_data_arr = packed;

		free(full);

		return true;
	}

	// Check through stbi's list of image formats
	*out_data_arr = stbi_load_from_memory ((stbi_uc*)data, (int)data_size, out_width, out_height, &channels, 4);
	if (*out_data_arr != nullptr) {
		*out_format      = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		*out_array_count = 1;
		*out_mip_count   = 1;
		return *out_data_arr != nullptr;
	}

	// Check for qoi images
	qoi_desc q_desc = {};
	*out_data_arr = qoi_decode(data, (int)data_size, &q_desc, 4);
	if (*out_data_arr != nullptr) {
		*out_width       = q_desc.width;
		*out_height      = q_desc.height;
		*out_array_count = 1;
		*out_mip_count   = 1;
		// If QOI claims it's linear, then we'll go with that!
		if (q_desc.colorspace == QOI_LINEAR) *out_format = tex_format_rgba32_linear;
		else                                 *out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		return *out_data_arr != nullptr;
	}

	// Check for KTX2
	if (ktx2_decode(data, data_size, ref_image_type, out_format, out_width, out_height, out_array_count, out_mip_count, out_data_arr))
		return true;

	// Check for basisu
	if (basisu_decode(data, data_size, out_format, out_width, out_height, out_array_count, out_mip_count, out_data_arr))
		return true;

	return false;
}

///////////////////////////////////////////
// Texture creation functions            //
///////////////////////////////////////////

asset_task_t tex_make_loading_task(tex_t texture, void *load_data, const asset_load_action_t *actions, int32_t action_count, int32_t priority, float complexity) {
	asset_task_t task = {};
	task.asset        = (asset_header_t*)texture;
	task.free_data    = tex_load_free;
	task.on_failure   = tex_load_on_failure;
	task.load_data    = load_data;
	task.actions      = (asset_load_action_t *)actions;
	task.action_count = action_count;
	task.priority     = priority;
	task.sort         = asset_sort(priority, complexity);
	return task;
}

///////////////////////////////////////////

tex_t tex_create_file_type(const char *file, tex_type_ type, bool32_t srgb_data, int32_t priority) {
	tex_t result = tex_find(file);
	if (result != nullptr)
		return result;

	result = tex_create(type);
	tex_set_id(result, file);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_malloc_zero_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(file);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_files,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_upload, backend_graphics_get() == backend_graphics_d3d11 ? asset_thread_asset : asset_thread_gpu },
	};
	assets_add_task( tex_make_loading_task(result, load_data, actions, _countof(actions), priority, 0) );

	return result;
}

///////////////////////////////////////////

tex_t tex_create_file(const char *file, bool32_t srgb_data, int32_t priority) {
	return tex_create_file_type(file, tex_type_image, srgb_data, priority);
}

///////////////////////////////////////////

tex_t tex_create_mem_type(tex_type_ type, void *data, size_t data_size, bool32_t srgb_data, int32_t priority) {
	tex_t result = tex_create(type);

	tex_load_t *load_data = sk_malloc_zero_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_sizes    = sk_malloc_t(size_t, 1);
	load_data->file_data     = sk_malloc_t(void *, 1);
	load_data->file_names[0] = string_copy("(memory)");
	load_data->file_sizes[0] = data_size;
	load_data->file_data [0] = sk_malloc(sizeof(uint8_t) * data_size);
	memcpy(load_data->file_data[0], data, data_size);

	// Grab the file meta right away since we already have the file data, no
	// point in delaying that until the task.
	tex_format_ format = tex_format_none;
	if (!tex_load_image_info(load_data->file_data[0], load_data->file_sizes[0], load_data->is_srgb, &result->type, &format, &load_data->color_width, &load_data->color_height, &load_data->color_array_count, &load_data->color_mip_count)) {
		log_warnf(tex_msg_invalid_fmt, load_data->file_names[0]);
		result->header.state = asset_state_error_unsupported;
		return result;
	}
	tex_set_meta(result, load_data->color_width, load_data->color_height, format);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_upload, backend_graphics_get() == backend_graphics_d3d11 ? asset_thread_asset : asset_thread_gpu },
	};
	assets_add_task( tex_make_loading_task(result, load_data, actions, _countof(actions), priority, (float)(load_data->color_width * load_data->color_height)) );

	return result;
}

///////////////////////////////////////////

tex_t tex_create_mem(void *data, size_t data_size, bool32_t srgb_data, int32_t priority) {
	return tex_create_mem_type(tex_type_image, data, data_size, srgb_data, priority);
}

///////////////////////////////////////////

tex_t tex_create(tex_type_ type, tex_format_ format) {
	tex_t result = (tex_t)assets_allocate(asset_type_tex);
	result->owned  = true;
	result->type   = type;
	result->format = format;
	result->address_mode = tex_address_wrap;
	result->sample_mode  = tex_sample_linear;
	result->anisotropy   = 4;
	result->header.state = asset_state_none;

	tex_set_fallback(result, tex_loading_texture);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_rendertarget(int32_t width, int32_t height, int32_t msaa, tex_format_ color_format, tex_format_ depth_format) {
	tex_t result;
	if (color_format == tex_format_none && depth_format != tex_format_none) {
		result = tex_create(tex_type_image_nomips | tex_type_depthtarget, depth_format);

		tex_set_color_arr(result, width, height, nullptr, 1, msaa);
	} else {
		result = tex_create(tex_type_image_nomips | tex_type_rendertarget, color_format);

		tex_set_color_arr(result, width, height, nullptr, 1, msaa);
		if (depth_format != tex_format_none)
			tex_add_zbuffer(result, depth_format);
	}
	return result;
}

///////////////////////////////////////////

tex_t tex_create_color32(color32 *data, int32_t width, int32_t height, bool32_t srgb_data) {
	tex_t result = tex_create(tex_type_image, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	tex_set_colors(result, width, height, data);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_color128(color128 *data, int32_t width, int32_t height, bool32_t srgb_data) {
	tex_t    result = tex_create(tex_type_image, srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear);
	color32 *color  = sk_malloc_t(color32, width * height);
	for (int32_t i = 0; i < width*height; i++)
		color[i] = color_to_32(data[i]);
	tex_set_colors(result, width, height, color);

	sk_free(color);
	return result;
}

///////////////////////////////////////////

tex_t _tex_create_file_arr(tex_type_ type, const char **files, int32_t file_count, bool32_t srgb_data, int32_t priority) {
	// Hash the names of all of the files together
	id_hash_t hash = default_hash_root;
	for (int32_t i = 0; i < file_count; i++) {
		hash = hash_string_with(files[i], hash);
	}
	char file_id[64];
	snprintf(file_id, sizeof(file_id), "sk/tex/array/%" PRIu64, hash);

	// And see if it's already been loaded
	tex_t result = tex_find(file_id);
	if (result != nullptr) {
		return result;
	}

	result = tex_create(type);
	tex_set_id(result, file_id);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_malloc_zero_t(tex_load_t, 1);
	load_data->is_srgb    = srgb_data;
	load_data->file_count = file_count;
	load_data->file_names = sk_malloc_t(char *, file_count);
	for (int32_t i = 0; i < file_count; i++) {
		load_data->file_names[i] = string_copy(files[i]);
	}

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_files,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_upload, backend_graphics_get() == backend_graphics_d3d11 ? asset_thread_asset : asset_thread_gpu },
	};
	assets_add_task( tex_make_loading_task(result, load_data, actions, _countof(actions), priority, 0) );


	return result;
}

///////////////////////////////////////////

tex_t tex_create_file_arr(const char **files, int32_t file_count, bool32_t srgb_data, int32_t priority) {
	return _tex_create_file_arr(tex_type_image, files, file_count, srgb_data, priority);
}
///////////////////////////////////////////

tex_t tex_create_cubemap_file(const char *cubemap_file, bool32_t srgb_data, int32_t priority) {
	profiler_zone();

	char cubemap_id[64];
	snprintf(cubemap_id, sizeof(cubemap_id), "sk/tex/cubemap/%" PRIu64, hash_string(cubemap_file));

	tex_t result = tex_find(cubemap_id);
	if (result != nullptr) {
		return result;
	}

	result = tex_create(tex_type_image | tex_type_cubemap);
	tex_set_id(result, cubemap_id);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_malloc_zero_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(cubemap_file);

	///////////////////////////////////////////

	bool32_t (*load)(asset_task_t*, asset_header_t*, void*) = [](asset_task_t* task, asset_header_t* asset, void* job_data) {
		if (!tex_load_arr_files_shared(task, asset, job_data))
			return (bool32_t)false;

		tex_load_t* data = (tex_load_t*)job_data;
		tex_t       tex = (tex_t)asset;

		int32_t size_w, size_h;
		if (data->color_array_count == 6) { // file contains a pre-built cubemap
			size_w = data->color_width;
			size_h = data->color_height;
		/*} else if (data->color_array_count == 1 && data->color_width / 4 == data->color_height / 3) { // Cubemap crosses are 4x3 aspect ratio
			size_w = size_h = data->color_height / 3;
		} else if (data->color_array_count == 1 && data->color_width / 6 == data->color_height    ) { // Cubemap lists are 6x1 aspect ratio
			size_w = size_h = data->color_height;*/
		} else if (data->color_array_count == 1) { // We'll treat this like an equirect, generally they're 2x1, but anything could work.
			size_w = size_h = data->color_height / 2;
		} else {
			log_warnf(tex_msg_invalid_cubemap, data->file_names[0]);
			tex->header.state = asset_state_error_unsupported;
			return (bool32_t)false;
		}

		tex_set_meta(tex, size_w, size_h, data->color_format);
		assets_task_set_complexity(task, size_w * size_h * 6);
		return (bool32_t)true;
	};

	///////////////////////////////////////////

	bool32_t(*upload)(asset_task_t*, asset_header_t*, void*) = [](asset_task_t* task, asset_header_t * asset, void* job_data) {
		profiler_zone();

		tex_load_t *data = (tex_load_t *)job_data;
		tex_t       tex  = (tex_t)asset;

		// If this is already a cubemap, we don't need to do anything fancy
		// here, just pass it along to our normal texture upload code.
		if (data->color_array_count == 6) {
			return tex_load_arr_upload(task, asset, job_data);
		}

		const vec3 up   [6] = { vec3_up, vec3_up, -vec3_forward, vec3_forward, vec3_up, vec3_up };
		const vec3 fwd  [6] = { {1,0, 0}, {-1,0,0}, {0,-1,0}, {0,1,0}, {0,0,1}, { 0,0,-1} };
		const vec3 right[6] = { {0,0,-1}, { 0,0,1}, {1, 0,0}, {1,0,0}, {1,0,0}, {-1,0, 0} };

		tex_t equirect = tex_create(tex_type_image_nomips, tex->format);
		tex_set_color_arr(equirect, data->color_width, data->color_height, data->color_data, data->file_count);
		tex_set_address  (equirect, tex_address_clamp);
	
		material_t convert_material = material_find(default_id_material_equirect);
		material_set_texture(convert_material, "source", equirect);

		tex_t    face           = tex_create(tex_type_rendertarget | tex_type_mips, equirect->format);
		void    *face_data[6]   = {};
		size_t   size           = tex_format_size(face->format, tex->width, tex->height);
		uint32_t mip_count      = skg_mip_count(tex->width, tex->height);
		uint32_t total_tex_size = 0;
		for (int32_t mip_level = 0; mip_level < mip_count; mip_level++) {
			int32_t  width  = 0;
			int32_t  height = 0;
			skg_mip_dimensions(tex->width, tex->height, mip_level, &width, &height);
			total_tex_size += skg_tex_fmt_memory(equirect->tex.format, width, height);
		}
		tex_set_color_arr_mips(face, tex->width, tex->height, nullptr, 1, mip_count);

		for (int32_t i = 0; i < 6; i++) {
			material_set_vector4(convert_material, "up",      { up   [i].x, up   [i].y, up   [i].z, 0 });
			material_set_vector4(convert_material, "right",   { right[i].x, right[i].y, right[i].z, 0 });
			material_set_vector4(convert_material, "forward", { fwd  [i].x, fwd  [i].y, fwd  [i].z, 0 });

			face_data[i] = sk_malloc(total_tex_size);

			// Blit conversion _definitely_ needs executed on the GPU thread
			struct blit_t {
				tex_t      face;
				material_t material;
				void      *face_data;
			};
			blit_t blit_data = { face, convert_material, face_data[i] };
			assets_execute_gpu([](void *data) {
				blit_t *blit_data = (blit_t *)data;
				render_blit (blit_data->face, blit_data->material);
				tex_gen_mips(blit_data->face);

				uint32_t mip_count = skg_mip_count(blit_data->face->width, blit_data->face->height);
				uint8_t* mip_data  = (uint8_t*)blit_data->face_data;
				for(int32_t mip_level = 0; mip_level < mip_count; mip_level++) {
					int32_t  width  = 0;
					int32_t  height = 0;
					skg_mip_dimensions(blit_data->face->width, blit_data->face->height, mip_level, &width, &height);
					size_t   mem_size = skg_tex_fmt_memory(blit_data->face->tex.format, width, height);

					tex_get_data(blit_data->face, mip_data, mem_size, mip_level);

					mip_data += mem_size;
				}
				return (bool32_t)true;
			}, &blit_data);

#if defined(SKG_OPENGL)
			void*    tmp        = sk_malloc(tex_format_pitch(face->format, tex->width));
			uint8_t* mip_data   = (uint8_t*)face_data[i];
			for (int32_t mip_level = 0; mip_level < mip_count; mip_level++) {
				int32_t  width  = 0;
				int32_t  height = 0;
				skg_mip_dimensions(tex->width, tex->height, mip_level, &width, &height);
				size_t line_size = tex_format_pitch(face->format, width);

				for (int32_t y = 0; y < height/2; y++) {
					void* top_line = mip_data + line_size * y;
					void* bot_line = mip_data + line_size * ((height-1) - y);
					memcpy(tmp,      top_line, line_size);
					memcpy(top_line, bot_line, line_size);
					memcpy(bot_line, tmp,      line_size);
				}

				size_t mem_size = skg_tex_fmt_memory(face->tex.format, width, height);
				mip_data += mem_size;
			}
			sk_free(tmp);
#endif
		}

		tex_release(face);
		material_release(convert_material);
		tex_release(equirect);

		// When tex_set_color_arr is called on _mipped_ textures from a non-gpu
		// thread, it uses the deferred context! This can lead to things
		// happening out of order when listening to asset_state_loaded, so we
		// force this call to happen on the GPU thread.
		struct set_tex_t {
			tex_t   tex;
			void**  data;
		};
		set_tex_t set_data = {};
		set_data.tex  = tex;
		set_data.data = (void**)&face_data;
		assets_execute_gpu([](void* data) {
			set_tex_t* set_data = (set_tex_t*)data;
			tex_set_color_arr_mips(set_data->tex, set_data->tex->width, set_data->tex->height, set_data->data, 6, skg_mip_count(set_data->tex->width, set_data->tex->height));
			return (bool32_t)true;
		}, &set_data);
		for (int32_t i = 0; i < 6; i++) {
			sk_free(face_data[i]);
		}

		tex->header.state = asset_state_loaded;
		return (bool32_t)true;
	};

	///////////////////////////////////////////

	static const asset_load_action_t actions[] = {
		asset_load_action_t {load,               asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse, asset_thread_asset},
		asset_load_action_t {upload,             backend_graphics_get() == backend_graphics_d3d11 ? asset_thread_asset : asset_thread_gpu },
	};
	assets_add_task( tex_make_loading_task(result, load_data, actions, _countof(actions), priority, 0) );

	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_files(const char **cube_face_file_xxyyzz, bool32_t srgb_data, int32_t priority) {
	return _tex_create_file_arr(tex_type_image | tex_type_cubemap, cube_face_file_xxyyzz, 6, srgb_data, priority);
}

///////////////////////////////////////////

tex_t tex_copy(const tex_t texture, tex_type_ type, tex_format_ format) {
	profiler_zone();

	tex_t result = tex_create(type, format == tex_format_none ? texture->format : format);
	tex_set_color_arr_mips(result, texture->width, texture->height, nullptr, 1, skg_mip_count(texture->width, texture->height));

	bool wants_mips = (type          & tex_type_mips) > 0;
	bool has_mips   = (texture->type & tex_type_mips) > 0;
	bool generate_mips = has_mips == false && wants_mips == true;

	if (generate_mips) {
		if (type & tex_type_rendertarget || backend_graphics_get() != backend_graphics_d3d11) {
			skg_tex_copy_to (&texture->tex, 0, &result->tex, 0);
			skg_tex_gen_mips(&result->tex);
		} else {
			// D3D11 needs a rendertarget to generate mips!
			skg_tex_t intermediate = skg_tex_create(skg_tex_type_rendertarget, skg_use_static, result->tex.format, skg_mip_generate);
			skg_tex_set_contents_arr(&intermediate, nullptr, texture->tex.array_count, skg_mip_count(texture->width, texture->height), texture->width, texture->height, 1);
			skg_tex_copy_to (&texture->tex, 0, &intermediate, 0);
			skg_tex_gen_mips(&intermediate);
			skg_tex_copy_to (&intermediate, -1, &result->tex, -1);
			skg_tex_destroy (&intermediate);
		}
	} else {
		skg_tex_copy_to(&texture->tex, 0, &result->tex, 0);
	}
	return result;
}

///////////////////////////////////////////

bool32_t tex_gen_mips(tex_t texture) {
	if ((texture->type & tex_type_mips        ) == 0 ||
		(texture->type & tex_type_rendertarget) == 0)
		return false;
	return skg_tex_gen_mips(&texture->tex);
}

///////////////////////////////////////////
// Texture manipulation functions        //
///////////////////////////////////////////

void tex_update_label(tex_t texture) {
#if !defined(SKG_OPENGL) && (defined(_DEBUG) || defined(SK_GPU_LABELS))
	if (texture->header.id_text != nullptr)
		skg_tex_name(&texture->tex, texture->header.id_text);
#else
	(void)texture;
#endif
}

///////////////////////////////////////////

void tex_add_zbuffer(tex_t texture, tex_format_ format) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err(tex_msg_requires_rendertarget);
		return;
	}

	char id[64];
	assets_unique_name(asset_type_tex, "sk/tex/zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex_create(tex_type_depth, format);
	tex_set_id       (texture->depth_buffer, id);
	tex_set_color_arr(texture->depth_buffer, texture->width, texture->height, nullptr, texture->tex.array_count, texture->tex.multisample);
	skg_tex_attach_depth(&texture->tex, &texture->depth_buffer->tex);
	texture->depth_buffer->header.state = asset_state_loaded;
}

///////////////////////////////////////////

void tex_set_zbuffer(tex_t texture, tex_t depth_texture) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err(tex_msg_requires_rendertarget);
		return;
	}
	if (depth_texture != nullptr && !(depth_texture->type & tex_type_depth)) {
		log_err(tex_msg_requires_depth);
		return;
	}
	if (depth_texture != nullptr) tex_addref(depth_texture);

	skg_tex_attach_depth(&texture->tex, depth_texture != nullptr ? &depth_texture->tex : nullptr);

	if (texture->depth_buffer != nullptr) tex_release(texture->depth_buffer);
	texture->depth_buffer = depth_texture;
}

///////////////////////////////////////////

tex_t tex_get_zbuffer(tex_t texture) {
	if (texture->depth_buffer == nullptr)
		return nullptr;

	tex_addref(texture->depth_buffer);
	return texture->depth_buffer;
}

///////////////////////////////////////////

void tex_set_surface(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count, int32_t multisample, int32_t framebuffer_multisample, bool32_t owned) {
	texture->owned = owned;
	
	if (texture->owned && skg_tex_is_valid(&texture->tex))
		skg_tex_destroy (&texture->tex);

	skg_tex_type_ skg_type = skg_tex_type_image;
	if      (type & tex_type_zbuffer     ) skg_type = skg_tex_type_zbuffer;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;
	else if (type & tex_type_depthtarget ) skg_type = skg_tex_type_depthtarget;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);
	texture->tex    = native_surface == nullptr ? skg_tex_t{} : skg_tex_create_from_existing(native_surface, skg_type, skg_tex_fmt_from_native(native_fmt), width, height, surface_count, multisample, framebuffer_multisample);
	texture->width  = texture->tex.width;
	texture->height = texture->tex.height;

	texture->header.state = skg_tex_is_valid(&texture->tex)
		? asset_state_loaded
		: asset_state_error;
	tex_set_fallback(texture, texture->header.state <= 0
		? tex_error_texture 
		: nullptr);
}

///////////////////////////////////////////

void* tex_get_surface(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded);
	return skg_tex_get_native(&texture->tex);
}

///////////////////////////////////////////

void tex_set_surface_layer(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_index) {
	if (skg_tex_is_valid(&texture->tex))
		skg_tex_destroy (&texture->tex);

	skg_tex_type_ skg_type = skg_tex_type_image;
	if      (type & tex_type_zbuffer     ) skg_type = skg_tex_type_zbuffer;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;
	else if (type & tex_type_depthtarget ) skg_type = skg_tex_type_depthtarget;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);
	texture->tex    = skg_tex_create_from_layer(native_surface, skg_type, skg_tex_fmt_from_native(native_fmt), width, height, surface_index);

	tex_update_label(texture);
}

///////////////////////////////////////////

void tex_set_fallback(tex_t texture, tex_t fallback) {
	if (texture->header.state >= asset_state_loaded && fallback != nullptr) return;

	if (fallback          != nullptr) tex_addref (fallback);
	if (texture->fallback != nullptr) tex_release(texture->fallback);

	texture->fallback  = fallback;
	texture->meta_hash = fallback == nullptr
		? tex_meta_hash(texture )
		: tex_meta_hash(fallback);
}

///////////////////////////////////////////

tex_t tex_find(const char *id) {
	tex_t result = (tex_t)assets_find(id, asset_type_tex);
	if (result != nullptr) {
		tex_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void tex_set_id(tex_t tex, const char *id) {
	assets_set_id(&tex->header, id);
	tex_update_label(tex);
}

///////////////////////////////////////////

const char* tex_get_id(const tex_t texture) {
	return texture->header.id_text;
}

///////////////////////////////////////////

void tex_addref(tex_t texture) {
	assets_addref(&texture->header);
}

///////////////////////////////////////////

void tex_release(tex_t texture) {
	if (texture == nullptr)
		return;
	assets_releaseref(&texture->header);
}

///////////////////////////////////////////

void tex_destroy(tex_t tex) {
	assets_on_load_remove(&tex->header, nullptr);

	sk_free(tex->light_info);
	if (tex->owned && skg_tex_is_valid(&tex->tex)) {
		skg_tex_destroy(&tex->tex);
	}
	if (tex->depth_buffer != nullptr) tex_release(tex->depth_buffer);
	
	*tex = {};
}

///////////////////////////////////////////

asset_state_ tex_asset_state(const tex_t texture) {
	return texture->header.state;
}

///////////////////////////////////////////

void tex_on_load(tex_t texture, void (*on_load)(tex_t texture, void *context), void *context) {
	assets_on_load(&texture->header, (void(*)(asset_header_t*,void*))on_load, context);
}

///////////////////////////////////////////

void tex_on_load_remove(tex_t texture, void (*on_load)(tex_t texture, void *context)) {
	assets_on_load_remove(&texture->header, (void(*)(asset_header_t*,void*))on_load);
}

///////////////////////////////////////////

void _tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **array_data, int32_t array_count, int32_t mip_count, int32_t multisample) {
	profiler_zone();

	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height || texture->tex.array_count != array_count;
	if (!different_size && (array_data == nullptr || *array_data == nullptr))
		return;
	if (!skg_tex_is_valid(&texture->tex) || different_size || (!different_size && !dynamic)) {
		if (!different_size && !dynamic)
			texture->type &= tex_type_dynamic;

		skg_tex_fmt_  format   = (skg_tex_fmt_)texture->format;
		skg_use_      use      = texture->type & tex_type_dynamic ? skg_use_dynamic  : skg_use_static;
		skg_mip_      use_mips = texture->type & tex_type_mips    ? skg_mip_generate : skg_mip_none;
		skg_tex_type_ type     = skg_tex_type_image;
		if (texture->type & tex_type_cubemap) use = (skg_use_)(use | skg_use_cubemap);
		if      (texture->type & tex_type_zbuffer)      type = skg_tex_type_zbuffer;
		else if (texture->type & tex_type_rendertarget) type = skg_tex_type_rendertarget;
		else if (texture->type & tex_type_depthtarget)  type = skg_tex_type_depthtarget;

		skg_tex_t new_tex = skg_tex_create(type, use, format, use_mips);
		_tex_set_options(&new_tex, texture->sample_mode, texture->address_mode, texture->sample_comp, texture->anisotropy);
		skg_tex_set_contents_arr(&new_tex, (const void**)array_data, array_count, mip_count, width, height, multisample);
		skg_tex_t old_tex = texture->tex;
		texture->tex = new_tex;
		if (skg_tex_is_valid(&old_tex))
			skg_tex_destroy (&old_tex);

		tex_set_meta(texture, width, height, texture->format);

		if (texture->depth_buffer != nullptr) {
			tex_set_color_arr(texture->depth_buffer, width, height, nullptr, texture->tex.array_count, multisample);
			tex_set_zbuffer  (texture, texture->depth_buffer);
		}
		tex_update_label(texture);
	} else if (dynamic) {
		skg_tex_set_contents_arr(&texture->tex, (const void**)array_data, array_count, mip_count, width, height, multisample);
	} else {
		log_warn("Attempting additional writes to a non-dynamic texture!");
	}

	if (skg_tex_is_valid(&texture->tex)) {
		if (_tex_valid_lighting_source(texture, false)) {
			texture->light_info = sk_malloc_t(spherical_harmonics_t, 1);
			_tex_get_cubemap_lighting(texture, texture->light_info);
		}

		tex_set_fallback(texture, nullptr);
		texture->header.state = asset_state_loaded;
	} else {
		tex_set_fallback(texture, tex_error_texture);
		texture->header.state = asset_state_error;
	}
}

///////////////////////////////////////////

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t multisample) {
	tex_set_color_arr_mips(texture, width, height, array_data, array_count, 1, multisample);
}

///////////////////////////////////////////

void tex_set_color_arr_mips(tex_t texture, int32_t width, int32_t height, void **array_data, int32_t array_count, int32_t mip_count, int32_t multisample) {
	profiler_zone();

	struct tex_upload_job_t {
		tex_t                  texture;
		int32_t                width;
		int32_t                height;
		void                 **array_data;
		int32_t                array_count;
		int32_t                mip_count;
		int32_t                multisample;
	};
	tex_upload_job_t job_data = {texture, width, height, array_data, array_count, mip_count, multisample};

	// OpenGL doesn't like multiple threads, but D3D is fine with it.
	if (backend_graphics_get() == backend_graphics_d3d11) {
		_tex_set_color_arr(job_data.texture, job_data.width, job_data.height, job_data.array_data, job_data.array_count, job_data.mip_count, job_data.multisample);
	} else {
		assets_execute_gpu([](void *data) {
			tex_upload_job_t *job_data = (tex_upload_job_t *)data;
			_tex_set_color_arr(job_data->texture, job_data->width, job_data->height, job_data->array_data, job_data->array_count, job_data->mip_count, job_data->multisample);
			return (bool32_t)true; 
		}, &job_data);
	}
}

///////////////////////////////////////////

void tex_set_mem(tex_t texture, void* data, size_t data_size, bool32_t srgb_data, bool32_t blocking, int32_t priority) {
	profiler_zone();

	tex_load_t* load_data = sk_malloc_zero_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char*,  1);
	load_data->file_sizes    = sk_malloc_t(size_t, 1);
	load_data->file_data     = sk_malloc_t(void*,  1);
	load_data->file_names[0] = string_copy("(memory)");
	load_data->file_sizes[0] = data_size;
	load_data->file_data [0] = sk_malloc(sizeof(uint8_t) * data_size);
	memcpy(load_data->file_data[0], data, data_size);

	// Grab the file meta right away since we already have the file data, no
	// point in delaying that until the task.
	tex_format_ format = tex_format_none;
	if (!tex_load_image_info(load_data->file_data[0], load_data->file_sizes[0], load_data->is_srgb, &texture->type, &format, &load_data->color_width, &load_data->color_height, &load_data->color_array_count, &load_data->color_mip_count)) {
		log_warnf(tex_msg_invalid_fmt, load_data->file_names[0]);
		texture->header.state = asset_state_error_unsupported;
		return;
	}
	tex_set_meta(texture, load_data->color_width, load_data->color_height, format);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_upload, backend_graphics_get() == backend_graphics_d3d11 ? asset_thread_asset : asset_thread_gpu  }, // DX can multithreaded upload, GL cannot
	};
	asset_task_t task = tex_make_loading_task(texture, load_data, actions, _countof(actions), priority, (float)(load_data->color_width * load_data->color_height));
	if (blocking) {
		for (int32_t i = 0; i < 2; i++) {
			if (!actions[i].action(&task, &texture->header, load_data))
				break;
		}
		tex_load_free(&texture->header, load_data);
	} else {
		assets_add_task(task);
	}
}

///////////////////////////////////////////

bool _tex_valid_lighting_source(const tex_t cubemap_texture, bool log) {
	skg_tex_t* tex = &cubemap_texture->tex;

	if (tex->width != tex->height || tex->array_count != 6) {
		if (log) log_warn("Invalid texture size for calculating spherical harmonics. Must be an equirect image, or have 6 images all same width and height.");
		return false;
	}
	else if (!(tex->format == skg_tex_fmt_rgba32 || tex->format == skg_tex_fmt_rgba32_linear || tex->format == skg_tex_fmt_rgba64f || tex->format == skg_tex_fmt_rgba128 || tex->format == tex_format_rg11b10)) {
		if (log) log_warn("Invalid texture format for calculating spherical harmonics, must be rgba32, rg11b10, rgba64f or rgba128.");
		return false;
	}
	return true;
}

///////////////////////////////////////////

bool _tex_get_cubemap_lighting(const tex_t cubemap_texture, spherical_harmonics_t *out_lighting) {
	*out_lighting = {};

	skg_tex_t* tex = &cubemap_texture->tex;

	if (!_tex_valid_lighting_source(cubemap_texture, true))
		return false;

	int32_t  mip_level = maxi((int32_t)0, (int32_t)skg_mip_count(tex->width, tex->height) - 6);
	int32_t  mip_w, mip_h;
	skg_mip_dimensions(tex->width, tex->height, mip_level, &mip_w, &mip_h);
	size_t   face_size = skg_tex_fmt_memory(tex->format, mip_w, mip_h);
	size_t   cube_size = face_size * 6;
	uint8_t* cube_color_data = (uint8_t*)sk_malloc(cube_size);
	void* data[6];
	for (int32_t f = 0; f < 6; f++) {
		data[f] = cube_color_data + face_size * f;
		skg_tex_get_mip_contents_arr(tex, mip_level, f, data[f], face_size);
	}
	*out_lighting = sh_calculate(data, cubemap_texture->format, mip_w);
	sk_free(cube_color_data);
	return true;
}

///////////////////////////////////////////

spherical_harmonics_t tex_get_cubemap_lighting(tex_t cubemap_texture) {
	profiler_zone();

	assets_block_until(&cubemap_texture->header, asset_state_loaded);

	skg_tex_t *tex = &cubemap_texture->tex;

	if (cubemap_texture->light_info == nullptr) {
		spherical_harmonics_t lighting;
		if (!_tex_get_cubemap_lighting(cubemap_texture, &lighting)) {
			cubemap_texture->light_info = sk_malloc_t(spherical_harmonics_t, 1);
			*cubemap_texture->light_info = lighting;
		}
	}
	return cubemap_texture->light_info ? *cubemap_texture->light_info : spherical_harmonics_t{};
}

///////////////////////////////////////////

void tex_set_colors(tex_t texture, int32_t width, int32_t height, void *data) {
	void *data_arr[1] = { data };
	tex_set_color_arr(texture, width, height, data_arr, 1);
}

///////////////////////////////////////////

void _tex_set_options(skg_tex_t *texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level) {
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

	skg_sample_compare_ skg_compare;
	switch (compare) {
	case tex_sample_comp_none:          skg_compare = skg_sample_compare_none;          break;
	case tex_sample_comp_less:          skg_compare = skg_sample_compare_less;          break;
	case tex_sample_comp_less_or_eq: 	skg_compare = skg_sample_compare_less_or_eq;    break;
	case tex_sample_comp_greater:       skg_compare = skg_sample_compare_greater;       break;
	case tex_sample_comp_greater_or_eq: skg_compare = skg_sample_compare_greater_or_eq; break;
	case tex_sample_comp_equal:         skg_compare = skg_sample_compare_equal;         break;
	case tex_sample_comp_not_equal:     skg_compare = skg_sample_compare_not_equal;     break;
	case tex_sample_comp_always:        skg_compare = skg_sample_compare_always;        break;
	case tex_sample_comp_never:         skg_compare = skg_sample_compare_never;         break;
	default:                            skg_compare = skg_sample_compare_none;          break;
	}

	skg_tex_settings(texture, skg_addr, skg_sample, skg_compare, anisotropy_level);
}

///////////////////////////////////////////

void tex_set_options(tex_t texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level) {
	texture->address_mode = address_mode;
	texture->anisotropy   = anisotropy_level;
	texture->sample_mode  = sample;
	texture->sample_comp  = compare;

	_tex_set_options(&texture->tex, sample, address_mode, texture->sample_comp, anisotropy_level);
	tex_update_label(texture);
}

///////////////////////////////////////////

tex_format_ tex_get_format(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded_meta);
	return texture->format;
}

///////////////////////////////////////////

int32_t tex_get_width(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded_meta);
	return texture->width;
}

///////////////////////////////////////////

int32_t tex_get_height(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded_meta);
	return texture->height;
}

///////////////////////////////////////////

void tex_set_sample(tex_t texture, tex_sample_ sample) {
	texture->sample_mode = sample;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->sample_comp, texture->anisotropy);
}

///////////////////////////////////////////

tex_sample_ tex_get_sample(tex_t texture) {
	return texture->sample_mode;
}

///////////////////////////////////////////

void tex_set_sample_comp(tex_t texture, tex_sample_comp_ compare) {
	texture->sample_comp = compare;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->sample_comp, texture->anisotropy);
}

///////////////////////////////////////////

tex_sample_comp_ tex_get_sample_comp(tex_t texture) {
	return texture->sample_comp;
}

///////////////////////////////////////////

void tex_set_address(tex_t texture, tex_address_ address_mode) {
	texture->address_mode = address_mode;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->sample_comp, texture->anisotropy);
}

///////////////////////////////////////////

tex_address_ tex_get_address(tex_t texture) {
	return texture->address_mode;
}

///////////////////////////////////////////

void tex_set_anisotropy(tex_t texture, int32_t anisotropy_level) {
	texture->anisotropy = anisotropy_level;
	tex_set_options(texture, texture->sample_mode, texture->address_mode, texture->sample_comp, texture->anisotropy);
}

///////////////////////////////////////////

int32_t tex_get_anisotropy(tex_t texture) {
	return texture->anisotropy;
}

///////////////////////////////////////////

int32_t tex_get_mips(tex_t texture) {
	return (texture->type & tex_type_mips)
		? skg_mip_count(tex_get_width(texture), tex_get_height(texture))
		: 1;
}

///////////////////////////////////////////

size_t tex_format_size(tex_format_ format, int32_t width, int32_t height) {
	return skg_tex_fmt_memory((skg_tex_fmt_)format, width, height);
}

///////////////////////////////////////////

size_t tex_format_pitch(tex_format_ format, int32_t width) {
	return skg_tex_fmt_pitch((skg_tex_fmt_)format, width);
}

///////////////////////////////////////////

tex_format_ tex_get_tex_format(int64_t native_fmt) {
	skg_tex_fmt_ skg_fmt = skg_tex_fmt_from_native(native_fmt);

	// tex_format_ should be kept to match skg_tex_fmt_, so this should 
	// always be valid.
	return (tex_format_)skg_fmt;
}

///////////////////////////////////////////

id_hash_t tex_meta_hash(tex_t texture) {
	id_hash_t result = hash_int     (texture->width);
	result           = hash_int_with(texture->height, result);
	// May want to consider texture format or some other items as well, but
	// this is plenty for now.
	return result;
}

///////////////////////////////////////////

void tex_set_meta(tex_t texture, int32_t width, int32_t height, tex_format_ format) {
	texture->width  = width;
	texture->height = height;
	texture->format = format;

	if (texture->fallback == nullptr) {
		texture->meta_hash = tex_meta_hash(texture);
	}

	// Make sure the asset knows it now has meta values, but don't overwrite
	// errors or more advanced progress.
	if (texture->header.state >= 0 && texture->header.state < asset_state_loaded_meta)
		texture->header.state = asset_state_loaded_meta;
}

///////////////////////////////////////////

void tex_get_data(tex_t texture, void* ref_color_buffer, size_t color_buffer_size, int32_t mip_level) {
	if (mip_level > tex_get_mips(texture)) {
		log_warn("Cannot retrieve invalid mip-level!");
		return;
	}

	assets_block_until(&texture->header, asset_state_loaded);

	struct tex_data_job_t {
		tex_t   texture;
		void*   ref_color_buffer;
		size_t  color_buffer_size;
		int32_t mip_level;
	};
	tex_data_job_t job_data = { texture, ref_color_buffer, color_buffer_size, mip_level };

	bool32_t result = assets_execute_gpu([](void *data) {
		tex_data_job_t *job_data = (tex_data_job_t *)data;
		return (bool32_t)skg_tex_get_mip_contents(&job_data->texture->tex, job_data->mip_level, job_data->ref_color_buffer, job_data->color_buffer_size);
	}, &job_data);

	if (!result) {
		log_warn("Couldn't get texture contents!");
		memset(ref_color_buffer, 0, color_buffer_size);
	}
}

///////////////////////////////////////////

void tex_set_loading_fallback(tex_t loading_texture) {
	if (loading_texture != nullptr) {
		assets_block_until(&loading_texture->header, asset_state_loaded);
		if (tex_asset_state(loading_texture) < 0) {
			log_err("Can't assign a texture with an error the default fallback!");
			return;
		}
		tex_addref(loading_texture);
	}
	tex_release(tex_loading_texture);
	tex_loading_texture = loading_texture;
}

///////////////////////////////////////////

void tex_set_error_fallback(tex_t error_texture) {
	if (error_texture != nullptr) {
		assets_block_until(&error_texture->header, asset_state_loaded);
		if (tex_asset_state(error_texture) < 0) {
			log_err("Can't assign a texture with an error the default fallback!");
			return;
		}
		tex_addref(error_texture);
	}
	tex_release(tex_error_texture);
	tex_error_texture = error_texture;
}

///////////////////////////////////////////

tex_t tex_gen_color(color128 color, int32_t width, int32_t height, tex_type_ type, tex_format_ format) {
	uint8_t data[sizeof(color128)] = {};
	size_t  data_step = 0;
	switch (format) {
	case tex_format_rgba32:
	case tex_format_rgba32_linear: { color32  c = color_to_32(color);                             memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_bgra32_linear:
	case tex_format_bgra32:        { color32  c = color_to_32({color.b,color.g,color.r,color.a}); memcpy(data, &c,     sizeof(c)); data_step = sizeof(c);} break;
	case tex_format_rgba128:       { memcpy(data, &color, sizeof(color)); data_step = sizeof(color); } break;
	case tex_format_rgba64f:       { uint16_t c[4]; fhf_f32_to_f16_x4(&color.r, c);               memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_rgb10a2:       { uint32_t c = fhf_f32_to_rgb10a2(&color.r);                   memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_rg11b10:       { uint32_t c = fhf_f32_to_r11g11ba10f(&color.r);               memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_r32:           { float    c = color.r;                                        memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_r16:           { uint16_t c = (uint16_t)(color.r*USHRT_MAX);                  memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	case tex_format_r8:            { uint8_t  c = (uint8_t )(color.r*255.0f   );                  memcpy(data, &c,     sizeof(c)); data_step = sizeof(c); } break;
	default: log_err("tex_gen_color doesn't support the provided color format."); return nullptr;
	}

	// Create an array of color values the size of our texture
	uint8_t *color_data = (uint8_t *)sk_malloc(data_step * width * height);
	uint8_t *color_curr = color_data;
	for (int32_t i = 0; i < width*height; i++) {
		memcpy(color_curr, data, data_step);
		color_curr += data_step;
	}

	// And upload it to the GPU
	tex_t result = tex_create(type, format);
	tex_set_colors(result, width, height, color_data);

	sk_free(color_data);

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_particle(int32_t width, int32_t height, float roundness, gradient_t gradient_linear) {
	if (roundness < 0.00001f)
		roundness = 0.00001f;

	gradient_t grad = gradient_linear;
	if (gradient_linear == nullptr) {
		gradient_key_t keys[2] = {
			{ {1,1,1,0}, 0 },
			{ {1,1,1,1}, 1 }
		};
		grad = gradient_create_keys(keys, 2);
	}
	// Create an array of color values the size of our texture
	color32* color_data = sk_malloc_t(color32, width * height);

	vec2  center   = { width / 2.0f, height / 2.0f };
	float max_dist = fminf((float)width, (float)height) / 2.0f;
	float power    = roundness * 2;
	for (int32_t px_y=0; px_y<height; px_y++) {
		for (int32_t px_x=0; px_x<width; px_x++) {
			// Constrain the point to the top right quadrant
			vec2 pt = {
				fabsf(px_x - center.x) / max_dist,
				fabsf(px_y - center.y) / max_dist};
			float minkowski_dist = powf(powf(pt.x, power) + powf(pt.y, power), 1.0f / power);

			color_data[px_x + px_y*width] = gradient_get32(grad, 1-minkowski_dist);
		}
	}

	// And upload it to the GPU
	tex_t result = tex_create(tex_type_image, tex_format_rgba32_linear);
	tex_set_colors(result, width, height, color_data);

	sk_free(color_data);
	if (gradient_linear == nullptr)
		gradient_destroy(grad);

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rg11b10);
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
	uint32_t*data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = sk_malloc_t(uint32_t, size2);
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
			vec3    pl    = vec3_lerp(p1, p4, py);
			vec3    pr    = vec3_lerp(p2, p3, py);
			int32_t ysize = y * size;
		for (int32_t x = 0; x < size; x++) {
			float px = x / (float)size + half_px;

			// Top face is flipped on both axes
			if (i == 2) {
				px = 1 - px;
			}

			vec3     pt  = vec3_normalize(vec3_lerp(pl, pr, px));
			float    pct = (vec3_dot(pt, gradient_dir)+1)*0.5f;
			color128 c   = gradient_get(gradient_bot_to_top, pct);
			data[i][x + ysize] = fhf_f32_to_r11g11ba10f(&c.r);
		}
		}
	}

	tex_set_color_arr(result, size, size, (void**)data, 6);

	for (int32_t i = 0; i < 6; i++) {
		sk_free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap_sh(const spherical_harmonics_t& lookup, int32_t face_size, float light_spot_size_pct, float light_spot_intensity) {
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rg11b10);
	if (result == nullptr) {
		return nullptr;
	}

	// Calculate information used to create the light spot
	vec3     light_dir = sh_dominant_dir(lookup);
	color128 light_col = sh_lookup      (lookup, -light_dir) * light_spot_intensity;
	vec3     light_pt  = { 100000,100000,100000 };
	for (int32_t i = 0; i < 6; i++) {
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4 + 1);
		vec3 p3 = math_cubemap_corner(i * 4 + 2);
		plane_t plane = plane_from_points(p1, p2, p3);
		vec3    pt;
		if (!plane_ray_intersect(plane, { vec3_zero, light_dir }, &pt) && vec3_magnitude_sq(pt) < vec3_magnitude_sq(light_pt))
			light_pt = pt;
	}

	int32_t size  = face_size;
	// make size a power of two
	int32_t power = (int32_t)logf((float)size);
	if (pow(2, power) < size)
		power += 1;
	size = (int32_t)pow(2, power);

	float     half_px = 0.5f / size;
	int32_t   size2 = size * size;
	uint32_t *data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = sk_malloc_t(uint32_t, size2);
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
			vec3    pl    = vec3_lerp(p1, p4, py);
			vec3    pr    = vec3_lerp(p2, p3, py);
			int32_t ysize = y * size;
			for (int32_t x = 0; x < size; x++) {
				float px = x / (float)size + half_px;

				// Top face is flipped on both axes
				if (i == 2) {
					px = 1 - px;
				}

				vec3     pt       = vec3_lerp(pl, pr, px);
				vec3     abs_diff = vec3_abs(pt - light_pt);
				float    dist     = fmaxf(fmaxf(abs_diff.x, abs_diff.y), abs_diff.z);
				color128 color    = dist < light_spot_size_pct
					? light_col
					: sh_lookup(lookup, vec3_normalize(pt));

				data[i][x + ysize] = fhf_f32_to_r11g11ba10f(&color.r);
			}
		}
	}

	tex_set_color_arr(result, size, size, (void**)data, 6);

	for (int32_t i = 0; i < 6; i++) {
		sk_free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

uint8_t* unzip_malloc(const uint8_t* buffer, int32_t len, int32_t* out_len) {
	return (uint8_t*)stbi_zlib_decode_malloc((const char*)buffer, len, out_len);
}

} // namespace sk
