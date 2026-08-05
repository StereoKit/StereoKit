// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "asset_tests.h"

#include <stereokit.h>

// Internal headers, reached through StereoKitC's public include root. The
// synthetic tasks here give controlled timing that real file loads can't.
#include <asset_types/assets.h>
#include <libraries/atomic_util.h>

#include <stdio.h>
#include <limits.h>

#if defined(_WIN32)
#include <windows.h>
static void ast_sleep_ms(int32_t ms) { Sleep(ms); }
#else
#include <unistd.h>
#include <pthread.h>
static void ast_sleep_ms(int32_t ms) { usleep(ms * 1000); }
#endif

using namespace sk;

///////////////////////////////////////////

static int ast_failures = 0;

#define AST_CHECK(condition, description) do { \
	if (condition) { log_infof("[asset_test] pass: %s", description); } \
	else           { log_errf ("[asset_test] FAIL: %s", description); ast_failures += 1; } \
	} while (0)

///////////////////////////////////////////

// Shared between the test's thread and task actions on asset threads.
// dep_state_at_run snapshots the dependency the moment the action runs.
struct ast_ctx_t {
	asset_header_t* dep;             // dependency observed by the action
	int32_t         ran;             // times the action executed
	int32_t         failed;          // times on_failure executed
	int32_t         hold;            // ast_action_hold spins until set
	int32_t         timed_out;       // ast_action_hold gave up waiting
	int32_t         dep_state_at_run;
};

static asset_header_t* ast_header(tex_t tex) { return (asset_header_t*)tex; }

///////////////////////////////////////////

static bool32_t ast_action_load(asset_task_t*, asset_header_t* asset, void* data) {
	ast_ctx_t* ctx = (ast_ctx_t*)data;
	if (ctx->dep != nullptr)
		atomic_store_i32(&ctx->dep_state_at_run, ctx->dep->state);
	asset->state = asset_state_loaded;
	atomic_increment(&ctx->ran);
	return true;
}

static bool32_t ast_action_load_slow(asset_task_t* task, asset_header_t* asset, void* data) {
	ast_sleep_ms(100);
	return ast_action_load(task, asset, data);
}

static bool32_t ast_action_meta(asset_task_t*, asset_header_t* asset, void*) {
	asset->state = asset_state_loaded_meta;
	return true;
}

// Parks the dependency's task mid-load so a test can assert its dependents
// ran at loaded_meta. Times out rather than hanging the suite on a bug.
static bool32_t ast_action_hold(asset_task_t* task, asset_header_t* asset, void* data) {
	ast_ctx_t* ctx    = (ast_ctx_t*)data;
	int32_t    waited = 0;
	while (atomic_load_i32_acq(&ctx->hold) == 0 && waited < 5000) {
		ast_sleep_ms(1);
		waited += 1;
	}
	if (waited >= 5000)
		atomic_store_i32_rel(&ctx->timed_out, 1);
	return ast_action_load(task, asset, data);
}

static bool32_t ast_action_fail(asset_task_t*, asset_header_t*, void*) {
	return false;
}

static void ast_on_failure(asset_header_t* asset, void* data) {
	asset->state = asset_state_error;
	atomic_increment(&((ast_ctx_t*)data)->failed);
}

///////////////////////////////////////////

static void ast_add_task(tex_t asset, ast_ctx_t* ctx, const asset_load_action_t* actions, int32_t action_count, tex_t depends_on, asset_state_ depends_state) {
	asset_task_t task  = {};
	task.asset         = ast_header(asset);
	task.load_data     = ctx;
	task.on_failure    = ast_on_failure;
	task.actions       = (asset_load_action_t*)actions;
	task.action_count  = action_count;
	task.priority      = 10;
	task.sort          = asset_sort(10, 0);
	task.depends_on    = depends_on ? ast_header(depends_on) : nullptr;
	task.depends_state = depends_state;
	assets_add_task(task);
}

///////////////////////////////////////////

// Waits for a cross-thread counter to reach `target`, pumping the main
// thread's asset step (on_load events, wake-up backstop) like a frame loop.
static bool ast_wait(volatile int32_t* counter, int32_t target, int32_t timeout_ms) {
	int32_t waited = 0;
	while (atomic_load_i32_acq(counter) < target && waited < timeout_ms) {
		sk_step(nullptr);
		ast_sleep_ms(1);
		waited += 1;
	}
	return atomic_load_i32_acq(counter) >= target;
}

///////////////////////////////////////////

static void ast_test_dep_satisfied() {
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loaded;

	static const asset_load_action_t actions[] = { ast_action_load };
	ast_ctx_t ctx = {};
	ctx.dep = ast_header(dep);
	ast_add_task(dependent, &ctx, actions, 1, dep, asset_state_loaded);

	AST_CHECK(ast_wait(&ctx.ran, 1, 3000),                         "pre-satisfied dependency runs immediately");
	AST_CHECK(ctx.dep_state_at_run >= asset_state_loaded,          "pre-satisfied dependency was loaded at run");
	AST_CHECK(ast_header(dependent)->state == asset_state_loaded,  "dependent reached loaded");

	tex_release(dep);
	tex_release(dependent);
}

///////////////////////////////////////////

static void ast_test_dep_completes_later() {
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loading;

	// The dependent is queued first, and must wait on the slow dependency.
	static const asset_load_action_t dependent_actions[] = { ast_action_load };
	static const asset_load_action_t dep_actions[]       = { ast_action_load_slow };
	ast_ctx_t dependent_ctx = {};
	ast_ctx_t dep_ctx       = {};
	dependent_ctx.dep = ast_header(dep);
	ast_add_task(dependent, &dependent_ctx, dependent_actions, 1, dep, asset_state_loaded);
	ast_add_task(dep,       &dep_ctx,       dep_actions,       1, nullptr, asset_state_none);

	AST_CHECK(ast_wait(&dependent_ctx.ran, 1, 3000),                  "dependent runs after dependency completes");
	AST_CHECK(dependent_ctx.dep_state_at_run >= asset_state_loaded,   "dependency was loaded when dependent ran");

	tex_release(dep);
	tex_release(dependent);
}

///////////////////////////////////////////

static void ast_test_dep_errors() {
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loading;

	static const asset_load_action_t dependent_actions[] = { ast_action_load };
	static const asset_load_action_t dep_actions[]       = { ast_action_fail };
	ast_ctx_t dependent_ctx = {};
	ast_ctx_t dep_ctx       = {};
	dependent_ctx.dep = ast_header(dep);
	ast_add_task(dependent, &dependent_ctx, dependent_actions, 1, dep, asset_state_loaded);
	ast_add_task(dep,       &dep_ctx,       dep_actions,       1, nullptr, asset_state_none);

	AST_CHECK(ast_wait(&dependent_ctx.failed, 1, 3000),              "errored dependency fires dependent's on_failure");
	AST_CHECK(atomic_load_i32_acq(&dependent_ctx.ran) == 0,          "errored dependency never runs the action");
	AST_CHECK(ast_header(dependent)->state == asset_state_error,     "on_failure owned the dependent's state");

	tex_release(dep);
	tex_release(dependent);
}

///////////////////////////////////////////

static void ast_test_dep_meta() {
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loading;

	// The dependency parks after publishing meta, so the dependent can only
	// pass this test by running in that window.
	static const asset_load_action_t dependent_actions[] = { ast_action_load };
	static const asset_load_action_t dep_actions[]       = { ast_action_meta, ast_action_hold };
	ast_ctx_t dependent_ctx = {};
	ast_ctx_t dep_ctx       = {};
	dependent_ctx.dep = ast_header(dep);
	ast_add_task(dependent, &dependent_ctx, dependent_actions, 1, dep, asset_state_loaded_meta);
	ast_add_task(dep,       &dep_ctx,       dep_actions,       2, nullptr, asset_state_none);

	bool ran_at_meta = ast_wait(&dependent_ctx.ran, 1, 3000);
	AST_CHECK(ran_at_meta,                                                  "meta dependency releases dependent at loaded_meta");
	AST_CHECK(dependent_ctx.dep_state_at_run == asset_state_loaded_meta,    "dependency was exactly loaded_meta at run");

	atomic_store_i32_rel(&dep_ctx.hold, 1);
	AST_CHECK(ast_wait(&dep_ctx.ran, 1, 6000),                              "held dependency finishes after release");
	// A timeout here means starvation, and explains an exact-state failure
	// above as the machine's fault rather than the scheduler's.
	AST_CHECK(atomic_load_i32_acq(&dep_ctx.timed_out) == 0,                 "hold released before its timeout");

	tex_release(dep);
	tex_release(dependent);
}

///////////////////////////////////////////

static void ast_test_many_dependents() {
	const int32_t count = 8;

	tex_t dep = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loading;

	// Half gate on meta, half on loaded; the dependency publishes meta then
	// dawdles to loaded. Worker scheduling supplies the completion order.
	static const asset_load_action_t dependent_actions[] = { ast_action_load };
	static const asset_load_action_t dep_actions[]       = { ast_action_meta, ast_action_load_slow };
	tex_t     dependents[count];
	ast_ctx_t ctxs      [count] = {};
	for (int32_t i = 0; i < count; i++) {
		dependents[i] = tex_create(tex_type_image, tex_format_rgba32);
		ctxs      [i] = {};
		ctxs      [i].dep = ast_header(dep);
		ast_add_task(dependents[i], &ctxs[i], dependent_actions, 1, dep, (i & 1) ? asset_state_loaded : asset_state_loaded_meta);
	}
	ast_ctx_t dep_ctx = {};
	ast_add_task(dep, &dep_ctx, dep_actions, 2, nullptr, asset_state_none);

	bool all_ran    = true;
	bool all_gated  = true;
	for (int32_t i = 0; i < count; i++) {
		if (!ast_wait(&ctxs[i].ran, 1, 3000)) { all_ran = false; break; }
		asset_state_ needed = (i & 1) ? asset_state_loaded : asset_state_loaded_meta;
		if (ctxs[i].dep_state_at_run < needed) all_gated = false;
	}
	AST_CHECK(all_ran,   "all dependents on one asset ran");
	AST_CHECK(all_gated, "every dependent saw at least its gate state");

	tex_release(dep);
	for (int32_t i = 0; i < count; i++)
		tex_release(dependents[i]);
}

///////////////////////////////////////////

// A dependency at asset_state_none has no load in flight, so nothing will
// ever advance it. The gate must fail open, not stall the task forever.
static void ast_test_dep_never_loading() {
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);

	static const asset_load_action_t actions[] = { ast_action_load };
	ast_ctx_t ctx = {};
	ctx.dep = ast_header(dep);
	ast_add_task(dependent, &ctx, actions, 1, dep, asset_state_loaded);

	AST_CHECK(ast_wait(&ctx.failed, 1, 3000),               "never-loading dependency fails the dependent promptly");
	AST_CHECK(atomic_load_i32_acq(&ctx.ran) == 0,           "never-loading dependency skips the action");

	// The public shape of the same bug: an empty tex spritized, then the
	// idiomatic wait-for-everything call. Both used to hang forever.
	sprite_t sprite = sprite_create(dep, sprite_type_single, "default");
	assets_block_for_priority(INT_MAX);
	AST_CHECK(((asset_header_t*)sprite)->state == asset_state_loaded, "sprite on an empty tex still reaches loaded");

	sprite_release(sprite);
	tex_release(dep);
	tex_release(dependent);
}

///////////////////////////////////////////

static void ast_test_self_dependency() {
	tex_t tex = tex_create(tex_type_image, tex_format_rgba32);

	// The guard logs an error, drops the dependency, and runs the task
	// ungated, so the log_err below this is expected output.
	static const asset_load_action_t actions[] = { ast_action_load };
	ast_ctx_t ctx = {};
	ast_add_task(tex, &ctx, actions, 1, tex, asset_state_loaded);

	AST_CHECK(ast_wait(&ctx.ran, 1, 3000),                  "self-dependency is dropped and the task runs");
	AST_CHECK(ast_header(tex)->state == asset_state_loaded, "self-dependent task completed normally");

	tex_release(tex);
}

///////////////////////////////////////////

// tex_set_colors from a thread StereoKit doesn't know about exercises
// assets_execute_blocking's queue-and-wait path, other threads run it inline.
struct ast_foreign_t {
	tex_t   tex;
	int32_t done;
};
static void ast_foreign_body(void* arg) {
	ast_foreign_t* job = (ast_foreign_t*)arg;
	color32 colors[16 * 16];
	for (int32_t i = 0; i < 16 * 16; i++)
		colors[i] = color32{ 255, 0, 255, 255 };
	tex_set_colors(job->tex, 16, 16, colors);
	atomic_store_i32_rel(&job->done, 1);
}
#if defined(_WIN32)
typedef HANDLE ast_thread_t;
static DWORD WINAPI ast_foreign_entry(LPVOID arg) { ast_foreign_body(arg); return 0; }
static ast_thread_t ast_foreign_start(void* arg) {
	return CreateThread(nullptr, 0, ast_foreign_entry, arg, 0, nullptr);
}
static void ast_foreign_join(ast_thread_t thread) {
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
}
#else
typedef pthread_t ast_thread_t;
static void* ast_foreign_entry(void* arg) { ast_foreign_body(arg); return nullptr; }
static ast_thread_t ast_foreign_start(void* arg) {
	pthread_t thread;
	pthread_create(&thread, nullptr, ast_foreign_entry, arg);
	return thread;
}
static void ast_foreign_join(ast_thread_t thread) { pthread_join(thread, nullptr); }
#endif

static void ast_test_blocking_job_foreign_thread() {
	ast_foreign_t job = {};
	job.tex = tex_create(tex_type_image, tex_format_rgba32);

	// Watchdog wait before the join: a hang here becomes a labeled failure
	// instead of a stuck suite, at the cost of leaking the thread and tex.
	ast_thread_t thread = ast_foreign_start(&job);
	bool         done   = ast_wait(&job.done, 1, 10000);
	AST_CHECK(done, "blocking job from a foreign thread completes");
	if (!done) return;

	ast_foreign_join(thread);
	AST_CHECK(tex_get_width(job.tex) == 16 && tex_get_height(job.tex) == 16, "foreign thread set applied");
	AST_CHECK(ast_header(job.tex)->state >= asset_state_loaded,              "foreign thread set left the texture loaded");
	tex_release(job.tex);
}

///////////////////////////////////////////

int asset_tests_run() {
	sk_settings_t settings = {};
	settings.app_name      = "StereoKitC Asset Tests";
	settings.mode          = app_mode_offscreen;
	settings.standby_mode  = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[asset_test] sk_init failed");
		return 1;
	}

	ast_test_dep_satisfied     ();
	ast_test_dep_completes_later();
	ast_test_dep_errors        ();
	ast_test_dep_meta          ();
	ast_test_many_dependents   ();
	ast_test_dep_never_loading ();
	ast_test_self_dependency   ();
	ast_test_blocking_job_foreign_thread();

	// Shutdown with a never-resolving dependency must not hang the drain,
	// and the dependent takes its failure path. ctx must outlive sk_shutdown.
	tex_t dep       = tex_create(tex_type_image, tex_format_rgba32);
	tex_t dependent = tex_create(tex_type_image, tex_format_rgba32);
	ast_header(dep)->state = asset_state_loading;

	static const asset_load_action_t dependent_actions[] = { ast_action_load };
	ast_ctx_t ctx = {};
	ctx.dep = ast_header(dep);
	ast_add_task(dependent, &ctx, dependent_actions, 1, dep, asset_state_loaded);
	tex_release(dep);
	tex_release(dependent);

	sk_shutdown();

	AST_CHECK(atomic_load_i32_acq(&ctx.failed) == 1, "shutdown routes unresolved gate through on_failure");
	AST_CHECK(atomic_load_i32_acq(&ctx.ran)    == 0, "unresolved gate never runs the action");

	if (ast_failures == 0) log_info("[asset_test] all tests passed!");
	else                   log_errf("[asset_test] %d checks failed!", ast_failures);
	return ast_failures;
}
