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
#include "../libraries/ferr_thread.h"
#include "../libraries/array.h"
#include "../libraries/profiler.h"
#include "../libraries/atomic_util.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../spherical_harmonics.h"
#include "../systems/defaults.h"
#include "shader.h"
#include "material.h"
#include "texture.h"
#include "texture_.h"
#include "texture_compression.h"

#define STBI_NO_STDIO
#include "../libraries/stb_image.h"

#include "../libraries/hdr_load.h"

#define __STDC_FORMAT_MACROS
#include <sk_app.h>

#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

namespace sk {

// tex_format_ and skr_tex_fmt_ are cast into each other all over this file, so
// they have to stay value-identical. These pin the spots a change would shift.
#define TEX_FMT_MATCHES(name) static_assert((int)tex_format_##name == (int)skr_tex_fmt_##name, "tex_format_ and skr_tex_fmt_ have diverged at " #name)
TEX_FMT_MATCHES(none);
TEX_FMT_MATCHES(rgba32_srgb);
TEX_FMT_MATCHES(depth32s8);
TEX_FMT_MATCHES(bc1_rgb_srgb);
TEX_FMT_MATCHES(bc7_rgba);
TEX_FMT_MATCHES(etc1_rgb);
TEX_FMT_MATCHES(etc2_rg11);
TEX_FMT_MATCHES(astc4x4_rgba_srgb);
TEX_FMT_MATCHES(astc8x8_rgba_hdr);
TEX_FMT_MATCHES(atc_rgba);
TEX_FMT_MATCHES(yuv420p);
#undef TEX_FMT_MATCHES

// out_data receives every image in one allocation, mip-major with images within
// a level. Only KTX2 decodes more than one image, and only from a lone file.
bool   tex_load_image_data(void* data, size_t data_size, tex_data_ flags, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data);
bool   tex_load_image_info(void* data, size_t data_size, tex_data_ flags, tex_type_* ref_image_type, tex_format_* out_format, int32_t *out_width, int32_t *out_height, int32_t* out_array_count, int32_t* out_mip_count);
void   tex_update_label   (tex_t texture);
size_t tex_format_pitch   (tex_format_ format, int32_t width);
void  _tex_set_options    (skr_tex_t* texture, tex_sample_ sample, tex_address_ address_mode, tex_sample_comp_ compare, int32_t anisotropy_level);
void   tex_compute_sh     (tex_t texture);
static void* tex_flatten_layers(tex_format_ format, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t mip_count);

const char *tex_msg_load_failed           = "Texture file failed to load: %s";
const char *tex_msg_invalid_fmt           = "Texture invalid format: %s";
const char *tex_msg_invalid_cubemap       = "Texture not recognized as valid cubemap format: %s";
const char *tex_msg_nested_arrays         = "Texture array cannot contain nested array texture: %s";
const char *tex_msg_mismatched_images     = "Texture array mismatched format or size: %s";
const char *tex_msg_inconsistent_parse    = "Texture data doesn't match earlier parse: %s";
const char *tex_msg_requires_rendertarget = "Zbuffer can only be attached to a rendertarget!";
const char *tex_msg_requires_depth        = "Zbuffer must be a depth texture!";

tex_t tex_error_texture           = nullptr;
tex_t tex_loading_texture         = nullptr;
tex_t tex_error_texture_cubemap   = nullptr;
tex_t tex_loading_texture_cubemap = nullptr;
tex_t tex_error_texture_3d        = nullptr;
tex_t tex_loading_texture_3d      = nullptr;

int32_t tex_compression_default = tex_data_quality; // a tex_data_, atomic since loads read it off-thread

tex_t _tex_get_loading_fallback(tex_t texture) {
	if (texture->type & tex_type_volume)  return tex_loading_texture_3d;
	if (texture->type & tex_type_cubemap) return tex_loading_texture_cubemap;
	return tex_loading_texture;
}

tex_t _tex_get_error_fallback(tex_t texture) {
	if (texture->type & tex_type_volume)  return tex_error_texture_3d;
	if (texture->type & tex_type_cubemap) return tex_error_texture_cubemap;
	return tex_error_texture;
}

///////////////////////////////////////////

bool tex_format_is_compressed(tex_format_ format) {
	return format >= tex_format_bc1_rgb_srgb && format <= tex_format_atc_rgba;
}

///////////////////////////////////////////

bool tex_format_has_alpha(tex_format_ format) {
	switch (format) {
	case tex_format_rgba32_srgb: case tex_format_rgba32_linear:
	case tex_format_bgra32_srgb: case tex_format_bgra32_linear:
	case tex_format_rgba64un:    case tex_format_rgba64sn:
	case tex_format_rgba64ui:    case tex_format_rgba64si:
	case tex_format_rgba64f:     case tex_format_rgba128: return true;
	default: return false;
	}
}

///////////////////////////////////////////

bool tex_format_is_mippable(tex_format_ format) {
	// Block-compressed formats can't be rendered into, so runtime mip
	// generation is not possible for them. YUV/multi-plane formats are
	// read-only via YCbCr conversion samplers and can't be mipped either.
	if (tex_format_is_compressed(format)) return false;
	if (format >= tex_format_nv12         && format <= tex_format_yuv420p) return false;
	return true;
}

///////////////////////////////////////////
// Helper functions for sk_renderer API  //
///////////////////////////////////////////

// tex_type_mips is intent, block and YUV formats can't generate a chain.
static bool tex_gens_mips(tex_type_ type, tex_format_ format) {
	return (type & tex_type_mips) && tex_format_is_mippable(format);
}

///////////////////////////////////////////

skr_tex_flags_ tex_type_to_skr_flags(tex_type_ type, tex_format_ format) {
	// Z-buffers are depth attachments (not sampled), in-pass readable so
	// post-process effects can take depth as an input attachment.
	if (type & tex_type_zbuffer) {
		return (skr_tex_flags_)(skr_tex_flags_writeable | skr_tex_flags_input_attachment);
	}

	// Most textures are sampled, transient attachments and swapchain images
	// never are. Both halves matter here, sk_renderer tests in_tile_msaa &&
	// !readable.
	skr_tex_flags_ flags = (type & tex_type_transient_internal) ? skr_tex_flags_in_tile_msaa
	                     : (type & tex_type_attachment_internal) ? skr_tex_flags_none
	                     : skr_tex_flags_readable;
	if (type & tex_type_cubemap)      flags = (skr_tex_flags_)(flags | skr_tex_flags_cubemap);
	if (type & tex_type_dynamic)      flags = (skr_tex_flags_)(flags | skr_tex_flags_dynamic);
	if (tex_gens_mips(type, format))  flags = (skr_tex_flags_)(flags | skr_tex_flags_gen_mips);
	if (type & tex_type_rendertarget) flags = (skr_tex_flags_)(flags | skr_tex_flags_writeable);
	if (type & tex_type_depthtarget)  flags = (skr_tex_flags_)(flags | skr_tex_flags_writeable | skr_tex_flags_input_attachment); // Readable depth (shadow maps)
	if (type & tex_type_compute)      flags = (skr_tex_flags_)(flags | skr_tex_flags_compute);
	if (type & tex_type_volume)       flags = (skr_tex_flags_)(flags | skr_tex_flags_3d);
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
static_assert((int32_t)tex_format_r8      == (int32_t)skr_tex_fmt_r8,      "tex_format_ is out of sync with skr_tex_fmt_");
static_assert((int32_t)tex_format_etc1_rgb== (int32_t)skr_tex_fmt_etc1_rgb,"tex_format_ is out of sync with skr_tex_fmt_");
static_assert((int32_t)tex_format_yuv420p == (int32_t)skr_tex_fmt_yuv420p, "tex_format_ is out of sync with skr_tex_fmt_");

// Swaps in a new GPU image, so readers never see an invalid handle.
static void tex_swap_gpu(tex_t tex, skr_tex_t gpu) {
	skr_tex_t old = tex->gpu_tex;
	tex->gpu_tex  = gpu;
	if (skr_tex_is_valid(&old)) skr_tex_destroy(&old);
}

///////////////////////////////////////////
// Write ordering                        //
///////////////////////////////////////////

static void tex_upload_queue(tex_t tex, int32_t priority, size_t bytes);

// Writes are numbered when requested, so one that lands late can't replace a
// newer one. Compared by difference, so the counter can wrap.
static int32_t tex_write_issue(tex_t tex)              { return atomic_increment(&tex->write_issued); }
static bool    tex_write_newer(int32_t a, int32_t b)   { return (int32_t)((uint32_t)a - (uint32_t)b) > 0; }

// Only one thread at a time may replace a texture's GPU image.
static bool tex_write_try_begin(tex_t tex) { return atomic_cas_i32(&tex->write_running, 0, 1); }
static void tex_write_begin    (tex_t tex) { while (!tex_write_try_begin(tex)) ska_time_sleep(1); }

// For actions that lost tex_write_try_begin. Without the sleep, a blocking
// job re-runs the action in a tight loop.
static asset_action_result_ tex_write_busy() { ska_time_sleep(1); return asset_action_continue; }

static void tex_write_end(tex_t tex) {
	atomic_exchange_i32(&tex->write_running, 0);
	// An async upload that arrived meanwhile may not have a task yet
	if (atomic_load_ptr(&tex->upload_pending) != nullptr)
		tex_upload_queue(tex, atomic_load_i32(&tex->upload_priority), 0);
}

// Between begin and end. False when a newer write already landed.
static bool tex_write_claim(tex_t tex, int32_t write) {
	if (tex_write_newer(tex->write_landed, write)) return false;
	atomic_store_i32(&tex->write_landed, write);
	return true;
}

// For writes that land as they're requested, so nothing earlier lands after.
static void tex_write_begin_now(tex_t tex) {
	tex_write_begin(tex);
	atomic_store_i32(&tex->write_landed, tex_write_issue(tex));
}

///////////////////////////////////////////

// Puts a finished GPU image in place, like a load completing. An invalid
// image is a failed load.
static void tex_land(tex_t tex, skr_tex_t gpu, int32_t width, int32_t height, int32_t depth, tex_format_ format) {
	if (!skr_tex_is_valid(&gpu)) {
		tex_set_fallback(tex, _tex_get_error_fallback(tex));
		tex->header.state = asset_state_error;
		return;
	}
	tex_swap_gpu(tex, gpu);
	// A newer request already set the meta it will land with
	if (!tex_write_newer(atomic_load_i32(&tex->write_issued), atomic_load_i32(&tex->write_landed)))
		tex_set_meta(tex, width, height, depth, format);
	tex_update_label(tex);
	tex_set_fallback(tex, nullptr);
	tex->header.state = asset_state_loaded;
}

///////////////////////////////////////////
// Texture loading stages                //
///////////////////////////////////////////

struct tex_load_t {
	tex_data_   flags;
	char      **file_names;
	int32_t     file_count;

	void      **file_data;
	size_t     *file_sizes;
	int32_t           file_read_curr;
	asset_file_read_t file_read;      // the in-flight read of file_read_curr

	int32_t       parse_file_curr;
	int32_t       parse_array_count;
	ktx2_slice_t *ktx2_slice;          // a level-paced transcode of parse_file_curr

	void      **color_data;  // One entry per file, see tex_load_image_data
	int32_t     color_width;
	int32_t     color_height;
	int32_t     color_array_count;
	int32_t     color_mip_count;
	tex_format_ color_format;
	tex_format_ gpu_format;  // compressed target, tex_format_none keeps color_format
	int32_t     write;       // from tex_write_issue, when the load was requested
	tex_type_   type;        // the texture's type, plus what the files add
	asset_state_ error;      // set by a failed stage, applied by tex_load_on_failure
};

// Pixels on their way to the GPU, from a file load or a set call.
struct tex_upload_t {
	tex_data_   flags;
	void*       data;        // mip-major, every layer of mip 0 then of mip 1
	int32_t     width;
	int32_t     height;
	int32_t     depth;
	int32_t     array_count;
	int32_t     mip_count;
	int32_t     multisample;
	tex_format_ format;      // what `data` holds
	tex_format_ gpu_format;  // compressed target, tex_format_none keeps `format`
	int32_t     write;       // from tex_write_issue
};

static void tex_upload_free(tex_upload_t* upload) {
	sk_free(upload->data);
	sk_free(upload);
}

static void tex_upload_run(tex_t tex, tex_upload_t* upload);
static void tex_upload_submit(tex_t tex, tex_upload_t* upload, int32_t priority);

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
	sk_free(data->file_read.data);
	ktx2_decode_end(data->ktx2_slice);
	sk_free(data);
}

///////////////////////////////////////////

static tex_load_t* tex_load_create(tex_t tex, tex_data_ flags, int32_t file_count) {
	tex_load_t* data = sk_malloc_zero_t(tex_load_t, 1);
	data->flags      = flags;
	data->file_count = file_count;
	data->write      = tex_write_issue(tex);
	data->type       = tex->type;
	return data;
}

///////////////////////////////////////////

// Newer data already landed, so finishing this load would only waste work.
static bool tex_load_superseded(tex_t tex, const tex_load_t* data) {
	return tex_write_newer(atomic_load_i32(&tex->write_landed), data->write);
}

///////////////////////////////////////////

asset_action_result_ tex_load_arr_files_shared(asset_task_t* task, asset_header_t* asset, void* job_data) {
	profiler_zone();

	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	if (data->file_data == nullptr) {
		data->file_data  = sk_malloc_zero_t(void *, data->file_count);
		data->file_sizes = sk_malloc_zero_t(size_t, data->file_count);
	}

	// Request files one at a time. Off the web the result lands before the
	// request returns; on the web a miss streams in and the task parks, and
	// the arrival re-runs this action, which resumes right here.
	while (data->file_read_curr < data->file_count) {
		asset_read_ read = assets_task_read_file(task, data->file_names[data->file_read_curr], &data->file_read);
		if (read == asset_read_in_flight)
			return asset_action_wait;
		if (read == asset_read_failed) {
			log_warnf(tex_msg_load_failed, data->file_names[data->file_read_curr]);
			data->error = asset_state_error_not_found;
			return asset_action_fail;
		}
		data->file_data [data->file_read_curr] = data->file_read.data;
		data->file_sizes[data->file_read_curr] = data->file_read.size;
		data->file_read       = {};
		data->file_read_curr += 1;
	}

	int32_t     final_width       = 0;
	int32_t     final_height      = 0;
	int32_t     final_array_count = 0;
	int32_t     final_mip_count   = 0;
	tex_format_ final_format      = tex_format_none;

	for (int32_t i = 0; i < data->file_count; i++) {
		// Grab the image metadata
		int32_t     curr_width       = 0;
		int32_t     curr_height      = 0;
		int32_t     curr_array_count = 0;
		int32_t     curr_mip_count   = 0;
		tex_format_ curr_format      = tex_format_none;
		if (!tex_load_image_info(data->file_data[i], data->file_sizes[i], data->flags, &data->type, &curr_format, &curr_width, &curr_height, &curr_array_count, &curr_mip_count)) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			data->error = asset_state_error_unsupported;
			return asset_action_fail;
		}

		// For multiple images, they should all be the same size/format/layout.
		if ((final_width       != 0          && final_width       != curr_width      ) ||
			(final_height      != 0          && final_height      != curr_height     ) ||
			(final_format != tex_format_none && final_format      != curr_format     ) ||
			(final_array_count != 0          && final_array_count != curr_array_count) ||
			(final_mip_count   != 0          && final_mip_count   != curr_mip_count  ) ) {
			log_warnf(tex_msg_mismatched_images, data->file_names[i]);
			data->error = asset_state_error_unsupported;
			return asset_action_fail;
		}
		// If the user has specified multiple image files, those files cannot
		// be array textures themselves. Or rather, they could, but we haven't
		// implemented that scenario here yet.
		if (data->file_count > 1 && curr_array_count > 1) {
			log_warnf(tex_msg_nested_arrays, data->file_names[i]);
			data->error = asset_state_error_unsupported;
			return asset_action_fail;
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
	return asset_action_done;
}

///////////////////////////////////////////

// The one place a GPU format gets picked from flags. none keeps `format`.
static tex_format_ tex_pick_gpu_format(tex_data_ flags, tex_type_ type, tex_format_ format, bool32_t alpha, int32_t width, int32_t height, int32_t layers) {
	return tex_compress_pick(flags, tex_get_compression_default(), type, format, alpha, width, height, layers, tex_compress_caps());
}

///////////////////////////////////////////

// Settles the GPU format from headers alone, so Tex.Format is final from the
// start. Returns the format the texture should report.
static tex_format_ tex_load_pick(tex_load_t* data, int32_t width, int32_t height, int32_t array_count) {
	bool32_t alpha = false;
	for (int32_t i = 0; i < data->file_count && !alpha; i++)
		alpha = tex_image_has_alpha(data->file_data[i], data->file_sizes[i]);

	data->gpu_format = tex_pick_gpu_format(data->flags, data->type, data->color_format, alpha, width, height, array_count);
	return data->gpu_format != tex_format_none ? data->gpu_format : data->color_format;
}

///////////////////////////////////////////

// Between tex_write_begin and end. A newer write already set the meta it
// will land with, so a load from before it leaves the meta alone.
static void tex_load_meta(tex_t tex, tex_load_t* data, int32_t width, int32_t height, int32_t array_count) {
	tex_format_ format = tex_load_pick(data, width, height, array_count);
	if (tex_write_newer(atomic_load_i32(&tex->write_issued), data->write)) return;
	tex->type        = data->type;
	tex->data_format = data->color_format;
	tex_set_meta(tex, width, height, 1, format);
}

///////////////////////////////////////////

asset_action_result_ tex_load_arr_files(asset_task_t *task, asset_header_t *asset, void *job_data) {
	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	// A read in flight still needs its buffer, so only check before the first
	if (data->file_data == nullptr && tex_load_superseded(tex, data)) return asset_action_done;
	asset_action_result_ read = tex_load_arr_files_shared(task, asset, job_data);
	if (read != asset_action_done)
		return read;

	// Re-running reads no files, only the headers again
	if (!tex_write_try_begin(tex)) return tex_write_busy();
	tex_load_meta(tex, data, data->color_width, data->color_height, data->color_array_count);
	tex_write_end(tex);
	return asset_action_done;
}

///////////////////////////////////////////

// The largest mips dominate transcode cost, the rest finish in one slice.
static const int32_t tex_ktx2_solo_mips = 2;

asset_action_result_ tex_load_arr_parse(asset_task_t *task, asset_header_t *asset, void *job_data) {
	profiler_zone();

	tex_load_t *data = (tex_load_t *)job_data;
	tex_t       tex  = (tex_t)asset;
	if (tex_load_superseded(tex, data)) return asset_action_done;

	// One allocation per file, zeroed so tex_load_free has nothing to free for
	// the entries a failed parse never reached.
	if (data->color_data == nullptr)
		data->color_data = sk_malloc_zero_t(void*, data->file_count);

	// The cursors persist, so a sliced transcode resumes where it left.
	for (; data->parse_file_curr < data->file_count; data->parse_file_curr++) {
		int32_t     i           = data->parse_file_curr;
		int32_t     width       = 0;
		int32_t     height      = 0;
		int32_t     array_count = 0;
		int32_t     mip_count   = 0;
		tex_format_ format      = tex_format_none;

		if (data->ktx2_slice != nullptr || ktx2_sniff(data->file_data[i], data->file_sizes[i])) {
			if (data->ktx2_slice == nullptr) {
				data->ktx2_slice = ktx2_decode_begin(data->file_data[i], data->file_sizes[i], &data->type, &format, &width, &height, &array_count, &mip_count, &data->color_data[i]);
				if (data->ktx2_slice == nullptr) {
					log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
					data->error = asset_state_error_unsupported;
					goto end;
				}
				// Validate against the meta pass up front, so a mismatch never
				// spends a single level of transcode.
				if (data->color_format    != format ||
					data->color_width     != width  ||
					data->color_height    != height ||
					data->color_mip_count != mip_count) {
					log_warnf(tex_msg_inconsistent_parse, data->file_names[i]);
					data->error = asset_state_error;
					goto end;
				}
				data->parse_array_count += array_count;
			}
			int32_t level = 0;
			bool    done  = false;
			do {
				if (!ktx2_decode_step(data->ktx2_slice, &level, &done)) {
					log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
					data->error = asset_state_error_unsupported;
					goto end;
				}
			} while (!done && level >= tex_ktx2_solo_mips);
			if (!done)
				return asset_action_continue;
			ktx2_decode_end(data->ktx2_slice);
			data->ktx2_slice = nullptr;
			continue; // the meta checks below already ran at begin
		}

		if (!tex_load_image_data(data->file_data[i], data->file_sizes[i], data->flags, &data->type, &format, &width, &height, &array_count, &mip_count, &data->color_data[i])) {
			log_warnf(tex_msg_invalid_fmt, data->file_names[i]);
			data->error = asset_state_error_unsupported;
			goto end;
		}
		data->parse_array_count += array_count;

		// Make sure the data in this image matches what we extracted in
		// earlier phases of texture creation. If it doesn't, then something
		// has gone very wrong!
		if (data->color_format    != format ||
			data->color_width     != width  ||
			data->color_height    != height ||
			data->color_mip_count != mip_count) {
			log_warnf(tex_msg_inconsistent_parse, data->file_names[i]);
			data->error = asset_state_error;
			goto end;
		}
	}

	if (data->color_array_count != data->parse_array_count) {
		log_warnf(tex_msg_inconsistent_parse, data->file_names[0]);
		data->error = asset_state_error;
		goto end;
	}

end:

	// Release file memory now that we're done with it
	for (int32_t i = 0; i < data->file_count; i++)
		sk_free(data->file_data[i]);

	return data->error < asset_state_none ? asset_action_fail : asset_action_done;
}

///////////////////////////////////////////

// array_data[layer] holds that layer's whole mip chain, and the GPU wants the
// transpose of that, so this gathers across layers into one allocation.
static void* tex_flatten_layers(tex_format_ format, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t mip_count) {
	skr_vec3i_t base_size  = { width, height, 1 };
	size_t      total_size = 0;
	for (int32_t mip = 0; mip < mip_count; mip++)
		total_size += skr_tex_calc_mip_size((skr_tex_fmt_)format, base_size, mip) * array_count;
	void* flat_data = sk_malloc(total_size);

	uint8_t* dst        = (uint8_t*)flat_data;
	uint64_t mip_offset = 0;
	for (int32_t mip = 0; mip < mip_count; mip++) {
		uint64_t mip_size = skr_tex_calc_mip_size((skr_tex_fmt_)format, base_size, mip);
		for (int32_t layer = 0; layer < array_count; layer++) {
			memcpy(dst, (uint8_t*)array_data[layer] + mip_offset, (size_t)mip_size);
			dst += mip_size;
		}
		mip_offset += mip_size;
	}
	return flat_data;
}

///////////////////////////////////////////

// Mips are generated on a staging copy in the source format, so they filter
// at full precision, and then the encoded result is swapped in.
static void tex_upload_compressed(tex_t tex, const tex_upload_t* upload) {
	profiler_zone();

	bool cube     = (tex->type & tex_type_cubemap) != 0;
	bool gen_mips = (tex->type & tex_type_mips) && upload->mip_count <= 1;

	skr_tex_flags_ flags = skr_tex_flags_readable;
	if (cube)     flags = (skr_tex_flags_)(flags | skr_tex_flags_cubemap);
	if (gen_mips) flags = (skr_tex_flags_)(flags | skr_tex_flags_gen_mips);
	skr_tex_sampler_t sampler  = tex_get_skr_sampler(tex);
	skr_tex_data_t    tex_data = {};
	tex_data.data        = upload->data;
	tex_data.mip_count   = upload->mip_count;
	tex_data.layer_count = upload->array_count;

	skr_cmd_begin();
	skr_tex_t staging = {};
	skr_tex_create((skr_tex_fmt_)upload->format, flags, sampler, { upload->width, upload->height, 1 }, 1, gen_mips ? 0 : upload->mip_count, &tex_data, &staging);
	skr_tex_t   result = {};
	tex_format_ format = upload->format;
	if (skr_tex_is_valid(&staging)) {
		if (gen_mips) skr_tex_generate_mips(&staging, nullptr);
		result = tex_compress_gpu(&staging, upload->gpu_format, sampler);
		if (skr_tex_is_valid(&result)) {
			format = (tex_format_)skr_tex_get_format(&result);
			// Inside the scope, so the destroy waits on this thread's submit.
			skr_tex_destroy(&staging);
		} else {
			log_warnf("Texture compression failed for '%s', keeping it uncompressed.", tex_get_id(tex));
			result = staging;
		}
	}
	skr_cmd_end();

	tex_land(tex, result, upload->width, upload->height, 1, format);
	// Same as _tex_set_color_flat, new content makes cached lighting stale.
	if (cube) tex->sh_dirty = true;
}

///////////////////////////////////////////

asset_action_result_ tex_load_arr_upload(asset_task_t *, asset_header_t *asset, void *job_data) {
	tex_load_t* data = (tex_load_t*)job_data;
	tex_t       tex  = (tex_t)asset;

	if (!tex_write_try_begin(tex)) return tex_write_busy();
	if (!tex_write_claim(tex, data->write)) {
		// Data set after this load was requested already landed
		tex_write_end(tex);
		return asset_action_done;
	}

	tex->type = data->type;

	// A lone file decodes mip-major already, several files arrive as one
	// image each and have to be interleaved.
	tex_upload_t* upload = sk_malloc_zero_t(tex_upload_t, 1);
	upload->write       = data->write;
	upload->flags       = data->flags;
	upload->width       = data->color_width;
	upload->height      = data->color_height;
	upload->depth       = 1;
	upload->array_count = data->color_array_count;
	upload->mip_count   = data->color_mip_count;
	upload->multisample = 1;
	upload->format      = data->color_format;
	upload->gpu_format  = data->gpu_format;
	if (data->file_count == 1) {
		upload->data        = data->color_data[0];
		data->color_data[0] = nullptr;
	} else {
		upload->data = tex_flatten_layers(data->color_format, data->color_width, data->color_height, data->color_data, data->color_array_count, data->color_mip_count);
		for (int32_t i = 0; i < data->file_count; i++) {
			sk_free(data->color_data[i]);
			data->color_data[i] = nullptr;
		}
	}
	tex_upload_run(tex, upload);
	bool loaded = tex->header.state == asset_state_loaded;
	tex_write_end (tex);
	return loaded ? asset_action_done : asset_action_fail;
}

///////////////////////////////////////////

// The one place a load's failure reaches the texture, and data that landed
// after the load was requested stays.
void tex_load_on_failure(asset_header_t *asset, void *job_data) {
	tex_t       tex  = (tex_t)asset;
	tex_load_t* data = (tex_load_t*)job_data;
	tex_write_begin(tex);
	if (tex_write_claim(tex, data->write)) {
		tex->header.state = data->error < asset_state_none ? data->error : asset_state_error;
		tex_set_fallback(tex, _tex_get_error_fallback(tex));
	}
	tex_write_end(tex);
}


///////////////////////////////////////////

bool tex_load_image_info(void *data, size_t data_size, tex_data_ flags, tex_type_* ref_image_type, tex_format_* out_format, int32_t *out_width, int32_t *out_height, int32_t* out_array_count, int32_t* out_mip_count) {

	// Check for valid .HDR formats
	hdr_header_t hdr_header = hdr_parse_header(data, data_size);
	if (hdr_header.valid) {
		*out_format      = tex_format_rg11b10;
		*out_array_count = 1;
		*out_mip_count   = 1;
		*out_width       = hdr_header.width;
		*out_height      = hdr_header.height;
		return true;
	}

	// Check STB image formats
	int32_t comp;
	bool success = stbi_info_from_memory((const stbi_uc*)data, (int)data_size, out_width, out_height, &comp) == 1;
	if (success) {
		*out_mip_count   = 1;
		*out_array_count = 1;
		if (stbi_is_hdr_from_memory((stbi_uc *)data, (int)data_size)) *out_format = tex_format_rg11b10;
		else                                                          *out_format = (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear;
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
		else                                 *out_format = (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear;
		return true;
	}

	if (ktx2_info(data, data_size, ref_image_type, out_format, out_width, out_height, out_array_count, out_mip_count))
		return true;

	return false;
}

///////////////////////////////////////////

bool tex_load_image_data(void *data, size_t data_size, tex_data_ flags, tex_type_* ref_image_type, tex_format_ *out_format, int32_t *out_width, int32_t *out_height, int32_t *out_array_count, int32_t *out_mip_count, void **out_data) {
	int32_t channels = 0;

	// Check for valid .HDR formats
	hdr_header_t hdr_header = hdr_parse_header(data, data_size);
	if (hdr_header.valid) {
		hdr_image_t img = hdr_decode_pixels_rg11b10(data, data_size, &hdr_header);
		*out_format      = tex_format_rg11b10;
		*out_array_count = 1;
		*out_mip_count   = 1;
		*out_width       = hdr_header.width;
		*out_height      = hdr_header.height;
		*out_data    = img.pixels;
		return true;
	}

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
		*out_data = packed;

		free(full);

		return true;
	}

	// Check through stbi's list of image formats
	*out_data = stbi_load_from_memory ((stbi_uc*)data, (int)data_size, out_width, out_height, &channels, 4);
	if (*out_data != nullptr) {
		*out_format      = (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear;
		*out_array_count = 1;
		*out_mip_count   = 1;
		return *out_data != nullptr;
	}

	// Check for qoi images
	qoi_desc q_desc = {};
	*out_data = qoi_decode(data, (int)data_size, &q_desc, 4);
	if (*out_data != nullptr) {
		*out_width       = q_desc.width;
		*out_height      = q_desc.height;
		*out_array_count = 1;
		*out_mip_count   = 1;
		// If QOI claims it's linear, then we'll go with that!
		if (q_desc.colorspace == QOI_LINEAR) *out_format = tex_format_rgba32_linear;
		else                                 *out_format = (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear;
		return *out_data != nullptr;
	}

	// Check for KTX2
	if (ktx2_decode(data, data_size, ref_image_type, out_format, out_width, out_height, out_array_count, out_mip_count, out_data))
		return true;

	return false;
}

///////////////////////////////////////////
// Texture creation functions            //
///////////////////////////////////////////

asset_task_t tex_make_loading_task(tex_t texture, void *load_data, const asset_action_t *actions, int32_t action_count, int32_t priority, int32_t complexity) {
	asset_task_t task = {};
	task.asset        = (asset_header_t*)texture;
	task.free_data    = tex_load_free;
	task.on_failure   = tex_load_on_failure;
	task.load_data    = load_data;
	task.actions      = (asset_action_t *)actions;
	task.action_count = action_count;
	task.priority     = priority;
	task.sort         = asset_sort(priority, complexity);
	return task;
}

///////////////////////////////////////////

static void tex_add_task(asset_task_t task, tex_data_ flags) {
	if (flags & tex_data_blocking) assets_run_blocking(task);
	else                           assets_add_task    (task);
}

///////////////////////////////////////////

tex_t tex_create_file_type(const char *file, tex_type_ type, tex_data_ flags, int32_t priority) {
	tex_t result = tex_find(file);
	if (result != nullptr)
		return result;

	result = tex_create(type);
	tex_set_id(result, file);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = tex_load_create(result, flags, 1);
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(file);

	static const asset_action_t actions[] = {
		{ tex_load_arr_files },
		{ tex_load_arr_parse, asset_affinity_heavy },
		{ tex_load_arr_upload },
	};
	tex_add_task(tex_make_loading_task(result, load_data, actions, _countof(actions), priority, asset_complexity_bytes(platform_file_size(file))), flags);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_file(const char *file, tex_data_ flags, int32_t priority) {
	return tex_create_file_type(file, tex_type_image, flags, priority);
}

///////////////////////////////////////////

tex_t tex_create_mem_type(tex_type_ type, void *data, size_t data_size, tex_data_ flags, int32_t priority) {
	tex_t result = tex_create(type);

	tex_load_t *load_data = tex_load_create(result, flags, 1);
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
	if (!tex_load_image_info(load_data->file_data[0], load_data->file_sizes[0], load_data->flags, &load_data->type, &format, &load_data->color_width, &load_data->color_height, &load_data->color_array_count, &load_data->color_mip_count)) {
		log_warnf(tex_msg_invalid_fmt, load_data->file_names[0]);
		load_data->error = asset_state_error_unsupported;
		tex_load_on_failure(&result->header, load_data);
		tex_load_free      (&result->header, load_data);
		return result;
	}
	load_data->color_format = format;
	tex_write_begin(result);
	tex_load_meta  (result, load_data, load_data->color_width, load_data->color_height, load_data->color_array_count);
	tex_write_end  (result);

	static const asset_action_t actions[] = {
		{ tex_load_arr_parse, asset_affinity_heavy },
		{ tex_load_arr_upload },
	};
	tex_add_task(tex_make_loading_task(result, load_data, actions, _countof(actions), priority, asset_complexity_bytes(data_size)), flags);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_mem(void *data, size_t data_size, tex_data_ flags, int32_t priority) {
	return tex_create_mem_type(tex_type_image, data, data_size, flags, priority);
}

///////////////////////////////////////////

tex_t tex_create(tex_type_ type, tex_format_ format) {
	tex_t result = (tex_t)assets_allocate(asset_type_tex);
	result->owned  = true;
	result->type        = type;
	result->format      = format;
	result->data_format = format;
	result->address_mode = tex_address_wrap;
	result->sample_mode  = tex_sample_linear;
	result->anisotropy   = 4;
	result->header.state = asset_state_none;

	tex_set_fallback(result, _tex_get_loading_fallback(result));

	return result;
}

///////////////////////////////////////////

tex_t tex_create_rendertarget(int32_t width, int32_t height, int32_t msaa, tex_format_ color_format, tex_format_ depth_format) {
	tex_t result;
	if (color_format == tex_format_none && depth_format != tex_format_none) {
		result = tex_create(tex_type_image_nomips | tex_type_depthtarget, depth_format);

		tex_set_size(result, width, height, 1, 1, msaa);
	} else {
		result = tex_create(tex_type_image_nomips | tex_type_rendertarget, color_format);

		tex_set_size(result, width, height, 1, 1, msaa);
		if (depth_format != tex_format_none)
			tex_add_zbuffer(result, depth_format);
	}
	return result;
}

///////////////////////////////////////////

tex_t tex_create_color32(color32 *data, int32_t width, int32_t height, tex_data_ flags) {
	tex_t result = tex_create(tex_type_image, (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear);
	tex_set_colors(result, width, height, data, flags);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_color128(color128 *data, int32_t width, int32_t height, tex_data_ flags) {
	tex_t    result = tex_create(tex_type_image, (flags & tex_data_srgb) ? tex_format_rgba32 : tex_format_rgba32_linear);
	color32 *color  = sk_malloc_t(color32, width * height);
	for (int32_t i = 0; i < width*height; i++)
		color[i] = color_to_32(data[i]);
	tex_set_colors(result, width, height, color, flags);

	sk_free(color);
	return result;
}

///////////////////////////////////////////

tex_t _tex_create_file_arr(tex_type_ type, const char **files, int32_t file_count, tex_data_ flags, int32_t priority) {
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

	tex_load_t *load_data = tex_load_create(result, flags, file_count);
	load_data->file_names = sk_malloc_t(char *, file_count);
	size_t total_size = 0;
	for (int32_t i = 0; i < file_count; i++) {
		load_data->file_names[i] = string_copy(files[i]);
		total_size              += platform_file_size(files[i]);
	}

	static const asset_action_t actions[] = {
		{ tex_load_arr_files },
		{ tex_load_arr_parse, asset_affinity_heavy },
		{ tex_load_arr_upload },
	};
	tex_add_task(tex_make_loading_task(result, load_data, actions, _countof(actions), priority, asset_complexity_bytes(total_size)), flags);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_file_arr(const char **files, int32_t file_count, tex_data_ flags, int32_t priority) {
	return _tex_create_file_arr(tex_type_image, files, file_count, flags, priority);
}
///////////////////////////////////////////

// Equirects are generally 2x1, but anything could work.
static int32_t tex_equirect_face_size(const tex_load_t* data) { return data->color_height / 2; }

///////////////////////////////////////////

tex_t tex_create_cubemap_file(const char *cubemap_file, tex_data_ flags, int32_t priority) {
	profiler_zone();

	char cubemap_id[64];
	snprintf(cubemap_id, sizeof(cubemap_id), "sk/tex/cubemap/%" PRIu64, hash_string(cubemap_file));

	tex_t result = tex_find(cubemap_id);
	if (result != nullptr) {
		return result;
	}

	// Skyboxes are raw radiance at mip 0, while mip chains belong to
	// reflection cubemaps. Mips shipped in the file itself are still kept.
	result = tex_create(tex_type_image_nomips | tex_type_cubemap);
	tex_set_id(result, cubemap_id);
	result->header.state = asset_state_loading;

	tex_load_t *load_data = tex_load_create(result, flags, 1);
	load_data->file_names    = sk_malloc_t(char *, 1);
	load_data->file_names[0] = string_copy(cubemap_file);

	///////////////////////////////////////////

	asset_load_action_t load = [](asset_task_t* task, asset_header_t* asset, void* job_data) {
		tex_load_t* data = (tex_load_t*)job_data;
		tex_t       tex  = (tex_t)asset;

		if (data->file_data == nullptr && tex_load_superseded(tex, data)) return asset_action_done;
		asset_action_result_ read = tex_load_arr_files_shared(task, asset, job_data);
		if (read != asset_action_done)
			return read;

		int32_t size_w, size_h;
		if (data->color_array_count == 6) { // file contains a pre-built cubemap
			size_w = data->color_width;
			size_h = data->color_height;
		/*} else if (data->color_array_count == 1 && data->color_width / 4 == data->color_height / 3) { // Cubemap crosses are 4x3 aspect ratio
			size_w = size_h = data->color_height / 3;
		} else if (data->color_array_count == 1 && data->color_width / 6 == data->color_height    ) { // Cubemap lists are 6x1 aspect ratio
			size_w = size_h = data->color_height;*/
		} else if (data->color_array_count == 1) { // We'll treat this like an equirect, generally they're 2x1, but anything could work.
			size_w = size_h = tex_equirect_face_size(data);
		} else {
			log_warnf(tex_msg_invalid_cubemap, data->file_names[0]);
			data->error = asset_state_error_unsupported;
			return asset_action_fail;
		}

		// An equirect becomes a cube too, so both pick as six faces.
		if (!tex_write_try_begin(tex)) return tex_write_busy();
		tex_load_meta(tex, data, size_w, size_h, 6);
		tex_write_end(tex);
		return asset_action_done;
	};

	///////////////////////////////////////////

	asset_load_action_t upload = [](asset_task_t* task, asset_header_t * asset, void* job_data) {
		profiler_zone();

		tex_load_t *data = (tex_load_t *)job_data;
		tex_t       tex  = (tex_t)asset;

		// If this is already a cubemap, we don't need to do anything fancy
		// here, just pass it along to our normal texture upload code.
		if (data->color_array_count == 6) {
			return tex_load_arr_upload(task, asset, job_data);
		}

		if (!tex_write_try_begin(tex)) return tex_write_busy();
		if (!tex_write_claim(tex, data->write)) {
			tex_write_end(tex);
			return asset_action_done;
		}
		tex->type = data->type;

		skr_cmd_begin();

		// Built in a local rather than through tex_set_color_arr, which would
		// flip the state to loaded and fire on_load before the convert runs.
		// Sized from the load rather than tex, whose meta a newer write may own.
		skr_tex_flags_    flags   = tex_type_to_skr_flags((tex_type_)(tex->type | tex_type_rendertarget), data->color_format);
		skr_tex_sampler_t sampler = tex_get_skr_sampler(tex);
		int32_t           face    = tex_equirect_face_size(data);
		skr_vec3i_t       size    = { face, face, 1 };
		skr_tex_t         cube    = {};
		skr_err_ err = skr_tex_create(
			(skr_tex_fmt_)data->color_format,
			flags,
			sampler,
			size,
			1,  // multisample
			1,  // mip 0 only, skyboxes don't carry mip chains
			nullptr,  // no initial data
			&cube);
		if (err != skr_err_success) {
			skr_cmd_end();
			tex_write_end(tex);
			log_err("Failed to create cubemap texture for equirect conversion");
			data->error = asset_state_error;
			return asset_action_fail;
		}

		// Make the texture for our source equirect data
		skr_tex_sampler_t equirect_sampler = {};
		equirect_sampler.sample  = skr_tex_sample_linear;
		equirect_sampler.address = skr_tex_address_wrap;
		skr_tex_data_t tex_data = {};
		tex_data.data        = data->color_data[0];
		tex_data.layer_count = 1;
		tex_data.mip_count   = 1;
		skr_tex_t equirect;
		skr_tex_create((skr_tex_fmt_)data->color_format, skr_tex_flags_readable, equirect_sampler, {data->color_width, data->color_height, 1}, 1, 0, &tex_data, &equirect);

		// Make the material for converting equirect to cubemap. Copying the
		// default material keeps this task's source private, while its
		// pipeline stays compiled with the default. The source is a raw
		// staging texture with no asset wrapper, so it binds at the skr level.
		material_t convert_mat = material_copy(sk_default_material_equirect);
		skr_material_set_tex(&convert_mat->gpu_mat, "source", &equirect);

		// Convert the equirect into the cubemap's only mip level
		skr_recti_t bounds = { 0, 0, size.x, size.y };
		skr_renderer_blit(&convert_mat->gpu_mat, &cube, bounds);

		material_release(convert_mat);
		skr_tex_destroy(&equirect);

		skr_tex_t   result = cube;
		tex_format_ format = data->color_format;
		if (data->gpu_format != tex_format_none) {
			skr_tex_t encoded = tex_compress_gpu(&cube, data->gpu_format, sampler);
			if (skr_tex_is_valid(&encoded)) {
				skr_tex_destroy(&cube);
				result = encoded;
				format = (tex_format_)skr_tex_get_format(&encoded);
			} else {
				log_warnf("Texture compression failed for '%s', keeping it uncompressed.", tex_get_id(tex));
			}
		}
		// Lighting data comes from tex_gen_cubemap_reflection, not from here.
		skr_cmd_end();

		// Only an uncompressed cube can still be rendered into.
		if (format == data->color_format) tex->type = (tex_type_)(tex->type | tex_type_rendertarget);
		tex_land     (tex, result, size.x, size.y, 1, format);
		tex_write_end(tex);
		return asset_action_done;
	};

	///////////////////////////////////////////

	static const asset_action_t actions[] = {
		{ load },
		{ tex_load_arr_parse, asset_affinity_heavy },
		{ upload },
	};
	tex_add_task(tex_make_loading_task(result, load_data, actions, _countof(actions), priority, asset_complexity_bytes(platform_file_size(cubemap_file))), flags);

	return result;
}

///////////////////////////////////////////

tex_t tex_create_cubemap_files(const char **cube_face_file_xxyyzz, tex_data_ flags, int32_t priority) {
	// Skyboxes are mip 0 only; mips shipped in the files are still kept.
	return _tex_create_file_arr(tex_type_image_nomips | tex_type_cubemap, cube_face_file_xxyyzz, 6, flags, priority);
}

///////////////////////////////////////////

tex_t tex_copy(const tex_t texture, tex_type_ type, tex_format_ format, tex_data_ flags, tex_t into) {
	profiler_zone();

	if (into == texture) {
		log_warn("tex_copy can't copy a texture over itself, pass a separate destination.");
		return nullptr;
	}

	int32_t layers        = (int32_t)texture->gpu_tex.layer_count;
	int32_t src_mip_count = (int32_t)texture->gpu_tex.mip_levels;
	bool    has_mips      = src_mip_count > 1;
	bool    cube          = (texture->gpu_tex.flags & skr_tex_flags_cubemap) != 0;
	if (cube) type = type | tex_type_cubemap; // a copy keeps the source's layers

	// An explicit format is exact, and the flags only pick one when it's none
	tex_format_ src_format = texture->format;
	tex_format_ gpu_format = tex_format_none;
	if (format == tex_format_none)
		gpu_format = tex_pick_gpu_format(flags, type, src_format, tex_format_has_alpha(src_format), texture->width, texture->height, layers);
	else if (tex_format_is_compressed(format))
		gpu_format = format;
	if (gpu_format != tex_format_none && (layers > 1 && !cube)) {
		log_warn("tex_copy can't compress array textures yet, copying uncompressed.");
		gpu_format = tex_format_none;
	}
	tex_format_ copy_format = (format == tex_format_none || gpu_format != tex_format_none) ? src_format : format;

	// A block format can keep the source's chain, but can't generate one
	bool wants_mips = (type & tex_type_mips) && (has_mips || tex_format_is_mippable(copy_format));

	// Destination mip count:
	//  - dest doesn't want mips           → 1 mip
	//  - dest wants mips, source has them → match the source (preserves a
	//                                       partial chain instead of leaving
	//                                       the tail uninitialized)
	//  - dest wants mips, source has none → full chain (generated below)
	int32_t dest_mip_count;
	if      (!wants_mips) dest_mip_count = 1;
	else if (has_mips)    dest_mip_count = src_mip_count;
	else                  dest_mip_count = skr_tex_calc_mip_count({ texture->width, texture->height, 1 });

	tex_t dest = into != nullptr ? into : tex_create(type, copy_format);

	// skr_tex_flags_dynamic is how sk_renderer grants the copy's TRANSFER_DST
	skr_tex_flags_ skr_flags = (skr_tex_flags_)((gpu_format != tex_format_none ? skr_tex_flags_readable : tex_type_to_skr_flags(type, copy_format)) | skr_tex_flags_dynamic);
	if (cube)                     skr_flags = (skr_tex_flags_)(skr_flags | skr_tex_flags_cubemap);
	else if (layers > 1)          skr_flags = (skr_tex_flags_)(skr_flags | skr_tex_flags_array);
	if (wants_mips && !has_mips)  skr_flags = (skr_tex_flags_)(skr_flags | skr_tex_flags_gen_mips);
	skr_tex_sampler_t sampler = tex_get_skr_sampler(dest);

	skr_cmd_begin();
	skr_tex_t copy = {};
	skr_tex_create((skr_tex_fmt_)copy_format, skr_flags, sampler, { texture->width, texture->height, cube ? 1 : layers }, 1, dest_mip_count, nullptr, &copy);

	// skr_tex_copy is one-mip-per-call; copy each level the source actually
	// has. If the source has no mips but the destination wants them, fall
	// through to skr_tex_generate_mips after the base copy.
	int32_t copy_count = (has_mips && wants_mips) ? src_mip_count : 1;
	for (int32_t m = 0; m < copy_count; m++) {
		skr_tex_copy(&texture->gpu_tex, &copy, m, 0, m, 0, layers);
	}
	if (wants_mips && !has_mips) {
		skr_tex_generate_mips(&copy, nullptr);
	}

	skr_tex_t   result        = copy;
	tex_format_ result_format = copy_format;
	if (gpu_format != tex_format_none) {
		skr_tex_t encoded = tex_compress_gpu(&copy, gpu_format, sampler);
		if (skr_tex_is_valid(&encoded)) {
			// Inside the scope, so the destroy waits on this thread's submit.
			skr_tex_destroy(&copy);
			result        = encoded;
			result_format = (tex_format_)skr_tex_get_format(&encoded);
		} else {
			log_warnf("Texture compression failed for a copy of '%s', keeping it uncompressed.", tex_get_id(texture));
		}
	}
	skr_cmd_end();

	tex_write_begin_now(dest);
	dest->type        = type;
	dest->data_format = src_format;
	tex_land(dest, result, texture->width, texture->height, 1, result_format);
	if (cube) dest->sh_dirty = true;
	tex_write_end(dest);
	if (into != nullptr) tex_addref(into);
	return dest;
}

///////////////////////////////////////////

bool32_t tex_gen_mips(tex_t texture) {
	if (!tex_gens_mips(texture->type, texture->format) ||
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

	// If we already have a zbuffer that matches the color texture's
	// resolution and sample count, keep it.
	int32_t msaa = skr_tex_get_multisample(&texture->gpu_tex);
	if (texture->depth_buffer != nullptr
		&& texture->depth_buffer->width  == texture->width
		&& texture->depth_buffer->height == texture->height
		&& skr_tex_get_multisample(&texture->depth_buffer->gpu_tex) == msaa) {
		return;
	}

	if (texture->depth_buffer != nullptr) tex_release(texture->depth_buffer);

	char id[64];
	assets_unique_name(asset_type_tex, "sk/tex/zbuffer/", id, sizeof(id));
	texture->depth_buffer = tex_create(tex_type_zbuffer, format);
	tex_set_id       (texture->depth_buffer, id);
	tex_set_size(texture->depth_buffer, texture->width, texture->height, 1, texture->gpu_tex.layer_count, msaa);
	texture->depth_buffer->header.state = asset_state_loaded;
}

///////////////////////////////////////////

void tex_set_zbuffer(tex_t texture, tex_t depth_texture) {
	if (!(texture->type & tex_type_rendertarget)) {
		log_err(tex_msg_requires_rendertarget);
		return;
	}
	if (depth_texture != nullptr && !(depth_texture->type & (tex_type_depth | tex_type_depthtarget))) {
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
	tex_write_begin_now(texture);

	// Always destroy old GPU resources when valid - skr_tex_destroy handles
	// is_external internally to decide whether to destroy the VkImage.
	if (skr_tex_is_valid(&texture->gpu_tex))
		skr_tex_destroy(&texture->gpu_tex);

	texture->owned = owned;

	texture->type   = type;
	texture->format = tex_get_tex_format(native_fmt);

	if (native_surface != nullptr) {
		skr_tex_external_info_t info = {};
		info.image         = (VkImage)native_surface;
		info.format        = skr_tex_fmt_from_native((uint32_t)native_fmt);
		info.flags         = tex_type_to_skr_flags(type, texture->format);
		info.size          = { width, height, 1 };
		info.sampler       = tex_get_skr_sampler(texture);
		info.multisample   = multisample;
		info.array_layers  = surface_count;
		info.owns_image    = owned;

		// Swapchain images arrive in, and must be handed back in, the
		// attachment layout. They're not readable, so render passes end there.
		if (type & tex_type_attachment_internal) {
			info.current_layout = (type & tex_type_zbuffer)
				? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		skr_tex_create_external_vk(info, &texture->gpu_tex);
	} else {
		texture->gpu_tex = {};
	}

	texture->width  = width;
	texture->height = height;

	texture->header.state = skr_tex_is_valid(&texture->gpu_tex)
		? asset_state_loaded
		: asset_state_error;
	tex_set_fallback(texture, texture->header.state <= 0
		? _tex_get_error_fallback(texture)
		: nullptr);
	tex_write_end(texture);
}

///////////////////////////////////////////

void* tex_get_surface(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded);
	return (void*)texture->gpu_tex.image;
}

///////////////////////////////////////////

tex_t tex_create_from_hardware_buffer(void *hardware_buffer, bool32_t owns_buffer) {
#if defined(SK_OS_ANDROID)
	if (hardware_buffer == nullptr) return nullptr;
	if (!skr_is_capable(skr_capability_external_ahb)) {
		log_warn("tex_create_from_hardware_buffer: AHardwareBuffer import is not supported on this device!");
		return nullptr;
	}

	tex_t result = tex_create(tex_type_image_nomips, tex_format_none);

	skr_tex_external_ahb_info_t info = {};
	info.hardware_buffer = hardware_buffer;
	info.format          = skr_tex_fmt_none;
	info.sampler         = tex_get_skr_sampler(result);
	info.owns_buffer     = owns_buffer;

	if (skr_tex_create_external_ahb(info, &result->gpu_tex) == skr_err_success) {
		skr_vec3i_t size = skr_tex_get_size(&result->gpu_tex);
		result->width    = size.x;
		result->height   = size.y;
		result->format   = (tex_format_)skr_tex_get_format(&result->gpu_tex);
	}

	result->header.state = skr_tex_is_valid(&result->gpu_tex)
		? asset_state_loaded
		: asset_state_error;
	tex_set_fallback(result, result->header.state <= 0
		? _tex_get_error_fallback(result)
		: nullptr);
	return result;
#else
	(void)hardware_buffer;
	(void)owns_buffer;
	return nullptr;
#endif
}

///////////////////////////////////////////

void* tex_get_hardware_buffer(tex_t texture) {
#if defined(SK_OS_ANDROID)
	assets_block_until(&texture->header, asset_state_loaded);
	return texture->gpu_tex.ahb_handle;
#else
	(void)texture;
	return nullptr;
#endif
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

	if (tex->sh_pending)
		skr_buffer_destroy(&tex->sh_buffer);
	sk_free(tex->light_info);
	// Always destroy GPU resources when valid - skr_tex_destroy checks is_external
	// internally to decide whether to destroy the VkImage (external images like
	// OpenXR swapchains won't have their VkImage destroyed, but ImageViews and
	// Framebuffers that we created will still be cleaned up).
	if (skr_tex_is_valid(&tex->gpu_tex)) {
		skr_tex_destroy(&tex->gpu_tex);
	}
	if (tex->depth_buffer != nullptr) tex_release(tex->depth_buffer);
	tex_upload_t* pending = (tex_upload_t*)atomic_exchange_ptr(&tex->upload_pending, nullptr);
	if (pending != nullptr) tex_upload_free(pending);

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

// Reflection generation runs as an asset task on the destination, gated on
// the source's load through the task's depends_on, which also keeps the
// source pointer here alive.
struct tex_reflection_job_t {
	tex_t   source;
	int32_t max_resolution;
	bool    fresh;         // dest was created for this job, so it may error out
	bool    has_source_sh; // source_sh was snapshotted at queue time
	spherical_harmonics_t source_sh;
};

///////////////////////////////////////////

// Completes a pending SH readback into light_info. Non-blocking calls
// return false while the GPU is still working on it.
static bool32_t tex_sh_resolve(tex_t texture, bool32_t block) {
	if (!texture->sh_pending) return texture->light_info != nullptr;
	if      (block) skr_future_wait(&texture->sh_future);
	else if (!skr_future_check(&texture->sh_future)) return false;

	if (texture->light_info == nullptr)
		texture->light_info = sk_malloc_t(spherical_harmonics_t, 1);
	skr_buffer_get    (&texture->sh_buffer, texture->light_info->coefficients, sizeof(spherical_harmonics_t));
	skr_buffer_destroy(&texture->sh_buffer);
	texture->sh_buffer  = {};
	texture->sh_pending = false;
	return true;
}

///////////////////////////////////////////

// The mip the SH projection samples: the first at or below 32px per face,
// where one sample per texel covers the sphere; a sparse grid on anything
// larger would miss concentrated sources like an HDRI sun. Warns and returns
// negative when the chain has no such mip; lighting for mip-less skyboxes
// comes from tex_gen_cubemap_reflection instead.
static int32_t _tex_sh_mip(tex_t texture) {
	skr_vec3i_t base_size = { texture->width, texture->height, 1 };
	int32_t     mip_count = (int32_t)texture->gpu_tex.mip_levels;
	for (int32_t m = 0; m < mip_count; m++) {
		if (skr_tex_calc_mip_dimensions(base_size, m).x <= 32) return m;
	}
	log_warnf("Lighting data needs a mip chain on cubemaps over 32px. Generate a reflection from '%s' with tex_gen_cubemap_reflection and query that instead.", tex_get_id(texture));
	return -1;
}

///////////////////////////////////////////

// Records the SH projection into the open command scope. The caller attaches
// the scope's future; the readback then resolves lazily on query. Returns
// false when the texture has no mip the projection can sample.
static bool _tex_compute_sh_dispatch(tex_t texture) {
	int32_t mip_level = _tex_sh_mip(texture);
	if (mip_level < 0) return false;
	skr_vec3i_t base_size = { texture->width, texture->height, 1 };
	skr_vec3i_t mip_size  = skr_tex_calc_mip_dimensions(base_size, mip_level);

	// A dispatch already in flight must land before its buffer is replaced.
	tex_sh_resolve(texture, true);

	skr_buffer_create(nullptr, 1, sizeof(spherical_harmonics_t), skr_buffer_type_storage, (skr_use_)(skr_use_dynamic | skr_use_compute_write), &texture->sh_buffer);

	skr_compute_t      sh_compute = {};
	skr_compute_info_t sh_info    = {};
	skr_compute_create(&sk_default_shader_sh_compute->gpu_shader, sh_info, &sh_compute);

	uint32_t params[4] = { (uint32_t)mip_size.x, (uint32_t)mip_level, 0, 0 };
	skr_compute_set_params(&sh_compute, params, sizeof(params));
	skr_compute_set_tex   (&sh_compute, "source", &texture->gpu_tex);
	skr_compute_set_buffer(&sh_compute, "sh_output", &texture->sh_buffer);
	skr_compute_execute   (&sh_compute, 1, 1, 1);

	// Destruction is deferred by sk_renderer, safe while work is in flight.
	skr_compute_destroy(&sh_compute);
	return true;
}

///////////////////////////////////////////

// Dispatches the SH compute shader without waiting on the result. Works
// inside an already-open command scope: the dispatch is flushed rather than
// ended there, which submits the work but leaves the scope open. The readback
// resolves lazily on a later query, which keeps any cached value answering
// until the fresh one is actually available.
static void tex_compute_sh_async(tex_t texture) {
	bool was_active = skr_cmd_is_active();
	skr_cmd_begin();
	if (!_tex_compute_sh_dispatch(texture)) {
		skr_cmd_end();
		return;
	}
	texture->sh_future  = was_active ? skr_cmd_flush() : skr_cmd_end();
	texture->sh_pending = true;
	if (was_active) skr_cmd_end();
}

///////////////////////////////////////////

// Same dispatch, but blocking until the result is in light_info.
void tex_compute_sh(tex_t texture) {
	profiler_zone();

	tex_compute_sh_async(texture);
	tex_sh_resolve(texture, true);
}

///////////////////////////////////////////

// Uploads one mip-major block: every layer of mip 0, then every layer of mip 1.
// Volumes are one layer of `depth` slices. flat_data is the caller's to free,
// and null resizes without touching pixels.
static void _tex_set_color_flat(tex_t texture, tex_format_ format, int32_t width, int32_t height, int32_t depth, void* flat_data, int32_t array_count, int32_t mip_count, int32_t multisample) {
	profiler_zone();

	bool volume = (texture->type & tex_type_volume) != 0;
	if (volume) { array_count = 1; multisample = 1; }
	else        { depth = 1; }

	// Texture creation caps the sample count to the GPU's max, so compare
	// against the capped value, or we'd rebuild the texture on every call.
	int32_t max_msaa = skr_get_max_msaa_samples();
	if (multisample > max_msaa) multisample = max_msaa;
	if (multisample < 1)        multisample = 1;

	// Compared against the GPU texture, async uploads set meta before they run
	bool        dynamic        = texture->type & tex_type_dynamic;
	bool        valid          = skr_tex_is_valid(&texture->gpu_tex);
	skr_vec3i_t gpu_size       = valid ? skr_tex_get_size(&texture->gpu_tex) : skr_vec3i_t{};
	bool        different_size = gpu_size.x != width || gpu_size.y != height || (volume ? gpu_size.z != depth : (int32_t)texture->gpu_tex.layer_count != array_count);
	bool        different_msaa = valid && skr_tex_get_multisample(&texture->gpu_tex) != multisample;
	bool        different_fmt  = valid && skr_tex_get_format(&texture->gpu_tex) != (skr_tex_fmt_)format;
	if (!different_size && !different_msaa && !different_fmt && flat_data == nullptr)
		return;

	skr_tex_data_t tex_data = {};
	if (flat_data != nullptr) {
		tex_data.data        = flat_data;
		tex_data.mip_count   = mip_count;
		tex_data.layer_count = array_count;
		tex_data.base_mip    = 0;
		tex_data.base_layer  = 0;
		tex_data.row_pitch   = 0; // tightly packed
	}

	bool gen_mips = tex_gens_mips(texture->type, format) && mip_count <= 1;
	if (!valid || different_size || different_msaa || different_fmt || !dynamic) {
		skr_tex_flags_    flags   = tex_type_to_skr_flags(texture->type, format);
		skr_tex_sampler_t sampler = tex_get_skr_sampler(texture);

		// Cubemaps carry their layers in skr_tex_flags_cubemap
		bool is_array = !volume && array_count > 1 && !(texture->type & tex_type_cubemap);
		if (is_array) flags = (skr_tex_flags_)(flags | skr_tex_flags_array);
		skr_vec3i_t size = { width, height, volume ? depth : (is_array ? array_count : 1) };

		// A caller-supplied chain caps the mip count, so no level is left
		// uninitialized. 0 asks for a full chain, generated below.
		skr_tex_t new_tex;
		skr_err_  err = skr_tex_create((skr_tex_fmt_)format, flags, sampler, size, multisample, gen_mips ? 0 : mip_count, flat_data != nullptr ? &tex_data : nullptr, &new_tex);
		if (err != skr_err_success) {
			log_err("Failed to create texture");
			new_tex = {};
		} else if (gen_mips && flat_data != nullptr) {
			skr_tex_generate_mips(&new_tex, nullptr);
		}
		tex_land(texture, new_tex, width, height, depth, format);

		if (texture->depth_buffer != nullptr) {
			tex_set_size   (texture->depth_buffer, width, height, 1, texture->gpu_tex.layer_count, multisample);
			tex_set_zbuffer(texture, texture->depth_buffer);
		}
	} else if (flat_data != nullptr) {
		// Dynamic textures update in place
		skr_tex_set_data(&texture->gpu_tex, &tex_data);
		if (gen_mips) skr_tex_generate_mips(&texture->gpu_tex, nullptr);
	}

	// New content makes cached lighting stale, but the cache keeps answering
	// queries until a fresh projection replaces it.
	if (skr_tex_is_valid(&texture->gpu_tex) && (texture->type & tex_type_cubemap) && flat_data != nullptr)
		texture->sh_dirty = true;
}

///////////////////////////////////////////

// Sizing a rendertarget, depth buffer, or empty volume has nothing to copy or
// encode, so it happens right away. Pixels still waiting to upload are dropped.
static void tex_resize(tex_t texture, int32_t width, int32_t height, int32_t depth, int32_t array_count, int32_t multisample, tex_format_ format) {
	tex_upload_t* upload = sk_malloc_zero_t(tex_upload_t, 1);
	upload->write       = tex_write_issue(texture);
	upload->flags       = tex_data_blocking;
	upload->width       = width;
	upload->height      = height;
	upload->depth       = depth;
	upload->array_count = array_count;
	upload->mip_count   = 1;
	upload->multisample = multisample;
	upload->format      = format;
	tex_upload_submit(texture, upload, 10);
}

///////////////////////////////////////////

void tex_set_size(tex_t texture, int32_t width, int32_t height, int32_t depth, int32_t array_count, int32_t multisample) {
	if (depth > 1 && !(texture->type & tex_type_volume))
		log_warn("tex_set_size: only volume textures have depth, it will be ignored.");
	tex_resize(texture, width, height, depth, array_count, multisample, texture->data_format);
}

///////////////////////////////////////////

// Settles the GPU format up front, so Tex.Format is final on return.
static tex_upload_t* tex_upload_create(tex_t tex, int32_t width, int32_t height, int32_t depth, void** layers, int32_t array_count, int32_t mip_count, int32_t multisample, tex_format_ format, tex_data_ flags) {
	tex_upload_t* upload = sk_malloc_zero_t(tex_upload_t, 1);
	upload->write       = tex_write_issue(tex);
	upload->flags       = flags;
	upload->width       = width;
	upload->height      = height;
	upload->depth       = depth;
	upload->array_count = array_count;
	upload->mip_count   = mip_count;
	upload->multisample = multisample;
	upload->format      = format;
	if (tex->type & tex_type_volume) {
		size_t size = tex_format_size(format, width, height) * depth;
		upload->data = sk_malloc(size);
		memcpy(upload->data, layers[0], size);
	} else {
		upload->data = tex_flatten_layers(format, width, height, layers, array_count, mip_count);
	}
	if (multisample <= 1)
		upload->gpu_format = tex_pick_gpu_format(flags, tex->type, format, tex_format_has_alpha(format), width, height, array_count);

	tex_set_meta(tex, width, height, depth, upload->gpu_format != tex_format_none ? upload->gpu_format : format);
	return upload;
}

///////////////////////////////////////////

// Needs a GPU thread, and takes ownership of the upload.
static void tex_upload_run(tex_t tex, tex_upload_t* upload) {
	if (upload->gpu_format != tex_format_none) {
		tex_upload_compressed(tex, upload);
	} else {
		_tex_set_color_flat(tex, upload->format, upload->width, upload->height, upload->depth, upload->data, upload->array_count, upload->mip_count, upload->multisample);
	}
	tex_upload_free(upload);
}

///////////////////////////////////////////

// Between tex_write_begin and end, so uploads land in order.
static void tex_upload_drain(tex_t tex) {
	// Submit before returning, since the next drain may be on another thread
	// and destroy what this one uploaded.
	bool was_active = skr_cmd_is_active();
	skr_cmd_begin();
	tex_upload_t* upload;
	while ((upload = (tex_upload_t*)atomic_exchange_ptr(&tex->upload_pending, nullptr)) != nullptr) {
		if (tex_write_claim(tex, upload->write)) tex_upload_run (tex, upload);
		else                                     tex_upload_free(upload);
	}
	if (was_active) skr_cmd_flush();
	skr_cmd_end();
}

///////////////////////////////////////////

static asset_action_result_ tex_upload_action(asset_task_t*, asset_header_t* asset, void*) {
	tex_t tex = (tex_t)asset;
	// Another write is landing, so try again once it's done
	if (!tex_write_try_begin(tex)) return tex_write_busy();
	tex_upload_drain(tex);
	atomic_exchange_i32(&tex->write_running, 0);
	atomic_exchange_i32(&tex->upload_queued,  0);
	// An upload that arrived after the drain needs a task, and this can be it
	return atomic_load_ptr(&tex->upload_pending) != nullptr && atomic_cas_i32(&tex->upload_queued, 0, 1)
		? asset_action_continue
		: asset_action_done;
}

///////////////////////////////////////////

static void tex_upload_queue(tex_t tex, int32_t priority, size_t bytes) {
	if (!atomic_cas_i32(&tex->upload_queued, 0, 1)) return;

	static const asset_action_t actions[] = { { tex_upload_action } };
	asset_task_t task = {};
	task.asset        = &tex->header;
	task.actions      = (asset_action_t*)actions;
	task.action_count = 1;
	task.priority     = priority;
	task.sort         = asset_sort(priority, asset_complexity_bytes(bytes));
	assets_add_task(task);
}

///////////////////////////////////////////

static void tex_upload_submit(tex_t tex, tex_upload_t* upload, int32_t priority) {
	size_t        bytes    = tex_format_size(upload->format, upload->width, upload->height) * upload->array_count * upload->depth;
	bool          blocking = (upload->flags & tex_data_blocking) != 0;
	atomic_store_i32(&tex->upload_priority, priority);
	tex_upload_t* replaced = (tex_upload_t*)atomic_exchange_ptr(&tex->upload_pending, upload);
	if (replaced != nullptr) tex_upload_free(replaced);

	if (!blocking) {
		tex_upload_queue(tex, priority, bytes);
		return;
	}

	// Only ever waits on a write that's already landing, never on queued work
	tex_write_begin(tex);
	assets_execute_blocking([](void* data) { tex_upload_drain((tex_t)data); return (bool32_t)true; }, tex);
	tex_write_end(tex);
}

///////////////////////////////////////////

void tex_set_color_arr_mips(tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t mip_count, int32_t multisample, tex_data_ flags, tex_format_ data_format, int32_t priority) {
	profiler_zone();

	if (texture->type & tex_type_volume) {
		log_warn("Use tex_set_colors_3d for volume textures, not tex_set_colors / tex_set_color_arr.");
		return;
	}
	tex_format_ format = data_format != tex_format_none ? data_format : texture->data_format;
	if (array_data == nullptr || array_data[0] == nullptr) tex_resize        (texture, width, height, 1, array_count, multisample, format);
	else                                                   tex_upload_submit (texture, tex_upload_create(texture, width, height, 1, array_data, array_count, mip_count, multisample, format, flags), priority);
}

///////////////////////////////////////////

void tex_set_color_arr(tex_t texture, int32_t width, int32_t height, void** array_data, int32_t array_count, int32_t multisample, tex_data_ flags, tex_format_ data_format, int32_t priority) {
	tex_set_color_arr_mips(texture, width, height, array_data, array_count, 1, multisample, flags, data_format, priority);
}

///////////////////////////////////////////

void tex_set_mem(tex_t texture, void* data, size_t data_size, tex_data_ flags, int32_t priority) {
	profiler_zone();

	tex_load_t* load_data = tex_load_create(texture, flags, 1);
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
	if (!tex_load_image_info(load_data->file_data[0], load_data->file_sizes[0], load_data->flags, &load_data->type, &format, &load_data->color_width, &load_data->color_height, &load_data->color_array_count, &load_data->color_mip_count)) {
		log_warnf(tex_msg_invalid_fmt, load_data->file_names[0]);
		load_data->error = asset_state_error_unsupported;
		tex_load_on_failure(&texture->header, load_data);
		tex_load_free      (&texture->header, load_data);
		return;
	}
	load_data->color_format = format;
	tex_write_begin(texture);
	tex_load_meta  (texture, load_data, load_data->color_width, load_data->color_height, load_data->color_array_count);
	tex_write_end  (texture);

	static const asset_action_t actions[] = {
		{ tex_load_arr_parse, asset_affinity_heavy },
		{ tex_load_arr_upload },
	};
	tex_add_task(tex_make_loading_task(texture, load_data, actions, _countof(actions), priority, asset_complexity_bytes(data_size)), flags);
}

///////////////////////////////////////////

// Non-blocking query for a cubemap's SH lighting, resolving a finished
// readback if one is pending. False while the GPU is still working, or
// when the texture has no lighting data at all.
static bool32_t tex_cubemap_lighting_try(tex_t cubemap_texture, spherical_harmonics_t* out_sh) {
	if (!tex_sh_resolve(cubemap_texture, false)) return false;
	*out_sh = *cubemap_texture->light_info;
	return true;
}

///////////////////////////////////////////

spherical_harmonics_t tex_get_cubemap_lighting(tex_t cubemap_texture) {
	assets_block_until(&cubemap_texture->header, asset_state_loaded);

	// Fold in a finished background refresh; an unfinished one leaves the
	// cache answering with its previous value.
	tex_sh_resolve(cubemap_texture, false);

	if (cubemap_texture->light_info != nullptr) {
		// Stale cache: kick a refresh, and keep answering with the cached
		// value until that projection lands on a later query.
		if (cubemap_texture->sh_dirty && !cubemap_texture->sh_pending) {
			cubemap_texture->sh_dirty = false;
			assets_execute_blocking([](void* data) {
				tex_compute_sh_async((tex_t)data);
				return (bool32_t)true;
			}, cubemap_texture);
		}
		return *cubemap_texture->light_info;
	}

	// No cached value to answer with, so block for first results. Reflection
	// generation usually leaves a readback in flight here; other cubemaps
	// pay for a blocking GPU projection on this first query. The mip check
	// skips the blocking hop for textures the projection can't sample.
	cubemap_texture->sh_dirty = false;
	if (cubemap_texture->sh_pending) {
		tex_sh_resolve(cubemap_texture, true);
	} else if ((cubemap_texture->type & tex_type_cubemap) != 0 &&
	           skr_tex_is_valid(&cubemap_texture->gpu_tex) &&
	           _tex_sh_mip(cubemap_texture) >= 0) {
		assets_execute_blocking([](void* data) {
			tex_compute_sh((tex_t)data);
			return (bool32_t)true;
		}, cubemap_texture);
	}

	return cubemap_texture->light_info
		? *cubemap_texture->light_info
		: spherical_harmonics_t{};
}

///////////////////////////////////////////

void tex_set_cubemap_lighting(tex_t cubemap_texture, const spherical_harmonics_t& lighting_info) {
	if ((cubemap_texture->type & tex_type_cubemap) == 0) {
		log_warn("tex_set_cubemap_lighting needs a cubemap texture.");
		return;
	}
	// The value describes the texture's content, so wait for that content: an
	// upload landing after this call would silently refresh over the value.
	assets_block_until(&cubemap_texture->header, asset_state_loaded);

	// An in-flight readback must land before its result is replaced.
	tex_sh_resolve(cubemap_texture, true);
	if (cubemap_texture->light_info == nullptr)
		cubemap_texture->light_info = sk_malloc_t(spherical_harmonics_t, 1);
	*cubemap_texture->light_info = lighting_info;
	cubemap_texture->sh_dirty    = false;
}

///////////////////////////////////////////

void tex_lighting_dirty(tex_t texture) {
	if (texture->type & tex_type_cubemap)
		texture->sh_dirty = true;
}

///////////////////////////////////////////

void tex_set_colors(tex_t texture, int32_t width, int32_t height, void *data, tex_data_ flags, tex_format_ data_format, int32_t priority) {
	void *data_arr[1] = { data };
	tex_set_color_arr(texture, width, height, data_arr, 1, 1, flags, data_format, priority);
}

///////////////////////////////////////////

void tex_set_colors_3d(tex_t texture, int32_t width, int32_t height, int32_t depth, void *data, tex_data_ flags, tex_format_ data_format, int32_t priority) {
	profiler_zone();

	if (!(texture->type & tex_type_volume)) {
		log_warn("Use tex_set_colors / tex_set_color_arr for non-volume textures, not tex_set_colors_3d.");
		return;
	}
	tex_format_ format = data_format != tex_format_none ? data_format : texture->data_format;
	if (data == nullptr) tex_resize        (texture, width, height, depth, 1, 1, format);
	else                 tex_upload_submit (texture, tex_upload_create(texture, width, height, depth, &data, 1, 1, 1, format, flags), priority);
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

int32_t tex_get_depth(tex_t texture) {
	assets_block_until(&texture->header, asset_state_loaded_meta);
	return texture->depth;
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
	// Return the actual stored mip count rather than recomputing from
	// dimensions. The two can differ: tex_type_mips just signals intent, but
	// the caller may have supplied a partial chain (KTX2 truncated mip set,
	// etc.), in which case gpu_tex.mip_levels is the truth.
	return texture->gpu_tex.mip_levels > 0
		? (int32_t)texture->gpu_tex.mip_levels
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
	result           = hash_int_with(texture->depth,  result);
	uint64_t image   = (uint64_t)texture->gpu_tex.image;
	result           = hash_int_with((int32_t)(image & 0xFFFFFFFF), result);
	result           = hash_int_with((int32_t)(image >> 32),        result);
	// May want to consider texture format or some other items as well, but
	// this is plenty for now.
	return result;
}

///////////////////////////////////////////

void tex_set_meta(tex_t texture, int32_t width, int32_t height, int32_t depth, tex_format_ format) {
	texture->width  = width;
	texture->height = height;
	texture->depth  = depth;
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

	static int32_t warned_compressed = 0;
	if (tex_format_is_compressed(texture->format) && atomic_load_i32(&warned_compressed) == 0) {
		atomic_store_i32(&warned_compressed, 1);
		log_warnf("'%s' is block compressed, so its data comes back as compressed blocks. Load it with tex_data_uncompressed to read pixels.", tex_get_id(texture));
	}

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
	// Assign to the appropriate fallback based on texture type, or clear all
	// slots if nullptr
	if (loading_texture == nullptr) {
		tex_release(tex_loading_texture);
		tex_release(tex_loading_texture_cubemap);
		tex_release(tex_loading_texture_3d);
		tex_loading_texture         = nullptr;
		tex_loading_texture_cubemap = nullptr;
		tex_loading_texture_3d      = nullptr;
	} else if (loading_texture->type & tex_type_volume) {
		tex_release(tex_loading_texture_3d);
		tex_loading_texture_3d = loading_texture;
	} else if (loading_texture->type & tex_type_cubemap) {
		tex_release(tex_loading_texture_cubemap);
		tex_loading_texture_cubemap = loading_texture;
	} else {
		tex_release(tex_loading_texture);
		tex_loading_texture = loading_texture;
	}
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
	// Assign to the appropriate fallback based on texture type, or clear all
	// slots if nullptr
	if (error_texture == nullptr) {
		tex_release(tex_error_texture);
		tex_release(tex_error_texture_cubemap);
		tex_release(tex_error_texture_3d);
		tex_error_texture         = nullptr;
		tex_error_texture_cubemap = nullptr;
		tex_error_texture_3d      = nullptr;
	} else if (error_texture->type & tex_type_volume) {
		tex_release(tex_error_texture_3d);
		tex_error_texture_3d = error_texture;
	} else if (error_texture->type & tex_type_cubemap) {
		tex_release(tex_error_texture_cubemap);
		tex_error_texture_cubemap = error_texture;
	} else {
		tex_release(tex_error_texture);
		tex_error_texture = error_texture;
	}
}

///////////////////////////////////////////

void tex_set_compression_default(tex_data_ compression) {
	atomic_store_i32(&tex_compression_default, compression & tex_data_policy_mask);
}

///////////////////////////////////////////

tex_data_ tex_get_compression_default() {
	return (tex_data_)atomic_load_i32(&tex_compression_default);
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
	tex_set_colors(result, width, height, color_data, tex_data_immediate);

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
	tex_set_colors(result, width, height, color_data, tex_data_immediate);

	sk_free(color_data);
	if (gradient_linear == nullptr)
		gradient_destroy(grad);

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap(const gradient_t gradient_bot_to_top, vec3 gradient_dir, int32_t resolution) {
	tex_t result = tex_create(tex_type_image_nomips | tex_type_cubemap, tex_format_rg11b10);
	if (result == nullptr) {
		return nullptr;
	}
	gradient_dir = vec3_normalize(gradient_dir);

	// round size up to a power of two, log2f(0) would be -inf
	int32_t size = 1 << (int32_t)ceilf(log2f((float)maxi(1, resolution)));

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

	// Compute SH by sampling the gradient at uniformly distributed directions
	// on a sphere using a Fibonacci spiral.
	spherical_harmonics_t sh = {};
	const int32_t sample_count = 64;
	const float   golden_ratio = 1.618033988749895f;
	const float   golden_angle = (MATH_PI * 2.0f) / (golden_ratio * golden_ratio);

	for (int32_t i = 0; i < sample_count; i++) {
		float t           = (float)i / (float)sample_count;
		float inclination = acosf(1.0f - 2.0f * t);
		float azimuth     = golden_angle * (float)i;

		vec3 dir = {
			sinf(inclination) * cosf(azimuth),
			cosf(inclination),
			sinf(inclination) * sinf(azimuth) };

		float    pct = (vec3_dot(dir, gradient_dir) + 1.0f) * 0.5f;
		color128 c   = gradient_get(gradient_bot_to_top, pct);
		sh_add(sh, dir, { c.r, c.g, c.b });
	}

	for (int32_t i = 0; i < 9; i++)
		sh.coefficients[i] /= (float)sample_count;

	sh_windowing(sh, 0.01f);

	// The gradient's exact SH is known here, saving the first lighting query
	// a GPU projection of the uploaded faces.
	tex_set_color_arr       (result, size, size, (void**)data, 6, 1, tex_data_immediate);
	tex_set_cubemap_lighting(result, sh);

	for (int32_t i = 0; i < 6; i++) {
		sk_free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

tex_t tex_gen_cubemap_sh(const spherical_harmonics_t& lookup, int32_t face_size, float light_spot_size_pct, float light_spot_intensity) {
	tex_t result = tex_create(tex_type_image_nomips | tex_type_cubemap, tex_format_rg11b10);
	if (result == nullptr) {
		return nullptr;
	}

	// The image is drawn in the radiance domain, which rings much harder than
	// the irradiance domain the SH was deringed for, so window a copy for the
	// image alone. The texture's lighting data stays the exact input.
	spherical_harmonics_t radiance = lookup;
	sh_window_fit_radiance(radiance);

	// Calculate information used to create the light spot, which sits toward
	// the light source.
	vec3     light_to  = sh_dominant_dir_to(lookup);
	color128 light_col = sh_lookup(lookup, light_to) * light_spot_intensity;
	vec3     light_pt  = { 100000,100000,100000 };
	for (int32_t i = 0; i < 6; i++) {
		vec3 p1 = math_cubemap_corner(i * 4);
		vec3 p2 = math_cubemap_corner(i * 4 + 1);
		vec3 p3 = math_cubemap_corner(i * 4 + 2);
		plane_t plane = plane_from_points(p1, p2, p3);
		vec3    pt;
		if (plane_ray_intersect(plane, { vec3_zero, light_to }, &pt) && vec3_magnitude_sq(pt) < vec3_magnitude_sq(light_pt))
			light_pt = pt;
	}

	// round size up to a power of two, log2f(0) would be -inf
	int32_t size = 1 << (int32_t)ceilf(log2f((float)maxi(1, face_size)));

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
					: sh_lookup_radiance(radiance, vec3_normalize(pt));

				data[i][x + ysize] = fhf_f32_to_r11g11ba10f(&color.r);
			}
		}
	}

	// The image came from this exact SH, saving the first lighting query a
	// GPU projection of the uploaded faces.
	tex_set_color_arr       (result, size, size, (void**)data, 6, 1, tex_data_immediate);
	tex_set_cubemap_lighting(result, lookup);

	for (int32_t i = 0; i < 6; i++) {
		sk_free(data[i]);
	}

	return result;
}

///////////////////////////////////////////

// Task action, runs on an asset thread once the source is loaded: sizes a
// fresh destination, fills its base level, convolves the GGX chain, and
// copies or dispatches the SH.
static asset_action_result_ tex_reflection_generate(asset_task_t*, asset_header_t* asset, void* data) {
	tex_reflection_job_t* job    = (tex_reflection_job_t*)data;
	tex_t                 source = job->source;
	tex_t                 dest   = (tex_t)asset;

	// The dependency gate only waits on load state, so validate the rest.
	if ((source->type & tex_type_cubemap) == 0 || !skr_tex_is_valid(&source->gpu_tex)) {
		log_warn("tex_gen_cubemap_reflection source wasn't a valid cubemap.");
		return asset_action_fail;
	}

	// Shape follows the source, now that its metadata is available.
	// Halving to the cap keeps base texels on whole source texel blocks.
	if (job->fresh) {
		int32_t size = source->width;
		while (size > job->max_resolution && size > 1) size /= 2;

		tex_format_ fmt = tex_format_is_mippable(source->format)
			? source->format
			: tex_format_rg11b10;
		tex_set_meta(dest, size, size, 1, fmt);
	}

	// Reflections need a full mip chain for the roughness ramp, and render
	// access for the convolution. (Re)create if the texture doesn't fit.
	if (!skr_tex_is_valid(&dest->gpu_tex) || dest->gpu_tex.mip_levels <= 1) {
		dest->type = (tex_type_)(dest->type | tex_type_mips | tex_type_rendertarget);

		// Storage usage lets the convolution run as compute dispatches instead
		// of one render pass per mip. Formats that can't do storage still work
		// through the shader's pixel stage fallback.
		skr_tex_flags_ flags = tex_type_to_skr_flags(dest->type, dest->format);
		if (skr_tex_fmt_is_supported((skr_tex_fmt_)dest->format, (skr_tex_flags_)(flags | skr_tex_flags_compute), 1))
			flags = (skr_tex_flags_)(flags | skr_tex_flags_compute);

		skr_tex_t new_tex;
		skr_err_ err = skr_tex_create(
			(skr_tex_fmt_)dest->format,
			flags,
			tex_get_skr_sampler(dest),
			{ dest->width, dest->height, 1 },
			1,  // multisample
			0,  // mip_count = 0 for a full auto-calculated chain
			nullptr,
			&new_tex);
		if (err != skr_err_success) {
			log_err("tex_gen_cubemap_reflection failed to create the reflection texture.");
			return asset_action_fail;
		}

		tex_swap_gpu    (dest, new_tex);
		tex_update_label(dest);
	}

	skr_cmd_begin();

	// Fill the destination's base level from the source, box filtered in
	// one pass. Copying the default material keeps this task's parameters
	// private, while its pipeline stays compiled with the default.
	{
		material_t downsample_mat = material_copy(sk_default_material_cubemap_downsample);
		material_set_texture(downsample_mat, "source", source);
		material_set_float  (downsample_mat, "size_ratio", (float)source->width / (float)dest->width);

		skr_recti_t bounds = { 0, 0, dest->width, dest->height };
		skr_renderer_blit(&downsample_mat->gpu_mat, &dest->gpu_tex, bounds);

		material_release(downsample_mat);
	}

	// Convolve the mip chain: each mip holds GGX-prefiltered radiance
	// for the roughness the PBR shader will sample it at.
	skr_tex_generate_mips(&dest->gpu_tex, &sk_default_shader_cubemap_ggx->gpu_shader);

	// A source SH snapshotted at queue time describes the same environment,
	// so prefer copying it over projecting the convolved chain. Either way it
	// resolves before the state flip: a loaded reflection carries lighting.
	if (job->has_source_sh) {
		// An older dispatch must land before its result is replaced.
		tex_sh_resolve(dest, true);
		if (dest->light_info == nullptr)
			dest->light_info = sk_malloc_t(spherical_harmonics_t, 1);
		*dest->light_info = job->source_sh;
		skr_cmd_end();
	} else {
		// The freshly convolved chain always has a mip small enough for the
		// projection to sample, so this won't warn and skip.
		tex_compute_sh(dest);
		skr_cmd_end();
	}
	// Both paths above leave lighting that matches this fresh content.
	dest->sh_dirty = false;

	tex_set_fallback(dest, nullptr);
	dest->header.state = asset_state_loaded;
	return asset_action_done;
}

// Failure path: a failed source load skips the action entirely, or the
// action itself bailed. Only never-generated destinations error out; an
// existing reflection keeps its previous content and loaded state.
static void tex_reflection_failed(asset_header_t* asset, void*) {
	tex_t dest = (tex_t)asset;
	if (dest->header.state < asset_state_loaded) {
		tex_set_fallback(dest, _tex_get_error_fallback(dest));
		dest->header.state = asset_state_error;
	}
}

static void tex_reflection_free(asset_header_t*, void* data) {
	sk_free(data);
}

///////////////////////////////////////////

tex_t tex_gen_cubemap_reflection(tex_t source_cubemap, tex_t into, int32_t max_resolution) {
	profiler_zone();

	if (source_cubemap == nullptr) return nullptr;
	if (into == source_cubemap) {
		log_warn("tex_gen_cubemap_reflection can't convolve a cubemap over itself, pass a separate destination.");
		return nullptr;
	}
	if (into != nullptr && (into->type & tex_type_cubemap) == 0) {
		log_warn("tex_gen_cubemap_reflection destination must be a cubemap.");
		return nullptr;
	}
	if (into != nullptr && !tex_format_is_mippable(into->format)) {
		log_warn("tex_gen_cubemap_reflection destination format can't generate a mip chain.");
		return nullptr;
	}
	if (max_resolution <= 0) max_resolution = 64;

	tex_t dest = into;
	if (dest == nullptr) {
		// Format and size follow the source's metadata, which may not have
		// loaded yet; the task fills them in when it runs.
		dest = tex_create((tex_type_)(tex_type_cubemap | tex_type_mips | tex_type_rendertarget), tex_format_none);
		dest->header.state = asset_state_loading;
	}

	tex_reflection_job_t* job = sk_malloc_zero_t(tex_reflection_job_t, 1);
	job->source         = source_cubemap;
	job->max_resolution = max_resolution;
	job->fresh          = into == nullptr;
	// Snapshot the source's lighting on this thread; the task runs on an
	// asset thread, where reading the live field would race a concurrent set.
	job->has_source_sh  = tex_cubemap_lighting_try(source_cubemap, &job->source_sh);

	static const asset_action_t actions[] = { { tex_reflection_generate } };
	asset_task_t task  = {};
	task.asset         = &dest->header;
	task.load_data     = job;
	task.free_data     = tex_reflection_free;
	task.on_failure    = tex_reflection_failed;
	task.actions       = (asset_action_t*)actions;
	task.action_count  = _countof(actions);
	task.priority      = asset_priority_default;
	task.sort          = asset_sort(asset_priority_default, 0);
	task.depends_on    = &source_cubemap->header;
	task.depends_state = asset_state_loaded;

	// A source uploaded on this thread can still sit in the thread's open
	// command batch, and the task may submit its convolution from another
	// thread before that batch lands. Submit now so queue order is right.
	if (skr_cmd_is_active()) skr_cmd_flush();

	assets_add_task(task);

	if (into != nullptr) tex_addref(into);
	return dest;
}

///////////////////////////////////////////

uint8_t* unzip_malloc(const uint8_t* buffer, int32_t len, int32_t* out_len) {
	return (uint8_t*)stbi_zlib_decode_malloc((const char*)buffer, len, out_len);
}

} // namespace sk
