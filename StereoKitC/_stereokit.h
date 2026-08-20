#pragma once

#include "stereokit.h"
#include "libraries/ferr_thread.h"

namespace sk {

const id_hash_t default_hash_root = 14695981039346656037UL;

void     sk_assert_thread_valid();
bool32_t sk_has_stepped        ();
bool32_t sk_is_initialized     ();
bool32_t sk_is_running         ();
bool32_t sk_use_manual_pos     ();
void     sk_set_app_focus      (app_focus_ focus_state);
ft_id_t  sk_main_thread        ();
void     sk_app_step           ();

// Waits that block inside sk_renderer's CPU frame window get charged here, so
// time_perf_cpu_us can take them back out. OpenXR's frame pacing, and on web
// the time the browser holds between frames.
void     sk_cpu_wait_add       (uint64_t wait_ticks);

const sk_settings_t* sk_get_settings_ref();
sk_settings_t*       sk_get_settings_ref_mut();
system_info_t*       sk_get_info_ref();

} // namespace sk