// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "texcompress_tests.h"

#include <stereokit.h>

// Internal compression helpers, reached through StereoKitC's include root.
#include <asset_types/texture_compression.h>
#include <asset_types/texture.h>
#include <libraries/atomic_util.h>

// The test builds its own image files, and StereoKitC keeps its copy of the
// encoder private.
#define QOI_IMPLEMENTATION
#include <libraries/qoi.h>

#include <stdlib.h>
#include <math.h>
#include <string.h>

using namespace sk;

///////////////////////////////////////////

static int tct_failures = 0;

#define TCT_CHECK(condition, description) do { \
	if (condition) { log_infof("[texcompress_test] pass: %s", description); } \
	else           { log_errf ("[texcompress_test] FAIL: %s", description); tct_failures += 1; } \
	} while (0)

static const tex_compress_caps_t tct_astc = { true,  true,  false };
static const tex_compress_caps_t tct_bc   = { false, false, true  };
static const tex_compress_caps_t tct_none = { false, false, false };
static const tex_compress_caps_t tct_both = { true,  true,  true  };

// A decoded image's format already follows the srgb flag, so `none` mirrors that.
static tex_format_ tct_pick(tex_data_ flags, tex_compress_caps_t caps, bool32_t alpha = false, tex_format_ src = tex_format_none, tex_type_ type = tex_type_image, int32_t size = 256, int32_t arrays = 1) {
	if (src == tex_format_none) src = (flags & tex_data_srgb) ? tex_format_rgba32_srgb : tex_format_rgba32_linear;
	return tex_compress_pick(flags, tex_data_compress_quality, type, src, alpha, size, size, arrays, caps);
}

///////////////////////////////////////////

static void tct_test_policy() {
	TCT_CHECK(tct_pick(tex_data_srgb,                                  tct_astc) == tex_format_astc4x4_rgba_srgb, "no policy uses the quality default");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_compress_small,                 tct_astc) == tex_format_astc6x6_rgba_srgb, "small picks ASTC 6x6");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_compress_small | tex_data_compress_quality, tct_astc) == tex_format_astc4x4_rgba_srgb, "quality wins over small");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_compress_none | tex_data_compress_quality, tct_astc) == tex_format_none, "uncompressed wins over quality");
	TCT_CHECK(tex_compress_pick(tex_data_srgb, tex_data_compress_none, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "an uncompressed default leaves textures without a policy alone");
	TCT_CHECK(tex_compress_pick(tex_data_srgb, tex_data_none,         tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "a zero default means no compression");
	TCT_CHECK(tex_compress_pick(tex_data_srgb | tex_data_compress_small, tex_data_compress_none, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_astc6x6_rgba_srgb, "a texture's own policy beats the default");
}

///////////////////////////////////////////

static void tct_test_families() {
	TCT_CHECK(tct_pick(tex_data_srgb, tct_both) == tex_format_astc4x4_rgba_srgb, "ASTC wins when both families are available");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_bc  ) == tex_format_bc7_rgba_srgb,     "BC quality picks BC7");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_none) == tex_format_none,              "no encoder family keeps the source");

	TCT_CHECK(tct_pick(tex_data_none,                    tct_astc) == tex_format_astc4x4_rgba, "linear data gets a linear ASTC format");
	TCT_CHECK(tct_pick(tex_data_none,                    tct_bc  ) == tex_format_bc7_rgba,     "linear data gets a linear BC format");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_normal,  tct_astc) == tex_format_astc4x4_rgba, "normals are linear even if tagged srgb");
	TCT_CHECK(tct_pick(tex_data_normal | tex_data_compress_small, tct_bc  ) == tex_format_bc1_rgb,      "small normals are treated as linear data");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32_linear) == tex_format_astc4x4_rgba,      "a known linear format wins over the srgb flag");
	TCT_CHECK(tct_pick(tex_data_none, tct_astc, false, tex_format_rgba32_srgb)   == tex_format_astc4x4_rgba_srgb, "a known sRGB format compresses as sRGB without the flag");
}

///////////////////////////////////////////

static void tct_test_alpha() {
	const tex_data_ small = tex_data_srgb | tex_data_compress_small;
	TCT_CHECK(tct_pick(small,                   tct_bc, false) == tex_format_bc1_rgb_srgb,  "small opaque BC picks BC1");
	TCT_CHECK(tct_pick(small,                   tct_bc, true ) == tex_format_bc7_rgba_srgb, "small BC with alpha keeps real alpha in BC7");
	TCT_CHECK(tct_pick(small | tex_data_cutout, tct_bc, true ) == tex_format_bc1_rgba_srgb, "cutout alpha takes BC1 punch-through");
	TCT_CHECK(tct_pick(small | tex_data_opaque, tct_bc, true ) == tex_format_bc1_rgb_srgb,  "opaque drops a present alpha channel");
	TCT_CHECK(tct_pick(tex_data_srgb,           tct_bc, true ) == tex_format_bc7_rgba_srgb, "quality alpha is BC7");
	TCT_CHECK(tct_pick(small,                   tct_astc, true) == tex_format_astc6x6_rgba_srgb, "ASTC picks per block, so alpha doesn't change it");
}

///////////////////////////////////////////

static void tct_test_hdr() {
	const tex_compress_caps_t astc_no_hdr = { true, false, false };
	TCT_CHECK(tct_pick(tex_data_none,  tct_bc,   false, tex_format_rg11b10) == tex_format_bc6h_rgbuf,        "HDR on BC is BC6H");
	TCT_CHECK(tct_pick(tex_data_compress_small, tct_astc, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr,  "small HDR on ASTC is 8x8 HDR");
	TCT_CHECK(tct_pick(tex_data_none,  tct_astc, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr,  "quality HDR on ASTC is 8x8 HDR");
	TCT_CHECK(tct_pick(tex_data_none,  tct_astc, false, tex_format_rg11b10, tex_type_image | tex_type_cubemap, 256, 6) == tex_format_astc8x8_rgba_hdr, "quality HDR cubemaps on ASTC are 8x8 HDR");
	TCT_CHECK(tct_pick(tex_data_none,  tct_both, false, tex_format_rg11b10, tex_type_image | tex_type_cubemap, 256, 6) == tex_format_astc8x8_rgba_hdr, "ASTC devices never mix in BC6H");
	TCT_CHECK(tct_pick(tex_data_compress_small, astc_no_hdr, false, tex_format_rg11b10) == tex_format_none, "no ASTC HDR support stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_none,  tex_compress_caps_t{ true, false, true }, false, tex_format_rg11b10) == tex_format_none, "ASTC without ASTC HDR doesn't fall back to BC6H");
	TCT_CHECK(tct_pick(tex_data_compress_small, tct_both, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr, "small HDR prefers ASTC 8x8 over BC6H");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, false, tex_format_rgba128) == tex_format_bc6h_rgbuf, "float RGBA is an HDR source");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, false, tex_format_rgba64f) == tex_format_bc6h_rgbuf, "half RGBA is an HDR source");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, true,  tex_format_rgba128) == tex_format_none,       "HDR with alpha stays uncompressed on BC");
	TCT_CHECK(tct_pick(tex_data_opaque, tct_bc, true, tex_format_rgba128) == tex_format_bc6h_rgbuf, "opaque HDR drops its alpha into BC6H");
	TCT_CHECK(tct_pick(tex_data_none, tct_astc, true, tex_format_rgba128) == tex_format_none,       "HDR with alpha stays uncompressed on ASTC");
	TCT_CHECK(tct_pick(tex_data_opaque, tct_astc, true, tex_format_rgba128) == tex_format_astc8x8_rgba_hdr, "opaque HDR drops its alpha into ASTC 8x8 HDR");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, false, tex_format_r8)     == tex_format_none,        "single channel data stays uncompressed");
}

///////////////////////////////////////////

static void tct_test_refusals() {
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_bc7_rgba_srgb) == tex_format_none,  "already compressed sources are kept");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_bgra32)       == tex_format_none,  "unsupported source formats are kept");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_dynamic)      == tex_format_none, "dynamic textures are never compressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_rendertarget) == tex_format_none, "rendertargets are never compressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 64) == tex_format_astc4x4_rgba_srgb, "small LDR images compress like any other");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 4)  == tex_format_none,              "LDR smaller than a block stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 32) == tex_format_bc6h_rgbuf, "small HDR still compresses");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 4)  == tex_format_none,       "HDR smaller than a block stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image, 256, 4) == tex_format_astc4x4_rgba_srgb, "texture arrays compress");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_cubemap, 256, 12) == tex_format_none, "cubemaps need exactly 6 layers");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image_nomips | tex_type_cubemap, 256, 6) == tex_format_astc4x4_rgba_srgb, "cubemaps compress");
}

///////////////////////////////////////////

// Minimal PNG: signature, IHDR, optional extra chunk, empty IDAT. stb skips
// CRCs, so those stay zero.
static size_t tct_png(uint8_t* out, uint8_t color_type, const char* extra_chunk) {
	static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
	size_t at = 0;
	memcpy(out, sig, 8); at += 8;
	const uint8_t ihdr[25] = { 0,0,0,13, 'I','H','D','R', 0,0,0,16, 0,0,0,16, 8, color_type, 0, 0, 0, 0,0,0,0 };
	memcpy(out + at, ihdr, sizeof(ihdr)); at += sizeof(ihdr);
	if (extra_chunk != nullptr) {
		const uint8_t chunk[18] = { 0,0,0,6, (uint8_t)extra_chunk[0], (uint8_t)extra_chunk[1], (uint8_t)extra_chunk[2], (uint8_t)extra_chunk[3], 0,0,0,0,0,0, 0,0,0,0 };
		memcpy(out + at, chunk, sizeof(chunk)); at += sizeof(chunk);
	}
	const uint8_t idat[12] = { 0,0,0,0, 'I','D','A','T', 0,0,0,0 };
	memcpy(out + at, idat, sizeof(idat)); at += sizeof(idat);
	return at;
}

static void tct_test_header_alpha() {
	uint8_t png[128];
	size_t  size;
	size = tct_png(png, 2, nullptr); TCT_CHECK(!tex_image_has_alpha(png, size), "RGB PNG has no alpha");
	size = tct_png(png, 6, nullptr); TCT_CHECK( tex_image_has_alpha(png, size), "RGBA PNG has alpha");
	size = tct_png(png, 4, nullptr); TCT_CHECK( tex_image_has_alpha(png, size), "gray+alpha PNG has alpha");
	size = tct_png(png, 2, "tRNS" ); TCT_CHECK( tex_image_has_alpha(png, size), "RGB PNG with a tRNS color key has alpha");
	size = tct_png(png, 2, "gAMA" ); TCT_CHECK(!tex_image_has_alpha(png, size), "an unrelated chunk doesn't imply alpha");

	const uint8_t jpg[16] = { 0xFF, 0xD8, 0xFF, 0xE0 };
	TCT_CHECK(!tex_image_has_alpha(jpg, sizeof(jpg)), "JPEG never has alpha");

	uint8_t qoi[22] = { 'q','o','i','f', 0,0,0,16, 0,0,0,16, 3, 0, 0,0,0,0,0,0,0,1 };
	TCT_CHECK(!tex_image_has_alpha(qoi, sizeof(qoi)), "3 channel QOI has no alpha");
	qoi[12] = 4;
	TCT_CHECK( tex_image_has_alpha(qoi, sizeof(qoi)), "4 channel QOI has alpha");

	const uint8_t junk[16] = { 1, 2, 3 };
	TCT_CHECK( tex_image_has_alpha(junk, sizeof(junk)), "unknown data assumes alpha");
}

///////////////////////////////////////////

static void* tct_qoi(int32_t size, bool alpha, int* out_len) {
	uint8_t* px = (uint8_t*)malloc((size_t)size * size * 4);
	for (int32_t y = 0; y < size; y++) {
		for (int32_t x = 0; x < size; x++) {
			uint8_t* p = &px[(y * size + x) * 4];
			p[0] = (uint8_t)(x * 255 / size);
			p[1] = (uint8_t)(y * 255 / size);
			p[2] = (uint8_t)((x ^ y) & 0xFF);
			p[3] = alpha ? (uint8_t)(255 - x * 255 / size) : 255;
		}
	}
	qoi_desc desc = { (unsigned int)size, (unsigned int)size, (unsigned char)(alpha ? 4 : 3), QOI_SRGB };
	void* result = qoi_encode(px, &desc, out_len);
	free(px);
	return result;
}

// Runs the real upload path. Whether it compresses depends on the device, so
// the expected format comes from the same pick the loader makes.
static void tct_test_gpu() {
	tex_compress_caps_t caps = tex_compress_caps();
	log_infof("[texcompress_test] caps: astc %d, astc_hdr %d, bc %d", caps.astc, caps.astc_hdr, caps.bc);

	int   len  = 0;
	void* file = tct_qoi(256, false, &len);

	tex_t quality = tex_create_mem(file, (size_t)len, tex_data_srgb);
	tex_t data    = tex_create_mem(file, (size_t)len, tex_data_none | tex_data_compress_small);
	tex_t raw     = tex_create_mem(file, (size_t)len, tex_data_srgb | tex_data_compress_none);
	tex_format_ expect_quality = tex_compress_pick(tex_data_srgb,                  tex_data_compress_quality, tex_type_image, tex_format_rgba32,        false, 256, 256, 1, caps);
	tex_format_ expect_data    = tex_compress_pick(tex_data_none | tex_data_compress_small, tex_data_compress_quality, tex_type_image, tex_format_rgba32_linear, false, 256, 256, 1, caps);

	// Format is settled from the header, before the pixels decode.
	TCT_CHECK(tex_get_format(quality) == (expect_quality != tex_format_none ? expect_quality : tex_format_rgba32), "format is final at creation");

	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(tex_asset_state(quality) == asset_state_loaded, "compressed texture loads");
	TCT_CHECK(tex_get_format (quality) == (expect_quality != tex_format_none ? expect_quality : tex_format_rgba32),        "quality lands in the picked format");
	TCT_CHECK(tex_get_format (data)    == (expect_data    != tex_format_none ? expect_data    : tex_format_rgba32_linear), "small linear data lands in the picked format");
	TCT_CHECK(tex_get_format (raw)     == tex_format_rgba32,         "uncompressed stays RGBA");
	TCT_CHECK(tex_get_mips(quality) == 9 && tex_get_mips(raw) == 9,  "compression keeps the full mip chain");

	// A burst from every asset thread at once shares the encoder pipelines.
	const int32_t burst = 24;
	tex_t many[burst];
	for (int32_t i = 0; i < burst; i++)
		many[i] = tex_create_mem(file, (size_t)len, (i & 1) ? tex_data_srgb : (tex_data_srgb | tex_data_compress_small));
	assets_block_for_priority(INT32_MAX);
	bool all_loaded = true;
	for (int32_t i = 0; i < burst; i++) {
		all_loaded = all_loaded && tex_asset_state(many[i]) == asset_state_loaded;
		tex_release(many[i]);
	}
	TCT_CHECK(all_loaded, "a concurrent burst of compressed loads all complete");

	tex_release(quality);
	tex_release(data);
	tex_release(raw);
	free(file);
}

///////////////////////////////////////////

static void tct_fill(color32* pixels, int32_t count, color32 color) {
	for (int32_t i = 0; i < count; i++) pixels[i] = color;
}

static void tct_test_set_colors() {
	tex_compress_caps_t caps   = tex_compress_caps();
	const int32_t       size   = 256;
	color32*            pixels = (color32*)malloc(sizeof(color32) * size * size);
	tct_fill(pixels, size * size, color32{ 200, 40, 90, 255 });

	tex_format_ expect = tex_compress_pick(tex_data_srgb, tex_data_compress_quality, tex_type_image, tex_format_rgba32, true, size, size, 1, caps);
	if (expect == tex_format_none) expect = tex_format_rgba32;

	// Async by default, with the format settled before the upload runs
	tex_t async = tex_create(tex_type_image, tex_format_rgba32);
	tex_set_colors(async, size, size, pixels);
	TCT_CHECK(tex_get_format(async) == expect, "set_colors settles its format on return");
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(tex_asset_state(async) == asset_state_loaded, "async set_colors loads");
	TCT_CHECK(tex_get_mips(async) == 9,                     "async set_colors generates mips before encoding");

	// The data format survives compression, so the next update still reads as RGBA
	tex_set_colors(async, size, size, pixels);
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(tex_get_format(async) == expect && tex_get_mips(async) == 9, "a second update on a compressed texture keeps format and mips");

	tex_t blocking = tex_create(tex_type_image, tex_format_rgba32);
	tex_set_colors(blocking, size, size, pixels, tex_data_srgb | tex_data_blocking);
	TCT_CHECK(tex_asset_state(blocking) == asset_state_loaded && tex_get_format(blocking) == expect, "blocking set_colors is loaded on return");

	// Rapid async updates land in order, so the last one wins
	const int32_t small = 64;
	tex_t latest = tex_create(tex_type_image_nomips, tex_format_rgba32);
	for (int32_t i = 0; i < 32; i++) {
		tct_fill(pixels, small * small, color32{ (uint8_t)i, 0, 0, 255 });
		tex_set_colors(latest, small, small, pixels, tex_data_srgb | tex_data_compress_none);
	}
	assets_block_for_priority(INT32_MAX);
	color32* readback = (color32*)malloc(sizeof(color32) * small * small);
	tex_get_data(latest, readback, sizeof(color32) * small * small, 0);
	TCT_CHECK(readback[0].r == 31, "the last of many async updates wins");

	// Holding the write lock keeps a replacement in flight, as a slow encode would
	atomic_exchange_i32(&latest->write_running, 1);
	tct_fill(pixels, small * small, color32{ 77, 0, 0, 255 });
	tex_set_colors(latest, small, small, pixels, tex_data_srgb | tex_data_compress_none);
	TCT_CHECK(tex_asset_state(latest) == asset_state_loaded_meta, "a loaded texture reads as loaded_meta while new data is in flight");
	atomic_exchange_i32(&latest->write_running, 0);
	tex_get_data(latest, readback, sizeof(color32) * small * small, 0);
	TCT_CHECK(readback[0].r == 77 && tex_asset_state(latest) == asset_state_loaded, "reading data waits for a replacement to land");
	free(readback);

	// An explicit float format is an HDR source for this call
	float* hdr = (float*)malloc(sizeof(float) * 4 * size * size);
	for (int32_t i = 0; i < size * size * 4; i++) hdr[i] = 2.0f;
	tex_format_ expect_hdr = tex_compress_pick(tex_data_srgb, tex_data_compress_quality, tex_type_image, tex_format_rgba128, true, size, size, 1, caps);
	tex_t floats = tex_create(tex_type_image, tex_format_rgba32);
	tex_set_colors(floats, size, size, hdr, tex_data_srgb | tex_data_blocking, tex_format_rgba128);
	TCT_CHECK(tex_get_format(floats) == (expect_hdr != tex_format_none ? expect_hdr : tex_format_rgba128), "an explicit data format applies to that call");
	free(hdr);

	// Like a font atlas gaining glyphs, a same-size update must reach every
	// mip, or the new content vanishes at a distance.
	const int32_t atlas_size = 64;
	uint8_t* atlas = (uint8_t*)calloc(atlas_size * atlas_size, 1);
	tex_t    glyphs = tex_create(tex_type_image | tex_type_dynamic, tex_format_r8);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_compress_none);
	memset(atlas, 255, atlas_size * atlas_size);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_compress_none);
	int32_t mips     = tex_get_mips(glyphs);
	uint8_t smallest = 0;
	tex_get_data(glyphs, &smallest, 1, mips - 1);
	TCT_CHECK(mips == 7,       "a same-size update keeps the mip chain");
	TCT_CHECK(smallest == 255, "a same-size update regenerates every mip");
	// Same-size updates to a dynamic texture write in place
	memset(atlas, 128, atlas_size * atlas_size);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_compress_none);
	tex_get_data(glyphs, &smallest, 1, tex_get_mips(glyphs) - 1);
	TCT_CHECK(smallest == 128, "an in-place update regenerates every mip");
	tex_release(glyphs);
	free(atlas);

	tex_release(async);
	tex_release(blocking);
	tex_release(latest);
	tex_release(floats);
	free(pixels);
}

///////////////////////////////////////////

static void tct_test_copy() {
	tex_compress_caps_t caps   = tex_compress_caps();
	const int32_t       size   = 256;
	color32*            pixels = (color32*)malloc(sizeof(color32) * size * size);
	tct_fill(pixels, size * size, color32{ 30, 160, 220, 255 });
	tex_t source = tex_create_color32(pixels, size, size, tex_data_srgb | tex_data_blocking | tex_data_compress_none);
	free(pixels);

	tex_format_ expect = tex_compress_pick(tex_data_srgb, tex_get_compression_default(), tex_type_image, tex_format_rgba32, true, size, size, 1, caps);
	if (expect == tex_format_none) expect = tex_format_rgba32;

	tex_t copied = tex_copy(source);
	TCT_CHECK(tex_asset_state(copied) == asset_state_loaded && tex_get_format(copied) == expect, "a copy compresses by default and is done on return");
	TCT_CHECK(tex_get_mips(copied) == 9,                                                          "a compressed copy generates mips before encoding");

	tex_t raw = tex_copy(source, tex_type_image, tex_format_none, tex_data_compress_none);
	TCT_CHECK(tex_get_format(raw) == tex_format_rgba32, "an uncompressed copy keeps the source format");

	if (caps.bc || caps.astc) {
		tex_format_ exact = caps.astc ? tex_format_astc6x6_rgba_srgb : tex_format_bc1_rgb_srgb;
		tex_t explicit_fmt = tex_copy(source, tex_type_image, exact);
		TCT_CHECK(tex_get_format(explicit_fmt) == exact, "an explicit block format is used as given");
		tex_release(explicit_fmt);
	}

	tex_t dest   = tex_create();
	tex_t landed = tex_copy(source, tex_type_image, tex_format_none, tex_data_srgb, dest);
	TCT_CHECK(landed == dest && tex_get_format(dest) == expect && tex_asset_state(dest) == asset_state_loaded, "a copy lands in `into` and returns it");
	tex_release(landed);

	TCT_CHECK(tex_copy(source, tex_type_image, tex_format_none, tex_data_srgb, source) == nullptr, "a copy over its own source is refused");

	tex_t target = tex_create_rendertarget(size, size, 1, tex_format_rgba32, tex_format_none);
	tex_t baked  = tex_copy(target);
	TCT_CHECK(tex_get_format(baked) == expect, "a rendertarget copies into a compressed image");

	tex_release(baked);
	tex_release(target);
	tex_release(dest);
	tex_release(raw);
	tex_release(copied);
	tex_release(source);
}

///////////////////////////////////////////

// Arrays and cubes encode every layer and mip in one pass
static void tct_test_layers() {
	tex_compress_caps_t caps   = tex_compress_caps();
	const int32_t       size   = 64;
	const int32_t       layers = 4;
	color32*            pixels[6];
	for (int32_t l = 0; l < 6; l++) {
		pixels[l] = (color32*)malloc(sizeof(color32) * size * size);
		tct_fill(pixels[l], size * size, color32{ (uint8_t)(l * 40), 100, 200, 255 });
	}

	tex_format_ expect_array = tex_compress_pick(tex_data_srgb, tex_data_compress_quality, tex_type_image, tex_format_rgba32, true, size, size, layers, caps);
	if (expect_array == tex_format_none) expect_array = tex_format_rgba32;
	tex_t array = tex_create(tex_type_image, tex_format_rgba32);
	tex_set_color_arr(array, size, size, (void**)pixels, layers, 1, tex_data_srgb | tex_data_blocking);
	TCT_CHECK(tex_asset_state(array) == asset_state_loaded && tex_get_format(array) == expect_array, "a texture array compresses");
	TCT_CHECK(tex_get_mips(array) == 7 && array->gpu_tex.layer_count == layers,                       "a compressed array keeps every layer and mip");
	TCT_CHECK((array->gpu_tex.flags & skr_tex_flags_array) != 0,                                     "a compressed array is still an array");

	tex_t array_copy = tex_copy(array, tex_type_image, tex_format_none, tex_data_srgb);
	TCT_CHECK(tex_get_format(array_copy) == tex_get_format(array) && array_copy->gpu_tex.layer_count == layers, "an array copy keeps its layers and format");

	tex_format_ expect_cube = tex_compress_pick(tex_data_srgb, tex_data_compress_quality, tex_type_image | tex_type_cubemap, tex_format_rgba32, true, size, size, 6, caps);
	if (expect_cube == tex_format_none) expect_cube = tex_format_rgba32;
	tex_t cube = tex_create(tex_type_image | tex_type_cubemap, tex_format_rgba32);
	tex_set_color_arr(cube, size, size, (void**)pixels, 6, 1, tex_data_srgb | tex_data_blocking);
	TCT_CHECK(tex_get_format(cube) == expect_cube && tex_get_mips(cube) == 7, "a cubemap with mips compresses every face and mip");
	TCT_CHECK((cube->gpu_tex.flags & skr_tex_flags_cubemap) != 0,             "a compressed cubemap is still a cubemap");

	tex_release(cube);
	tex_release(array_copy);
	tex_release(array);
	for (int32_t l = 0; l < 6; l++) free(pixels[l]);
}

///////////////////////////////////////////

static tex_t tct_hdr_cube(int32_t size, float scale) {
	color128* faces[6];
	for (int32_t f = 0; f < 6; f++) {
		faces[f] = (color128*)malloc(sizeof(color128) * size * size);
		for (int32_t i = 0; i < size * size; i++)
			faces[f][i] = color128{ scale * (f + 1), scale * 0.5f * (i % size) / size, scale * (6 - f) * 0.25f, 1 };
	}
	tex_t cube = tex_create(tex_type_image_nomips | tex_type_cubemap, tex_format_rgba128);
	tex_set_color_arr(cube, size, size, (void**)faces, 6, 1, tex_data_blocking | tex_data_compress_none);
	for (int32_t f = 0; f < 6; f++) free(faces[f]);
	return cube;
}

static bool tct_sh_close(spherical_harmonics_t a, spherical_harmonics_t b) {
	for (int32_t i = 0; i < 9; i++) {
		vec3 d = a.coefficients[i] - b.coefficients[i];
		if (fabsf(d.x) > 1e-3f || fabsf(d.y) > 1e-3f || fabsf(d.z) > 1e-3f) return false;
	}
	return true;
}

static void tct_test_reflection() {
	tex_compress_caps_t caps   = tex_compress_caps();
	const int32_t       size   = 64;
	tex_t               bright = tct_hdr_cube(128, 2.0f);
	tex_t               dim    = tct_hdr_cube(128, 0.5f);

	tex_format_ expect = tex_compress_pick(tex_data_opaque, tex_get_compression_default(), tex_type_image | tex_type_cubemap, tex_format_rgba128, false, size, size, 6, caps);
	if (expect == tex_format_none) expect = tex_format_rgba128;

	tex_t raw  = tex_gen_cubemap_reflection(bright, nullptr, size, tex_data_compress_none);
	tex_t refl = tex_gen_cubemap_reflection(bright, nullptr, size);
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(tex_asset_state(refl) == asset_state_loaded && tex_get_format(refl) == expect, "reflections compress by default");
	TCT_CHECK(tex_get_input_format(refl) == tex_format_rgba128 && tex_get_mips(refl) == 7,  "a compressed reflection keeps its convolution format as input, and every mip");
	TCT_CHECK(tex_get_format(raw) == tex_format_rgba128 && (raw->type & tex_type_rendertarget) != 0, "CompressNone keeps a reflection convolved in place");
	TCT_CHECK(tct_sh_close(tex_get_cubemap_lighting(refl), tex_get_cubemap_lighting(raw)), "compression doesn't change a reflection's lighting");

	// Regenerating into the same texture, with the later request winning
	tex_t dim_raw = tex_gen_cubemap_reflection(dim, nullptr, size, tex_data_compress_none);
	tex_t again   = tex_gen_cubemap_reflection(bright, refl, size);
	tex_t latest  = tex_gen_cubemap_reflection(dim,    refl, size);
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(again == refl && latest == refl,                                                  "regenerating returns the same reflection");
	TCT_CHECK(tex_asset_state(refl) == asset_state_loaded && tex_get_format(refl) == expect,    "a compressed reflection regenerates compressed");
	TCT_CHECK(tct_sh_close(tex_get_cubemap_lighting(refl), tex_get_cubemap_lighting(dim_raw)), "the latest regeneration wins");

	tex_t uncompressed = tex_gen_cubemap_reflection(bright, refl, size, tex_data_compress_none);
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(uncompressed == refl && tex_get_format(refl) == tex_format_rgba128, "a compressed reflection can regenerate uncompressed");
	TCT_CHECK(tct_sh_close(tex_get_cubemap_lighting(refl), tex_get_cubemap_lighting(raw)), "an uncompressed regeneration carries the new lighting");

	// A source that fails to load leaves the previous reflection in place
	tex_t missing      = tex_create_cubemap_file("tct_missing_cubemap.hdr");
	tex_t from_missing = tex_gen_cubemap_reflection(missing, refl, size);
	assets_block_for_priority(INT32_MAX);
	TCT_CHECK(tex_asset_state(refl) == asset_state_loaded, "a regeneration from a failed source resolves, keeping the reflection loaded");
	TCT_CHECK(tct_sh_close(tex_get_cubemap_lighting(refl), tex_get_cubemap_lighting(raw)), "a regeneration from a failed source keeps the previous lighting");
	tex_release(from_missing);
	tex_release(missing);

	// Blocking finishes before returning, with no frame or asset stepping
	tex_t blocked = tex_gen_cubemap_reflection(dim, nullptr, size, tex_data_blocking);
	TCT_CHECK(tex_asset_state(blocked) == asset_state_loaded && tex_get_format(blocked) == expect, "a blocking reflection is loaded on return");
	tex_t reblocked = tex_gen_cubemap_reflection(bright, blocked, size, tex_data_blocking);
	TCT_CHECK(tct_sh_close(tex_get_cubemap_lighting(blocked), tex_get_cubemap_lighting(raw)), "a blocking regeneration lands on return");

	tex_release(reblocked);
	tex_release(blocked);
	tex_release(uncompressed);
	tex_release(latest);
	tex_release(again);
	tex_release(dim_raw);
	tex_release(refl);
	tex_release(raw);
	tex_release(dim);
	tex_release(bright);
}

///////////////////////////////////////////

int texcompress_tests_run() {
	tct_failures = 0;
	log_info("[texcompress_test] Running texture compression tests");

	tct_test_policy      ();
	tct_test_families    ();
	tct_test_alpha       ();
	tct_test_hdr         ();
	tct_test_refusals    ();
	tct_test_header_alpha();

	sk_settings_t settings = {};
	settings.app_name      = "StereoKitC Texture Compression Tests";
	settings.mode          = app_mode_offscreen;
	settings.standby_mode  = standby_mode_none;
	if (sk_init(settings)) {
		tct_test_gpu();
		tct_test_set_colors();
		tct_test_copy();
		tct_test_layers();
		tct_test_reflection();
		sk_shutdown();
	} else {
		TCT_CHECK(false, "sk_init for the GPU tests");
	}

	log_infof("[texcompress_test] %d failure(s)", tct_failures);
	return tct_failures;
}
