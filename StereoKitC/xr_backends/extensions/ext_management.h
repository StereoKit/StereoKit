/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../openxr.h"

namespace sk {

bool is_ext_explicitly_requested          (const char*   extension_name);
void ext_management_sys_register          (xr_system_t system);
bool ext_management_get_use_min           ();
void ext_management_mark_loaded           (const char**  extension_names, int32_t count);
void ext_management_get_exts              (const char*** out_ext_names, int32_t* out_count);
void ext_management_get_excludes          (const char*** out_ext_names, int32_t* out_count);
void ext_management_cleanup               ();
void ext_management_evt_pre_session_create(XrSessionCreateInfo* ref_session_info);
bool ext_management_evt_session_ready     ();
void ext_management_evt_step_begin        ();
void ext_management_evt_step_end          ();
void ext_management_evt_poll_event        (const XrEventDataBuffer* event_data);

}