#include "../stereokit.h"
#include "../platforms/platform_utils.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/qoi.h"
#include "../libraries/stref.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../spherical_harmonics.h"
#include "texture.h"
#include "texture_.h"

#pragma warning(push)
#pragma warning(disable : 26451 6011 6262 6308 6387 28182 26819 )
#include "../libraries/stb_image.h"
#pragma warning(pop)

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string.h>
#include <limits.h>

namespace sk {

void *tex_load_image_data(void *data, size_t data_size, bool32_t srgb_data, tex_format_ *out_format, int32_t *out_width, int32_t *out_height);
bool  tex_load_image_info(void *data, size_t data_size, bool32_t srgb_data, int32_t *out_width, int32_t *out_height, tex_format_ *out_format);
void  tex_update_label   (tex_t texture);
void _tex_set_options    (skg_tex_t* texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level);

const char *tex_msg_load_failed           = "Texture file failed to load: %s";
const char *tex_msg_invalid_fmt           = "Texture invalid format: %s";
const char *tex_msg_mismatched_images     = "Texture array mismatched format or size: %s";
const char *tex_msg_requires_rendertarget = "Zbuffer can only be attached to a rendertarget!";
const char *tex_msg_requires_depth        = "Zbuffer must be a depth texture!";

tex_t tex_error_texture   = nullptr;
tex_t tex_loading_texture = nullptr;

///////////////////////////////////////////
// Texture loading stages                //
///////////////////////////////////////////

struct tex_load_t {
	bool32_t  is_srgb;
	char    **file_names;
	int32_t   file_count;

	void    **file_data;
	size_t   *file_sizes;

	void    **color_data;
	int32_t   color_width;
	int32_t   color_height;
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

bool32_t tex_load_arr_files(asset_task_t *task, asset_header_t *asset, void *job_data) {
	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	data->file_data  = sk_malloc_t(void *, data->file_count);
	data->file_sizes = sk_malloc_t(size_t, data->file_count);

	// Load all files
	int32_t     width  = 0;
	int32_t     height = 0;
	tex_format_ format = tex_format_none;
	for (int32_t i = 0; i < data->file_count; i++) {
		// Read from file

		char*    asset_filename = assets_file(data->file_names[i]);
		bool32_t loaded         = platform_read_file(asset_filename, &data->file_data[i], &data->file_sizes[i]);
		sk_free(asset_filename);
		if (!loaded) {
			log_warnf(tex_msg_load_failed, data->file_names[i]);
			tex->header.state = asset_state_error_not_found;
			return false;
		}

		// Grab the image metadata
		tex_format_ color_format = tex_format_none;
		if (!tex_load_image_info(data->file_data[i], data->file_sizes[i], data->is_srgb, &data->color_width, &data->color_height, &color_format)) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}

		// Check if there were issues, or one of the images is the wrong size!
		if ((width  != 0               && width  != data->color_width ) ||
			(height != 0               && height != data->color_height) ||
			(format != tex_format_none && format != color_format)) {
			log_warnf(tex_msg_mismatched_images, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}
		width  = data->color_width;
		height = data->color_height;
		format = color_format;
	}

	tex_set_meta(tex, width, height, format);
	assets_task_set_complexity(task, width * height * data->file_count);
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_arr_parse(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	data->color_data = sk_malloc_t(void*, data->file_count);

	// Parse all files
	for (int32_t i = 0; i < data->file_count; i++) {
		int         width  = 0;
		int         height = 0;
		tex_format_ format = tex_format_none;
		data->color_data[i] = tex_load_image_data(data->file_data[i], data->file_sizes[i], data->is_srgb, &format, &width, &height);

		if (data->color_data[i] == nullptr) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			tex->header.state = asset_state_error_unsupported;
			return false;
		}

		// This shouldn't happen, tex_load_image_data and tex_load_image_info
		// should always agree with eachother
		if (tex->width  != width  ||
			tex->height != height ||
			tex->format != format) {
			log_warnf("Texture data mismatch: %s", data->file_names[i]);
			tex_set_fallback(tex, tex_error_texture);
			tex->header.state = asset_state_error;
			return false;
		}

		// Release file memory as soon as we're done with it
		sk_free(data->file_data[i]);
	}
	tex->header.state = asset_state_loaded_meta;
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_equirect_file(asset_task_t *task, asset_header_t *asset, void *job_data) {
	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	data->file_data  = sk_malloc_t(void *, data->file_count);
	data->file_sizes = sk_malloc_t(size_t, data->file_count);

	char*    asset_filename = assets_file(data->file_names[0]);
	bool32_t loaded         = platform_read_file(asset_filename, &data->file_data[0], &data->file_sizes[0]);
	sk_free(asset_filename);
	if (!loaded) {
		log_warnf(tex_msg_load_failed, data->file_names[0]);
		tex->header.state = asset_state_error_not_found;
		return false;
	}

	tex_format_ format;
	if (!tex_load_image_info(data->file_data[0], data->file_sizes[0], data->is_srgb, &data->color_width, &data->color_height, &format)) {
		log_warnf(tex_msg_invalid_fmt, data->file_names[0]);
		tex->header.state = asset_state_error_unsupported;
		return false;
	}

	int32_t tex_size = data->color_height / 2;
	tex_set_meta(tex, tex_size, tex_size, format);
	assets_task_set_complexity(task, tex_size * 6);
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_equirect_parse(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	data->color_data = sk_malloc_t(void*, 1);

	tex_format_ format = tex_format_none;
	data->color_data[0] = tex_load_image_data(data->file_data[0], data->file_sizes[0], data->is_srgb, &format, &data->color_width, &data->color_height);

	if (data->color_data[0] == nullptr) {
		log_warnf(tex_msg_invalid_fmt, data->file_names[0]);
		tex->header.state = asset_state_error_unsupported;
		return false;
	}

	// Release file memory as soon as we're done with it
	sk_free(data->file_data[0]);

	return true;
}

///////////////////////////////////////////

bool32_t tex_load_equirect_upload(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	const vec3 up   [6] = { vec3_up, vec3_up, -vec3_forward, vec3_forward, vec3_up, vec3_up };
	const vec3 fwd  [6] = { {1,0,0}, {-1,0,0}, {0,-1,0}, {0,1,0}, {0,0,1}, {0,0,-1} };
	const vec3 right[6] = { {0,0,-1}, {0,0,1}, {1,0,0}, {1,0,0}, {1,0,0}, {-1,0,0} };

	tex_t equirect = tex_create(tex_type_image_nomips, tex->format);
	tex_set_color_arr(equirect, data->color_width, data->color_height, data->color_data, data->file_count);
	tex_set_address  (equirect, tex_address_clamp);
	
	material_t convert_material = material_find(default_id_material_equirect);
	material_set_texture(convert_material, "source", equirect);

	tex_t    face         = tex_create(tex_type_rendertarget, equirect->format);
	void    *face_data[6] = {};
	size_t   size         = (size_t)tex->width*(size_t)tex->height*tex_format_size(equirect->format);
	tex_set_colors(face, tex->width, tex->height, nullptr);
	for (int32_t i = 0; i < 6; i++) {
		material_set_vector(convert_material, "up",      { up   [i].x, up   [i].y, up   [i].z, 0 });
		material_set_vector(convert_material, "right",   { right[i].x, right[i].y, right[i].z, 0 });
		material_set_vector(convert_material, "forward", { fwd  [i].x, fwd  [i].y, fwd  [i].z, 0 });

		face_data[i] = sk_malloc(size);

		// Blit conversion _definitely_ needs executed on the GPU thread
		struct blit_t {
			tex_t      face;
			material_t material;
			void      *face_data;
			size_t     size;
		};
		blit_t blit_data = { face, convert_material, face_data[i], size };
		assets_execute_gpu([](void *data) {
			blit_t *blit_data = (blit_t *)data;
			render_blit (blit_data->face, blit_data->material);
			tex_get_data(blit_data->face, blit_data->face_data, blit_data->size);
			return (bool32_t)true;
		}, &blit_data);
		
		
#if defined(SKG_OPENGL)
		int32_t line_size = tex_format_size(equirect->format) * tex->width;
		void   *tmp       = sk_malloc(line_size);
		for (int32_t y = 0; y < tex->height/2; y++) {
			void *top_line = ((uint8_t*)face_data[i]) + line_size * y;
			void *bot_line = ((uint8_t*)face_data[i]) + line_size * ((tex->height-1) - y);
			memcpy(tmp,      top_line, line_size);
			memcpy(top_line, bot_line, line_size);
			memcpy(bot_line, tmp,      line_size);
		}
		sk_free(tmp);
#endif
	}

	tex_release(face);
	material_release(convert_material);
	tex_release(equirect);

	tex_set_color_arr(tex, tex->width, tex->height, (void**)&face_data, 6);
	for (int32_t i = 0; i < 6; i++) {
		sk_free(face_data[i]);
	}

	tex->header.state = asset_state_loaded;
	return true;
}

///////////////////////////////////////////

bool32_t tex_load_arr_upload(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;

	// Create with the data we have
	tex_set_color_arr(tex, tex->width, tex->height, data->color_data, data->file_count);

	return true;
}

///////////////////////////////////////////

void tex_load_on_failure(asset_header_t *asset, void *) {
	tex_set_fallback((tex_t)asset, tex_error_texture);
}

///////////////////////////////////////////

bool tex_load_image_info(void *data, size_t data_size, bool32_t srgb_data, int32_t *out_width, int32_t *out_height, tex_format_ *out_format) {
	// Check STB image formats
	int32_t comp;
	bool success = stbi_info_from_memory((const stbi_uc*)data, (int)data_size, out_width, out_height, &comp) == 1;
	if (success) {
		if (stbi_is_hdr_from_memory((stbi_uc *)data, (int)data_size)) *out_format = tex_format_rgba128;
		else                                                          *out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		return true;
	}

	// Check QOI
	qoi_desc qoi = {};
	success = qoi_info(data, (int)data_size, &qoi);
	if (success) {
		*out_width  = qoi.width;
		*out_height = qoi.height;
		if (qoi.colorspace == QOI_LINEAR) *out_format = tex_format_rgba32_linear;
		else                              *out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;
		return true;
	}

	return false;
}

///////////////////////////////////////////

void *tex_load_image_data(void *data, size_t data_size, bool32_t srgb_data, tex_format_ *out_format, int32_t *out_width, int32_t *out_height) {
	int32_t channels = 0;
	*out_format = srgb_data ? tex_format_rgba32 : tex_format_rgba32_linear;

	// Check for an stbi HDR image
	if (stbi_is_hdr_from_memory((stbi_uc*)data, (int)data_size)) {
		*out_format = tex_format_rgba128;
		return (uint8_t *)stbi_loadf_from_memory((stbi_uc *)data, (int)data_size, out_width, out_height, &channels, 4);
	}

	// Check through stbi's list of image formats
	void *result = stbi_load_from_memory ((stbi_uc*)data, (int)data_size, out_width, out_height, &channels, 4);
	if (result != nullptr)
		return result;

	// Check for qoi images
	qoi_desc q_desc = {};
	result = qoi_decode(data, (int)data_size, &q_desc, 4);
	if (result != nullptr) {
		*out_width    = q_desc.width;
		*out_height   = q_desc.height;
		// If QOI claims it's linear, then we'll go with that!
		if (q_desc.colorspace == QOI_LINEAR)
			*out_format = tex_format_rgba32_linear;
		return result;
	}

	return nullptr;
}

///////////////////////////////////////////
// Texture creation functions            //
///////////////////////////////////////////

void tex_add_loading_task(tex_t texture, void *load_data, const asset_load_action_t *actions, int32_t action_count, int32_t priority, float complexity) {
	asset_task_t task = {};
	task.asset        = (asset_header_t*)texture;
	task.free_data    = tex_load_free;
	task.on_failure   = tex_load_on_failure;
	task.load_data    = load_data;
	task.actions      = (asset_load_action_t *)actions;
	task.action_count = action_count;
	task.priority     = priority;
	task.sort         = asset_sort(priority, complexity);

	assets_add_task(task);
}

///////////////////////////////////////////

tex_t tex_create_file_type(const char *file, tex_type_ type, bool32_t srgb_data, int32_t priority) {
	tex_t result = tex_find(file);
	if (result != nullptr)
		return result;

	result = tex_create(type);
	tex_set_id(result, file);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_calloc_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(file);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_files,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
#if defined(SKG_OPENGL)
		asset_load_action_t {tex_load_arr_upload, asset_thread_gpu  },
#else
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
#endif
	};
	tex_add_loading_task(result, load_data, actions, _countof(actions), priority, 0);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_file(const char *file, bool32_t srgb_data, int32_t priority) {
	return tex_create_file_type(file, tex_type_image, srgb_data, priority);
}

///////////////////////////////////////////

tex_t tex_create_mem_type(tex_type_ type, void *data, size_t data_size, bool32_t srgb_data, int32_t priority) {
	tex_t result = tex_create(type);

	tex_load_t *load_data = sk_calloc_t(tex_load_t, 1);
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
	int32_t     width  = 0;
	int32_t     height = 0;
	tex_format_ format = tex_format_none;
	if (!tex_load_image_info(load_data->file_data[0], load_data->file_sizes[0], load_data->is_srgb, &width, &height, &format)) {
		log_warnf(tex_msg_invalid_fmt, load_data->file_names[0]);
		result->header.state = asset_state_error_unsupported;
		return result;
	}
	tex_set_meta(result, width, height, format);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
#if defined(SKG_OPENGL)
		asset_load_action_t {tex_load_arr_upload, asset_thread_gpu  },
#else
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
#endif
	};
	tex_add_loading_task(result, load_data, actions, _countof(actions), priority, (float)(width * height));

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
	result->header.state = asset_state_loaded_meta;

	tex_set_fallback(result, tex_loading_texture);

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

tex_t _tex_create_file_arr(tex_type_ type, const char **files, int32_t file_count, bool32_t srgb_data, spherical_harmonics_t *out_sh_lighting_info, int32_t priority) {
	// Hash the names of all of the files together
	uint64_t hash = HASH_FNV64_START;
	for (int32_t i = 0; i < file_count; i++) {
		hash = hash_fnv64_string(files[i], hash);
	}
	char file_id[64];
	snprintf(file_id, sizeof(file_id), "sk_arr::%" PRIu64, hash);

	// And see if it's already been loaded
	tex_t result = tex_find(file_id);
	if (result != nullptr) {
		if (out_sh_lighting_info != nullptr)
			*out_sh_lighting_info = tex_get_cubemap_lighting(result);
		return result;
	}

	result = tex_create(type);
	tex_set_id(result, file_id);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_calloc_t(tex_load_t, 1);
	load_data->is_srgb    = srgb_data;
	load_data->file_count = file_count;
	load_data->file_names = sk_malloc_t(char *, file_count);
	for (int32_t i = 0; i < file_count; i++) {
		load_data->file_names[i] = string_copy(files[i]);
	}

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_arr_files,  asset_thread_asset},
		asset_load_action_t {tex_load_arr_parse,  asset_thread_asset},
#if defined(SKG_OPENGL)
		asset_load_action_t {tex_load_arr_upload, asset_thread_gpu},
#else
		asset_load_action_t {tex_load_arr_upload, asset_thread_asset},
#endif
	};
	tex_add_loading_task(result, load_data, actions, _countof(actions), priority, 0);

	// NOTE: this will block execution if it occurs, as it requires the cubemap
	// to be loaded!
	if (out_sh_lighting_info != nullptr)
		*out_sh_lighting_info = tex_get_cubemap_lighting(result);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_file_arr(const char **files, int32_t file_count, bool32_t srgb_data, int32_t priority) {
	return _tex_create_file_arr(tex_type_image, files, file_count, srgb_data, nullptr, priority);
}

///////////////////////////////////////////

tex_t tex_create_cubemap_file(const char *equirectangular_file, bool32_t srgb_data, spherical_harmonics_t *out_sh_lighting_info, int32_t priority) {
	char equirect_id[64];
	snprintf(equirect_id, sizeof(equirect_id), "sk_equi::%" PRIu64, hash_fnv64_string(equirectangular_file));

	tex_t result = tex_find(equirect_id);
	if (result != nullptr) {
		if (out_sh_lighting_info != nullptr)
			*out_sh_lighting_info = tex_get_cubemap_lighting(result);
		return result;
	}

	result = tex_create(tex_type_image | tex_type_cubemap);
	tex_set_id(result, equirect_id);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = sk_calloc_t(tex_load_t, 1);
	load_data->is_srgb       = srgb_data;
	load_data->file_count    = 1;
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(equirectangular_file);

	static const asset_load_action_t actions[] = {
		asset_load_action_t {tex_load_equirect_file,   asset_thread_asset},
		asset_load_action_t {tex_load_equirect_parse,  asset_thread_asset},
#if defined(SKG_OPENGL)
		asset_load_action_t {tex_load_equirect_upload, asset_thread_gpu},
#else
		asset_load_action_t {tex_load_equirect_upload, asset_thread_asset},
#endif
	};
	tex_add_loading_task(result, load_data, actions, _countof(actions), priority, 0);

	// NOTE: this will block execution if it occurs, as it requires the cubemap
	// to be loaded!
	if (out_sh_lighting_info != nullptr)
		*out_sh_lighting_info = tex_get_cubemap_lighting(result);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_files(const char **cube_face_file_xxyyzz, bool32_t srgb_data, spherical_harmonics_t *out_sh_lighting_info, int32_t priority) {
	return _tex_create_file_arr(tex_type_image | tex_type_cubemap, cube_face_file_xxyyzz, 6, srgb_data, out_sh_lighting_info, priority);
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

tex_t tex_add_zbuffer(tex_t texture, tex_format_ format) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err(tex_msg_requires_rendertarget);
		return nullptr;
	}

	char id[64];
	assets_unique_name(asset_type_tex, "zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex_create(tex_type_depth, format);
	tex_set_id       (texture->depth_buffer, id);
	tex_set_color_arr(texture->depth_buffer, texture->width, texture->height, nullptr, texture->tex.array_count, nullptr, texture->tex.multisample);
	skg_tex_attach_depth(&texture->tex, &texture->depth_buffer->tex);
	texture->depth_buffer->header.state = asset_state_loaded;
	
	tex_addref(texture->depth_buffer);
	return texture->depth_buffer;
}

///////////////////////////////////////////

void tex_set_zbuffer(tex_t texture, tex_t depth_texture) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err(tex_msg_requires_rendertarget);
		return;
	}
	if (!(depth_texture->type & tex_type_depth)) {
		log_err(tex_msg_requires_depth);
		return;
	}
	tex_addref(depth_texture);
	if (texture->depth_buffer != nullptr)
		tex_release(texture->depth_buffer);
	texture->depth_buffer = depth_texture;

	skg_tex_attach_depth(&texture->tex, &depth_texture->tex);
}

///////////////////////////////////////////

void tex_set_surface(tex_t texture, void *native_surface, tex_type_ type, int64_t native_fmt, int32_t width, int32_t height, int32_t surface_count, bool32_t owned) {
	texture->owned = owned;
	
	if (texture->owned && skg_tex_is_valid(&texture->tex))
		skg_tex_destroy (&texture->tex);

	skg_tex_type_ skg_type = skg_tex_type_image;
	if      (type & tex_type_cubemap     ) skg_type = skg_tex_type_cubemap;
	else if (type & tex_type_depth       ) skg_type = skg_tex_type_depth;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);
	texture->tex    = native_surface == nullptr ? skg_tex_t{} : skg_tex_create_from_existing(native_surface, skg_type, skg_tex_fmt_from_native(native_fmt), width, height, surface_count);
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
	if      (type & tex_type_cubemap     ) skg_type = skg_tex_type_cubemap;
	else if (type & tex_type_depth       ) skg_type = skg_tex_type_depth;
	else if (type & tex_type_rendertarget) skg_type = skg_tex_type_rendertarget;

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
	if(tex->owned)
		skg_tex_destroy(&tex->tex);
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

void _tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info, int32_t multisample) {
	bool dynamic        = texture->type & tex_type_dynamic;
	bool different_size = texture->width != width || texture->height != height || texture->tex.array_count != data_count;
	if (!different_size && (data == nullptr || *data == nullptr))
		return;
	if (!skg_tex_is_valid(&texture->tex) || different_size || (!different_size && !dynamic)) {
		if (!different_size && !dynamic)
			texture->type &= tex_type_dynamic;

		skg_tex_fmt_  format   = (skg_tex_fmt_)texture->format;
		skg_use_      use      = texture->type & tex_type_dynamic ? skg_use_dynamic  : skg_use_static;
		skg_mip_      use_mips = texture->type & tex_type_mips    ? skg_mip_generate : skg_mip_none;
		skg_tex_type_ type     = skg_tex_type_image;
		if      (texture->type & tex_type_cubemap)      type = skg_tex_type_cubemap;
		else if (texture->type & tex_type_depth)        type = skg_tex_type_depth;
		else if (texture->type & tex_type_rendertarget) type = skg_tex_type_rendertarget;

		skg_tex_t new_tex = skg_tex_create(type, use, format, use_mips);
		_tex_set_options(&new_tex, texture->sample_mode, texture->address_mode, texture->anisotropy);
		skg_tex_set_contents_arr(&new_tex, (const void**)data, data_count, width, height, multisample);
		skg_tex_t old_tex = texture->tex;
		texture->tex = new_tex;
		skg_tex_destroy(&old_tex);

		tex_set_meta(texture, width, height, texture->format);

		if (texture->depth_buffer != nullptr) {
			tex_set_color_arr(texture->depth_buffer, width, height, nullptr, texture->tex.array_count, nullptr, multisample);
			tex_set_zbuffer  (texture, texture->depth_buffer);
		}
		tex_update_label(texture);
	} else if (dynamic) {
		skg_tex_set_contents_arr(&texture->tex, (const void**)data, data_count, width, height, multisample);
	} else {
		log_warn("Attempting additional writes to a non-dynamic texture!");
	}

	if (skg_tex_is_valid(&texture->tex)) {
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

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void **data, int32_t data_count, spherical_harmonics_t *sh_lighting_info, int32_t multisample) {
	struct tex_upload_job_t {
		tex_t                  texture;
		int32_t                width;
		int32_t                height;
		void                 **data;
		int32_t                data_count;
		spherical_harmonics_t *sh_lighting_info;
		int32_t                multisample;
	};
	tex_upload_job_t job_data = {texture, width, height, data, data_count, sh_lighting_info, multisample};

	// OpenGL doesn't like multiple threads, but D3D is fine with it.
#if defined(SKG_OPENGL)
	assets_execute_gpu([](void *data) {
		tex_upload_job_t *job_data = (tex_upload_job_t *)data;
		_tex_set_color_arr(job_data->texture, job_data->width, job_data->height, job_data->data, job_data->data_count, job_data->sh_lighting_info, job_data->multisample);
		return (bool32_t)true; 
	}, &job_data);
#else
	_tex_set_color_arr(job_data.texture, job_data.width, job_data.height, job_data.data, job_data.data_count, job_data.sh_lighting_info, job_data.multisample);
#endif
}

///////////////////////////////////////////

spherical_harmonics_t tex_get_cubemap_lighting(tex_t cubemap_texture) {
	assets_block_until(&cubemap_texture->header, asset_state_loaded);

	skg_tex_t *tex = &cubemap_texture->tex;

	if (cubemap_texture->light_info != nullptr)
		return *cubemap_texture->light_info;

	// If they want spherical harmonics, lets calculate it for them, or give
	// them a good error message!
	if (tex->width != tex->height || tex->array_count != 6) {
		log_warn("Invalid texture size for calculating spherical harmonics. Must be an equirect image, or have 6 images all same width and height.");
		return {};
	} else if (!(tex->format == skg_tex_fmt_rgba32 || tex->format == skg_tex_fmt_rgba32_linear || tex->format == skg_tex_fmt_rgba128)) {
		log_warn("Invalid texture format for calculating spherical harmonics, must be rgba32 or rgba128.");
		return {};
	} else {
		int32_t  mip_level = maxi((int32_t)0, (int32_t)skg_mip_count(tex->width, tex->height) - 6);
		int32_t  mip_w, mip_h;
		skg_mip_dimensions(tex->width, tex->height, mip_level, &mip_w, &mip_h);
		size_t   face_size       = skg_tex_fmt_size(tex->format) * mip_w * mip_h;
		size_t   cube_size       = face_size * 6;
		uint8_t *cube_color_data = (uint8_t*)sk_malloc(cube_size);
		void    *data[6];
		for (int32_t f = 0; f < 6; f++) {
			data[f] = cube_color_data + face_size * f;
			skg_tex_get_mip_contents_arr(tex, mip_level, f, data[f], face_size);
		}

		cubemap_texture ->light_info = sk_malloc_t(spherical_harmonics_t, 1);
		*cubemap_texture->light_info = sh_calculate(data, cubemap_texture->format, mip_w);
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

void _tex_set_options(skg_tex_t *texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
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

	skg_tex_settings(texture, skg_addr, skg_sample, anisotropy_level);
}

///////////////////////////////////////////

void tex_set_options(tex_t texture, tex_sample_ sample, tex_address_ address_mode, int32_t anisotropy_level) {
	texture->address_mode = address_mode;
	texture->anisotropy   = anisotropy_level;
	texture->sample_mode  = sample;

	_tex_set_options(&texture->tex, sample, address_mode, anisotropy_level);
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

int32_t tex_get_mips(tex_t texture) {
	return (texture->type & tex_type_mips)
		? skg_mip_count(tex_get_width(texture), tex_get_height(texture))
		: 1;
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

uint64_t tex_meta_hash(tex_t texture) {
	uint64_t result = hash_fnv64_data(&texture->width,  sizeof(texture->width));
	result          = hash_fnv64_data(&texture->height, sizeof(texture->height), result);
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

void tex_get_data(tex_t texture, void *out_data, size_t out_data_size) {
	tex_get_data_mip(texture, out_data, out_data_size, 0);
}

///////////////////////////////////////////

void tex_get_data_mip(tex_t texture, void* out_data, size_t out_data_size, int32_t mip_level) {
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

	bool32_t result = assets_execute_gpu([](void *data) {
		tex_data_job_t *job_data = (tex_data_job_t *)data;
		return (bool32_t)skg_tex_get_mip_contents(&job_data->texture->tex, job_data->mip_level, job_data->out_data, job_data->out_data_size);
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
	case tex_format_rgba128:       {                                                              memcpy(data, &color, sizeof(color)); data_step = sizeof(color); } break;
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
		gradient_release(grad);

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution, spherical_harmonics_t *out_sh_lighting_info) {
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
		data[i] = sk_malloc_t(color128, size2);
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
	tex_t result = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba128);
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
	color128 *data[6];
	for (int32_t i = 0; i < 6; i++) {
		data[i] = sk_malloc_t(color128, size2);
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
				float dist = fmaxf(fmaxf(fabsf(pt.x-light_pt.x), fabsf(pt.y-light_pt.y)), fabsf(pt.z-light_pt.z));
				pt = vec3_normalize(pt);

				if (dist < light_spot_size_pct) {
					data[i][x + y * size] = light_col;
				} else {
					data[i][x + y * size] = sh_lookup(lookup, pt);
				}
			}
		}
	}

	tex_set_color_arr(result, size, size, (void**)data, 6);

	for (int32_t i = 0; i < 6; i++) {
		sk_free(data[i]);
	}

	return result;
}

} // namespace sk