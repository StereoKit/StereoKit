#include "texture_compression.h"
#include "../sk_memory.h"

#include <sk_renderer.h>
#include <sk_ktx2.h>

#include <inttypes.h>

// The subset we call from libraries/zstddeclib.c. Its copy of zstd.h is baked
// into the amalgamation, so there's no header to include.
extern "C" {
size_t   ZSTD_decompress(void* dst, size_t dst_capacity, const void* src, size_t src_size);
unsigned ZSTD_isError   (size_t code);
}

namespace sk {

// ETC1S is entropy coded, so a few hundred bytes can ask for gigabytes. sk_ktx2
// reports that size rather than refusing it, and sk_malloc aborts on failure.
#define TEX_KTX2_MAX_BYTES (1024 * 1024 * 1024)

struct texture_compression_state_t {
	ktx2_context_t ktx2; // ETC1S tables, ~43KB, read-only after init so asset threads can share it
};
static texture_compression_state_t local = {};

///////////////////////////////////////////

static size_t ktx2_zstd_inflate(void*, const void* src, size_t src_bytes, void* out_dst, size_t dst_bytes) {
	size_t result = ZSTD_decompress(out_dst, dst_bytes, src, src_bytes);
	return ZSTD_isError(result) ? 0 : result;
}

///////////////////////////////////////////

void texture_compression_init() {
	local.ktx2.zstd = ktx2_zstd_inflate;
	ktx2_context_prepare(&local.ktx2);
}

///////////////////////////////////////////

// sk_ktx2 asks for capability by family, which is how the hardware reports it:
// BC1-BC7 arrive as a single feature bit, and ASTC LDR as another.
static ktx2_caps_ texture_compression_caps() {
	ktx2_caps_ caps = ktx2_caps_none;
	if (skr_tex_fmt_is_supported(skr_tex_fmt_bc7_rgba,     (skr_tex_flags_)0, 1)) caps = (ktx2_caps_)(caps | ktx2_caps_bc);
	if (skr_tex_fmt_is_supported(skr_tex_fmt_etc2_rgba,    (skr_tex_flags_)0, 1)) caps = (ktx2_caps_)(caps | ktx2_caps_etc2);
	if (skr_tex_fmt_is_supported(skr_tex_fmt_astc4x4_rgba, (skr_tex_flags_)0, 1)) caps = (ktx2_caps_)(caps | ktx2_caps_astc_ldr);
	return caps;
}

///////////////////////////////////////////

static tex_format_ texture_compression_format(ktx2_fmt_ format) {
	switch (format) {
	case ktx2_fmt_etc1_rgb:          return tex_format_etc1_rgb;
	case ktx2_fmt_etc1_rgb_srgb:     return tex_format_etc1_rgb_srgb;
	case ktx2_fmt_etc2_rgba:         return tex_format_etc2_rgba;
	case ktx2_fmt_etc2_rgba_srgb:    return tex_format_etc2_rgba_srgb;
	case ktx2_fmt_eac_r11:           return tex_format_etc2_r11;
	case ktx2_fmt_eac_rg11:          return tex_format_etc2_rg11;
	case ktx2_fmt_bc1_rgb:           return tex_format_bc1_rgb;
	case ktx2_fmt_bc1_rgb_srgb:      return tex_format_bc1_rgb_srgb;
	case ktx2_fmt_bc3_rgba:          return tex_format_bc3_rgba;
	case ktx2_fmt_bc3_rgba_srgb:     return tex_format_bc3_rgba_srgb;
	case ktx2_fmt_bc4_r:             return tex_format_bc4_r;
	case ktx2_fmt_bc5_rg:            return tex_format_bc5_rg;
	case ktx2_fmt_bc7_rgba:          return tex_format_bc7_rgba;
	case ktx2_fmt_bc7_rgba_srgb:     return tex_format_bc7_rgba_srgb;
	case ktx2_fmt_astc4x4_rgba:      return tex_format_astc4x4_rgba;
	case ktx2_fmt_astc4x4_rgba_srgb: return tex_format_astc4x4_rgba_srgb;
	case ktx2_fmt_r8:                return tex_format_r8;
	case ktx2_fmt_rg8:               return tex_format_r8g8;
	case ktx2_fmt_rgba32:            return tex_format_rgba32_linear;
	case ktx2_fmt_rgba32_srgb:       return tex_format_rgba32_srgb;
	default:
		log_errf("Unmapped KTX2 output format: %s", ktx2_fmt_str(format));
		return tex_format_none;
	}
}

///////////////////////////////////////////

// One open+plan drives both info and decode. They have to agree on the format,
// and the plan is the only place that choice gets made.
static bool ktx2_prepare(void* data, size_t data_size, ktx2_reader_t* out_reader, ktx2_plan_t* out_plan) {
	ktx2_result_ result = ktx2_open(data, data_size, out_reader);
	if (result == ktx2_result_not_ktx2) return false; // Just not our format, the caller is still guessing
	if (result != ktx2_result_success) {
		log_warnf("KTX2 file rejected: %s", ktx2_result_str(result));
		return false;
	}

	result = ktx2_plan(out_reader, &local.ktx2, texture_compression_caps(), out_plan);
	if (result != ktx2_result_success) {
		log_warnf("KTX2 file unusable: %s", ktx2_result_str(result));
		return false;
	}
	if (out_plan->data_bytes > TEX_KTX2_MAX_BYTES) {
		log_warnf("KTX2 file wants %" PRIu64 " bytes, over the %d byte limit", (uint64_t)out_plan->data_bytes, TEX_KTX2_MAX_BYTES);
		return false;
	}
	// Checked here so a format we can't name fails the same way in both entry
	// points, rather than reaching the GPU as tex_format_none.
	if (texture_compression_format(out_plan->format) == tex_format_none)
		return false;
	// tex_type_ has no flag for a layered cubemap, and the upload path would
	// build a 6 layer one and then hand it layers*6 images.
	ktx2_info_t info = ktx2_get_info(out_reader);
	if (info.face_count > 1 && info.layer_count > 1) {
		log_warnf("KTX2 cubemap arrays aren't supported, this file has %d layers", info.layer_count);
		return false;
	}
	return true;
}

///////////////////////////////////////////

// Layers and cube faces are both just images to us, and the file orders them
// layer-then-face. ktx2_prepare rejects anything carrying both.
static int32_t ktx2_image_count(const ktx2_info_t* info) {
	return info->layer_count * info->face_count;
}

///////////////////////////////////////////

bool ktx2_info(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count) {
	ktx2_reader_t reader = {};
	ktx2_plan_t   plan   = {};
	if (!ktx2_prepare(data, data_size, &reader, &plan)) return false;

	ktx2_info_t info = ktx2_get_info(&reader);
	*out_format      = texture_compression_format(plan.format);
	*out_width       = info.width;
	*out_height      = info.height;
	*out_mip_count   = plan.mip_count;
	*out_array_count = ktx2_image_count(&info);
	// tex_type_ is a bit field, and the caller may already have set mips,
	// dynamic or rendertarget on it. Assigning here would drop those.
	if (info.face_count > 1) *ref_image_type |= tex_type_cubemap;
	return true;
}

///////////////////////////////////////////

bool ktx2_decode(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data) {
	ktx2_reader_t reader = {};
	ktx2_plan_t   plan   = {};
	if (!ktx2_prepare(data, data_size, &reader, &plan)) return false;

	ktx2_info_t info       = ktx2_get_info(&reader);
	int32_t     images     = ktx2_image_count(&info);
	tex_format_ format     = texture_compression_format(plan.format);
	skr_vec3i_t base_size  = { info.width, info.height, 1 };
	uint64_t    image_size = 0;
	for (int32_t mip = 0; mip < plan.mip_count; mip++)
		image_size += skr_tex_calc_mip_size((skr_tex_fmt_)format, base_size, mip);

	// sk_ktx2 sizes the mip chain independently of sk_renderer, so a mismatch
	// here means one of the two is wrong about the format, not about this file.
	if (image_size * images != plan.data_bytes) {
		log_errf("KTX2 size disagreement for %s: sk_ktx2 says %" PRIu64 " bytes, sk_renderer says %" PRIu64, ktx2_fmt_str(plan.format), (uint64_t)plan.data_bytes, image_size * images);
		return false;
	}

	// Scratch is ours rather than the library's so both allocations go through
	// sk_malloc and stay accountable.
	void*        all     = sk_malloc(plan.data_bytes);
	void*        scratch = plan.scratch_bytes > 0 ? sk_malloc(plan.scratch_bytes) : nullptr;
	ktx2_result_ result  = ktx2_transcode(&plan, all, plan.data_bytes, scratch);
	sk_free(scratch);
	if (result != ktx2_result_success) {
		log_warnf("KTX2 transcode failed: %s", ktx2_result_str(result));
		sk_free(all);
		return false;
	}

	// Already mip-major with images within a level, which is the layout both the
	// GPU and tex_load_image_data want, so it hands over untouched.
	*out_data = all;

	*out_format      = format;
	*out_width       = info.width;
	*out_height      = info.height;
	*out_mip_count   = plan.mip_count;
	*out_array_count = images;
	// tex_type_ is a bit field, and the caller may already have set mips,
	// dynamic or rendertarget on it. Assigning here would drop those.
	if (info.face_count > 1) *ref_image_type |= tex_type_cubemap;
	return true;
}

}
