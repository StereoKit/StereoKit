/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "../stereokit.h"
#include "openxr_input.h"
#include <openxr/openxr.h>
#include <stdint.h>

typedef struct XR_MAY_ALIAS XrBaseHeader {
	XrStructureType             type;
	const void* XR_MAY_ALIAS    next;
} XrBaseHeader;

#define xr_check_ret(xResult, message, ret_val) do { XrResult xr_call_result = xResult; if (XR_FAILED(xr_call_result)) { log_errf("%s [%s]", message, openxr_string(xr_call_result)); return ret_val; } } while(0)
#define xr_check(xResult, message) xr_check_ret(xResult, message, false)
inline void xr_insert_next(XrBaseHeader *xr_base, XrBaseHeader *xr_next) { xr_next->next = xr_base->next; xr_base->next = xr_next; }

// Some "X Macros" to simplify function loading. Otherwise, function loading
// tends to have a lot of manual code duplication that gets spread out across
// multiple spots.
#define _OPENXR_DEFINE_FN(name) PFN_##name name;
#define OPENXR_DEFINE_FN(list) list(_OPENXR_DEFINE_FN)
#define _OPENXR_CLEAR_FN(name) name = nullptr;
#define OPENXR_CLEAR_FN(list) list(_OPENXR_CLEAR_FN)
#define _OPENXR_DEFINE_FN_STATIC(name) static PFN_##name name;
#define OPENXR_DEFINE_FN_STATIC(list) list(_OPENXR_DEFINE_FN_STATIC)
#define _OPENXR_LOAD_FN_RESULT(name) if (xrGetInstanceProcAddr(xr_instance, #name, (PFN_xrVoidFunction*)((PFN_##name*)(&name)))<0) { result = result && false; }
#define OPENXR_LOAD_FN_RETURN(list, failure_result) do { bool result = true; list(_OPENXR_LOAD_FN_RESULT); if (!result) return failure_result; } while(0);

namespace sk {

typedef enum xr_system_ {
	xr_system_succeed       = 1,
	xr_system_fail          = 0,
	xr_system_fail_critical = -1,
} xr_system_;

typedef struct context_callback_t {
	void       (*callback)(void* context);
	void*        context;
} context_callback_t;

typedef struct create_info_callback_t {
	xr_system_ (*callback)(void* context, XrBaseHeader* create_info);
	void*        context;
} create_info_callback_t;

typedef struct context_result_callback_t {
	xr_system_ (*callback)(void* context);
	void*        context;
} context_result_callback_t;

typedef struct profile_callback_t {
	void       (*callback)(void* context, xr_interaction_profile_t *ref_profile);
	void*        context;
} profile_callback_t;

typedef struct poll_event_callback_t {
	void       (*callback)(void* context, void* XrEventDataBuffer);
	void*        context;
} poll_event_callback_t;

typedef struct xr_system_t {
	const char* request_exts[4];
	int32_t     request_ext_count;
	const char* request_opt_exts[4];
	int32_t     request_opt_ext_count;
	bool        required;

	create_info_callback_t    evt_pre_instance;
	create_info_callback_t    evt_pre_session;
	create_info_callback_t    evt_begin_session;
	context_result_callback_t evt_initialize;
	profile_callback_t        evt_profile;
	// These callbacks are only called if "initialize" succeeds, or if
	// "initialize" isn't specified.
	context_callback_t        evt_step_begin;
	context_callback_t        evt_step_end;
	context_callback_t        evt_shutdown;
	poll_event_callback_t     evt_poll;
} xr_system_t;

bool openxr_create_system();  // Idempotent: creates XrInstance and XrSystemId
bool openxr_init        ();
void openxr_cleanup     ();
void openxr_shutdown    ();
void openxr_step_begin  ();
void openxr_step_end    ();
bool openxr_poll_events ();
bool openxr_render_frame();

bool32_t      openxr_get_space        (XrSpace space, pose_t *out_pose, XrTime time = 0);
const char*   openxr_string           (XrResult result);
void          openxr_set_origin_offset(pose_t offset);
bool          openxr_get_stage_bounds (vec2* out_size, pose_t* out_pose, XrTime time);
button_state_ openxr_space_tracked    ();

extern XrSpace    xr_app_space;
extern XrSpace    xr_head_space;
extern XrInstance xr_instance;
extern XrSession  xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system_id;
extern bool       xr_has_bounds;
extern XrTime     xr_time;
extern XrTime     xr_eyes_sample_time;
extern vec2       xr_bounds_size;
extern pose_t     xr_bounds_pose_local;

#define XR_PRIMARY_CONFIG XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO

} // namespace sk
#endif