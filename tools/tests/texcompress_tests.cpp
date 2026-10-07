// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "texcompress_tests.h"

#include <stereokit.h>

// Internal compression helpers, reached through StereoKitC's include root.
#include <asset_types/texture_compression.h>

// The test builds its own image files, and StereoKitC keeps its copy of the
// encoder private.
#define QOI_IMPLEMENTATION
#include <libraries/qoi.h>

#include <stdlib.h>
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
	return tex_compress_pick(flags, tex_data_quality, type, src, alpha, size, size, arrays, caps);
}

///////////////////////////////////////////

static void tct_test_policy() {
	TCT_CHECK(tct_pick(tex_data_srgb,                                  tct_astc) == tex_format_astc4x4_rgba_srgb, "no policy uses the quality default");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_small,                 tct_astc) == tex_format_astc6x6_rgba_srgb, "small picks ASTC 6x6");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_small | tex_data_quality, tct_astc) == tex_format_astc4x4_rgba_srgb, "quality wins over small");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_uncompressed | tex_data_quality, tct_astc) == tex_format_none, "uncompressed wins over quality");
	TCT_CHECK(tex_compress_pick(tex_data_srgb, tex_data_uncompressed, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "an uncompressed default leaves hint-less textures alone");
	TCT_CHECK(tex_compress_pick(tex_data_srgb, tex_data_none,         tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "a zero default means no compression");
	TCT_CHECK(tex_compress_pick(tex_data_srgb | tex_data_small, tex_data_uncompressed, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_astc6x6_rgba_srgb, "a texture's own policy beats the default");
}

///////////////////////////////////////////

static void tct_test_families() {
	TCT_CHECK(tct_pick(tex_data_srgb, tct_both) == tex_format_astc4x4_rgba_srgb, "ASTC wins when both families are available");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_bc  ) == tex_format_bc7_rgba_srgb,     "BC quality picks BC7");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_none) == tex_format_none,              "no encoder family keeps the source");

	TCT_CHECK(tct_pick(tex_data_none,                    tct_astc) == tex_format_astc4x4_rgba, "linear data gets a linear ASTC format");
	TCT_CHECK(tct_pick(tex_data_none,                    tct_bc  ) == tex_format_bc7_rgba,     "linear data gets a linear BC format");
	TCT_CHECK(tct_pick(tex_data_srgb | tex_data_normal,  tct_astc) == tex_format_astc4x4_rgba, "normals are linear even if tagged srgb");
	TCT_CHECK(tct_pick(tex_data_normal | tex_data_small, tct_bc  ) == tex_format_bc1_rgb,      "small normals are treated as linear data");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32_linear) == tex_format_astc4x4_rgba,      "a known linear format wins over the srgb flag");
	TCT_CHECK(tct_pick(tex_data_none, tct_astc, false, tex_format_rgba32_srgb)   == tex_format_astc4x4_rgba_srgb, "a known sRGB format compresses as sRGB without the flag");
}

///////////////////////////////////////////

static void tct_test_alpha() {
	const tex_data_ small = tex_data_srgb | tex_data_small;
	TCT_CHECK(tct_pick(small,                   tct_bc, false) == tex_format_bc1_rgb_srgb,  "small opaque BC picks BC1");
	TCT_CHECK(tct_pick(small,                   tct_bc, true ) == tex_format_bc7_rgba_srgb, "small BC with alpha keeps real alpha in BC7");
	TCT_CHECK(tct_pick(small | tex_data_cutout, tct_bc, true ) == tex_format_bc1_rgba_srgb, "cutout alpha takes BC1 punch-through");
	TCT_CHECK(tct_pick(small | tex_data_opaque, tct_bc, true ) == tex_format_bc1_rgb_srgb,  "opaque drops a present alpha channel");
	TCT_CHECK(tct_pick(tex_data_srgb,           tct_bc, true ) == tex_format_bc7_rgba_srgb, "quality alpha is BC7");
	TCT_CHECK(tct_pick(small,                   tct_astc, true) == tex_format_astc6x6_rgba_srgb, "ASTC picks per block, so alpha doesn't change it");
}

///////////////////////////////////////////

static void tct_test_hdr() {
	TCT_CHECK(tct_pick(tex_data_none,  tct_bc,   false, tex_format_rg11b10) == tex_format_bc6h_rgbuf,        "HDR on BC is BC6H");
	TCT_CHECK(tct_pick(tex_data_small, tct_astc, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr,  "small HDR on ASTC is 8x8 HDR");
	TCT_CHECK(tct_pick(tex_data_none,  tct_astc, false, tex_format_rg11b10) == tex_format_none,              "quality HDR on ASTC stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_small, tex_compress_caps_t{ true, false, false }, false, tex_format_rg11b10) == tex_format_none, "no ASTC HDR support stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_small, tct_both, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr, "small HDR prefers ASTC 8x8 over BC6H");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, true, tex_format_rgba128) == tex_format_bc6h_rgbuf,  "float RGBA is an HDR source");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, true, tex_format_rgba64f) == tex_format_bc6h_rgbuf,  "half RGBA is an HDR source");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc, false, tex_format_r8)     == tex_format_none,        "single channel data stays uncompressed");
}

///////////////////////////////////////////

static void tct_test_refusals() {
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_bc7_rgba_srgb) == tex_format_none,  "already compressed sources are kept");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba128)      == tex_format_none,  "unsupported source formats are kept");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_dynamic)      == tex_format_none, "dynamic textures are never compressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_rendertarget) == tex_format_none, "rendertargets are never compressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 64) == tex_format_astc4x4_rgba_srgb, "small LDR images compress like any other");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 4)  == tex_format_none,              "LDR smaller than a block stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 32) == tex_format_bc6h_rgbuf, "small HDR still compresses");
	TCT_CHECK(tct_pick(tex_data_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 4)  == tex_format_none,       "HDR smaller than a block stays uncompressed");
	TCT_CHECK(tct_pick(tex_data_srgb, tct_astc, false, tex_format_rgba32, tex_type_image, 256, 4) == tex_format_none, "texture arrays stay uncompressed for now");
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
	tex_t data    = tex_create_mem(file, (size_t)len, tex_data_none | tex_data_small);
	tex_t raw     = tex_create_mem(file, (size_t)len, tex_data_srgb | tex_data_uncompressed);
	tex_format_ expect_quality = tex_compress_pick(tex_data_srgb,                  tex_data_quality, tex_type_image, tex_format_rgba32,        false, 256, 256, 1, caps);
	tex_format_ expect_data    = tex_compress_pick(tex_data_none | tex_data_small, tex_data_quality, tex_type_image, tex_format_rgba32_linear, false, 256, 256, 1, caps);

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
		many[i] = tex_create_mem(file, (size_t)len, (i & 1) ? tex_data_srgb : (tex_data_srgb | tex_data_small));
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

	tex_format_ expect = tex_compress_pick(tex_data_srgb, tex_data_quality, tex_type_image, tex_format_rgba32, true, size, size, 1, caps);
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
		tex_set_colors(latest, small, small, pixels, tex_data_srgb | tex_data_uncompressed);
	}
	assets_block_for_priority(INT32_MAX);
	color32* readback = (color32*)malloc(sizeof(color32) * small * small);
	tex_get_data(latest, readback, sizeof(color32) * small * small, 0);
	TCT_CHECK(readback[0].r == 31, "the last of many async updates wins");
	free(readback);

	// An explicit float format is an HDR source for this call
	float* hdr = (float*)malloc(sizeof(float) * 4 * size * size);
	for (int32_t i = 0; i < size * size * 4; i++) hdr[i] = 2.0f;
	tex_format_ expect_hdr = tex_compress_pick(tex_data_srgb, tex_data_quality, tex_type_image, tex_format_rgba128, true, size, size, 1, caps);
	tex_t floats = tex_create(tex_type_image, tex_format_rgba32);
	tex_set_colors(floats, size, size, hdr, tex_data_srgb | tex_data_blocking, tex_format_rgba128);
	TCT_CHECK(tex_get_format(floats) == (expect_hdr != tex_format_none ? expect_hdr : tex_format_rgba128), "an explicit data format applies to that call");
	free(hdr);

	// Like a font atlas gaining glyphs, a same-size update must reach every
	// mip, or the new content vanishes at a distance.
	const int32_t atlas_size = 64;
	uint8_t* atlas = (uint8_t*)calloc(atlas_size * atlas_size, 1);
	tex_t    glyphs = tex_create(tex_type_image | tex_type_dynamic, tex_format_r8);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_uncompressed);
	memset(atlas, 255, atlas_size * atlas_size);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_uncompressed);
	int32_t mips     = tex_get_mips(glyphs);
	uint8_t smallest = 0;
	tex_get_data(glyphs, &smallest, 1, mips - 1);
	TCT_CHECK(mips == 7,       "a same-size update keeps the mip chain");
	TCT_CHECK(smallest == 255, "a same-size update regenerates every mip");
	// Same-size updates to a dynamic texture write in place
	memset(atlas, 128, atlas_size * atlas_size);
	tex_set_colors(glyphs, atlas_size, atlas_size, atlas, tex_data_blocking | tex_data_uncompressed);
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
	tex_t source = tex_create_color32(pixels, size, size, tex_data_srgb | tex_data_blocking | tex_data_uncompressed);
	free(pixels);

	tex_format_ expect = tex_compress_pick(tex_data_srgb, tex_get_compression_default(), tex_type_image, tex_format_rgba32, true, size, size, 1, caps);
	if (expect == tex_format_none) expect = tex_format_rgba32;

	tex_t copied = tex_copy(source);
	TCT_CHECK(tex_asset_state(copied) == asset_state_loaded && tex_get_format(copied) == expect, "a copy compresses by default and is done on return");
	TCT_CHECK(tex_get_mips(copied) == 9,                                                          "a compressed copy generates mips before encoding");

	tex_t raw = tex_copy(source, tex_type_image, tex_format_none, tex_data_uncompressed);
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
		sk_shutdown();
	} else {
		TCT_CHECK(false, "sk_init for the GPU tests");
	}

	log_infof("[texcompress_test] %d failure(s)", tct_failures);
	return tct_failures;
}
