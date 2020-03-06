#pragma once

#include "../../stereokit.h"

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <d3d11.h>
#include <openxr/openxr.h>
#include "openxr_extensions.h"

#include <stdint.h>

#define xr_check(xResult, message, ...) {XrResult xr_call_result = xResult; if (XR_FAILED(xr_call_result)) {log_infof(message, openxr_string(xr_call_result), __VA_ARGS__); return false;}}

namespace sk {

enum xr_hand_state_ {
	xr_hand_state_uncertain,
	xr_hand_state_present,
	xr_hand_state_unavailable,
};

bool openxr_init          (const char *app_name);
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
void openxr_poll_events   ();
bool openxr_render_frame  ();
void openxr_make_actions  ();
void openxr_poll_actions  ();

int64_t     openxr_get_time();
bool32_t    openxr_get_space(XrSpace space, pose_t &out_pose, XrTime time = 0);
const char* openxr_string(XrResult result);

extern XrSpace    xr_hand_space[2];
extern XrSpace    xr_app_space;
extern XrInstance xr_instance;
extern XrSession  xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system_id;
extern bool       xr_articulated_hands;
extern bool       xr_depth_lsr;
extern XrExtTable xr_extensions;
extern XrTime     xr_time;
extern xr_hand_state_ xr_hand_state; // TODO: This can go when OpenXR has real hands

} // namespace sk