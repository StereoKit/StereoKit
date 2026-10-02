// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "frame_pacer.h"
#include "../_stereokit.h"
#include "../device.h"

///////////////////////////////////////////

// A present that hasn't shown after this long is not coming; don't hang on it
#define PACER_WAIT_TIMEOUT_NS 100000000ull

namespace sk {

///////////////////////////////////////////

static const frame_pacer_t* pacer_main = nullptr;

///////////////////////////////////////////

void frame_pacer_step(uint64_t display_ns, uint64_t prev_display_ns) {
	bool paced = display_ns != 0 && prev_display_ns != 0 && display_ns > prev_display_ns;
	time_step_next(paced ? display_ns - prev_display_ns : 0);
}

///////////////////////////////////////////

static uint32_t frame_pacer_run_ahead(bool fullscreen) {
#if defined(SK_OS_ANDROID)
	(void)fullscreen;
	return 2;
#else
	return fullscreen ? 2 : 0;
#endif
}

///////////////////////////////////////////

void frame_pacer_begin(frame_pacer_t* ref_pacer, skr_surface_t* ref_surface, const ska_window_t* window, bool fullscreen) {
	// Re-sent every frame since the window can move to a monitor with a
	// different rate. The vblank crosses from sk_app's clock as an age.
	float    refresh_hz = ska_window_get_refresh_rate(window);
	uint64_t vblank_ns  = ska_window_get_vblank_ns   (window);
	skr_surface_set_refresh(ref_surface, refresh_hz > 0.0f ? (uint64_t)(1e9 / refresh_hz) : 0);
	skr_surface_set_vblank (ref_surface, vblank_ns ? skr_time_now_ns() - (ska_time_get_elapsed_ns() - vblank_ns) : 0);
	uint64_t refresh_ns = skr_surface_get_refresh_ns(ref_surface);
	uint64_t next_id    = skr_surface_get_next_id   (ref_surface);
	ref_pacer->refresh_ns            = refresh_ns;
	device_data.display_refresh_rate = 1e9f / (float)refresh_ns;

	// Waiting on the present before the allowed ones caps the queue depth
	uint32_t ahead = frame_pacer_run_ahead(fullscreen);
	if (ahead != 0 && next_id > ahead)
		skr_surface_wait_present(ref_surface, next_id - ahead, PACER_WAIT_TIMEOUT_NS);
}

///////////////////////////////////////////

void frame_pacer_end(frame_pacer_t* ref_pacer, skr_surface_t* ref_surface) {
	skr_present_info_t presents[16];
	int32_t            count = skr_surface_present_history(ref_surface, presents, 16);
	for (int32_t i = 0; i < count; i++) {
		const skr_present_info_t* p    = &presents[i];
		frame_pacer_present_t*    slot = &ref_pacer->history[p->id % FRAME_PACER_HISTORY];
		*slot = { p->id, p->cpu_present_ns, p->display_ns };
		if (p->id > ref_pacer->newest_id) ref_pacer->newest_id = p->id;

		// A floor anchor is a guess for its id, the report for it is the truth
		bool newer = p->id > ref_pacer->anchor_id || (ref_pacer->anchor_is_floor && p->id == ref_pacer->anchor_id);
		if (p->source >= skr_timing_source_vblank && newer) {
			ref_pacer->anchor_id         = p->id;
			ref_pacer->anchor_display_ns = p->display_ns;
			ref_pacer->anchor_is_floor   = false;
		}
	}

	// The next present's display time is predicted from the newest trusted
	// one, a refresh per present after it. Nothing is known before the first,
	// and the wall clock stands in.
	uint64_t next_id   = skr_surface_get_next_id(ref_surface);
	uint64_t predicted = 0;
	if (ref_pacer->anchor_id) {
		predicted = ref_pacer->anchor_display_ns + (next_id - ref_pacer->anchor_id) * ref_pacer->refresh_ns;

		// After a stall the anchor is stale, and the next present can't show
		// before the refresh after now. Anchoring there puts the stall on the
		// next step instead of waiting for reports to settle.
		uint64_t now = skr_time_now_ns();
		if (now > predicted) {
			predicted = now + ref_pacer->refresh_ns - (now - ref_pacer->anchor_display_ns) % ref_pacer->refresh_ns;
			ref_pacer->anchor_id         = next_id;
			ref_pacer->anchor_display_ns = predicted;
			ref_pacer->anchor_is_floor   = true;
		}
	}
	frame_pacer_step(predicted, ref_pacer->predicted_ns);
	ref_pacer->predicted_ns = predicted;
}

///////////////////////////////////////////

void frame_pacer_reset(frame_pacer_t* ref_pacer) {
	*ref_pacer = {};
}

///////////////////////////////////////////

present_stats_t frame_pacer_stats(const frame_pacer_t* pacer) {
	present_stats_t stats = {};
	if (pacer->newest_id == 0) return stats;

	const frame_pacer_present_t* newest = &pacer->history[pacer->newest_id % FRAME_PACER_HISTORY];
	stats.latency_us = newest->display_ns > newest->cpu_present_ns ? (newest->display_ns - newest->cpu_present_ns) / 1000 : 0;

	// A gap of a refresh and a half or more between consecutive display times
	// means the earlier frame was shown at least twice. A coarse time can sit
	// before its predecessor, that pair is skipped rather than underflowed.
	uint64_t first        = pacer->newest_id > FRAME_PACER_HISTORY ? pacer->newest_id - FRAME_PACER_HISTORY + 1 : 1;
	uint64_t prev_display = 0;
	for (uint64_t id = first; id <= pacer->newest_id; id++) {
		const frame_pacer_present_t* p = &pacer->history[id % FRAME_PACER_HISTORY];
		if (p->id != id || p->display_ns == 0) continue;
		stats.sample_count++;
		if (prev_display && p->display_ns >= prev_display && (p->display_ns - prev_display) * 2 >= pacer->refresh_ns * 3)
			stats.repeat_count++;
		prev_display = p->display_ns;
	}
	return stats;
}

///////////////////////////////////////////

void frame_pacer_set_main(const frame_pacer_t* pacer) {
	pacer_main = pacer;
}

///////////////////////////////////////////

present_stats_t frame_pacer_stats_main() {
	return pacer_main ? frame_pacer_stats(pacer_main) : present_stats_t{};
}

///////////////////////////////////////////

}
