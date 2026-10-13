// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "future_tests.h"

#include <stereokit.h>

// Internal headers, reached through StereoKitC's public include root
#include <libraries/atomic_util.h>
#include <libraries/ferr_thread.h>

using namespace sk;

///////////////////////////////////////////

static int ft_failures = 0;

#define FT_CHECK(condition, description) do { \
	if (condition) { log_infof("[future_test] pass: %s", description); } \
	else           { log_errf ("[future_test] FAIL: %s", description); ft_failures += 1; } \
	} while (0)

///////////////////////////////////////////

static void ft_test_zero() {
	FT_CHECK(future_check({}) == future_state_none, "the zero future checks as none");
	FT_CHECK(future_wait ({}) == future_state_none, "waiting on the zero future returns none right away");
}

static void ft_test_wait() {
	future_t future = render_gpu_future();
	FT_CHECK(future_check(future) != future_state_none,    "render_gpu_future makes a valid future");
	FT_CHECK(future_wait (future) == future_state_ready,   "waiting on a GPU future ends ready");
	FT_CHECK(future_check(future) == future_state_ready,   "a waited future stays ready");
}

static void ft_test_poll() {
	future_t future = render_gpu_future();
	int32_t  frames = 0;
	while (future_check(future) == future_state_pending && frames < 60) {
		sk_step(nullptr);
		frames++;
	}
	FT_CHECK(future_check(future) == future_state_ready, "a polled GPU future finishes within a few frames");
}

static void ft_test_stale() {
	future_t old = render_gpu_future();
	future_wait(old);
	// More than the starting pool, so the old slot gets reused
	for (int32_t i = 0; i < 64; i++) future_wait(render_gpu_future());
	FT_CHECK(future_check(old) == future_state_ready, "a handle to a reused slot reads ready");
}

///////////////////////////////////////////

#define FT_THREADS    6
#define FT_ITERATIONS 300

struct ft_thread_data_t {
	int32_t failures;
	int32_t finished;
};

// Half the futures get waited on, half get polled, so both paths race slot reuse
static int32_t ft_thread(void* arg) {
	ft_thread_data_t* data = (ft_thread_data_t*)arg;
	for (int32_t i = 0; i < FT_ITERATIONS; i++) {
		future_t future = render_gpu_future();
		if (i % 2 == 0) {
			if (future_wait(future) != future_state_ready) atomic_increment(&data->failures);
		} else {
			while (future_check(future) == future_state_pending) ft_yield();
		}
		if (future_check(future) != future_state_ready) atomic_increment(&data->failures);
	}
	atomic_increment(&data->finished);
	return 0;
}

static void ft_test_threads() {
	ft_thread_data_t data = {};
	for (int32_t t = 0; t < FT_THREADS; t++)
		ft_thread_create(ft_thread, &data);

	// Frames keep submitting real work while the threads race for slots
	while (atomic_load_i32_acq(&data.finished) < FT_THREADS)
		sk_step(nullptr);

	FT_CHECK(atomic_load_i32(&data.failures) == 0, "threads making, waiting on, and polling futures all see them finish ready");
}

///////////////////////////////////////////

struct ft_shutdown_data_t {
	int32_t stop;
	int32_t waits;
	int32_t failures;
	int32_t finished;
};

// Keeps waiting on fresh futures straight through sk_shutdown
static int32_t ft_shutdown_thread(void* arg) {
	ft_shutdown_data_t* data = (ft_shutdown_data_t*)arg;
	while (atomic_load_i32_acq(&data->stop) == 0) {
		if (future_wait(render_gpu_future()) == future_state_pending) atomic_increment(&data->failures);
		atomic_increment(&data->waits);
	}
	atomic_increment(&data->finished);
	return 0;
}

// Runs its own sk_init/sk_shutdown pairs, after the main session ends
static void ft_test_shutdown(sk_settings_t settings) {
	if (!sk_init(settings)) { FT_CHECK(false, "StereoKit re-initializes for the shutdown tests"); return; }
	sk_step(nullptr);

	ft_shutdown_data_t data = {};
	for (int32_t t = 0; t < FT_THREADS; t++)
		ft_thread_create(ft_shutdown_thread, &data);
	while (atomic_load_i32_acq(&data.waits) < 100) sk_step(nullptr);

	future_t before = render_gpu_future();
	sk_shutdown();
	atomic_store_i32_rel(&data.stop, 1);
	while (atomic_load_i32_acq(&data.finished) < FT_THREADS) ft_yield();

	FT_CHECK(atomic_load_i32(&data.failures) == 0, "threads waiting through sk_shutdown never get pending back");
	FT_CHECK(future_check(before)              == future_state_failed, "a future from before shutdown reads failed after it");
	FT_CHECK(future_check(render_gpu_future()) == future_state_none,   "render_gpu_future makes no future once StereoKit is shut down");

	if (!sk_init(settings)) { FT_CHECK(false, "StereoKit initializes a second session"); return; }
	FT_CHECK(future_check(before) == future_state_failed,            "an old session's future reads failed in a new session");
	FT_CHECK(future_wait(render_gpu_future()) == future_state_ready, "a new session's futures work");
	// Shutting down before startup's reflection task finishes crashes, a separate bug
	for (int32_t i = 0; i < 120; i++) sk_step(nullptr);
	sk_shutdown();
}

///////////////////////////////////////////

int future_tests_run() {
	sk_settings_t settings = {};
	settings.app_name      = "StereoKitC Future Tests";
	settings.mode          = app_mode_offscreen;
	settings.standby_mode  = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[future_test] sk_init failed");
		return 1;
	}
	sk_step(nullptr);

	ft_test_zero   ();
	ft_test_wait   ();
	ft_test_poll   ();
	ft_test_stale  ();
	ft_test_threads();

	sk_shutdown();
	ft_test_shutdown(settings);

	if (ft_failures == 0) log_info ("[future_test] all tests passed!");
	else                  log_errf ("[future_test] %d failure(s)", ft_failures);
	return ft_failures;
}
