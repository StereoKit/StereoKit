/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../openxr.h"
#include "../../libraries/array.h"

namespace sk {

bool ext_management_is_user_requested      (const char* extension_name);
void ext_management_request_ext            (const char* extension_name);
bool ext_management_enumerate_available    ();
bool ext_management_ext_available          (const char* extension_name);
bool ext_management_select_exts            (bool minimum_exts, array_t<char*>* ref_all_available_exts, array_t<const char*>* ref_request_exts);
void ext_management_sys_register           (xr_system_t system);
bool ext_management_get_use_min            ();
void ext_management_get_exts               (const char*** out_ext_names, int32_t* out_count);
void ext_management_get_excludes           (const char*** out_ext_names, int32_t* out_count);
void ext_management_cleanup                ();

// These are listed in order of execution
bool ext_management_evt_pre_instance_create(XrInstanceCreateInfo*     ref_instance_info);
bool ext_management_evt_pre_session_create (XrSessionCreateInfo*      ref_session_info);
void ext_management_evt_profile_suggest    (xr_interaction_profile_t* ref_profile);
void ext_management_evt_session_begin      (XrSessionBeginInfo*       ref_session_info);
bool ext_management_evt_session_ready      ();
void ext_management_evt_step_begin         ();
void ext_management_evt_step_end           ();
void ext_management_evt_poll_event         (const XrEventDataBuffer* event_data);

}