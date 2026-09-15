// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#pragma once

#include "../stereokit.h"
#include <sk_renderer.h>
#include <sk_app.h>

namespace sk {

// Paces a flatscreen backend against its display: the simulation step
// follows the surface's presentation timeline instead of the wall clock, and
// the CPU is held from running further ahead of the screen than the display
// mode makes safe. sk_renderer reports and waits, this is the policy on top.
// One pacer per surface, owned by the backend, zero-init is a valid start.

#define FRAME_PACER_HISTORY 128

struct frame_pacer_present_t {
	uint64_t id;
	uint64_t cpu_present_ns;
	uint64_t display_ns;
};

struct frame_pacer_t {
	uint64_t              anchor_display_ns; // newest trusted display time, and the present it belongs to
	uint64_t              anchor_id;
	bool                  anchor_is_floor;   // the anchor is the earliest its present could show, not a report
	uint64_t              predicted_ns;      // display time predicted for the previous frame
	uint64_t              refresh_ns;
	frame_pacer_present_t history[FRAME_PACER_HISTORY]; // by present id
	uint64_t              newest_id;
};

// Before the frame's work, after the events are pumped. Feeds the window's
// refresh and vblank to the surface and blocks per the run ahead policy.
// Fullscreen (and Android, which always is) allows two frames in flight;
// through a compositor the swapchain's own depth is the cap.
void            frame_pacer_begin(frame_pacer_t* ref_pacer, skr_surface_t* ref_surface, const ska_window_t* window, bool fullscreen);
// After skr_surface_present. Drains the surface's finished presents and
// hands the timer the next frame's display driven step. A stall shows on
// the step right after it, from the earliest refresh the next present can
// make, and that present's report tops up the remainder.
void            frame_pacer_end  (frame_pacer_t* ref_pacer, skr_surface_t* ref_surface);
// The surface is gone; the next one's present ids start over.
void            frame_pacer_reset(frame_pacer_t* ref_pacer);
present_stats_t frame_pacer_stats(const frame_pacer_t* pacer);

// The pacer behind time_perf_present. The backend registers its main one at
// init and clears it before the memory goes away. Null reports zeros.
void            frame_pacer_set_main  (const frame_pacer_t* pacer);
present_stats_t frame_pacer_stats_main();

// Makes the next frame's step the gap between two display times on any
// clock, so animation moves as far as the display will show it moving. A
// zero, or no forward motion, leaves the next frame on the wall clock.
// Shared with the XR backend, which anchors on xrWaitFrame.
void            frame_pacer_step (uint64_t display_ns, uint64_t prev_display_ns);

}
