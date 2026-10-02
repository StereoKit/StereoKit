#include "texture_compression.h"
#include "texture_.h"
#include "../sk_memory.h"
#include "../libraries/profiler.h"
#include "../libraries/qoi.h"

#define STBI_NO_STDIO
#include "../libraries/stb_image.h"

#include <sk_renderer.h>
#include <sk_ktx2.h>
#include <sk_texenc.h>

#include <inttypes.h>
#include <string.h>

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
	ktx2_context_t      ktx2; // ETC1S tables, ~43KB, read-only after init so asset threads can share it
	tex_compress_caps_t caps; // read-only after init
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

void texture_compression_gpu_init() {
	sk_texenc_init();
#if defined(SKR_WEBGPU)
	// Waits on WebGPU integration, where the encoders' WGSL still needs vetting.
	local.caps = {};
#else
	local.caps.astc     = sk_texenc_available(sk_texenc_fmt_astc4x4, sk_texenc_flags_none) && sk_texenc_available(sk_texenc_fmt_astc6x6, sk_texenc_flags_none);
	local.caps.astc_hdr = sk_texenc_available(sk_texenc_fmt_astc8x8hdr, sk_texenc_flags_none);
	local.caps.bc       = sk_texenc_available(sk_texenc_fmt_bc1, sk_texenc_flags_none) && sk_texenc_available(sk_texenc_fmt_bc7, sk_texenc_flags_none) && sk_texenc_available(sk_texenc_fmt_bc6h, sk_texenc_flags_none);
#endif
}

///////////////////////////////////////////

void texture_compression_gpu_shutdown() {
	sk_texenc_shutdown();
	local.caps = {};
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

///////////////////////////////////////////

bool ktx2_sniff(const void* data, size_t data_size) {
	static const uint8_t magic[12] = { 0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, '\r', '\n', 0x1A, '\n' };
	return data_size >= sizeof(magic) && memcmp(data, magic, sizeof(magic)) == 0;
}

///////////////////////////////////////////

struct ktx2_slice_t {
	ktx2_reader_t reader;
	ktx2_plan_t   plan;      // points at this struct's own reader
	void*         scratch;
	uint8_t*      out;       // the caller's output buffer, not owned here
	tex_format_   format;
	skr_vec3i_t   base_size;
	int32_t       images;
	int32_t       level_curr;
	size_t        level_offset; // where level_curr starts in `out`
};

ktx2_slice_t* ktx2_decode_begin(void* data, size_t data_size, tex_type_* ref_image_type, tex_format_* out_format, int32_t* out_width, int32_t* out_height, int32_t* out_array_count, int32_t* out_mip_count, void** out_data) {
	ktx2_slice_t* slice = sk_malloc_zero_t(ktx2_slice_t, 1);
	if (!ktx2_prepare(data, data_size, &slice->reader, &slice->plan)) {
		sk_free(slice);
		return nullptr;
	}

	ktx2_info_t info = ktx2_get_info(&slice->reader);
	slice->images    = ktx2_image_count(&info);
	slice->format    = texture_compression_format(slice->plan.format);
	slice->base_size = { info.width, info.height, 1 };

	// See ktx2_decode: a disagreement means one side is wrong about the format
	uint64_t image_size = 0;
	for (int32_t mip = 0; mip < slice->plan.mip_count; mip++)
		image_size += skr_tex_calc_mip_size((skr_tex_fmt_)slice->format, slice->base_size, mip);
	if (image_size * slice->images != slice->plan.data_bytes) {
		log_errf("KTX2 size disagreement for %s: sk_ktx2 says %" PRIu64 " bytes, sk_renderer says %" PRIu64, ktx2_fmt_str(slice->plan.format), (uint64_t)slice->plan.data_bytes, image_size * slice->images);
		sk_free(slice);
		return nullptr;
	}

	slice->out     = (uint8_t*)sk_malloc(slice->plan.data_bytes);
	slice->scratch = slice->plan.scratch_bytes > 0 ? sk_malloc(slice->plan.scratch_bytes) : nullptr;

	*out_data        = slice->out;
	*out_format      = slice->format;
	*out_width       = info.width;
	*out_height      = info.height;
	*out_mip_count   = slice->plan.mip_count;
	*out_array_count = slice->images;
	if (info.face_count > 1) *ref_image_type |= tex_type_cubemap;
	return slice;
}

bool ktx2_decode_step(ktx2_slice_t* slice, int32_t* out_level, bool* out_done) {
	int32_t level = slice->level_curr;
	size_t  bytes = (size_t)slice->images * skr_tex_calc_mip_size((skr_tex_fmt_)slice->format, slice->base_size, level);

	ktx2_result_ result = ktx2_transcode_level(&slice->plan, level, slice->out + slice->level_offset, bytes, slice->scratch);
	if (result != ktx2_result_success) {
		log_warnf("KTX2 transcode failed: %s", ktx2_result_str(result));
		return false;
	}

	slice->level_curr   += 1;
	slice->level_offset += bytes;
	*out_level = level;
	*out_done  = slice->level_curr >= slice->plan.mip_count;
	return true;
}

void ktx2_decode_end(ktx2_slice_t* slice) {
	if (slice == nullptr) return;
	sk_free(slice->scratch);
	sk_free(slice);
}

///////////////////////////////////////////

static bool tex_format_is_ldr_source(tex_format_ format) {
	return format == tex_format_rgba32_srgb || format == tex_format_rgba32_linear;
}

///////////////////////////////////////////

tex_format_ tex_compress_pick(tex_hint_ hints, tex_hint_ default_policy, tex_type_ type, tex_format_ src_format, bool32_t src_alpha, int32_t width, int32_t height, int32_t array_count, tex_compress_caps_t caps) {
	tex_hint_ policy = hints & tex_hint_policy_mask;
	if (policy == tex_hint_none) policy = default_policy & tex_hint_policy_mask;
	if (policy == tex_hint_none || (policy & tex_hint_uncompressed)) return tex_format_none;
	bool quality = (policy & tex_hint_quality) != 0;

	// GPU written or CPU updated textures can't be block compressed.
	const tex_type_ refuse = tex_type_rendertarget | tex_type_depth | tex_type_depthtarget | tex_type_dynamic | tex_type_compute | tex_type_volume;
	if (type & refuse)                                            return tex_format_none;
	if (array_count != 1 && !(array_count == 6 && (type & tex_type_cubemap))) return tex_format_none;
	// Formats need at least one whole block, and 8x8 is the largest.
	if (width < 8 || height < 8)                                  return tex_format_none;

	if (src_format == tex_format_rg11b10) {
		if (!quality && caps.astc_hdr) return tex_format_astc8x8_rgba_hdr;
		if (caps.bc)                   return tex_format_bc6h_rgbuf;
		return tex_format_none;
	}
	if (!tex_format_is_ldr_source(src_format)) return tex_format_none;
	// Tiny LDR images are usually palettes or pixel art, where a block blends
	// unrelated texels, and they'd save at most 16KB anyway.
	if ((int64_t)width * height <= 64 * 64) return tex_format_none;

	// Normals are reserved for a two channel format, until then they're data.
	bool srgb = (hints & tex_hint_srgb) && !(hints & tex_hint_normal);
	if (caps.astc) {
		if (quality) return srgb ? tex_format_astc4x4_rgba_srgb : tex_format_astc4x4_rgba;
		else         return srgb ? tex_format_astc6x6_rgba_srgb : tex_format_astc6x6_rgba;
	}
	if (caps.bc) {
		bool alpha = src_alpha && !(hints & tex_hint_opaque);
		if (quality || (alpha && !(hints & tex_hint_cutout))) return srgb ? tex_format_bc7_rgba_srgb : tex_format_bc7_rgba;
		if (alpha) return srgb ? tex_format_bc1_rgba_srgb : tex_format_bc1_rgba;
		else       return srgb ? tex_format_bc1_rgb_srgb  : tex_format_bc1_rgb;
	}
	return tex_format_none;
}

///////////////////////////////////////////

// stb reports 3 channels for a non-paletted PNG whose alpha comes from a tRNS
// color key, so the chunks ahead of the pixel data are checked directly.
static bool tex_png_has_trns(const uint8_t* data, size_t size) {
	size_t at = 8;
	while (at + 8 <= size) {
		uint32_t length = ((uint32_t)data[at] << 24) | ((uint32_t)data[at+1] << 16) | ((uint32_t)data[at+2] << 8) | data[at+3];
		const uint8_t* tag = &data[at + 4];
		if (memcmp(tag, "tRNS", 4) == 0) return true;
		if (memcmp(tag, "IDAT", 4) == 0) return false;
		at += 12 + (size_t)length;
	}
	return false;
}

///////////////////////////////////////////

bool32_t tex_image_has_alpha(const void* data, size_t data_size) {
	const uint8_t* bytes = (const uint8_t*)data;
	if (data_size > 3 && bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF) return false;

	qoi_desc q_desc = {};
	if (qoi_info(data, (int)data_size, &q_desc)) return q_desc.channels == 4;

	int32_t w, h, comp;
	if (stbi_info_from_memory(bytes, (int)data_size, &w, &h, &comp) != 1) return true;
	if (comp == 2 || comp == 4) return true;

	bool png = data_size > 8 && bytes[0] == 0x89 && bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G';
	return png && tex_png_has_trns(bytes, data_size);
}

///////////////////////////////////////////

tex_compress_caps_t tex_compress_caps() {
	return local.caps;
}

///////////////////////////////////////////

static bool tex_compress_encoder(tex_format_ format, sk_texenc_fmt_* out_fmt, sk_texenc_flags_* out_flags) {
	*out_flags = sk_texenc_flags_none;
	switch (format) {
	case tex_format_bc1_rgb_srgb:      *out_fmt = sk_texenc_fmt_bc1;        return true;
	case tex_format_bc1_rgba_srgb:     *out_fmt = sk_texenc_fmt_bc1_alpha;  return true;
	case tex_format_bc7_rgba_srgb:     *out_fmt = sk_texenc_fmt_bc7;        return true;
	case tex_format_astc4x4_rgba_srgb: *out_fmt = sk_texenc_fmt_astc4x4;    return true;
	case tex_format_astc6x6_rgba_srgb: *out_fmt = sk_texenc_fmt_astc6x6;    return true;
	case tex_format_bc6h_rgbuf:        *out_fmt = sk_texenc_fmt_bc6h;       return true;
	case tex_format_astc8x8_rgba_hdr:  *out_fmt = sk_texenc_fmt_astc8x8hdr; return true;
	default: break;
	}
	*out_flags = sk_texenc_flags_linear;
	switch (format) {
	case tex_format_bc1_rgb:      *out_fmt = sk_texenc_fmt_bc1;       return true;
	case tex_format_bc1_rgba:     *out_fmt = sk_texenc_fmt_bc1_alpha; return true;
	case tex_format_bc7_rgba:     *out_fmt = sk_texenc_fmt_bc7;       return true;
	case tex_format_astc4x4_rgba: *out_fmt = sk_texenc_fmt_astc4x4;   return true;
	case tex_format_astc6x6_rgba: *out_fmt = sk_texenc_fmt_astc6x6;   return true;
	default: return false;
	}
}

///////////////////////////////////////////

skr_tex_t tex_compress_gpu(skr_tex_t* source, tex_format_ format, skr_tex_sampler_t sampler) {
	profiler_zone();

	sk_texenc_fmt_   enc_fmt;
	sk_texenc_flags_ enc_flags;
	if (!tex_compress_encoder(format, &enc_fmt, &enc_flags))
		return {};

	return (source->flags & skr_tex_flags_cubemap)
		? sk_texenc_cube(source, enc_fmt, enc_flags, sampler)
		: sk_texenc_2d  (source, enc_fmt, enc_flags, sampler);
}

}
