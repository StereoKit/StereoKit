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
#include "../systems/defaults.h"
#include "shader.h"
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
void  _tex_set_options    (skr_tex_t* texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level);

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
// Helper functions for sk_renderer API  //
///////////////////////////////////////////

skr_tex_flags_ tex_type_to_skr_flags(tex_type_ type) {
	// Z-buffers are write-only depth attachments (not sampled)
	if (type & tex_type_zbuffer) {
		return skr_tex_flags_writeable;
	}

	// Most textures are sampled
	skr_tex_flags_ flags = skr_tex_flags_readable;
	if (type & tex_type_cubemap)      flags = (skr_tex_flags_)(flags | skr_tex_flags_cubemap);
	if (type & tex_type_dynamic)      flags = (skr_tex_flags_)(flags | skr_tex_flags_dynamic);
	if (type & tex_type_mips)         flags = (skr_tex_flags_)(flags | skr_tex_flags_gen_mips);
	if (type & tex_type_rendertarget) flags = (skr_tex_flags_)(flags | skr_tex_flags_writeable);
	if (type & tex_type_depthtarget)  flags = (skr_tex_flags_)(flags | skr_tex_flags_writeable); // Readable depth (shadow maps)
	return flags;
}

skr_tex_sampler_t tex_get_skr_sampler(tex_t texture) {
	skr_tex_sampler_t sampler = {};

	switch (texture->address_mode) {
	case tex_address_clamp:  sampler.address = skr_tex_address_clamp;  break;
	case tex_address_wrap:   sampler.address = skr_tex_address_wrap;   break;
	case tex_address_mirror: sampler.address = skr_tex_address_mirror; break;
	default:                 sampler.address = skr_tex_address_wrap;
	}

	switch (texture->sample_mode) {
	case tex_sample_linear:      sampler.sample = skr_tex_sample_linear;      break;
	case tex_sample_point:       sampler.sample = skr_tex_sample_point;       break;
	case tex_sample_anisotropic: sampler.sample = skr_tex_sample_anisotropic; break;
	default:                     sampler.sample = skr_tex_sample_linear;
	}

	switch (texture->sample_comp) {
	case tex_sample_comp_none:          sampler.sample_compare = skr_compare_none;          break;
	case tex_sample_comp_less:          sampler.sample_compare = skr_compare_less;          break;
	case tex_sample_comp_less_or_eq:    sampler.sample_compare = skr_compare_less_or_eq;    break;
	case tex_sample_comp_greater:       sampler.sample_compare = skr_compare_greater;       break;
	case tex_sample_comp_greater_or_eq: sampler.sample_compare = skr_compare_greater_or_eq; break;
	case tex_sample_comp_equal:         sampler.sample_compare = skr_compare_equal;         break;
	case tex_sample_comp_not_equal:     sampler.sample_compare = skr_compare_not_equal;     break;
	case tex_sample_comp_always:        sampler.sample_compare = skr_compare_always;        break;
	case tex_sample_comp_never:         sampler.sample_compare = skr_compare_never;         break;
	default:                            sampler.sample_compare = skr_compare_none;          break;
	}

	sampler.anisotropy = texture->anisotropy;
	return sampler;
}

// tex_format_ and skr_tex_fmt_ have matching enum values, so we can cast between them

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
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
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
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
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

		tex_set_color_arr(result, width, height, nullptr, 1, msaa, nullptr);
	} else {
		result = tex_create(tex_type_image_nomips | tex_type_rendertarget, color_format);

		tex_set_color_arr(result, width, height, nullptr, 1, msaa, nullptr);
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
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
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

		skr_cmd_begin();

		// Create the cubemap destination texture directly (NOT using tex_set_color_arr,
		// which would set state to loaded and trigger premature on_load callbacks)
		tex->type = (tex_type_)(tex->type | tex_type_rendertarget);
		skr_tex_flags_    flags   = tex_type_to_skr_flags(tex->type);
		skr_tex_sampler_t sampler = tex_get_skr_sampler(tex);
		skr_vec3i_t       size    = { tex->width, tex->height, 1 };
		skr_err_ err = skr_tex_create(
			(skr_tex_fmt_)tex->format,
			flags,
			sampler,
			size,
			1,  // multisample
			0,  // mip_count = 0 for auto-calculate
			nullptr,  // no initial data
			&tex->gpu_tex);
		if (err != skr_err_success) {
			skr_cmd_end();
			log_err("Failed to create cubemap texture for equirect conversion");
			tex->header.state = asset_state_error;
			return (bool32_t)false;
		}
		tex_set_meta(tex, tex->width, tex->height, tex->format);
		tex_update_label(tex);

		shader_t convert_shader = shader_find(default_id_shader_equirect);

		// Make the texture for our source equirect data
		skr_tex_sampler_t equirect_sampler = {};
		equirect_sampler.sample  = skr_tex_sample_linear;
		equirect_sampler.address = skr_tex_address_wrap;
		skr_tex_data_t tex_data = {};
		tex_data.data        = data->color_data[0];
		tex_data.layer_count = 1;
		tex_data.mip_count   = 1;
		skr_tex_t equirect;
		skr_tex_create((skr_tex_fmt_)tex->format, skr_tex_flags_readable, equirect_sampler, {data->color_width, data->color_height, 1}, 1, 0, &tex_data, &equirect);

		// Make the material for converting equirect to cubemap
		skr_material_info_t mat_info = {};
		mat_info.shader     = &convert_shader->gpu_shader;
		mat_info.write_mask = skr_write_rgba;
		mat_info.depth_test = skr_compare_always;
		skr_material_t convert_mat;
		skr_material_create(mat_info, &convert_mat);
		skr_material_set_tex(&convert_mat, "source", &equirect);

		// Now convert the first layer and generate the rest of the mips
		skr_vec3i_t blit_size = skr_tex_get_size(&tex->gpu_tex);
		skr_recti_t bounds    = { 0, 0, blit_size.x, blit_size.y };
		skr_renderer_blit    (&convert_mat, &tex->gpu_tex, bounds);
		skr_tex_generate_mips(&tex->gpu_tex, nullptr);

		skr_material_destroy(&convert_mat);
		skr_tex_destroy(&equirect);
		shader_release(convert_shader);

		// Compute spherical harmonics on GPU using a small mip level
		skr_vec3i_t base_size = { tex->width, tex->height, 1 };
		int32_t     mip_count = skr_tex_calc_mip_count(base_size);
		int32_t     mip_level = maxi(0, mip_count - 6);
		skr_vec3i_t mip_size  = skr_tex_calc_mip_dimensions(base_size, mip_level);

		skr_buffer_t sh_buffer = {};
		skr_buffer_create(nullptr, 1, sizeof(spherical_harmonics_t), skr_buffer_type_storage, (skr_use_)(skr_use_dynamic | skr_use_compute_write), &sh_buffer);

		skr_compute_t sh_compute = {};
		skr_compute_create(&sk_default_shader_sh_compute->gpu_shader, &sh_compute);

		uint32_t params[4] = { (uint32_t)mip_size.x, (uint32_t)mip_level, 0, 0 };
		skr_compute_set_params(&sh_compute, params, sizeof(params));
		skr_compute_set_tex   (&sh_compute, "source", &tex->gpu_tex);
		skr_compute_set_buffer(&sh_compute, "sh_output", &sh_buffer);
		skr_compute_execute   (&sh_compute, 1, 1, 1);

		// Wait for GPU work to complete before marking texture as loaded
		// This ensures layout transitions are visible to other threads
		skr_future_t future = skr_cmd_end();
		skr_future_wait(&future);

		// Read back SH coefficients and store in texture
		tex->light_info = sk_malloc_t(spherical_harmonics_t, 1);
		skr_buffer_get(&sh_buffer, tex->light_info->coefficients, sizeof(spherical_harmonics_t));

		skr_compute_destroy(&sh_compute);
		skr_buffer_destroy (&sh_buffer);

		tex_set_fallback(tex, nullptr);
		tex->header.state = asset_state_loaded;
		return (bool32_t)true;
	};

	///////////////////////////////////////////

	static const asset_load_action_t actions[] = {
		asset_load_action_t {load,               asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse, asset_thread_asset},
		asset_load_action_t {upload,             asset_thread_asset},
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

	skr_vec3i_t tex_size = { texture->width, texture->height, 1 };
	tex_t       result   = tex_create(type, format == tex_format_none ? texture->format : format);
	tex_set_color_arr_mips(result, texture->width, texture->height, nullptr, 1, skr_tex_calc_mip_count(tex_size));

	bool wants_mips    = (type          & tex_type_mips) > 0;
	bool has_mips      = (texture->type & tex_type_mips) > 0;
	bool generate_mips = has_mips == false && wants_mips == true;

	// Copy base mip (and generate remaining mips if needed)
	skr_tex_copy(&texture->gpu_tex, &result->gpu_tex, 0, 0, 0, 0, texture->gpu_tex.layer_count);
	if (generate_mips) {
		skr_tex_generate_mips(&result->gpu_tex, nullptr);
	}
	return result;
}

///////////////////////////////////////////

bool32_t tex_gen_mips(tex_t texture) {
	if ((texture->type & tex_type_mips        ) == 0 ||
		(texture->type & tex_type_rendertarget) == 0)
		return false;
	skr_tex_generate_mips(&texture->gpu_tex, nullptr);
	return true;
}

///////////////////////////////////////////
// Texture manipulation functions        //
///////////////////////////////////////////

void tex_update_label(tex_t texture) {
//#if (defined(_DEBUG) || defined(SK_GPU_LABELS))
	if (texture->header.id_text != nullptr)
		skr_tex_set_name(&texture->gpu_tex, texture->header.id_text);
//#else
//	(void)texture;
//#endif
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
	tex_set_color_arr(texture->depth_buffer, texture->width, texture->height, nullptr, texture->gpu_tex.layer_count, skr_tex_get_multisample(&texture->gpu_tex), nullptr);
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

void tex_set_surface(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count, int32_t multisample, bool32_t owned) {
	texture->owned = owned;

	if (texture->owned && skr_tex_is_valid(&texture->gpu_tex))
		skr_tex_destroy(&texture->gpu_tex);

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);

	if (native_surface != nullptr) {
		skr_tex_external_info_t info = {};
		info.image         = (VkImage)native_surface;
		info.format        = skr_tex_fmt_from_native((uint32_t)native_fmt);
		info.size          = { width, height, 1 };
		info.sampler       = tex_get_skr_sampler(texture);
		info.multisample   = multisample;
		info.array_layers  = surface_count;
		info.owns_image    = owned;

		skr_tex_create_external(info, &texture->gpu_tex);
	} else {
		texture->gpu_tex = {};
	}

	texture->width  = width;
	texture->height = height;

	texture->header.state = skr_tex_is_valid(&texture->gpu_tex)
		? asset_state_loaded
		: asset_state_error;
	tex_set_fallback(texture, texture->header.state <= 0
		? tex_error_texture
		: nullptr);
}

///////////////////////////////////////////

void* tex_get_surface(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded);
	return (void*)texture->gpu_tex.image;
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
	if (tex->owned && skr_tex_is_valid(&tex->gpu_tex)) {
		skr_tex_destroy(&tex->gpu_tex);
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

// TODO: would be nice to maybe merge these into one function, simplify the memory layout
void _tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **array_data, int32_t array_count, int32_t mip_count, spherical_harmonics_t *sh_lighting_info, int32_t multisample) {
	profiler_zone();

	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height || (int32_t)texture->gpu_tex.layer_count != array_count;
	if (!different_size && (array_data == nullptr || *array_data == nullptr))
		return;

	// Build texture data descriptor from array_data
	// array_data layout: array_data[layer * mip_count + mip] for layer-major
	// skr_tex_data_t expects mip-major: all layers for mip0, then all layers for mip1, etc.
	skr_tex_data_t tex_data = {};
	void*          flat_data = nullptr;
	size_t         total_size = 0;

	if (array_data != nullptr && *array_data != nullptr) {
		// Calculate total size and flatten data to mip-major layout
		skr_vec3i_t base_size = { width, height, 1 };
		for (int32_t mip = 0; mip < mip_count; mip++) {
			total_size += skr_tex_calc_mip_size((skr_tex_fmt_)texture->format, base_size, mip) * array_count;
		}
		flat_data = sk_malloc(total_size);

		uint8_t* dst = (uint8_t*)flat_data;

		// Convert from layer-major (array_data[layer * mip_count + mip]) to mip-major
		for (int32_t mip = 0; mip < mip_count; mip++) {
			uint64_t mip_size = skr_tex_calc_mip_size((skr_tex_fmt_)texture->format, base_size, mip);
			for (int32_t layer = 0; layer < array_count; layer++) {
				int32_t src_idx = layer * mip_count + mip;
				memcpy(dst, array_data[src_idx], (size_t)mip_size);
				dst += mip_size;
			}
		}

		tex_data.data        = flat_data;
		tex_data.mip_count   = mip_count;
		tex_data.layer_count = array_count;
		tex_data.base_mip    = 0;
		tex_data.base_layer  = 0;
		tex_data.row_pitch   = 0; // tightly packed
	}

	if (!skr_tex_is_valid(&texture->gpu_tex) || different_size || (!different_size && !dynamic)) {
		if (!different_size && !dynamic)
			texture->type &= tex_type_dynamic;

		// Convert tex_type_ to skr_tex_flags_
		skr_tex_flags_    flags   = tex_type_to_skr_flags(texture->type);
		skr_tex_fmt_      format  = (skr_tex_fmt_)texture->format;
		skr_tex_sampler_t sampler = tex_get_skr_sampler(texture);

		// For array textures (non-cubemap with multiple layers), set the array flag
		// Cubemaps are handled separately by skr_tex_flags_cubemap
		bool is_array = array_count > 1 && !(texture->type & tex_type_cubemap);
		if (is_array) flags = (skr_tex_flags_)(flags | skr_tex_flags_array);
		skr_vec3i_t size = { width, height, is_array ? array_count : 1 };

		// Determine mip count for creation
		int32_t create_mip_count = (texture->type & tex_type_mips) ? 0 : mip_count; // 0 = auto-calculate

		// Create new texture into a temporary first
		skr_tex_t new_tex;
		skr_err_ err = skr_tex_create(
			format,
			flags,
			sampler,
			size,
			multisample,
			create_mip_count,
			(flat_data != nullptr) ? &tex_data : nullptr,
			&new_tex);

		if (err != skr_err_success) {
			log_err("Failed to create texture");
		} else {
			// Atomic swap: save old, put new in place, then destroy old
			// This ensures gpu_tex always has valid handles for the render thread
			skr_tex_t old_tex = texture->gpu_tex;
			texture->gpu_tex = new_tex;
			if (skr_tex_is_valid(&old_tex))
				skr_tex_destroy(&old_tex);
		}

		// Generate mipmaps if texture has mips flag but only one mip level of data was provided
		if ((texture->type & tex_type_mips) && mip_count == 1 && flat_data != nullptr) {
			skr_tex_generate_mips(&texture->gpu_tex, nullptr);
		}

		tex_set_meta(texture, width, height, texture->format);

		if (texture->depth_buffer != nullptr) {
			tex_set_color_arr(texture->depth_buffer, width, height, nullptr, texture->gpu_tex.layer_count, multisample, nullptr);
			tex_set_zbuffer  (texture, texture->depth_buffer);
		}
		tex_update_label(texture);
	} else if (dynamic) {
		// Update existing dynamic texture
		if (flat_data != nullptr) {
			skr_tex_set_data(&texture->gpu_tex, &tex_data);
			// Regenerate mipmaps for dynamic textures
			if ((texture->type & tex_type_mips) && mip_count == 1) {
				skr_tex_generate_mips(&texture->gpu_tex, nullptr);
			}
		}
	} else {
		log_warn("Attempting additional writes to a non-dynamic texture!");
	}

	sk_free(flat_data);

	if (skr_tex_is_valid(&texture->gpu_tex)) {
		if (sh_lighting_info != nullptr)
			*sh_lighting_info = tex_get_cubemap_lighting(texture);

		tex_set_fallback(texture, nullptr);
		texture->header.state = asset_state_loaded;
	} else {
		tex_set_fallback(texture, tex_error_texture);
		texture->header.state = asset_state_error;
	}
}

///////////////////////////////////////////

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t multisample, spherical_harmonics_t* out_sh_lighting_info) {
	tex_set_color_arr_mips(texture, width, height, array_data, array_count, 1, multisample, out_sh_lighting_info);
}

///////////////////////////////////////////

void tex_set_color_arr_mips(tex_t texture, int32_t width, int32_t height, void **array_data, int32_t array_count, int32_t mip_count, int32_t multisample, spherical_harmonics_t * out_sh_lighting_info) {
	profiler_zone();

	struct tex_upload_job_t {
		tex_t                  texture;
		int32_t                width;
		int32_t                height;
		void                 **array_data;
		int32_t                array_count;
		int32_t                mip_count;
		spherical_harmonics_t *sh_lighting_info;
		int32_t                multisample;
	};
	tex_upload_job_t job_data = {texture, width, height, array_data, array_count, mip_count, out_sh_lighting_info, multisample};

	assets_execute_blocking([](void *data) {
		tex_upload_job_t *job_data = (tex_upload_job_t *)data;
		_tex_set_color_arr(job_data->texture, job_data->width, job_data->height, job_data->array_data, job_data->array_count, job_data->mip_count, job_data->sh_lighting_info, job_data->multisample);
		return (bool32_t)true;
	}, &job_data);
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
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
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

spherical_harmonics_t tex_get_cubemap_lighting(tex_t cubemap_texture) {
	profiler_zone();

	assets_block_until(&cubemap_texture->header, asset_state_loaded);

	skr_tex_t *tex = &cubemap_texture->gpu_tex;

	if (cubemap_texture->light_info != nullptr)
		return *cubemap_texture->light_info;

	// If they want spherical harmonics, lets calculate it for them, or give
	// them a good error message!
	if (cubemap_texture->width != cubemap_texture->height || skr_tex_get_array_count(tex) != 6) {
		log_warn("Invalid texture size for calculating spherical harmonics. Must be an equirect image, or have 6 images all same width and height.");
		return {};
	} else if (!(cubemap_texture->format == tex_format_rgba32_srgb || cubemap_texture->format == tex_format_rgba32_linear || cubemap_texture->format == tex_format_rgba64f || cubemap_texture->format == tex_format_rgba128 || cubemap_texture->format == tex_format_rg11b10)) {
		log_warn("Invalid texture format for calculating spherical harmonics, must be rgba32, rg11b10, rgba64f or rgba128.");
		return {};
	} else {
		skr_vec3i_t base_size       = { cubemap_texture->width, cubemap_texture->height, 1 };
		int32_t     mip_level       = maxi((int32_t)0, (int32_t)skr_tex_calc_mip_count(base_size) - 6);
		skr_vec3i_t mip_size        = skr_tex_calc_mip_dimensions(base_size, mip_level);
		size_t      face_size       = skr_tex_calc_mip_size((skr_tex_fmt_)cubemap_texture->format, mip_size, 0);
		size_t      cube_size       = face_size * 6;
		uint8_t*    cube_color_data = (uint8_t*)sk_malloc(cube_size);
		void*       data[6];

		// Read back each face using async readback API
		for (int32_t f = 0; f < 6; f++) {
			data[f] = cube_color_data + face_size * f;
			skr_tex_readback_t readback = {};
			if (skr_tex_readback(tex, mip_level, f, &readback) == skr_err_success) {
				skr_future_wait(&readback.future);
				memcpy(data[f], readback.data, face_size);
				skr_tex_readback_destroy(&readback);
			}
		}

		cubemap_texture ->light_info = sk_malloc_t(spherical_harmonics_t, 1);
		*cubemap_texture->light_info = sh_calculate(data, cubemap_texture->format, mip_size.x);
		sk_free(cube_color_data);
		return *cubemap_texture->light_info;
	}
}

///////////////////////////////////////////

void tex_set_colors(tex_t texture, int32_t width, int32_t height, void *data) {
	void *data_arr[1] = { data };
	tex_set_color_arr(texture, width, height, data_arr, 1);
}

///////////////////////////////////////////

void _tex_set_options(skr_tex_t *texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level) {
	skr_tex_sampler_t sampler = {};

	// Map address mode (tex_address_ matches skr_tex_address_)
	switch (address_mode) {
	case tex_address_clamp:  sampler.address = skr_tex_address_clamp;  break;
	case tex_address_wrap:   sampler.address = skr_tex_address_wrap;   break;
	case tex_address_mirror: sampler.address = skr_tex_address_mirror; break;
	default:                 sampler.address = skr_tex_address_wrap;
	}

	// Map sample mode (tex_sample_ matches skr_tex_sample_)
	switch (sample) {
	case tex_sample_linear:      sampler.sample = skr_tex_sample_linear;      break;
	case tex_sample_point:       sampler.sample = skr_tex_sample_point;       break;
	case tex_sample_anisotropic: sampler.sample = skr_tex_sample_anisotropic; break;
	default:                     sampler.sample = skr_tex_sample_linear;
	}

	// Map compare mode (tex_sample_comp_ matches skr_compare_)
	switch (compare) {
	case tex_sample_comp_none:          sampler.sample_compare = skr_compare_none;          break;
	case tex_sample_comp_less:          sampler.sample_compare = skr_compare_less;          break;
	case tex_sample_comp_less_or_eq:    sampler.sample_compare = skr_compare_less_or_eq;    break;
	case tex_sample_comp_greater:       sampler.sample_compare = skr_compare_greater;       break;
	case tex_sample_comp_greater_or_eq: sampler.sample_compare = skr_compare_greater_or_eq; break;
	case tex_sample_comp_equal:         sampler.sample_compare = skr_compare_equal;         break;
	case tex_sample_comp_not_equal:     sampler.sample_compare = skr_compare_not_equal;     break;
	case tex_sample_comp_always:        sampler.sample_compare = skr_compare_always;        break;
	case tex_sample_comp_never:         sampler.sample_compare = skr_compare_never;         break;
	default:                            sampler.sample_compare = skr_compare_none;          break;
	}

	sampler.anisotropy = anisotropy_level;

	skr_tex_set_sampler(texture, sampler);
}

///////////////////////////////////////////

void tex_set_options(tex_t texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level) {
	texture->address_mode = address_mode;
	texture->anisotropy   = anisotropy_level;
	texture->sample_mode  = sample;
	texture->sample_comp  = compare;

	_tex_set_options(&texture->gpu_tex, sample, address_mode, texture->sample_comp, anisotropy_level);
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
		? skr_tex_calc_mip_count(texture->gpu_tex.size)
		: 1;
}

///////////////////////////////////////////

size_t tex_format_size(tex_format_ format, int32_t width, int32_t height) {
	return skr_tex_calc_mip_size((skr_tex_fmt_)format, { width, height, 1 }, 0);
}

///////////////////////////////////////////

size_t tex_format_pitch(tex_format_ format, int32_t width) {
	uint32_t block_width, block_height, bytes_per_block;
	skr_tex_fmt_block_info((skr_tex_fmt_)format, &block_width, &block_height, &bytes_per_block);
	uint32_t blocks_wide = (width + block_width - 1) / block_width;
	return (size_t)(blocks_wide * bytes_per_block);
}

///////////////////////////////////////////

tex_format_ tex_get_tex_format(int64_t native_fmt) {
	skr_tex_fmt_ skr_fmt = skr_tex_fmt_from_native((uint32_t)native_fmt);
	return (tex_format_)skr_fmt;
}

///////////////////////////////////////////

int64_t tex_fmt_to_native(tex_format_ format) {
	return (int64_t)skr_tex_fmt_to_native((skr_tex_fmt_)format);
}

///////////////////////////////////////////

tex_format_ tex_get_supported_depth_format(tex_format_ preferred, bool needs_stencil, int32_t multisample) {
	// Build the flags for a depth render target
	skr_tex_flags_ flags = (skr_tex_flags_)(skr_tex_flags_writeable | skr_tex_flags_readable);

	// Check if preferred format is supported
	if (skr_tex_fmt_is_supported((skr_tex_fmt_)preferred, flags, multisample)) {
		return preferred;
	}

	// D24S8 is often unsupported on Linux/Mesa - try fallbacks
	// Prioritize formats with stencil if stencil is needed
	tex_format_ fallbacks_with_stencil[] = { tex_format_depth32s8, tex_format_depth24s8, tex_format_depth16s8 };
	tex_format_ fallbacks_no_stencil[]   = { tex_format_depth32, tex_format_depth16 };

	if (needs_stencil) {
		for (int i = 0; i < 3; i++) {
			if (skr_tex_fmt_is_supported((skr_tex_fmt_)fallbacks_with_stencil[i], flags, multisample)) {
				return fallbacks_with_stencil[i];
			}
		}
	}

	// Fall back to depth-only formats
	for (int i = 0; i < 2; i++) {
		if (skr_tex_fmt_is_supported((skr_tex_fmt_)fallbacks_no_stencil[i], flags, multisample)) {
			return fallbacks_no_stencil[i];
		}
	}

	// Last resort - return the preferred format and let it fail later with a proper error
	log_warn("No supported depth format found, using preferred format which may fail");
	return preferred;
}

///////////////////////////////////////////

id_hash_t tex_meta_hash(tex_t texture) {
	id_hash_t result = hash_int     (texture->width);
	result           = hash_int_with(texture->height, result);
	uint64_t image   = (uint64_t)texture->gpu_tex.image;
	result           = hash_int_with((int32_t)(image & 0xFFFFFFFF), result);
	result           = hash_int_with((int32_t)(image >> 32),        result);
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

void tex_get_data(tex_t texture, void* out_data, size_t out_data_size, int32_t mip_level) {
	if (mip_level > tex_get_mips(texture)) {
		log_warn("Cannot retrieve invalid mip-level!");
		return;
	}

	assets_block_until(&texture->header, asset_state_loaded);

	struct tex_data_job_t {
		tex_t   texture;
		void*   out_data;
		size_t  out_data_size;
		int32_t mip_level;
	};
	tex_data_job_t job_data = { texture, out_data, out_data_size, mip_level };

	bool32_t result = assets_execute_blocking([](void *data) {
		tex_data_job_t *job_data = (tex_data_job_t *)data;

		// Use async readback API with blocking wait
		skr_tex_readback_t readback = {};
		if (skr_tex_readback(&job_data->texture->gpu_tex, job_data->mip_level, 0, &readback) == skr_err_success) {
			skr_cmd_flush();  // Force-submit the current batch so the readback commands are executed
			skr_future_wait(&readback.future);
			size_t copy_size = (readback.size < job_data->out_data_size) ? readback.size : job_data->out_data_size;
			memcpy(job_data->out_data, readback.data, copy_size);
			skr_tex_readback_destroy(&readback);
			return (bool32_t)true;
		}
		return (bool32_t)false;
	}, &job_data);

	if (!result) {
		log_warn("Couldn't get texture contents!");
		memset(out_data, 0, out_data_size);
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

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution, spherical_harmonics_t *out_sh_lighting_info) {
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

	if (out_sh_lighting_info != nullptr)
		*out_sh_lighting_info = tex_get_cubemap_lighting(result);

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
