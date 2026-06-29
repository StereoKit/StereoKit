/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_input.h"
#include "extensions/ext_management.h"
#include "extensions/eye_interaction.h"
#include "extensions/fb_haptic.h"
#include "../systems/input.h"
#include "../systems/render.h"

#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../stereokit.h"

#include <openxr/openxr.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

namespace sk {

///////////////////////////////////////////

struct xrc_profile_info_t {
	const char*   name;
	XrPath        profile;
	XrPath        top_level_path;
	pose_t        offset;
	bool          is_hand;
	bool          has_haptic;
};

struct xr_top_level_t {
	const char*   name;
	XrPath        path;
	int32_t       id;
	int32_t       active_profile;
};

struct haptic_stream_t {
	array_t<float> pending;           // user samples not yet submitted to the runtime
	int32_t        pending_offset;    // offset into 'pending' of the next un-submitted sample
	float          rate_hz;           // sample rate of the pending buffer
	bool           active;            // is there a stream currently being drained?
};

struct xrc_state_t {
	XrActionSet                       action_set;
	array_t<XrAction>                 actions[xra_type_max];
	array_t<XrSpace>                  pose_spaces;

	array_t<pose_t>                   input_poses;
	array_t<float>                    input_floats;
	array_t<button_state_>            input_bools;
	array_t<vec2>                     input_xys;

	button_state_                     tracked_state;

	array_t<xr_interaction_profile_t> registered_profiles;
	bool                              registration_finished;

	array_t<xrc_profile_info_t>       profiles;
	array_t<xr_top_level_t>           top_levels;

	haptic_stream_t                   haptic_streams  [input_haptic_max];
	float                             haptic_pref_rate[input_haptic_max];
	input_haptic_caps_                haptic_caps     [input_haptic_max];
};
static xrc_state_t local = {};

///////////////////////////////////////////

xr_system_ oxri_init                (void*);
void       oxri_shutdown            (void*);
void       oxri_update_frame        (void*);
void       oxri_poll                (void*, XrEventDataBuffer* event);

void        oxri_update_profiles      ();
XrAction    oxri_get_or_create_action (xra_type_ type, uint32_t xra_val);
bool        oxri_bind_profile         (xr_interaction_profile_t* profiles, int32_t profile_count);
static void oxri_haptic_recompute_caps(input_haptic_ haptic_type);

///////////////////////////////////////////

void oxri_register() {
	xr_system_t system = {};
	system.required       = true;
	system.evt_initialize = { oxri_init };
	system.evt_shutdown   = { oxri_shutdown };
	system.evt_step_begin = { oxri_update_frame };
	system.evt_poll       = { (void (*)(void*, void*))oxri_poll };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

xr_system_ oxri_init(void*) {
	for (int32_t i = 0; i < input_haptic_max; i++) local.haptic_pref_rate[i] = -1;

	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	snprintf(actionset_info.actionSetName,          sizeof(actionset_info.actionSetName),          "input");
	snprintf(actionset_info.localizedActionSetName, sizeof(actionset_info.localizedActionSetName), "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &local.action_set);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return xr_system_fail_critical;
	}

	// Bind the actions we just created to specific locations on the input
	// profiles! These are 'suggested' because they may be overridden by the
	// runtime preferences. For example, if the runtime allows you to remap
	// buttons, or provides input accessibility settings.

	// OpenXR's ideal situation is that you provide bindings for the
	// controllers that you know and use. Runtimes are then supposed to remap
	// those bindings to the controller that is actually present. But in
	// reality, the remapping doesn't always happen.

	// Here, we provide bindings for as many controllers as possible, so that
	// runtimes don't have the opportunity to drop the ball. These profiles are
	// all part of the core spec, but there are a few more specified as
	// extensions in /xr_backends/extensions/input_profiles.cpp.

	// microsoft/motion_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_microsoft_mixed_reality_motion_controller_profile
	{
		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };

		xr_interaction_profile_t profile_l = { "microsoft/motion_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.palm_offset    = palm_offset;
		profile_l.is_hand        = false;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_grip,         "grip/pose"        };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_aim,          "aim/pose"         };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_trigger,     "trigger/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_grip,        "squeeze/click"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_xy,     input_xy_l_stick,          "thumbstick"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_stick,      "thumbstick/click" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_menu,       "menu/click"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_haptic, input_haptic_l_controller, "haptic"           };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "microsoft/motion_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.palm_offset    = palm_offset;
		profile_r.is_hand        = false;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_grip,         "grip/pose"        };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_aim,          "aim/pose"         };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_trigger,     "trigger/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_grip,        "squeeze/click"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_xy,     input_xy_r_stick,          "thumbstick"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_stick,      "thumbstick/click" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_menu,       "menu/click"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_haptic, input_haptic_r_controller, "haptic"           };
		oxri_register_profile(profile_r);
	}

	// htc/vive_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_htc_vive_controller_profile
	{
		xr_interaction_profile_t profile_l = { "htc/vive_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.palm_offset    = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile_l.is_hand        = false;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_grip,         "grip/pose"     };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_aim,          "aim/pose"      };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_trigger,     "trigger/value" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_grip,        "squeeze/click" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_menu,       "menu/click"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_haptic, input_haptic_l_controller, "haptic"        };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "htc/vive_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.palm_offset    = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile_r.is_hand        = false;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_grip,         "grip/pose"     };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_aim,          "aim/pose"      };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_trigger,     "trigger/value" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_grip,        "squeeze/click" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_menu,       "menu/click"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_haptic, input_haptic_r_controller, "haptic"        };
		oxri_register_profile(profile_r);
	}

	// valve/index_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_valve_index_controller_profile
	{
		xr_interaction_profile_t profile_l = { "valve/index_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.palm_offset    = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile_l.is_hand        = false;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_grip,         "grip/pose"        };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_aim,          "aim/pose"         };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_trigger,     "trigger/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_grip,        "squeeze/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_xy,     input_xy_l_stick,          "thumbstick"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_stick,      "thumbstick/click" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_menu,       "system/click"     };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_x1,         "a/click"          };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_x2,         "b/click"          };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_haptic, input_haptic_l_controller, "haptic"           };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "valve/index_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.palm_offset    = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		profile_r.is_hand        = false;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_grip,         "grip/pose"        };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_aim,          "aim/pose"         };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_trigger,     "trigger/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_grip,        "squeeze/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_xy,     input_xy_r_stick,          "thumbstick"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_stick,      "thumbstick/click" };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_menu,       "system/click"     };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_x1,         "a/click"          };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_x2,         "b/click"          };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_haptic, input_haptic_r_controller, "haptic"           };
		oxri_register_profile(profile_r);
	}

	// oculus/touch_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_oculus_touch_controller_profile
	{
		xr_interaction_profile_t profile_l = { "oculus/touch_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.palm_offset    = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile_l.is_hand        = false;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_grip,         "grip/pose"        };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_aim,          "aim/pose"         };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_trigger,     "trigger/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_grip,        "squeeze/value"    };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_xy,     input_xy_l_stick,          "thumbstick"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_stick,      "thumbstick/click" };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_menu,       "menu/click"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_x1,         "x/click"          };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_x2,         "y/click"          };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_haptic, input_haptic_l_controller, "haptic"           };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "oculus/touch_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.palm_offset    = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		profile_r.is_hand        = false;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_grip,         "grip/pose"        };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_aim,          "aim/pose"         };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_trigger,     "trigger/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_grip,        "squeeze/value"    };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_xy,     input_xy_r_stick,          "thumbstick"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_stick,      "thumbstick/click" };

		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_x1,         "a/click"          };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_x2,         "b/click"          };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_haptic, input_haptic_r_controller, "haptic"           };
		oxri_register_profile(profile_r);
	}

	// khr/simple_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_khronos_simple_controller_profile
	{
		xr_interaction_profile_t profile_l = { "khr/simple_controller" };
		profile_l.top_level_path = "/user/hand/left";
		profile_l.palm_offset    = pose_identity;
		profile_l.is_hand        = false;
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_grip,         "grip/pose"        };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_pose,   input_pose_l_aim,          "aim/pose"         };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_float,  input_float_l_trigger,     "select/click"     };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_bool,   input_button_l_menu,       "menu/click"       };
		profile_l.binding[profile_l.binding_ct++] = { xra_type_haptic, input_haptic_l_controller, "haptic"           };
		oxri_register_profile(profile_l);

		xr_interaction_profile_t profile_r = { "khr/simple_controller" };
		profile_r.top_level_path = "/user/hand/right";
		profile_r.palm_offset    = pose_identity;
		profile_r.is_hand        = false;
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_grip,         "grip/pose"        };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_pose,   input_pose_r_aim,          "aim/pose"         };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_float,  input_float_r_trigger,     "select/click"     };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_bool,   input_button_r_menu,       "menu/click"       };
		profile_r.binding[profile_r.binding_ct++] = { xra_type_haptic, input_haptic_r_controller, "haptic"           };
		oxri_register_profile(profile_r);
	}

	// Suggest all our input profiles
	local.registration_finished = true;
	// We can only submit one binding per interaction profile, so here we
	// combine each uniqu top-level path belonging to the same interaction
	// profile.

	// Figure out all the unique interaction profiles
	array_t<const char*>              unique_profiles = {};
	array_t<xr_interaction_profile_t> top_levels      = {};
	for (int32_t p = 0; p < local.registered_profiles.count; p++) {
		// Make sure we haven't registered this interaction profile yet
		const char* curr_profile = local.registered_profiles[p].name;
		bool        found        = false;
		for (int32_t u = 0; u < unique_profiles.count; u++) {
			if (string_eq(unique_profiles[u], curr_profile)) {
				found = true;
				break;
			}
		}
		if (found) continue;

		// Find all top-levels associated with this interaction profile.
		unique_profiles.add(curr_profile);
		top_levels     .clear();
		for (int32_t t = p; t < local.registered_profiles.count; t++) {
			if (!string_eq(local.registered_profiles[t].name, curr_profile)) continue;
			// Check if we've added this top-level already
			bool unique_top_level = true;
			for (int32_t i = 0; i < top_levels.count; i++) {
				if (string_eq(top_levels[i].name, curr_profile) && string_eq(top_levels[i].top_level_path, local.registered_profiles[t].top_level_path)) {
					unique_top_level = false;
					break;
				}
			}
			top_levels.add(local.registered_profiles[t]);
		}

		// Now register it
		if (oxri_bind_profile(top_levels.data, top_levels.count)) {
			for (int32_t t = 0; t < top_levels.count; t++) {

				char buffer[XR_MAX_PATH_LENGTH];
				snprintf(buffer, sizeof(buffer), "/interaction_profiles/%s", top_levels[t].name);

				xrc_profile_info_t bind_info = {};
				xrStringToPath(xr_instance, top_levels[t].top_level_path, &bind_info.top_level_path);
				xrStringToPath(xr_instance, buffer,                       &bind_info.profile);
				bind_info.name    = top_levels[t].name;
				bind_info.is_hand = top_levels[t].is_hand;
				bind_info.offset  = top_levels[t].palm_offset;
				for (int32_t b = 0; b < top_levels[t].binding_ct; b++) {
					if (top_levels[t].binding[b].xra_type == xra_type_haptic) { bind_info.has_haptic = true; break; }
				}
				local.profiles.add(bind_info);

				// Track all our top level paths
				bool found = false;
				for (int32_t i = 0; i < local.top_levels.count; i++) {
					xr_top_level_t* top_level = &local.top_levels[i];
					if (string_eq(top_level->name, top_levels[t].top_level_path)) {
						found = true;
						break;
					}
				}
				if (!found) {
					xr_top_level_t top = {};
					top.name           = top_levels[t].top_level_path;
					top.active_profile = -1;
					top.id             = local.top_levels.count;
					xrStringToPath(xr_instance, top.name, &top.path);
					local.top_levels.add(top);
				}
			}
		}
	}
	unique_profiles.free();
	top_levels     .free();
	local.registered_profiles.free();

	// Create a space/reference frame for each input pose action
	for (int32_t i = 0; i < local.actions[xra_type_pose].count; i++) {
		XrSpace space = XR_NULL_HANDLE;
		if (local.actions[xra_type_pose][i] != XR_NULL_HANDLE) {
			XrActionSpaceCreateInfo create_space = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			create_space.action            = local.actions[xra_type_pose][i];
			create_space.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			result = xrCreateActionSpace(xr_session, &create_space, &space);
			if (XR_FAILED(result)) {
				log_errf("%s [%s]", "xrCreateActionSpace", openxr_string(result));
				space = XR_NULL_HANDLE;
			}
		}
		local.pose_spaces.add(space);
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &local.action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_errf("%s [%s]", "xrAttachSessionActionSets", openxr_string(result));
		return xr_system_fail_critical;
	}

	return xr_system_succeed;
}

///////////////////////////////////////////

void oxri_shutdown(void*) {
	local.profiles.free();


	for (int32_t i = 0; i < local.pose_spaces.count; i++) {
		if (local.pose_spaces[i]) xrDestroySpace(local.pose_spaces[i]);
	}
	local.pose_spaces.free();

	for (int32_t i = 0; i < xra_type_max; i++)
		local.actions[i].free();

	for (int32_t i = 0; i < input_haptic_max; i++)
		local.haptic_streams[i].pending.free();

	local.profiles    .free();
	local.input_bools .free();
	local.input_floats.free();
	local.input_poses .free();
	local.input_xys   .free();

	if (local.action_set) { xrDestroyActionSet(local.action_set); local.action_set = {}; }
	local = {};
}

///////////////////////////////////////////

void oxri_register_profile(xr_interaction_profile_t profile) {
	if (local.registration_finished == true) {
		log_errf("Interaction profile '%s' was registered too late! Please add it before StereoKit initialization.", profile.name);
		return;
	}
	// If we already have a registered profile with this name, we prefer the
	// earlier one.
	for (int32_t i = 0; i < local.registered_profiles.count; i++) {
		if (string_eq(profile.name, local.registered_profiles[i].name) && string_eq(profile.top_level_path, local.registered_profiles[i].top_level_path))
			return;
	}
	local.registered_profiles.add(profile);
}

///////////////////////////////////////////

bool oxri_bind_profile(xr_interaction_profile_t *profiles, int32_t profile_count) {
	array_t<XrActionSuggestedBinding> binds = {};
	char                              buffer[XR_MAX_PATH_LENGTH];
	for (int32_t p = 0; p < profile_count; p++) {
		xr_interaction_profile_t* profile = &profiles[p];

		// Notify our extensions that we're adding a profile! This is for
		// extensions such as EXT_palm_pose that inject poses into _all_ input
		// profiles involving a hand.
		ext_management_evt_profile_suggest(profile);

		XrPath   top_level_path;
		XrResult xr = xrStringToPath(xr_instance, profile->top_level_path, &top_level_path);
		if (XR_FAILED(xr)) {
			log_errf("xrStringToPath failed for %s: [%s]", profile->top_level_path,openxr_string(xr));
			binds.free();
			return false;
		}

		for (int32_t i = 0; i < profile->binding_ct; i++) {
			XrActionSuggestedBinding bind = {};
			bind.action = oxri_get_or_create_action((xra_type_)profile->binding[i].xra_type, (uint32_t)profile->binding[i].xra_type_val);

			const char* direction = profile->binding[i].xra_type == xra_type_haptic ? "output" : "input";
			snprintf(buffer, sizeof(buffer), "%s/%s/%s", profile->top_level_path, direction, profile->binding[i].path);
			xr = xrStringToPath(xr_instance, buffer, &bind.binding);
			if (XR_FAILED(xr)) { log_errf("xrStringToPath failed for %s '%s': [%s]", profile->name, buffer, openxr_string(xr)); binds.free(); return false; }
			else               { binds.add(bind); }
		}
	}

	XrPath interaction_path;
	snprintf(buffer, sizeof(buffer), "/interaction_profiles/%s", profiles[0].name);
	xrStringToPath(xr_instance, buffer, &interaction_path);

	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggested_binds.interactionProfile     = interaction_path;
	suggested_binds.suggestedBindings      = binds.data;
	suggested_binds.countSuggestedBindings = binds.count;
	if (XR_FAILED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds))) {
		binds.free();
		return false;
	}
	binds.free();
	return true;
}

///////////////////////////////////////////

button_state_ openxr_space_tracked() {
	return local.tracked_state;
}

///////////////////////////////////////////

void oxri_update_poses() {
	// We occasionally need to update poses multiple times per frame, to
	// account for camera movement, or predicted time updates. This code is 
	// called separate from button press and tracking events to prevent
	// issues with hiding 'just_active/inactive' events.

	// Track the head location, and use it to determine the tracking state of
	// the world.
	XrSpaceLocation head_location = { XR_TYPE_SPACE_LOCATION };
	XrResult        res           = xrLocateSpace(xr_head_space, xr_app_space, xr_time, &head_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && 
		(head_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT   ) != 0 &&
		(head_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {

		memcpy(&input_head_pose_local.position,    &head_location.pose.position,    sizeof(vec3));
		memcpy(&input_head_pose_local.orientation, &head_location.pose.orientation, sizeof(quat));
	}
	// We report tracking of the device based on the positional tracking
	// reported here. Rotational tracking is pretty much always available to
	// devices containing an accelerometer/gyroscope, so positional is the best
	// metric for tracking quality.
	local.tracked_state = button_make_state(
		(local.tracked_state            & button_state_active)                    != 0,
		(head_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) != 0);

	// Get input from whatever controllers may be present
	XrEyeGazeSampleTimeEXT gaze_sample_time = { XR_TYPE_EYE_GAZE_SAMPLE_TIME_EXT };
	for (int32_t i = 0; i < local.pose_spaces.count; i++) {
		if (local.pose_spaces[i] == XR_NULL_HANDLE) continue;

		XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
		if (i == input_pose_eyes && xr_ext_eye_gaze_available()) {
			space_location.next = &gaze_sample_time;
		}
		if (local.pose_spaces[i] == XR_NULL_HANDLE || !XR_UNQUALIFIED_SUCCESS(xrLocateSpace(local.pose_spaces[i], xr_app_space, xr_time, &space_location))) continue;

		track_state_ tr_pos = (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT   ) ? track_state_known : ((space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT   ) ? track_state_inferred : track_state_lost);
		track_state_ tr_rot = (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT) ? track_state_known : ((space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) ? track_state_inferred : track_state_lost);
		pose_t       pose   = input_pose_get_local((input_pose_)i);
		// Orientation is updated before position so the poke offset below can
		// use this frame's orientation.
		if (tr_rot != track_state_lost)
			memcpy(&pose.orientation, &space_location.pose.orientation, sizeof(quat));
		if (tr_pos != track_state_lost) {
			memcpy(&pose.position, &space_location.pose.position, sizeof(vec3));

			// SK models poke as a sphere centered at the joint, so pull the
			// fingertip-surface poke pose back one tip radius along +Z.
			if (i == input_pose_l_poke || i == input_pose_r_poke)
				pose.position = pose.position + (pose.orientation * vec3{ 0,0,1 }) * 0.007f;
		}
		input_pose_inject((input_pose_)i, pose, tr_pos, tr_rot);

		if (i == input_pose_eyes && xr_ext_eye_gaze_available()) {
			xr_eyes_sample_time = gaze_sample_time.time;
		}
	}
}

///////////////////////////////////////////

void oxri_update_frame(void*) {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED)
		return;

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet = local.action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets = &action_set;
	xrSyncActions(xr_session, &sync_info);

	// Update poses, this is separate because we update poses multiple times
	// per frame, once at the start, and once when we have our "predicted time"
	// for low latency poses.
	oxri_update_poses();

	for (int32_t i = 0; i < local.actions[xra_type_bool].count; i++) {
		if (local.actions[xra_type_bool][i] == XR_NULL_HANDLE) continue;

		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		XrActionStateBoolean state    = { XR_TYPE_ACTION_STATE_BOOLEAN  };
		get_info.action = local.actions[xra_type_bool][i];
		xrGetActionStateBoolean(xr_session, &get_info, &state);
		input_button_inject((input_button_)i, state.currentState);
	}

	for (int32_t i = 0; i < local.actions[xra_type_float].count; i++) {
		if (local.actions[xra_type_float][i] == XR_NULL_HANDLE) continue;

		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		XrActionStateFloat   state    = { XR_TYPE_ACTION_STATE_FLOAT    };
		get_info.action = local.actions[xra_type_float][i];
		xrGetActionStateFloat(xr_session, &get_info, &state);
		input_float_inject((input_float_)i, state.currentState);
	}

	for (int32_t i = 0; i < local.actions[xra_type_xy].count; i++) {
		if (local.actions[xra_type_xy][i] == XR_NULL_HANDLE) continue;

		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		XrActionStateVector2f   state = { XR_TYPE_ACTION_STATE_VECTOR2F };
		get_info.action = local.actions[xra_type_xy][i];
		xrGetActionStateVector2f(xr_session, &get_info, &state);
		input_xy_inject((input_xy_)i, { state.currentState.x, state.currentState.y });
	}

	oxri_haptic_step();
}

///////////////////////////////////////////

void oxri_poll(void*, XrEventDataBuffer* event) {
	if (event->type != XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED)
		return;

	oxri_update_profiles();
}

///////////////////////////////////////////

void oxri_update_profiles() {
	bool reset = false;

	for (int32_t t = 0; t < local.top_levels.count; t++) {
		xr_top_level_t* top_level = &local.top_levels[t];

		XrInteractionProfileState new_profile = { XR_TYPE_INTERACTION_PROFILE_STATE };
		if (XR_FAILED(xrGetCurrentInteractionProfile(xr_session, top_level->path, &new_profile)))
			continue;

		if (top_level->active_profile >= 0 && new_profile.interactionProfile == local.profiles[top_level->active_profile].profile)
			continue;

		for (int32_t p = 0; p < local.profiles.count; p++) {
			xrc_profile_info_t* profile = &local.profiles[p];

			if (profile->profile != new_profile.interactionProfile || profile->top_level_path != top_level->path)
				continue;

			reset = true;

			top_level->active_profile = p;
			if      (string_eq(top_level->name, "/user/hand/left" )) { input_set_palm_offset(handed_left,  profile->offset); input_controller_set_hand(handed_left,  profile->is_hand); oxri_haptic_stop(input_haptic_l_controller); local.haptic_pref_rate[input_haptic_l_controller] = -1; oxri_haptic_recompute_caps(input_haptic_l_controller); }
			else if (string_eq(top_level->name, "/user/hand/right")) { input_set_palm_offset(handed_right, profile->offset); input_controller_set_hand(handed_right, profile->is_hand); oxri_haptic_stop(input_haptic_r_controller); local.haptic_pref_rate[input_haptic_r_controller] = -1; oxri_haptic_recompute_caps(input_haptic_r_controller); }
			
			log_diagf("Switched %s to %s", top_level->name, profile->name);
			break;
		}
	}

	if (reset)
		input_reset();
}

///////////////////////////////////////////

void oxri_make_action_name(XrActionCreateInfo *ref_action_info, int32_t hand, const char* action, const char* display) {
	const char* hand_str_action [3] = { "_left", "_right", "" };
	const char* hand_str_display[3] = { " left", " right", "" };
	snprintf(ref_action_info->actionName,          sizeof(ref_action_info->actionName         ), "%s%s", action,  hand_str_action [hand]);
	snprintf(ref_action_info->localizedActionName, sizeof(ref_action_info->localizedActionName), "%s%s", display, hand_str_display[hand]);
}

///////////////////////////////////////////

void oxri_make_action_default(XrActionCreateInfo *ref_action_info, int32_t hand, const char* category, int32_t idx) {
	const char* hand_str_action [3] = { "_left", "_right", "" };
	const char* hand_str_display[3] = { " left", " right", "" };
	snprintf(ref_action_info->actionName,          sizeof(ref_action_info->actionName         ), "%s_%d%s", category, idx, hand_str_action [hand]);
	snprintf(ref_action_info->localizedActionName, sizeof(ref_action_info->localizedActionName), "%s %d%s", category, idx, hand_str_display[hand]);
}

///////////////////////////////////////////

XrAction oxri_get_or_create_action(xra_type_ type, uint32_t xra_val) {
	array_t<XrAction>* actions = &local.actions[type];

	// Ensure we have capacity for this action
	if (actions->count <= (int32_t)xra_val) {
		actions->resize(xra_val + 1);
		for (int32_t i = actions->count; i < actions->capacity; i++)
			actions->set(i, XR_NULL_HANDLE);
		actions->count = actions->capacity;
	}

	XrAction result = actions->get(xra_val);

	// If we don't have an action for this yet, make it!
	if (result == XR_NULL_HANDLE) {
		XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
		switch (type) {
		case xra_type_bool:
			action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
			switch (xra_val) {
			case input_button_l_aim_ready: oxri_make_action_name(&action_info, handed_left,  "aim_ready",   "Aim ready");   break;
			case input_button_r_aim_ready: oxri_make_action_name(&action_info, handed_right, "aim_ready",   "Aim ready");   break;
			case input_button_l_stick:     oxri_make_action_name(&action_info, handed_left,  "stick_click", "Stick click"); break;
			case input_button_r_stick:     oxri_make_action_name(&action_info, handed_right, "stick_click", "Stick click"); break;
			case input_button_l_x1:        oxri_make_action_name(&action_info, handed_left,  "x1_click",    "X1 click");    break;
			case input_button_r_x1:        oxri_make_action_name(&action_info, handed_right, "x1_click",    "X1 click");    break;
			case input_button_l_x2:        oxri_make_action_name(&action_info, handed_left,  "x2_click",    "X2 click");    break;
			case input_button_r_x2:        oxri_make_action_name(&action_info, handed_right, "x2_click",    "X2 click");    break;
			case input_button_l_menu:      oxri_make_action_name(&action_info, handed_left,  "menu_click",  "Menu click");  break;
			case input_button_r_menu:      oxri_make_action_name(&action_info, handed_right, "menu_click",  "Menu click");  break;
			default:                       oxri_make_action_default(&action_info, 2, "bool", xra_val); break;
			}
			break;
		case xra_type_float:
			action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
			switch (xra_val) {
			case input_float_l_trigger:    oxri_make_action_name(&action_info, handed_left,  "trigger", "Trigger"); break;
			case input_float_r_trigger:    oxri_make_action_name(&action_info, handed_right, "trigger", "Trigger"); break;
			case input_float_l_grip:       oxri_make_action_name(&action_info, handed_left,  "grip",    "Grip");    break;
			case input_float_r_grip:       oxri_make_action_name(&action_info, handed_right, "grip",    "Grip");    break;
			default:                       oxri_make_action_default(&action_info, 2, "float", xra_val); break;
			}
			break;
		case xra_type_pose:
			action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
			switch (xra_val) {
			case input_pose_l_grip:        oxri_make_action_name(&action_info, handed_left,  "grip_pose",    "Grip pose");     break;
			case input_pose_r_grip:        oxri_make_action_name(&action_info, handed_right, "grip_pose",    "Grip pose");     break;
			case input_pose_l_palm:        oxri_make_action_name(&action_info, handed_left,  "palm_pose",    "Palm pose");     break;
			case input_pose_r_palm:        oxri_make_action_name(&action_info, handed_right, "palm_pose",    "Palm pose");     break;
			case input_pose_l_aim:         oxri_make_action_name(&action_info, handed_left,  "aim_pose",     "Aim pose");      break;
			case input_pose_r_aim:         oxri_make_action_name(&action_info, handed_right, "aim_pose",     "Aim pose");      break;
			case input_pose_l_poke:        oxri_make_action_name(&action_info, handed_left,  "poke_pose",    "Poke pose");     break;
			case input_pose_r_poke:        oxri_make_action_name(&action_info, handed_right, "poke_pose",    "Poke pose");     break;
			case input_pose_l_pinch:       oxri_make_action_name(&action_info, handed_left,  "pinch_pose",   "Pinch pose");    break;
			case input_pose_r_pinch:       oxri_make_action_name(&action_info, handed_right, "pinch_pose",   "Pinch pose");    break;
			case input_pose_eyes:          oxri_make_action_name(&action_info, 2,            "eyegaze_pose", "Eye gaze pose"); break;
			default:                       oxri_make_action_default(&action_info, 2, "pose", xra_val); break;
			}
			break;
		case xra_type_xy:
			action_info.actionType = XR_ACTION_TYPE_VECTOR2F_INPUT;
			switch (xra_val) {
			case input_xy_l_stick:         oxri_make_action_name(&action_info, handed_left,  "stick_xy", "Stick XY"); break;
			case input_xy_r_stick:         oxri_make_action_name(&action_info, handed_right, "stick_xy", "Stick XY"); break;
			default:                       oxri_make_action_default(&action_info, 2, "xy", xra_val); break;
			}
			break;
		case xra_type_haptic:
			action_info.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
			switch (xra_val) {
			case input_haptic_l_controller: oxri_make_action_name(&action_info, handed_left,  "haptic", "Haptic"); break;
			case input_haptic_r_controller: oxri_make_action_name(&action_info, handed_right, "haptic", "Haptic"); break;
			default:                        oxri_make_action_default(&action_info, 2, "haptic", xra_val); break;
			}
			break;
		case xra_type_max: break;
		}

		XrResult xr = xrCreateAction(local.action_set, &action_info, &result);
		if (XR_FAILED(xr)) {
			log_infof("%s [%s]", "xrCreateAction", openxr_string(xr));
			return XR_NULL_HANDLE;
		} else {
			actions->set(xra_val, result);
		}
	}
	return result;
}

///////////////////////////////////////////
// Haptic output                          //
///////////////////////////////////////////

// Pacing target: roughly two frames' worth of samples queued in the runtime at
// any time. ~33ms covers a 60fps app with one frame of headroom for stutters.
const float HAPTIC_CHUNK_SECONDS = 0.033f;
// Hard cap on a single xrApplyHapticFeedback submission. Quest's effective limit
// is in this neighborhood; mirroring the FB envelope extension's 4000-sample
// cap is a safe default for PCM as well.
const int32_t HAPTIC_MAX_CHUNK_SAMPLES = 4000;

///////////////////////////////////////////

static XrAction oxri_haptic_action(input_haptic_ idx) {
	array_t<XrAction>* actions = &local.actions[xra_type_haptic];
	if (idx >= actions->count) return XR_NULL_HANDLE;
	return actions->get(idx);
}

///////////////////////////////////////////

static int32_t oxri_haptic_top_level_for(input_haptic_ idx) {
	const char* str = idx == input_haptic_l_controller
		? "/user/hand/left"
		: "/user/hand/right";
	for (int32_t i = 0; i < local.top_levels.count; i++) {
		if (string_eq(local.top_levels[i].name, str)) return i;
	}
	return -1;
}

///////////////////////////////////////////

static void oxri_haptic_clear_stream(input_haptic_ idx) {
	haptic_stream_t* s = &local.haptic_streams[idx];
	s->pending.clear();
	s->pending_offset  = 0;
	s->active          = false;
	s->rate_hz         = 0;
}

///////////////////////////////////////////

// Computes the caps for a single haptic output given the current active
// interaction profile. Called only on interaction-profile change events; the
// public oxri_haptic_caps just returns the cached value.
static void oxri_haptic_recompute_caps(input_haptic_ haptic_type) {
	if (haptic_type >= input_haptic_max)                   { local.haptic_caps[haptic_type] = input_haptic_caps_none; return; }
	if (oxri_haptic_action(haptic_type) == XR_NULL_HANDLE) { local.haptic_caps[haptic_type] = input_haptic_caps_none; return; }

	int32_t tl = oxri_haptic_top_level_for(haptic_type);
	if (tl < 0 || local.top_levels[tl].active_profile < 0) { local.haptic_caps[haptic_type] = input_haptic_caps_none; return; }

	// The action exists if *any* profile bound it, but the runtime won't route
	// haptics unless the *currently active* profile is one of those. A profile
	// without a haptic binding (e.g. hand-tracking-only) means no output.
	if (!local.profiles[local.top_levels[tl].active_profile].has_haptic) { local.haptic_caps[haptic_type] = input_haptic_caps_none; return; }

	int32_t caps = input_haptic_caps_pulse;
	if (xr_fb_haptic_pcm_available()     ) caps |= input_haptic_caps_waveform;
	if (xr_fb_haptic_envelope_available()) caps |= input_haptic_caps_curve;
	local.haptic_caps[haptic_type] = (input_haptic_caps_)caps;
}

///////////////////////////////////////////

input_haptic_caps_ oxri_haptic_caps(input_haptic_ haptic_type) {
	if (haptic_type >= input_haptic_max) return input_haptic_caps_none;
	return local.haptic_caps[haptic_type];
}

///////////////////////////////////////////

float oxri_haptic_preferred_rate(input_haptic_ haptic_type) {
	if (haptic_type >= input_haptic_max) return 0;
	if (!xr_fb_haptic_pcm_available())   return 0;
	// -1 means "uncached", 0 means "queried, runtime accepts any rate". Without
	// this sentinel a 0 result would re-query every frame.
	if (local.haptic_pref_rate[haptic_type] >= 0)
		return local.haptic_pref_rate[haptic_type];

	XrAction action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE) return 0;

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action          = action;
	info.subactionPath   = XR_NULL_PATH;

	local.haptic_pref_rate[haptic_type] = xr_fb_haptic_pcm_get_sample_rate(&info);
	return local.haptic_pref_rate[haptic_type];
}

///////////////////////////////////////////

void oxri_haptic_pulse(input_haptic_ haptic_type, float frequency, float amplitude, double duration_seconds) {
	XrAction action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE) return;

	// Any new submission of a different type clears the streaming queue, so the
	// previous waveform doesn't keep leaking out chunks behind the new pulse.
	oxri_haptic_clear_stream(haptic_type);

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action        = action;
	info.subactionPath = XR_NULL_PATH;

	XrHapticVibration vibration = { XR_TYPE_HAPTIC_VIBRATION };
	vibration.amplitude = fmaxf(0.0f, fminf(1.0f, amplitude));
	vibration.frequency = frequency        > 0 ? frequency                            : XR_FREQUENCY_UNSPECIFIED;
	vibration.duration  = duration_seconds > 0 ? (XrDuration)(duration_seconds * 1e9) : XR_MIN_HAPTIC_DURATION;

	XrResult r = xrApplyHapticFeedback(xr_session, &info, (const XrHapticBaseHeader*)&vibration);
	if (XR_FAILED(r)) log_warnf("xrApplyHapticFeedback (pulse) failed [%s]", openxr_string(r));
}

///////////////////////////////////////////

void oxri_haptic_stop(input_haptic_ haptic_type) {
	XrAction action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE) return;

	oxri_haptic_clear_stream(haptic_type);

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action          = action;
	info.subactionPath   = XR_NULL_PATH;
	XrResult r = xrStopHapticFeedback(xr_session, &info);
	if (XR_FAILED(r)) log_warnf("xrStopHapticFeedback failed [%s]", openxr_string(r));
}

///////////////////////////////////////////

// Submits up to one chunk worth of pending samples to the runtime. Returns the
// runtime-reported samplesConsumed for the previous submission, or 0 if there
// was nothing in flight. is_append=false starts a fresh stream (cancelling any
// prior playback); is_append=true queues onto an already-running stream.
static int32_t oxri_haptic_submit_chunk(input_haptic_ haptic_type, bool is_append) {
	haptic_stream_t* s      = &local.haptic_streams[haptic_type];
	XrAction         action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE ||
		s->pending_offset >= s->pending.count) return 0;

	int32_t remaining   = s->pending.count - s->pending_offset;
	int32_t chunk_count = (int32_t)(s->rate_hz * HAPTIC_CHUNK_SECONDS);
	if (chunk_count > HAPTIC_MAX_CHUNK_SAMPLES) chunk_count = HAPTIC_MAX_CHUNK_SAMPLES;
	if (chunk_count < 1)                        chunk_count = 1;
	if (chunk_count > remaining)                chunk_count = remaining;

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action          = action;
	info.subactionPath   = XR_NULL_PATH;

	uint32_t consumed = 0;
	XrHapticPcmVibrationFB pcm = { XR_TYPE_HAPTIC_PCM_VIBRATION_FB };
	pcm.bufferSize      = (uint32_t)chunk_count;
	pcm.buffer          = &s->pending[s->pending_offset];
	pcm.sampleRate      = s->rate_hz;
	pcm.append          = is_append ? XR_TRUE : XR_FALSE;
	pcm.samplesConsumed = &consumed;
	XrResult r = xrApplyHapticFeedback(xr_session, &info, (const XrHapticBaseHeader*)&pcm);
	if (XR_FAILED(r)) log_warnf("xrApplyHapticFeedback (waveform) failed [%s]", openxr_string(r));

	s->pending_offset += chunk_count;
	s->active          = true;

	// Compact when we've drained more than half the buffer, to bound memory
	// growth on long-running streams without thrashing reallocations.
	if (s->pending_offset > 0 && s->pending_offset >= s->pending.count / 2) {
		int32_t keep = s->pending.count - s->pending_offset;
		if (keep > 0) memmove(s->pending.data, &s->pending.data[s->pending_offset], keep * sizeof(float));
		s->pending.count   = keep;
		s->pending_offset  = 0;
	}

	if (s->pending_offset >= s->pending.count) {
		// Buffer fully submitted to the runtime. Drop the pending storage but
		// leave 'active' true so subsequent appends know whether to use
		// append=true on their first chunk.
		s->pending.clear();
		s->pending_offset = 0;
	}

	return (int32_t)consumed;
}

///////////////////////////////////////////

void oxri_haptic_waveform(input_haptic_ haptic_type, const float* samples, int32_t sample_count, float sample_rate_hz, bool append, int32_t* out_prev_consumed) {
	if (out_prev_consumed) *out_prev_consumed = 0;
	if (!xr_fb_haptic_pcm_available()      ||
		haptic_type    >= input_haptic_max ||
		sample_count   <= 0                ||
		sample_rate_hz <= 0) return;

	haptic_stream_t* s      = &local.haptic_streams[haptic_type];
	XrAction         action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE) return;

	// Different sample rate forces a restart: a single in-flight stream can't
	// have its rate changed mid-flight, and append=false explicitly restarts.
	bool restart = !append || s->rate_hz != sample_rate_hz;
	if (restart) {
		if (s->active) {
			XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
			info.action        = action;
			info.subactionPath = XR_NULL_PATH;
			xrStopHapticFeedback(xr_session, &info);
		}
		oxri_haptic_clear_stream(haptic_type);
	}

	s->rate_hz = sample_rate_hz;
	s->pending.add_range(samples, sample_count);

	int32_t consumed = oxri_haptic_submit_chunk(haptic_type, restart ? false : true);
	if (out_prev_consumed) *out_prev_consumed = consumed;
}

///////////////////////////////////////////

void oxri_haptic_curve(input_haptic_ haptic_type, const float* amplitudes, int32_t sample_count, float sample_rate_hz) {
	if (!xr_fb_haptic_envelope_available() ||
		haptic_type    >= input_haptic_max ||
		sample_count   <= 0                ||
		sample_rate_hz <= 0) return;

	XrAction action = oxri_haptic_action(haptic_type);
	if (action == XR_NULL_HANDLE) return;

	// XR_FB_haptic_amplitude_envelope has no append concept; each submission
	// replaces the in-flight playback. So a curve always cancels any
	// streaming PCM as a side effect, and we cap at the documented limit.
	oxri_haptic_clear_stream(haptic_type);

	if (sample_count > XR_MAX_HAPTIC_AMPLITUDE_ENVELOPE_SAMPLES_FB) {
		sample_count = XR_MAX_HAPTIC_AMPLITUDE_ENVELOPE_SAMPLES_FB;
	}

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action        = action;
	info.subactionPath = XR_NULL_PATH;

	XrHapticAmplitudeEnvelopeVibrationFB env = { XR_TYPE_HAPTIC_AMPLITUDE_ENVELOPE_VIBRATION_FB };
	env.duration       = (XrDuration)((sample_count / sample_rate_hz) * 1e9);
	env.amplitudeCount = (uint32_t)sample_count;
	env.amplitudes     = amplitudes;
	XrResult r = xrApplyHapticFeedback(xr_session, &info, (const XrHapticBaseHeader*)&env);
	if (XR_FAILED(r)) log_warnf("xrApplyHapticFeedback (curve) failed [%s]", openxr_string(r));
}

///////////////////////////////////////////

void oxri_haptic_step() {
	// Drain one chunk per output per frame. The chunk size is sized to a few
	// frames' worth of samples, so the runtime stays a step or two ahead of
	// real time without unbounded queue growth.
	for (int32_t i = 0; i < input_haptic_max; i++) {
		haptic_stream_t* s = &local.haptic_streams[i];
		if (!s->active) continue;
		if (s->pending_offset < s->pending.count) {
			oxri_haptic_submit_chunk((input_haptic_)i, true);
		} else {
			// Nothing left to submit; the runtime is responsible for finishing
			// playback of what we already gave it.
			s->active = false;
		}
	}
}

} // namespace sk
#endif