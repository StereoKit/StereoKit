#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_extensions.h"
#include "openxr_input.h"
#include "../hands/hand_oxr_controller.h"
#include "../hands/input_hand.h"
#include "../systems/input.h"
#include "../systems/render.h"

#include "../libraries/array.h"
#include "../stereokit.h"
#include "../_stereokit.h"

#include <openxr/openxr.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct xrc_profile_info_t {
	const char* name;
	XrPath      profile;
	pose_t      offset[2];
};

union XrRLPath {
	struct {
		XrPath left;
		XrPath right;
	};
	struct {
		XrPath paths[2];
	};
};

XrActionSet xrc_action_set;
XrAction    xrc_action_eyes;
XrAction    xrc_action_pose_grip;
XrAction    xrc_action_pose_palm;
XrAction    xrc_action_pose_aim;
XrAction    xrc_action_trigger;
XrAction    xrc_action_grip;
XrAction    xrc_action_stick_xy;
XrAction    xrc_action_stick_click;
XrAction    xrc_action_x1;
XrAction    xrc_action_x2;
XrAction    xrc_action_menu;

XrRLPath    xrc_hand_subaction;
XrSpace     xrc_space_aim          [2];
XrSpace     xrc_space_grip         [2] = {};
XrSpace     xrc_space_palm         [2] = {};
XrSpace     xr_gaze_space              = {};

int32_t       xr_eyes_pointer;
button_state_ xr_tracked_state = button_state_inactive;

array_t<xrc_profile_info_t> xrc_profile_offsets = {};
XrPath                      xrc_active_profile[2] = { 0xFFFFFFFF, 0xFFFFFFFF };

///////////////////////////////////////////

void oxri_set_profile(handed_ hand, XrPath profile);

XrRLPath _bind_paths    (const char* name);
void     _bind          (array_t<XrActionSuggestedBinding>* arr, XrAction action, XrPath   path);
void     _bind_rl       (array_t<XrActionSuggestedBinding>* arr, XrAction action, XrRLPath rlpath);
bool     _bind_suggest  (const char* profile_name, const array_t<XrActionSuggestedBinding> binding_arr, pose_t palm_left_offset, pose_t palm_right_offset, xrc_profile_info_t* out_profile);
bool     _make_action_rl(const char* action_name, const char* display_name, XrActionType type, XrRLPath subaction_path, XrAction *out_action);
bool     _make_action   (const char* action_name, const char* display_name, XrActionType type, XrAction *out_action);

///////////////////////////////////////////

bool oxri_init() {
	xrc_offset_pos[0] = vec3_zero;
	xrc_offset_pos[1] = vec3_zero;
	xrc_offset_rot[0] = quat_identity;
	xrc_offset_rot[1] = quat_identity;

	xr_eyes_pointer = input_add_pointer(input_source_gaze | (device_has_eye_gaze() ? input_source_gaze_eyes : input_source_gaze_head));

	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	snprintf(actionset_info.actionSetName,          sizeof(actionset_info.actionSetName),          "input");
	snprintf(actionset_info.localizedActionSetName, sizeof(actionset_info.localizedActionSetName), "Input");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &xrc_action_set);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return false;
	}

	xrStringToPath(xr_instance, "/user/hand/left",  &xrc_hand_subaction.left);
	xrStringToPath(xr_instance, "/user/hand/right", &xrc_hand_subaction.right);

	if (!_make_action_rl("grip_pose",   "Grip Pose",   XR_ACTION_TYPE_POSE_INPUT,     xrc_hand_subaction, &xrc_action_pose_grip  )) return false;
	if (!_make_action_rl("aim_pose",    "Aim Pose",    XR_ACTION_TYPE_POSE_INPUT,     xrc_hand_subaction, &xrc_action_pose_aim   )) return false;
	if (!_make_action_rl("trigger",     "Trigger",     XR_ACTION_TYPE_FLOAT_INPUT,    xrc_hand_subaction, &xrc_action_trigger    )) return false;
	if (!_make_action_rl("grip",        "Grip",        XR_ACTION_TYPE_FLOAT_INPUT,    xrc_hand_subaction, &xrc_action_grip       )) return false;
	if (!_make_action_rl("stick_xy",    "Stick XY",    XR_ACTION_TYPE_VECTOR2F_INPUT, xrc_hand_subaction, &xrc_action_stick_xy   )) return false;
	if (!_make_action_rl("stick_click", "Stick Click", XR_ACTION_TYPE_BOOLEAN_INPUT,  xrc_hand_subaction, &xrc_action_stick_click)) return false;
	if (!_make_action_rl("menu",        "Menu",        XR_ACTION_TYPE_BOOLEAN_INPUT,  xrc_hand_subaction, &xrc_action_menu       )) return false;
	if (!_make_action_rl("button_x1",   "Button X1",   XR_ACTION_TYPE_BOOLEAN_INPUT,  xrc_hand_subaction, &xrc_action_x1         )) return false;
	if (!_make_action_rl("button_x2",   "Button X2",   XR_ACTION_TYPE_BOOLEAN_INPUT,  xrc_hand_subaction, &xrc_action_x2         )) return false;
	if (!_make_action_rl("palm_pose",   "Palm Pose",   XR_ACTION_TYPE_POSE_INPUT,     xrc_hand_subaction, &xrc_action_pose_palm  )) return false;
	if (!_make_action   ("eye_gaze",    "Eye Gaze",    XR_ACTION_TYPE_POSE_INPUT,                         &xrc_action_eyes       )) return false;

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrRLPath path_pose_aim      = _bind_paths("aim/pose");
	XrRLPath path_pose_grip     = _bind_paths("grip/pose");
	XrRLPath path_trigger_val   = _bind_paths("trigger/value");
	XrRLPath path_select_val    = _bind_paths("select/value");
	XrRLPath path_select_click  = _bind_paths("select/click");
	XrRLPath path_squeeze_val   = _bind_paths("squeeze/value");
	XrRLPath path_squeeze_click = _bind_paths("squeeze/click");
	XrRLPath path_stick_xy      = _bind_paths("thumbstick");
	XrRLPath path_stick_click   = _bind_paths("thumbstick/click");
	XrRLPath path_system_click  = _bind_paths("system/click");
	XrRLPath path_menu_click    = _bind_paths("menu/click");
	XrRLPath path_back_click    = _bind_paths("back/click");
	XrRLPath path_btn_x         = _bind_paths("x/click");
	XrRLPath path_btn_y         = _bind_paths("y/click");
	XrRLPath path_btn_a         = _bind_paths("a/click");
	XrRLPath path_btn_b         = _bind_paths("b/click");

	XrRLPath path_pinch_val     = _bind_paths("pinch_ext/value");
	XrRLPath path_grasp_val     = _bind_paths("grasp_ext/value");
	XrRLPath path_pose_palm     = _bind_paths("palm_ext/pose");

	// OpenXR's ideal situation is that you provide bindings for the
	// controllers that you know and use. Runtimes are then supposed to remap
	// those bindings to the controller that is actually present. But in
	// reality, the remapping doesn't always happen.
	//
	// Here, we provide bindings for as many controllers as possible, so that
	// runtimes don't have the opportunity to drop the ball.


	xrc_profile_info_t                bind_info = {};
	array_t<XrActionSuggestedBinding> bind_arr  = {};

	// microsoft/motion_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_microsoft_mixed_reality_motion_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip    );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim     );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_trigger_val  );
		_bind_rl(&bind_arr, xrc_action_grip,        path_squeeze_click);
		_bind_rl(&bind_arr, xrc_action_stick_xy,    path_stick_xy     );
		_bind_rl(&bind_arr, xrc_action_stick_click, path_stick_click  );
		_bind_rl(&bind_arr, xrc_action_menu,        path_menu_click   );
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };
		if (_bind_suggest("microsoft/motion_controller", bind_arr, palm_offset, palm_offset, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// hp/mixed_reality_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hp_mixed_reality_controller
	if (xr_ext_available.EXT_hp_mixed_reality_controller) {
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip  );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim   );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_trigger_val);
		_bind_rl(&bind_arr, xrc_action_grip,        path_squeeze_val);
		_bind_rl(&bind_arr, xrc_action_stick_xy,    path_stick_xy   );
		_bind_rl(&bind_arr, xrc_action_stick_click, path_stick_click);
		_bind_rl(&bind_arr, xrc_action_menu,        path_menu_click );
		_bind_rl(&bind_arr, xrc_action_x1,          {path_btn_x.left, path_btn_a.right});
		_bind_rl(&bind_arr, xrc_action_x2,          {path_btn_y.left, path_btn_b.right});
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset = device_display_get_blend() == display_blend_opaque
			? pose_t{ {0.01f, -0.01f,  0.015f}, quat_from_angles(-45, 0, 0) }
			: pose_t{ {0,      0.005f, 0     }, quat_from_angles(-68, 0, 0) };
		if (_bind_suggest("hp/mixed_reality_controller", bind_arr, palm_offset, palm_offset, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// ext/hand_interaction_ext
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#ext_hand_interaction_profile
	if (xr_ext_available.EXT_hand_interaction) {
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip, path_pose_grip);
		_bind_rl(&bind_arr, xrc_action_pose_aim,  path_pose_aim );
		_bind_rl(&bind_arr, xrc_action_trigger,   path_pinch_val);
		_bind_rl(&bind_arr, xrc_action_grip,      path_grasp_val);
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		if (_bind_suggest("ext/hand_interaction_ext", bind_arr, pose_identity, pose_identity, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// Bytedance PICO Neo3
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
	if (xr_ext_available.BD_controller_interaction) {
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,  path_pose_grip  );
		_bind_rl(&bind_arr, xrc_action_pose_aim,   path_pose_aim   );
		_bind_rl(&bind_arr, xrc_action_trigger,    path_trigger_val);
		_bind_rl(&bind_arr, xrc_action_grip,       path_squeeze_val);
		_bind_rl(&bind_arr, xrc_action_stick_xy,   path_stick_xy   );
		_bind_rl(&bind_arr, xrc_action_stick_click,path_stick_click);
		_bind_rl(&bind_arr, xrc_action_menu,       path_menu_click );
		_bind_rl(&bind_arr, xrc_action_x1,         {path_btn_x.left, path_btn_a.right});
		_bind_rl(&bind_arr, xrc_action_x2,         {path_btn_y.left, path_btn_b.right});
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("bytedance/pico_neo3_controller", bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// Bytedance Pico 4
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#XR_BD_controller_interaction
	// Note that on the pico 4 OS 5.5 OpenXR SDK 2.2, the xrGetCurrentInteractionProfile will return '/interaction_profiles/bytedance/pico_neo3_controller'
	// instead of the expected '/interaction_profiles/bytedance/pico4_controller'
	if (xr_ext_available.BD_controller_interaction) {
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,  path_pose_grip  );
		_bind_rl(&bind_arr, xrc_action_pose_aim,   path_pose_aim   );
		_bind_rl(&bind_arr, xrc_action_trigger,    path_trigger_val);
		_bind_rl(&bind_arr, xrc_action_grip,       path_squeeze_val);
		_bind_rl(&bind_arr, xrc_action_stick_xy,   path_stick_xy   );
		_bind_rl(&bind_arr, xrc_action_stick_click,path_stick_click);
		_bind   (&bind_arr, xrc_action_menu,       path_menu_click.left);
		_bind_rl(&bind_arr, xrc_action_x1,         {path_btn_x.left, path_btn_a.right});
		_bind_rl(&bind_arr, xrc_action_x2,         {path_btn_y.left, path_btn_b.right});
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("bytedance/pico4_controller", bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// htc/vive_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_htc_vive_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip    );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim     );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_trigger_val  );
		_bind_rl(&bind_arr, xrc_action_grip,        path_squeeze_click);
		_bind_rl(&bind_arr, xrc_action_menu,        path_menu_click   );
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset_left  = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		if (_bind_suggest("htc/vive_controller", bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// valve/index_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_valve_index_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip   );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim    );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_trigger_val );
		_bind_rl(&bind_arr, xrc_action_grip,        path_squeeze_val );
		_bind_rl(&bind_arr, xrc_action_stick_xy,    path_stick_xy    );
		_bind_rl(&bind_arr, xrc_action_stick_click, path_stick_click );
		_bind_rl(&bind_arr, xrc_action_menu,        path_system_click);
		_bind_rl(&bind_arr, xrc_action_x1,          path_btn_a       );
		_bind_rl(&bind_arr, xrc_action_x2,          path_btn_b       );
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset_left  = pose_t{ {-0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.035f, 0, 0}, quat_from_angles(-40, 0, 0) };
		if (_bind_suggest("valve/index_controller", bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}

	// oculus/touch_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_oculus_touch_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip  );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim   );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_trigger_val);
		_bind_rl(&bind_arr, xrc_action_grip,        path_squeeze_val);
		_bind_rl(&bind_arr, xrc_action_stick_xy,    path_stick_xy   );
		_bind_rl(&bind_arr, xrc_action_stick_click, path_stick_click);
		_bind   (&bind_arr, xrc_action_menu,        path_menu_click.left);
		_bind_rl(&bind_arr, xrc_action_x1,          {path_btn_x.left, path_btn_a.right} );
		_bind_rl(&bind_arr, xrc_action_x2,          {path_btn_y.left, path_btn_b.right} );
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		pose_t palm_offset_left  = pose_t{ {-0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		pose_t palm_offset_right = pose_t{ { 0.03f, 0.01f, 0 }, quat_from_angles(-80, 0, 0) };
		if (_bind_suggest("oculus/touch_controller", bind_arr, palm_offset_left, palm_offset_right, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}
	bind_arr.clear();

#if !defined(SK_OS_ANDROID)
	// khr/simple_controller
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_khronos_simple_controller_profile
	{
		bind_arr.clear();
		_bind_rl(&bind_arr, xrc_action_pose_grip,   path_pose_grip    );
		_bind_rl(&bind_arr, xrc_action_pose_aim,    path_pose_aim     );
		_bind_rl(&bind_arr, xrc_action_trigger,     path_select_click );
		_bind_rl(&bind_arr, xrc_action_menu,        path_menu_click   );
		if (xr_ext_available.EXT_palm_pose) _bind_rl(&bind_arr, xrc_action_pose_palm, path_pose_palm);

		if (_bind_suggest("khr/simple_controller", bind_arr, pose_identity, pose_identity, &bind_info))
			xrc_profile_offsets.add(bind_info);
	}
#endif

	// Eye tracking
	if (device_has_eye_gaze()) {
		XrPath gaze_path;
		xrStringToPath(xr_instance, "/user/eyes_ext/input/gaze_ext/pose", &gaze_path);

		bind_arr.clear();
		_bind(&bind_arr, xrc_action_eyes, gaze_path);

		if (_bind_suggest("ext/eye_gaze_interaction", bind_arr, pose_identity, pose_identity, &bind_info)) {
			XrActionSpaceCreateInfo create_space = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
			create_space.action            = xrc_action_eyes;
			create_space.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			result = xrCreateActionSpace(xr_session, &create_space, &xr_gaze_space);
			if (XR_FAILED(result)) {
				log_warnf("Gaze xrCreateActionSpace failed: [%s]", openxr_string(result));
			}
		}
	}

	bind_arr.free();

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction.paths[i];
		action_space_info.action            = xrc_action_pose_grip;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_grip[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xrc_hand_subaction.paths[i];
		action_space_info.action            = xrc_action_pose_aim;
		result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_aim[i]);
		if (XR_FAILED(result)) {
			log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return false;
		}

		if (xr_ext_available.EXT_palm_pose) {
			action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
			action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
			action_space_info.subactionPath     = xrc_hand_subaction.paths[i];
			action_space_info.action            = xrc_action_pose_palm;
			result = xrCreateActionSpace(xr_session, &action_space_info, &xrc_space_palm[i]);
			if (XR_FAILED(result)) {
				log_errf("xrCreateActionSpace failed: [%s]", openxr_string(result));
				return false;
			}
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xrc_action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_errf("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return false;
	}

	oxri_set_profile(handed_left,  xrc_active_profile[handed_left ]);
	oxri_set_profile(handed_right, xrc_active_profile[handed_right]);
	return true;
}

///////////////////////////////////////////

void oxri_shutdown() {
	xrc_profile_offsets.free();
	if (xrc_space_grip[0]) { xrDestroySpace    (xrc_space_grip[0]); xrc_space_grip[0] = {}; }
	if (xrc_space_grip[1]) { xrDestroySpace    (xrc_space_grip[1]); xrc_space_grip[1] = {}; }
	if (xrc_action_set   ) { xrDestroyActionSet(xrc_action_set   ); xrc_action_set    = {}; }
}


///////////////////////////////////////////

button_state_ openxr_space_tracked() {
	return xr_tracked_state;
}

///////////////////////////////////////////

void oxri_update_poses() {
	// We occasionally need to update poses multiple times per frame, to
	// account for camera movement, or predicted time updates. This code is 
	// called separate from button press and tracking events to prevent
	// issues with hiding 'just_active/inactive' events.

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xrc_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	matrix root   = render_get_cam_final    ();
	quat   root_q = matrix_extract_rotation(root);

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
	xr_tracked_state = button_make_state(
		(xr_tracked_state            & button_state_active)                    != 0,
		(head_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT) != 0);

	input_head_pose_world = matrix_transform_pose(root, input_head_pose_local);

	// Get input from whatever controllers may be present
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction.paths[hand];

		//// Pose actions

		// Controller grip pose
		XrActionStatePose state_grip = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_action_pose_grip;
		xrGetActionStatePose(xr_session, &get_info, &state_grip);

		// Get the grip pose the verbose way, since we want to know if
		// rotation or position are tracked independently of eachother.
		XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
		res = xrLocateSpace(xrc_space_grip[hand], xr_app_space, xr_time, &space_location);
		bool tracked_pos = false;
		bool tracked_rot = false;
		bool valid_pos   = false;
		bool valid_rot   = false;
		if (XR_UNQUALIFIED_SUCCESS(res)) {
			tracked_pos = (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT)    != 0;
			tracked_rot = (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT) != 0;
			valid_pos   = (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)      != 0;
			valid_rot   = (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)   != 0;
			if (valid_pos) {
				vec3 local_pos;
				memcpy(&local_pos, &space_location.pose.position, sizeof(vec3));
				input_controllers[hand].pose.position = root * local_pos;
			}
			if (valid_rot) {
				quat local_rot;
				memcpy(&local_rot, &space_location.pose.orientation, sizeof(quat));
				input_controllers[hand].pose.orientation = local_rot * root_q;
			}
			input_controllers[hand].tracked_pos = tracked_pos ? track_state_known : (valid_pos ? track_state_inferred : track_state_lost);
			input_controllers[hand].tracked_rot = tracked_rot ? track_state_known : (valid_rot ? track_state_inferred : track_state_lost);
		}

		// Get the palm position from either the extension, or our previous
		// fallback offsets from the grip pose.
		if (xr_ext_available.EXT_palm_pose) {
			space_location = { XR_TYPE_SPACE_LOCATION };
			res = xrLocateSpace(xrc_space_palm[hand], xr_app_space, xr_time, &space_location);
			if (XR_UNQUALIFIED_SUCCESS(res)) {
				pose_t local_palm;
				if (space_location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) {
					memcpy(&local_palm.position, &space_location.pose.position, sizeof(vec3));
					input_controllers[hand].palm.position = root * local_palm.position;
				}
				if (space_location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) {
					memcpy(&local_palm.orientation, &space_location.pose.orientation, sizeof(quat));
					input_controllers[hand].palm.orientation = local_palm.orientation * root_q;
				}
			}
		} else {
			input_controllers[hand].palm.orientation = xrc_offset_rot[hand] * input_controllers[hand].pose.orientation;
			input_controllers[hand].palm.position    = input_controllers[hand].pose.position + input_controllers[hand].palm.orientation * xrc_offset_pos[hand];
		}

		// Controller aim pose
		XrActionStatePose state_aim = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xrc_action_pose_aim;
		xrGetActionStatePose(xr_session, &get_info, &state_aim);
		pose_t local_aim;
		openxr_get_space(xrc_space_aim[hand], &local_aim);
		input_controllers[hand].aim = { root * local_aim.position, local_aim.orientation * root_q };
	}

	// eye input
	if (device_has_eye_gaze()) {
		pointer_t           *pointer     = input_get_pointer(xr_eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = xrc_action_eyes;
		xrGetActionStatePose(xr_session, &action_info, &action_pose);

		if (action_pose.isActive && openxr_get_gaze_space(&input_eyes_pose_local, xr_eyes_sample_time)) {
			input_eyes_pose_world.position    = root   * input_eyes_pose_local.position;
			input_eyes_pose_world.orientation = root_q * input_eyes_pose_local.orientation;

			pointer->ray.pos     = input_eyes_pose_world.position;
			pointer->ray.dir     = input_eyes_pose_world.orientation * vec3_forward;
			pointer->orientation = input_eyes_pose_world.orientation;
		}
	}
}

///////////////////////////////////////////

void oxri_update_frame() {
	// This function call already syncs OpenXR actions
	oxri_update_poses();

	// Get input from whatever controllers may be present
	bool menu_button = false;
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xrc_hand_subaction.paths[hand];

		//// Pose actions

		input_controllers[hand].tracked = button_make_state(
			input_controllers[hand].tracked & button_state_active,
			(input_controllers[hand].tracked_pos != track_state_lost ||
			 input_controllers[hand].tracked_rot != track_state_lost));

		//// Float actions

		// Trigger
		XrActionStateFloat state_trigger = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_action_trigger;
		xrGetActionStateFloat(xr_session, &get_info, &state_trigger);
		input_controllers[hand].trigger = state_trigger.currentState;

		// Grip button
		XrActionStateFloat state_grip_btn = { XR_TYPE_ACTION_STATE_FLOAT };
		get_info.action = xrc_action_grip;
		xrGetActionStateFloat(xr_session, &get_info, &state_grip_btn);
		input_controllers[hand].grip = state_grip_btn.currentState;

		// Analog stick X and Y
		XrActionStateVector2f grip_stick_xy = { XR_TYPE_ACTION_STATE_VECTOR2F };
		get_info.action = xrc_action_stick_xy;
		xrGetActionStateVector2f(xr_session, &get_info, &grip_stick_xy);
		input_controllers[hand].stick.x = -grip_stick_xy.currentState.x;
		input_controllers[hand].stick.y =  grip_stick_xy.currentState.y;

		//// Bool actions

		// Menu button
		XrActionStateBoolean state_menu = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_menu;
		xrGetActionStateBoolean(xr_session, &get_info, &state_menu);
		menu_button = menu_button || state_menu.currentState;

		// Stick click
		XrActionStateBoolean state_stick_click = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_stick_click;
		xrGetActionStateBoolean(xr_session, &get_info, &state_stick_click);
		input_controllers[hand].stick_click = button_make_state(input_controllers[hand].stick_click & button_state_active, state_stick_click.currentState);

		// Button x1 and x2
		XrActionStateBoolean state_x1 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		XrActionStateBoolean state_x2 = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xrc_action_x1;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x1);
		get_info.action = xrc_action_x2;
		xrGetActionStateBoolean(xr_session, &get_info, &state_x2);
		input_controllers[hand].x1 = button_make_state(input_controllers[hand].x1 & button_state_active, state_x1.currentState);
		input_controllers[hand].x2 = button_make_state(input_controllers[hand].x2 & button_state_active, state_x2.currentState);
	}
	input_controller_menubtn = button_make_state(input_controller_menubtn & button_state_active, menu_button);

	// eye input
	if (device_has_eye_gaze()) {
		pointer_t           *pointer     = input_get_pointer(xr_eyes_pointer);
		XrActionStatePose    action_pose = {XR_TYPE_ACTION_STATE_POSE};
		XrActionStateGetInfo action_info = {XR_TYPE_ACTION_STATE_GET_INFO};
		action_info.action = xrc_action_eyes;
		xrGetActionStatePose(xr_session, &action_info, &action_pose);

		input_eyes_track_state = button_make_state(input_eyes_track_state & button_state_active, action_pose.isActive);
		pointer->tracked = input_eyes_track_state;
	}
}

///////////////////////////////////////////

void oxri_set_profile(handed_ hand, XrPath profile) {
	xrc_active_profile[hand] = profile;
	for (int32_t i = 0; i < xrc_profile_offsets.count; i++) {
		if (xrc_profile_offsets[i].profile == profile) {
			xrc_offset_pos[hand] = xrc_profile_offsets[i].offset[hand].position;
			xrc_offset_rot[hand] = xrc_profile_offsets[i].offset[hand].orientation;
			log_diagf("Switched %s controller profile to %s", hand == handed_left ? "left" : "right", xrc_profile_offsets[i].name);
			break;
		}
	}
}

///////////////////////////////////////////

void oxri_update_interaction_profile() {
	XrPath path[2];
	xrStringToPath(xr_instance, "/user/hand/left",  &path[handed_left]);
	xrStringToPath(xr_instance, "/user/hand/right", &path[handed_right]);

	XrInteractionProfileState active_profile = { XR_TYPE_INTERACTION_PROFILE_STATE };
	for (int32_t h = 0; h < handed_max; h++) {
		if (XR_FAILED(xrGetCurrentInteractionProfile(xr_session, path[h], &active_profile)))
			continue;
		if (active_profile.interactionProfile != xrc_active_profile[h])
			oxri_set_profile((handed_)h, active_profile.interactionProfile);
	}
}

///////////////////////////////////////////

XrRLPath _bind_paths(const char* name)
{
	XrRLPath result = {};
	char     buffer[256];
	snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "left", name);
	XrResult xr = xrStringToPath(xr_instance, buffer, &result.left);
	if (XR_FAILED(xr)) {
		log_infof("xrStringToPath failed for '%s': [%s]", buffer, openxr_string(xr));
	}
	snprintf(buffer, sizeof(buffer), "/user/hand/%s/input/%s", "right", name);
	xr = xrStringToPath(xr_instance, buffer, &result.right);
	if (XR_FAILED(xr)) {
		log_infof("xrStringToPath failed for '%s': [%s]", buffer, openxr_string(xr));
	}
	return result;
}

///////////////////////////////////////////

void _bind_rl(array_t<XrActionSuggestedBinding>* arr, XrAction action, XrRLPath rlpath) {
	arr->add({ action, rlpath.left });
	arr->add({ action, rlpath.right });
}

///////////////////////////////////////////

void _bind(array_t<XrActionSuggestedBinding>* arr, XrAction action, XrPath path) {
	arr->add({ action, path });
}

///////////////////////////////////////////

bool _bind_suggest(const char *profile_name, const array_t<XrActionSuggestedBinding> binding_arr, pose_t palm_left_offset, pose_t palm_right_offset, xrc_profile_info_t *out_profile) {
	*out_profile = {};

	XrPath interaction_path;
	char   buffer[256];
	snprintf(buffer, sizeof(buffer), "/interaction_profiles/%s", profile_name);
	xrStringToPath(xr_instance, buffer, &interaction_path);

	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggested_binds.interactionProfile     = interaction_path;
	suggested_binds.suggestedBindings      = binding_arr.data;
	suggested_binds.countSuggestedBindings = binding_arr.count;
	if (XR_FAILED(xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds)))
		return false;

	out_profile->profile              = interaction_path;
	out_profile->name                 = profile_name;
	out_profile->offset[handed_left ] = palm_left_offset;
	out_profile->offset[handed_right] = palm_right_offset;
	return true;
}

///////////////////////////////////////////

bool _make_action_arr(const char* action_name, const char* display_name, XrActionType type, XrPath *subaction_path, int32_t path_count, XrAction *out_action) {
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = path_count;
	action_info.subactionPaths      = subaction_path;
	action_info.actionType          = type;
	snprintf(action_info.actionName,          sizeof(action_info.actionName         ), "%s", action_name);
	snprintf(action_info.localizedActionName, sizeof(action_info.localizedActionName), "%s", display_name);
	XrResult result = xrCreateAction(xrc_action_set, &action_info, out_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return false;
	}
	return true;
}

///////////////////////////////////////////

bool _make_action_rl(const char* action_name, const char* display_name, XrActionType type, XrRLPath subaction_path, XrAction *out_action) {
	XrPath paths[2] = { subaction_path.left, subaction_path.right };
	return _make_action_arr(action_name, display_name, type, paths, 2, out_action);
}

///////////////////////////////////////////

bool _make_action(const char* action_name, const char* display_name, XrActionType type, XrAction *out_action) {
	return _make_action_arr(action_name, display_name, type, nullptr, 0, out_action);
}

} // namespace sk
#endif