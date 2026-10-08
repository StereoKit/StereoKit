// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "tex_tests.h"

#include <stereokit.h>
#include <libraries/qoi.h>
#include <libraries/atomic_util.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

using namespace sk;

static int tst_failures = 0;

#define TST_CHECK(condition, description) do { \
	if (condition) { log_infof("[tex_test] pass: %s", description); } \
	else           { log_errf ("[tex_test] FAIL: %s", description); tst_failures += 1; } \
	} while (0)

///////////////////////////////////////////
// Helpers
///////////////////////////////////////////

#if defined(_WIN32)
typedef HANDLE tst_thread_t;
struct tst_thread_ctx_t { void (*fn)(void*); void* arg; };
static DWORD WINAPI tst_thread_entry(LPVOID ctx) { tst_thread_ctx_t* c = (tst_thread_ctx_t*)ctx; c->fn(c->arg); free(c); return 0; }
static tst_thread_t tst_thread_start(void (*fn)(void*), void* arg) {
	tst_thread_ctx_t* ctx = (tst_thread_ctx_t*)malloc(sizeof(tst_thread_ctx_t));
	*ctx = { fn, arg };
	return CreateThread(nullptr, 0, tst_thread_entry, ctx, 0, nullptr);
}
static void tst_thread_join(tst_thread_t thread) { WaitForSingleObject(thread, INFINITE); CloseHandle(thread); }
#else
typedef pthread_t tst_thread_t;
struct tst_thread_ctx_t { void (*fn)(void*); void* arg; };
static void* tst_thread_entry(void* ctx) { tst_thread_ctx_t* c = (tst_thread_ctx_t*)ctx; c->fn(c->arg); free(c); return nullptr; }
static tst_thread_t tst_thread_start(void (*fn)(void*), void* arg) {
	tst_thread_ctx_t* ctx = (tst_thread_ctx_t*)malloc(sizeof(tst_thread_ctx_t));
	*ctx = { fn, arg };
	pthread_t thread;
	pthread_create(&thread, nullptr, tst_thread_entry, ctx);
	return thread;
}
static void tst_thread_join(tst_thread_t thread) { pthread_join(thread, nullptr); }
#endif

// The repo's Examples/Assets, found from this file so it works from any cwd.
static void tst_asset_path(const char* name, char* out, size_t out_size) {
	const char* here = __FILE__;
	const char* cut  = strstr(here, "tools/tests/");
	if (cut == nullptr) cut = strstr(here, "tools\\tests\\");
	int prefix = cut ? (int)(cut - here) : 0;
	snprintf(out, out_size, "%.*sExamples/Assets/%s", prefix, here, name);
}

// Writes a QOI image to a temp file, so file loads have something real to read.
static bool tst_write_qoi(const char* path, int32_t size) {
	uint8_t* px = (uint8_t*)malloc((size_t)size * size * 4);
	for (int32_t i = 0; i < size * size; i++) {
		px[i*4+0] = (uint8_t)(i * 7);
		px[i*4+1] = (uint8_t)(i * 13);
		px[i*4+2] = (uint8_t)(i * 29);
		px[i*4+3] = 255;
	}
	qoi_desc desc = { (unsigned int)size, (unsigned int)size, 4, QOI_SRGB };
	int   len  = 0;
	void* data = qoi_encode(px, &desc, &len);
	free(px);
	FILE* fp = fopen(path, "wb");
	if (fp == nullptr) { free(data); return false; }
	fwrite(data, 1, (size_t)len, fp);
	fclose(fp);
	free(data);
	return true;
}

static void tst_fill(color32* px, int32_t count, color32 color) {
	for (int32_t i = 0; i < count; i++) px[i] = color;
}

static color32 tst_read_first(tex_t tex, int32_t size) {
	color32* px = (color32*)malloc(sizeof(color32) * size * size);
	tex_get_data(tex, px, sizeof(color32) * size * size, 0);
	color32 result = px[0];
	free(px);
	return result;
}

///////////////////////////////////////////
// Blocking loads
///////////////////////////////////////////

static char tst_qoi_path       [512];
static char tst_qoi_path_foreign[512]; // loaded only by the foreign thread, so it's never cached
static char tst_qoi_path_order  [512]; // loaded only by the ordering test, so it's never cached

static void tst_blocking_file() {
	tex_t tex = tex_create_file(tst_qoi_path, tex_data_srgb | tex_data_blocking);
	TST_CHECK(tex_asset_state(tex) == asset_state_loaded, "a blocking file load is loaded on return");
	TST_CHECK(tex_get_width(tex) == 128,                  "a blocking file load has its size on return");
	tex_release(tex);

	tex_t missing = tex_create_file("tex_tests_no_such_file.qoi", tex_data_srgb | tex_data_blocking);
	TST_CHECK(tex_asset_state(missing) < 0, "a blocking load of a missing file fails on return");
	tex_release(missing);
}

struct tst_foreign_load_t {
	tex_t        tex;
	asset_state_ state_on_return;
};
static void tst_foreign_load(void* arg) {
	tst_foreign_load_t* job = (tst_foreign_load_t*)arg;
	tex_t        tex   = tex_create_file(tst_qoi_path_foreign, tex_data_srgb | tex_data_uncompressed | tex_data_blocking);
	asset_state_ state = tex_asset_state(tex);
	job->tex = tex;
	atomic_store_i32_rel((int32_t*)&job->state_on_return, (int32_t)state);
}

static void tst_blocking_foreign_thread() {
	tst_foreign_load_t job = {};
	tst_thread_t thread = tst_thread_start(tst_foreign_load, &job);
	// The main thread keeps stepping, like an app would
	for (int32_t i = 0; i < 600 && atomic_load_i32_acq((int32_t*)&job.state_on_return) == asset_state_none; i++) sk_step(nullptr);
	tst_thread_join(thread);
	TST_CHECK(job.state_on_return == asset_state_loaded, "a blocking file load from a foreign thread is loaded on return");
	tex_release(job.tex);
}

static void tst_blocking_model() {
	char path[600];
	tst_asset_path("Radio.glb", path, sizeof(path));
	FILE* fp = fopen(path, "rb");
	if (fp == nullptr) { log_warnf("[tex_test] skipping the model test, no %s", path); return; }
	fclose(fp);

	model_t model = model_create_file(path, nullptr, 10, tex_data_uncompressed | tex_data_blocking);
	TST_CHECK(model_asset_state(model) == asset_state_loaded, "a blocking model load is loaded on return");

	bool    all_loaded = true;
	int32_t textures   = 0;
	for (int32_t v = 0; v < model_node_visual_count(model); v++) {
		material_t mat = model_node_get_material(model, model_node_visual_index(model, v));
		for (int32_t p = 0; p < material_get_param_count(mat); p++) {
			char* name; material_param_ type;
			material_get_param_info(mat, p, &name, &type);
			if (type != material_param_texture) continue;
			tex_t tex = material_get_texture(mat, name);
			if (tex == nullptr) continue;
			textures  += 1;
			all_loaded = all_loaded && tex_asset_state(tex) == asset_state_loaded;
			tex_release(tex);
		}
		material_release(mat);
	}
	TST_CHECK(textures > 0 && all_loaded, "a blocking model's textures are loaded on return");
	model_release(model);
}

///////////////////////////////////////////
// Upload paths
///////////////////////////////////////////

static void tst_set_size() {
	tex_t rt = tex_create_rendertarget(64, 64, 1, tex_format_rgba32, tex_format_depth16);
	tex_set_size(rt, 128, 96);
	tex_t depth = tex_get_zbuffer(rt);
	TST_CHECK(tex_get_width(rt) == 128 && tex_get_height(rt) == 96,       "tex_set_size resizes a rendertarget");
	TST_CHECK(depth != nullptr && tex_get_width(depth) == 128 && tex_get_height(depth) == 96, "a rendertarget's depth buffer follows its size");
	tex_release(depth);

	tex_t legacy = tex_create(tex_type_image_nomips | tex_type_rendertarget, tex_format_rgba32);
	tex_set_colors(legacy, 32, 16, nullptr);
	TST_CHECK(tex_get_width(legacy) == 32 && tex_asset_state(legacy) == asset_state_loaded, "null data in set_colors still resizes");
	tex_release(legacy);
	tex_release(rt);
}

static void tst_volume() {
	const int32_t size = 8;
	color32* px = (color32*)malloc(sizeof(color32) * size * size * size);
	tst_fill(px, size * size * size, color32{ 10, 20, 30, 255 });

	tex_t async = tex_create(tex_type_image_nomips | tex_type_volume, tex_format_rgba32);
	tex_set_colors_3d(async, size, size, size, px);
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_asset_state(async) == asset_state_loaded && tex_get_depth(async) == size, "an async volume upload loads");
	TST_CHECK(tex_get_format(async) == tex_format_rgba32,                                  "volumes are never compressed");

	tex_t blocking = tex_create(tex_type_image_nomips | tex_type_volume, tex_format_rgba32);
	tex_set_colors_3d(blocking, size, size, size, px, tex_data_blocking);
	TST_CHECK(tex_asset_state(blocking) == asset_state_loaded, "a blocking volume upload is loaded on return");

	tex_t empty = tex_create(tex_type_image_nomips | tex_type_volume, tex_format_rgba32);
	tex_set_colors_3d(empty, size, size, size, nullptr);
	TST_CHECK(tex_get_depth(empty) == size, "a volume can be sized without data");
	tex_set_size(empty, size, size, size * 2);
	TST_CHECK(tex_get_depth(empty) == size * 2, "tex_set_size resizes a volume's depth");

	tex_release(empty);
	tex_release(blocking);
	tex_release(async);
	free(px);
}

static void tst_dynamic() {
	const int32_t size = 32;
	color32 px[size * size];
	tex_t tex = tex_create(tex_type_image_nomips | tex_type_dynamic, tex_format_rgba32);
	for (int32_t i = 0; i < 20; i++) {
		tst_fill(px, size * size, color32{ (uint8_t)(100 + i), 0, 0, 255 });
		tex_set_colors(tex, size, size, px, tex_data_srgb | tex_data_quality);
		sk_step(nullptr);
	}
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_get_format(tex) == tex_format_rgba32,  "dynamic textures ignore compression flags");
	TST_CHECK(tst_read_first(tex, size).r == 119,        "async updates to a dynamic texture land in order");
	tex_release(tex);
}

static void tst_data_format() {
	const int32_t size = 16;
	float   hdr[size * size * 4];
	color32 ldr[size * size];
	for (int32_t i = 0; i < size * size * 4; i++) hdr[i] = 0.5f;
	tst_fill(ldr, size * size, color32{ 1, 2, 3, 255 });

	tex_t tex = tex_create(tex_type_image_nomips, tex_format_rgba32);
	tex_set_colors(tex, size, size, hdr, tex_data_uncompressed | tex_data_blocking, tex_format_rgba128);
	TST_CHECK(tex_get_format(tex) == tex_format_rgba128, "an explicit data format changes an uncompressed texture's format");
	tex_set_colors(tex, size, size, ldr, tex_data_uncompressed | tex_data_blocking);
	TST_CHECK(tex_get_format(tex) == tex_format_rgba32,  "an explicit data format doesn't stick to later calls");
	TST_CHECK(tst_read_first(tex, size).b == 3,          "data in the creation format reads back after a format change");
	tex_release(tex);
}

static void tst_release_pending() {
	const int32_t size = 64;
	color32* px = (color32*)malloc(sizeof(color32) * size * size);
	tst_fill(px, size * size, color32{ 9, 9, 9, 255 });
	for (int32_t t = 0; t < 8; t++) {
		tex_t tex = tex_create(tex_type_image, tex_format_rgba32);
		for (int32_t i = 0; i < 4; i++) tex_set_colors(tex, size, size, px);
		tex_release(tex);
	}
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(true, "releasing textures with uploads pending finishes cleanly");
	free(px);
}

///////////////////////////////////////////
// Write ordering
///////////////////////////////////////////

static void tst_order() {
	const int32_t size = 32;
	color32 px[size * size];
	tst_fill(px, size * size, color32{ 40, 50, 60, 255 });

	tex_t resized = tex_create(tex_type_image_nomips | tex_type_rendertarget, tex_format_rgba32);
	tex_set_colors(resized, size, size, px, tex_data_srgb | tex_data_uncompressed);
	tex_set_size  (resized, 128, 96);
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_get_width(resized) == 128 && tex_get_height(resized) == 96, "a resize isn't undone by an earlier async upload");
	tex_release(resized);

	tex_t loading = tex_create_file(tst_qoi_path_order, tex_data_srgb | tex_data_uncompressed);
	tex_set_colors(loading, size, size, px, tex_data_srgb | tex_data_uncompressed | tex_data_blocking);
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_get_width(loading) == size && tst_read_first(loading, size).g == 50, "data set during a file load isn't replaced by the file");
	tex_release(loading);

	tex_t failing = tex_create_file("tex_tests_missing_order.qoi", tex_data_srgb | tex_data_uncompressed);
	tex_set_colors(failing, size, size, px, tex_data_srgb | tex_data_uncompressed | tex_data_blocking);
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_asset_state(failing) == asset_state_loaded, "a failed load doesn't mark data set after it as failed");
	tex_release(failing);

	tex_t src  = tex_create_rendertarget(64, 64, 1, tex_format_rgba32, tex_format_none);
	tex_t dest = tex_create(tex_type_image_nomips, tex_format_rgba32);
	tex_set_colors(dest, size, size, px, tex_data_srgb | tex_data_uncompressed);
	tex_copy(src, tex_type_image_nomips, tex_format_none, tex_data_uncompressed, dest);
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tex_get_width(dest) == 64, "a copy isn't replaced by an earlier async upload");
	tex_release(dest);
	tex_release(dest);
	tex_release(src);
}

///////////////////////////////////////////
// Threading
///////////////////////////////////////////

struct tst_stress_t {
	tex_t   tex;
	int32_t id;
	int32_t done;
};
static void tst_stress_body(void* arg) {
	tst_stress_t* job = (tst_stress_t*)arg;
	const int32_t size = 32;
	color32 px[size * size];
	for (int32_t i = 0; i < 40; i++) {
		tst_fill(px, size * size, color32{ (uint8_t)job->id, (uint8_t)i, 0, 255 });
		tex_data_ flags = (i % 3 == 0)
			? tex_data_srgb | tex_data_uncompressed | tex_data_blocking
			: tex_data_srgb | tex_data_uncompressed;
		tex_set_colors(job->tex, size, size, px, flags);
	}
	atomic_store_i32_rel(&job->done, 1);
}

static void tst_mixed_threads() {
	const int32_t threads = 4;
	tex_t        tex = tex_create(tex_type_image_nomips, tex_format_rgba32);
	static tst_stress_t jobs[threads]; // outlives this call if a deadlock leaves the threads running
	tst_thread_t        handles[threads];
	for (int32_t t = 0; t < threads; t++) {
		jobs[t]    = { tex, t + 1, 0 };
		handles[t] = tst_thread_start(tst_stress_body, &jobs[t]);
	}
	// Bounded, so a deadlock fails the test instead of hanging it
	bool finished = false;
	for (int32_t f = 0; f < 3000 && !finished; f++) {
		sk_step(nullptr);
		finished = true;
		for (int32_t t = 0; t < threads; t++) finished = finished && atomic_load_i32_acq(&jobs[t].done);
	}
	TST_CHECK(finished, "mixed blocking and async updates from several threads finish");
	if (!finished) return; // joining would hang on the deadlock
	for (int32_t t = 0; t < threads; t++) tst_thread_join(handles[t]);
	assets_block_for_priority(INT32_MAX);

	color32 last = tst_read_first(tex, 32);
	TST_CHECK(last.r >= 1 && last.r <= threads && last.g == 39, "the texture ends on some thread's last update");
	tex_release(tex);
}

// A visible texture replaced every frame, while the main thread draws it.
static void tst_swap_while_drawn() {
	const int32_t size = 64;
	color32* px = (color32*)malloc(sizeof(color32) * size * size);
	tex_t      tex  = tex_create(tex_type_image_nomips, tex_format_rgba32);
	tst_fill(px, size * size, color32{ 0, 0, 0, 255 });
	tex_set_colors(tex, size, size, px, tex_data_srgb | tex_data_uncompressed | tex_data_blocking);

	material_t base = material_find(default_id_material);
	material_t mat  = material_copy(base);
	mesh_t     quad = mesh_find(default_id_mesh_quad);
	material_set_texture(mat, "diffuse", tex);

	for (int32_t f = 0; f < 240; f++) {
		tst_fill(px, size * size, color32{ (uint8_t)f, 0, 0, 255 });
		tex_set_colors(tex, size, size, px, tex_data_srgb | tex_data_uncompressed);
		render_add_mesh(quad, mat, matrix_t({ 0, 0, -0.5f }));
		sk_step(nullptr);
	}
	assets_block_for_priority(INT32_MAX);
	TST_CHECK(tst_read_first(tex, size).r == 239, "a texture replaced while drawn ends on its last update");

	mesh_release    (quad);
	material_release(mat);
	material_release(base);
	tex_release     (tex);
	free(px);
}

///////////////////////////////////////////

int tex_tests_run() {
	tst_failures = 0;
	log_info("[tex_test] Running texture tests");

	// Absolute, since relative paths resolve against the asset folder
	char cwd[400] = ".";
#if defined(_WIN32)
	GetCurrentDirectoryA(sizeof(cwd), cwd);
#else
	if (getcwd(cwd, sizeof(cwd)) == nullptr) cwd[0] = '.';
#endif
	snprintf(tst_qoi_path,         sizeof(tst_qoi_path),         "%s/tex_tests_a.qoi", cwd);
	snprintf(tst_qoi_path_foreign, sizeof(tst_qoi_path_foreign), "%s/tex_tests_b.qoi", cwd);
	snprintf(tst_qoi_path_order,   sizeof(tst_qoi_path_order),   "%s/tex_tests_c.qoi", cwd);
	if (!tst_write_qoi(tst_qoi_path, 128) || !tst_write_qoi(tst_qoi_path_foreign, 128) || !tst_write_qoi(tst_qoi_path_order, 128)) {
		TST_CHECK(false, "write a temp image for the file tests");
		return tst_failures;
	}

	sk_settings_t settings = {};
	settings.app_name      = "StereoKitC Texture Tests";
	settings.mode          = app_mode_offscreen;
	settings.standby_mode  = standby_mode_none;
	if (sk_init(settings)) {
		tst_blocking_file();
		tst_blocking_foreign_thread();
		tst_blocking_model();
		tst_set_size();
		tst_volume();
		tst_dynamic();
		tst_data_format();
		tst_release_pending();
		tst_order();
		tst_mixed_threads();
		tst_swap_while_drawn();

		// Shutdown with an upload still queued must clean up after it
		color32 px[16 * 16] = {};
		tex_t pending = tex_create(tex_type_image, tex_format_rgba32);
		tex_set_colors(pending, 16, 16, px);
		tex_release(pending);
		sk_shutdown();
	} else {
		TST_CHECK(false, "sk_init for the texture tests");
	}
	remove(tst_qoi_path);
	remove(tst_qoi_path_foreign);
	remove(tst_qoi_path_order);

	log_infof("[tex_test] %d failure(s)", tst_failures);
	return tst_failures;
}
