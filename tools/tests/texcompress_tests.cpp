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

static tex_format_ tct_pick(tex_hint_ hints, tex_compress_caps_t caps, bool32_t alpha = false, tex_format_ src = tex_format_rgba32, tex_type_ type = tex_type_image, int32_t size = 256, int32_t arrays = 1) {
	return tex_compress_pick(hints, tex_hint_quality, type, src, alpha, size, size, arrays, caps);
}

///////////////////////////////////////////

static void tct_test_policy() {
	TCT_CHECK(tct_pick(tex_hint_srgb,                                  tct_astc) == tex_format_astc4x4_rgba_srgb, "no policy uses the quality default");
	TCT_CHECK(tct_pick(tex_hint_srgb | tex_hint_small,                 tct_astc) == tex_format_astc6x6_rgba_srgb, "small picks ASTC 6x6");
	TCT_CHECK(tct_pick(tex_hint_srgb | tex_hint_small | tex_hint_quality, tct_astc) == tex_format_astc4x4_rgba_srgb, "quality wins over small");
	TCT_CHECK(tct_pick(tex_hint_srgb | tex_hint_uncompressed | tex_hint_quality, tct_astc) == tex_format_none, "uncompressed wins over quality");
	TCT_CHECK(tex_compress_pick(tex_hint_srgb, tex_hint_uncompressed, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "an uncompressed default leaves hint-less textures alone");
	TCT_CHECK(tex_compress_pick(tex_hint_srgb, tex_hint_none,         tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_none, "a zero default means no compression");
	TCT_CHECK(tex_compress_pick(tex_hint_srgb | tex_hint_small, tex_hint_uncompressed, tex_type_image, tex_format_rgba32, false, 256, 256, 1, tct_astc) == tex_format_astc6x6_rgba_srgb, "a texture's own policy beats the default");
}

///////////////////////////////////////////

static void tct_test_families() {
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_both) == tex_format_astc4x4_rgba_srgb, "ASTC wins when both families are available");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_bc  ) == tex_format_bc7_rgba_srgb,     "BC quality picks BC7");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_none) == tex_format_none,              "no encoder family keeps the source");

	TCT_CHECK(tct_pick(tex_hint_none,                    tct_astc) == tex_format_astc4x4_rgba, "linear data gets a linear ASTC format");
	TCT_CHECK(tct_pick(tex_hint_none,                    tct_bc  ) == tex_format_bc7_rgba,     "linear data gets a linear BC format");
	TCT_CHECK(tct_pick(tex_hint_srgb | tex_hint_normal,  tct_astc) == tex_format_astc4x4_rgba, "normals are linear even if tagged srgb");
	TCT_CHECK(tct_pick(tex_hint_normal | tex_hint_small, tct_bc  ) == tex_format_bc1_rgb,      "small normals are treated as linear data");
}

///////////////////////////////////////////

static void tct_test_alpha() {
	const tex_hint_ small = tex_hint_srgb | tex_hint_small;
	TCT_CHECK(tct_pick(small,                   tct_bc, false) == tex_format_bc1_rgb_srgb,  "small opaque BC picks BC1");
	TCT_CHECK(tct_pick(small,                   tct_bc, true ) == tex_format_bc7_rgba_srgb, "small BC with alpha keeps real alpha in BC7");
	TCT_CHECK(tct_pick(small | tex_hint_cutout, tct_bc, true ) == tex_format_bc1_rgba_srgb, "cutout alpha takes BC1 punch-through");
	TCT_CHECK(tct_pick(small | tex_hint_opaque, tct_bc, true ) == tex_format_bc1_rgb_srgb,  "opaque drops a present alpha channel");
	TCT_CHECK(tct_pick(tex_hint_srgb,           tct_bc, true ) == tex_format_bc7_rgba_srgb, "quality alpha is BC7");
	TCT_CHECK(tct_pick(small,                   tct_astc, true) == tex_format_astc6x6_rgba_srgb, "ASTC picks per block, so alpha doesn't change it");
}

///////////////////////////////////////////

static void tct_test_hdr() {
	TCT_CHECK(tct_pick(tex_hint_none,  tct_bc,   false, tex_format_rg11b10) == tex_format_bc6h_rgbuf,        "HDR on BC is BC6H");
	TCT_CHECK(tct_pick(tex_hint_small, tct_astc, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr,  "small HDR on ASTC is 8x8 HDR");
	TCT_CHECK(tct_pick(tex_hint_none,  tct_astc, false, tex_format_rg11b10) == tex_format_none,              "quality HDR on ASTC stays uncompressed");
	TCT_CHECK(tct_pick(tex_hint_small, tex_compress_caps_t{ true, false, false }, false, tex_format_rg11b10) == tex_format_none, "no ASTC HDR support stays uncompressed");
	TCT_CHECK(tct_pick(tex_hint_small, tct_both, false, tex_format_rg11b10) == tex_format_astc8x8_rgba_hdr, "small HDR prefers ASTC 8x8 over BC6H");
}

///////////////////////////////////////////

static void tct_test_refusals() {
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_bc7_rgba_srgb) == tex_format_none,  "already compressed sources are kept");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba128)      == tex_format_none,  "unsupported source formats are kept");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_dynamic)      == tex_format_none, "dynamic textures are never compressed");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32, tex_type_image | tex_type_rendertarget) == tex_format_none, "rendertargets are never compressed");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 64) == tex_format_none,       "palette sized LDR images stay uncompressed");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32,  tex_type_image, 65) != tex_format_none,       "LDR just past the palette size compresses");
	TCT_CHECK(tct_pick(tex_hint_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 32) == tex_format_bc6h_rgbuf, "small HDR still compresses");
	TCT_CHECK(tct_pick(tex_hint_none, tct_bc,   false, tex_format_rg11b10, tex_type_image, 4)  == tex_format_none,       "HDR smaller than a block stays uncompressed");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32, tex_type_image, 256, 4) == tex_format_none, "texture arrays stay uncompressed for now");
	TCT_CHECK(tct_pick(tex_hint_srgb, tct_astc, false, tex_format_rgba32, tex_type_image_nomips | tex_type_cubemap, 256, 6) == tex_format_astc4x4_rgba_srgb, "cubemaps compress");
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

	tex_t quality = tex_create_mem(file, (size_t)len, tex_hint_srgb);
	tex_t data    = tex_create_mem(file, (size_t)len, tex_hint_none | tex_hint_small);
	tex_t raw     = tex_create_mem(file, (size_t)len, tex_hint_srgb | tex_hint_uncompressed);
	tex_format_ expect_quality = tex_compress_pick(tex_hint_srgb,                  tex_hint_quality, tex_type_image, tex_format_rgba32,        false, 256, 256, 1, caps);
	tex_format_ expect_data    = tex_compress_pick(tex_hint_none | tex_hint_small, tex_hint_quality, tex_type_image, tex_format_rgba32_linear, false, 256, 256, 1, caps);

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
		many[i] = tex_create_mem(file, (size_t)len, (i & 1) ? tex_hint_srgb : (tex_hint_srgb | tex_hint_small));
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
		sk_shutdown();
	} else {
		TCT_CHECK(false, "sk_init for the GPU tests");
	}

	log_infof("[texcompress_test] %d failure(s)", tct_failures);
	return tct_failures;
}
