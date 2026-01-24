/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "../stereokit.h"
#include "../_stereokit.h"

#include "../platforms/platform.h"

#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_view.h"
#include "openxr_platform.h"

#include "../sk_memory.h"
#include "../log.h"
#include "../device.h"
#include "../libraries/stref.h"
#include "../systems/render.h"
#include "../systems/render_pipeline.h"
#include "../systems/audio.h"
#include "../systems/input.h"
#include "../systems/world.h"

#include "extensions/ext_management.h"
#include "extensions/_registration.h"
#include "extensions/vulkan_enable.h"
#include "extensions/android_thread.h"
#include "extensions/loader_init.h"
#include "ska_input.h"

#include <openxr/openxr.h>
#include <openxr/openxr_reflection.h>

#include <sk_app.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

XrFormFactor xr_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
const char *xr_request_layers[] = {
#if defined(SK_DEBUG)
	"XR_APILAYER_LUNARG_core_validation",
#else
	"",
#endif
};

XrInstance           xr_instance          = {};
XrSession            xr_session           = {};
XrSessionState       xr_session_state     = XR_SESSION_STATE_UNKNOWN;
bool                 xr_has_session       = false;
XrSpace              xr_app_space         = {};
XrReferenceSpaceType xr_app_space_type    = {};
XrSpace              xr_stage_space       = {};
XrSpace              xr_head_space        = {};
XrSystemId           xr_system_id         = XR_NULL_SYSTEM_ID;
XrTime               xr_time              = 0;
XrTime               xr_eyes_sample_time  = 0;
bool                 xr_system_created    = false;
bool                 xr_system_success    = false;
xr_runtime_          xr_known_runtime     = xr_runtime_none;

bool                 xr_has_bounds        = false;
vec2                 xr_bounds_size       = {};
pose_t               xr_bounds_pose_local = pose_identity;

XrReferenceSpaceType xr_refspace;

///////////////////////////////////////////

bool32_t openxr_try_get_app_space(XrSession session, origin_mode_ mode, XrTime time, XrReferenceSpaceType *out_space_type, pose_t* out_space_offset, XrSpace *out_app_space);
void     openxr_list_layers      (array_t<char*>* ref_available_layers, array_t<const char*>* ref_request_layers);
bool     openxr_blank_frame      ();

///////////////////////////////////////////

inline bool openxr_loc_valid(XrSpaceLocation &loc) {
	return
		(loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT   ) != 0 &&
		(loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
}

///////////////////////////////////////////

const char *openxr_string(XrResult result) {
	switch (result) {
#define ENTRY(NAME, VALUE) \
	case VALUE: return #NAME;
		XR_LIST_ENUM_XrResult(ENTRY)
#undef ENTRY
	default: return "<UNKNOWN>";
	}
}

///////////////////////////////////////////

bool openxr_get_stage_bounds(vec2 *out_size, pose_t *out_pose, XrTime time) {
	if (!xr_stage_space) return false;

	if (!openxr_get_space(xr_stage_space, out_pose, time))
		return false;

	XrExtent2Df bounds;
	XrResult    res = xrGetReferenceSpaceBoundsRect(xr_session, XR_REFERENCE_SPACE_TYPE_STAGE, &bounds);
	if (XR_SUCCEEDED(res) && res != XR_SPACE_BOUNDS_UNAVAILABLE) {
		out_size->x = bounds.width;
		out_size->y = bounds.height;
		log_diagf("Bounds updated: %.2g<~BLK>x<~clr>%.2g at (%.1g,%.1g,%.1g) (%.2g,%.2g,%.2g,%.2g)",
			out_size->x, out_size->y,
			out_pose->position   .x, out_pose->position   .y, out_pose->position   .z,
			out_pose->orientation.x, out_pose->orientation.y, out_pose->orientation.z, out_pose->orientation.w);
	}

	return true;
}

///////////////////////////////////////////

void openxr_show_ext_table(array_t<const char*> exts_request, array_t<char*> exts_available, array_t<const char*> layers_request, array_t<char*> layers_available) {
	if (exts_request    .count == 0 &&
		exts_available  .count == 0 &&
		layers_request  .count == 0 &&
		layers_available.count == 0) return;

	log_diag("OpenXR extensions:");
	log_diag("<~BLK>___________________________________<~clr>");
	log_diag("<~BLK>|     <~YLW>Usage <~BLK>| <~YLW>Extension<~clr>");
	log_diag("<~BLK>|-----------|----------------------<~clr>");
	for (int32_t i = 0; i < exts_available.count; i++) log_diagf("<~BLK>|   <~clr>present <~BLK>| <~clr>%s",       exts_available[i]);
	for (int32_t i = 0; i < exts_request  .count; i++) log_diagf("<~BLK>| <~CYN>ACTIVATED <~BLK>| <~GRN>%s<~clr>", exts_request  [i]);
	if (layers_request.count != 0 || layers_available.count != 0) {
		log_diag("<~BLK>|-----------|----------------------<~clr>");
		log_diag("<~BLK>|           | <~YLW>Layers<~clr>");
		log_diag("<~BLK>|-----------|----------------------<~clr>");
		for (int32_t i = 0; i < layers_available.count; i++) log_diagf("<~BLK>|   <~clr>present <~BLK>| <~clr>%s",       layers_available[i]);
		for (int32_t i = 0; i < layers_request  .count; i++) log_diagf("<~BLK>| <~CYN>ACTIVATED <~BLK>| <~GRN>%s<~clr>", layers_request  [i]);
	}
	log_diag("<~BLK>|___________|______________________<~clr>");
}

///////////////////////////////////////////

bool openxr_create_system() {
	if (xr_system_created == true) return xr_system_success;
	xr_system_success = false;
	xr_system_created = true;

	// Loader initialization must come before even instance data is enumerated
	if (xr_ext_loader_init() == xr_system_fail_critical) {
		log_warnf("OpenXR initialization failed during event: %s", "Loader Initialization");
		openxr_cleanup();
		return false;
	}

	// Enumerate available extensions first, so ext_management_ext_available
	// can be used during registration
	if (!ext_management_enumerate_available()) {
		log_warnf("OpenXR initialization failed during event: %s", "Extension Enumeration");
		openxr_cleanup();
		return false;
	}

	if (!ext_registration()) {
		log_warnf("OpenXR initialization failed during event: %s", "Extension Registration");
		openxr_cleanup();
		return false;
	}

	array_t<const char*> exts_request   = {};
	array_t<char*>       exts_available = {};
	ext_management_select_exts(ext_management_get_use_min(), &exts_available, &exts_request);

	array_t<const char*> layers_request   = {};
	array_t<char*>       layers_available = {};
	openxr_list_layers(&layers_available, &layers_request);

	XrInstanceCreateInfo create_info = { XR_TYPE_INSTANCE_CREATE_INFO };
	create_info.enabledExtensionCount = (uint32_t)exts_request.count;
	create_info.enabledExtensionNames =           exts_request.data;
	create_info.enabledApiLayerCount  = (uint32_t)layers_request.count;
	create_info.enabledApiLayerNames  =           layers_request.data;
	create_info.applicationInfo.applicationVersion = 1;

	// Use a version Id formatted as 0xMMMiiPPP
	create_info.applicationInfo.engineVersion =
		(SK_VERSION_MAJOR << 20)              |
		(SK_VERSION_MINOR << 12 & 0x000FF000) |
		(SK_VERSION_PATCH       & 0x00000FFF);

	create_info.applicationInfo.apiVersion = XR_MAKE_VERSION(1,0,XR_VERSION_PATCH(XR_CURRENT_API_VERSION));
	snprintf(create_info.applicationInfo.applicationName, sizeof(create_info.applicationInfo.applicationName), "%s", sk_get_settings_ref()->app_name);
	snprintf(create_info.applicationInfo.engineName,      sizeof(create_info.applicationInfo.engineName     ), "StereoKit");

	// Execute any extensions that want to run right before creating the
	// instance! This can also give them the chance to modify or chain items
	// into the create_info for the instance.
	if (!ext_management_evt_pre_instance_create(&create_info)) {
		log_warnf("OpenXR initialization failed during event: %s", "Pre-Instance Creation");
		openxr_cleanup();
		return false;
	}

	XrResult result = xrCreateInstance(&create_info, &xr_instance);

	// If we succeeded, log some infor g about our setup.
	if (XR_SUCCEEDED(result) && xr_instance != XR_NULL_HANDLE) {
		log_diagf("OpenXR API:     %u.%u.%u",
			XR_VERSION_MAJOR(create_info.applicationInfo.apiVersion),
			XR_VERSION_MINOR(create_info.applicationInfo.apiVersion),
			XR_VERSION_PATCH(create_info.applicationInfo.apiVersion));

		// Fetch the runtime name/info, for logging and for a few other checks
		XrInstanceProperties inst_properties = { XR_TYPE_INSTANCE_PROPERTIES };
		xr_check(xrGetInstanceProperties(xr_instance, &inst_properties),
			"xrGetInstanceProperties");

		device_data.runtime = string_copy(inst_properties.runtimeName);
		log_diagf("OpenXR runtime: <~grn>%s<~clr> %u.%u.%u",
			inst_properties.runtimeName,
			XR_VERSION_MAJOR(inst_properties.runtimeVersion),
			XR_VERSION_MINOR(inst_properties.runtimeVersion),
			XR_VERSION_PATCH(inst_properties.runtimeVersion));

		// This is an incomplete list of runtimes, we sometimes use these
		// internally for runtime compatibility fixes.
		xr_known_runtime = xr_runtime_unknown;
		if      (string_startswith(inst_properties.runtimeName, "Meta"                 )) xr_known_runtime = xr_runtime_meta;
		else if (string_startswith(inst_properties.runtimeName, "Oculus"               )) xr_known_runtime = xr_runtime_meta;
		else if (string_startswith(inst_properties.runtimeName, "Monado"               )) xr_known_runtime = xr_runtime_monado;
		else if (string_startswith(inst_properties.runtimeName, "Android XR"           )) xr_known_runtime = xr_runtime_android_xr;
		else if (string_startswith(inst_properties.runtimeName, "Vive"                 )) xr_known_runtime = xr_runtime_vive;
		else if (string_startswith(inst_properties.runtimeName, "SteamVR/OpenXR"       )) xr_known_runtime = xr_runtime_steamvr;
		else if (string_startswith(inst_properties.runtimeName, "Windows Mixed Reality")) xr_known_runtime = xr_runtime_wmr;
		else if (strstr           (inst_properties.runtimeName, "Snapdragon"           )) xr_known_runtime = xr_runtime_snapdragon;
	}

	// Always log the extension table, this may contain information about why
	// we failed to load.
	openxr_show_ext_table(exts_request, exts_available, layers_request, layers_available);
	exts_request.free();
	for (int32_t i =0; i<exts_available.count; i++) sk_free(exts_available[i]);
	exts_available.free();
	layers_request.free();
	for (int32_t i =0; i<layers_available.count; i++) sk_free(layers_available[i]);
	layers_available.free();

	// If the instance is null here, the user needs to install an OpenXR
	// runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	// Request a form factor from the device (HMD, Handheld, etc.)
	XrSystemGetInfo system_info = { XR_TYPE_SYSTEM_GET_INFO };
	system_info.formFactor = xr_config_form;
	result = xrGetSystem(xr_instance, &system_info, &xr_system_id);
	if (XR_FAILED(result)) {
		log_fail_reasonf(90, log_inform, "Couldn't find our desired MR form factor, no MR device attached/ready? [%s]", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	xr_system_success = true;
	return xr_system_success;
}

///////////////////////////////////////////

bool openxr_init() {
	if (!openxr_create_system())
		return false;

	device_data.display_type = display_type_stereo;

	// The Session gets created before checking capabilities! In certain
	// contexts, such as Holographic Remoting, the system won't know about its
	// capabilities until the session is ready. Holographic Remoting knows to
	// make the session ready immediately after xrCreateSession, so we don't
	// need to wait all the way until the OpenXR message loop tells us.
	// See here for more info on Holographic Remoting's lifecycle:
	// https://learn.microsoft.com/en-us/windows/mixed-reality/develop/native/holographic-remoting-create-remote-openxr?source=recommendations#connect-to-the-device

	// A session represents this application's desire to display things! This
	// is where we hook up our graphics API. This does not start the session,
	// for that, you'll need a call to xrBeginSession, which we do in
	// openxr_poll_events
	XrSessionCreateInfo session_info = { XR_TYPE_SESSION_CREATE_INFO };
	session_info.systemId = xr_system_id;

	// Before we call xrCreateSession, lets fire an event for anyone that needs
	// to set things up! This gives extensions the opportunity to insert items
	// into the session creation "next" chain, such as our graphics API info.
	if (!ext_management_evt_pre_session_create(&session_info)) {
		log_warnf("OpenXR initialization failed during event: %s", "Pre-Session Creation");
		openxr_cleanup();
		return false;
	}

	XrResult result = xrCreateSession(xr_instance, &session_info, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (XR_FAILED(result) || xr_session == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	// Figure out what this device is capable of!
	XrSystemProperties properties = { XR_TYPE_SYSTEM_PROPERTIES };
	xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties),
		"xrGetSystemProperties");
	log_diagf("System name: <~grn>%s<~clr>", properties.systemName);
	device_data.name     = string_copy(properties.systemName);
	device_data.tracking = device_tracking_none;
	if      (properties.trackingProperties.positionTracking)    device_data.tracking = device_tracking_6dof;
	else if (properties.trackingProperties.orientationTracking) device_data.tracking = device_tracking_3dof;

	if (!openxr_views_create()) {
		openxr_cleanup();
		return false;
	}

	// The space for our head
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_VIEW;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_head_space);
	if (XR_FAILED(result)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	// And stage space, so we can get bounds and floor info if it's available
	ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_STAGE;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_stage_space);
	if (XR_FAILED(result)) {
		xr_stage_space = {};
	}

	// Wait for the session to start before we do anything more with the 
	// spaces, reference spaces are sometimes invalid before session start. We
	// need to submit blank frames in order to get past the READY state.
	while (xr_session_state == XR_SESSION_STATE_IDLE || xr_session_state == XR_SESSION_STATE_UNKNOWN) {
		ska_time_sleep(33);
		if (!openxr_poll_events()) { log_infof("Exit event during initialization"); openxr_cleanup(); return false; }
	}
	// Blank frames should only be submitted when the session is READY
	while (xr_session_state == XR_SESSION_STATE_READY) {
		openxr_blank_frame();
		if (!openxr_poll_events()) { log_infof("Exit event during initialization"); openxr_cleanup(); return false; }
	}

	// Initialize XR systems
	if (!ext_management_evt_session_ready()) {
		log_warnf("OpenXR initialization failed during event: %s", "Session Ready");
		openxr_cleanup();
		return false;
	}

	// On Android, tell OpenXR what kind of thread this is. This can be
	// important on Android systems so we don't get treated as a low priority
	// thread by accident.
	xr_ext_android_thread_set_type(xr_thread_type_render_main);

	// Create reference spaces! So we can find stuff relative to them :) Some
	// platforms still take time after session start before the spaces provide
	// valid data, so we'll wait for that here.
	// TODO: Loop here may be optional, but some platforms may need it? Waiting
	// for some feedback here.
	// - HTC Vive XR Elite requires around 50 frames
	int32_t ref_space_tries = 1000;
	while (openxr_try_get_app_space(xr_session, sk_get_settings_ref()->origin, xr_time, &xr_app_space_type, &world_origin_offset, &xr_app_space) == false && openxr_poll_events() && ref_space_tries > 0) {
		ref_space_tries--;
		log_diagf("Failed getting reference spaces: %d tries remaining", ref_space_tries);
		openxr_blank_frame();
	}
	switch (xr_app_space_type) {
		case XR_REFERENCE_SPACE_TYPE_STAGE:           world_origin_mode = origin_mode_stage; break;
		case XR_REFERENCE_SPACE_TYPE_LOCAL:           world_origin_mode = origin_mode_local; break;
		case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:  world_origin_mode = origin_mode_local; break;
		case XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT: world_origin_mode = origin_mode_floor; break;
		default:                                      world_origin_mode = origin_mode_local; break;
	}

	xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);

	return true;
}

///////////////////////////////////////////

bool openxr_blank_frame() {
	XrFrameWaitInfo wait_info   = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState    frame_state = { XR_TYPE_FRAME_STATE };
	xr_check(xrWaitFrame(xr_session, &wait_info, &frame_state),
		"blank xrWaitFrame");

	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	xr_check(xrBeginFrame(xr_session, &begin_info),
		"blank xrBeginFrame");

	XrFrameEndInfo end_info = { XR_TYPE_FRAME_END_INFO };
	end_info.displayTime = frame_state.predictedDisplayTime;
	if      (xr_blend_valid(display_blend_opaque  )) end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	else if (xr_blend_valid(display_blend_additive)) end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;
	else if (xr_blend_valid(display_blend_blend   )) end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND;
	xr_check(xrEndFrame(xr_session, &end_info),
		"blank xrEndFrame");

	return true;
}

///////////////////////////////////////////

void openxr_list_layers(array_t<char*>* ref_available_layers, array_t<const char*>* ref_request_layers) {
	// Find what extensions are available on this system!
	uint32_t layer_count = 0;
	xrEnumerateApiLayerProperties(0, &layer_count, nullptr);
	XrApiLayerProperties *layers = sk_malloc_t(XrApiLayerProperties, layer_count);
	for (uint32_t i = 0; i < layer_count; i++) layers[i] = { XR_TYPE_API_LAYER_PROPERTIES };
	xrEnumerateApiLayerProperties(layer_count, &layer_count, layers);

	// Fill up our arrays based on if we want to request them, or if they're
	// just available.
	for (uint32_t i = 0; i < layer_count; i++) {
		int32_t is_requested = -1;
		for (int32_t e = 0; e < _countof(xr_request_layers); e++) {
			if (strcmp(layers[i].layerName, xr_request_layers[e]) == 0) {
				is_requested = e;
				break;
			}
		}
		if (is_requested != -1) ref_request_layers  ->add(xr_request_layers[is_requested]);
		else                    ref_available_layers->add(string_copy(layers[i].layerName));
	}

	sk_free(layers);
}

///////////////////////////////////////////

bool32_t openxr_space_try_get_offset(XrSession session, XrTime time, XrReferenceSpaceType space_type, XrReferenceSpaceType base_space_type, pose_t *out_pose) {
	*out_pose = pose_identity;

	bool32_t result     = false;
	XrSpace  base_space = {};
	XrReferenceSpaceCreateInfo view_info = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	view_info.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	view_info.referenceSpaceType   = base_space_type;
	if (XR_FAILED(xrCreateReferenceSpace(session, &view_info, &base_space))) {
		return false;
	}

	XrSpace space = {};
	view_info.referenceSpaceType = space_type;
	if (XR_FAILED(xrCreateReferenceSpace(session, &view_info, &space))) {
		if (base_space != XR_NULL_HANDLE) xrDestroySpace(base_space);
		return false;
	}

	XrSpaceLocation loc = { XR_TYPE_SPACE_LOCATION };
	XrResult        res = xrLocateSpace(space, base_space, time, &loc);
	if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(loc)) {
		result = true;
		memcpy(&out_pose->position,    &loc.pose.position,    sizeof(vec3));
		memcpy(&out_pose->orientation, &loc.pose.orientation, sizeof(quat));
	} else {
		result = false;
	}
	if (base_space != XR_NULL_HANDLE) xrDestroySpace(base_space);
	if (space      != XR_NULL_HANDLE) xrDestroySpace(space);
	return result;
}

///////////////////////////////////////////

bool32_t openxr_try_get_app_space(XrSession session, origin_mode_ mode, XrTime time, XrReferenceSpaceType *out_space_type, pose_t *out_space_offset, XrSpace *out_app_space) {
	// Find the spaces OpenXR has access to on this device
	uint32_t refspace_count = 0;
	xrEnumerateReferenceSpaces(session, 0, &refspace_count, nullptr);
	XrReferenceSpaceType *refspace_types = sk_malloc_t(XrReferenceSpaceType, refspace_count);
	xrEnumerateReferenceSpaces(session, refspace_count, &refspace_count, refspace_types);

	// Turn the list into easy flags
	bool has_local       = false; // This must always be found, according to the spec
	bool has_stage       = false;
	bool has_local_floor = false;
	bool has_unbounded   = false;
	for (uint32_t i = 0; i < refspace_count; i++) {
		switch (refspace_types[i]) {
		case XR_REFERENCE_SPACE_TYPE_LOCAL:           has_local       = true; break;
		case XR_REFERENCE_SPACE_TYPE_STAGE:           has_stage       = true; break;
		case XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT: has_local_floor = true; break;
		// It's possible runtimes may be providing this despite the extension
		// not being enabled? So we're forcing it here.
		case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:  has_unbounded   = backend_openxr_ext_enabled(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME); break;
		default: break;
		}
	}
	sk_free(refspace_types);

	XrReferenceSpaceType base_space = XR_REFERENCE_SPACE_TYPE_LOCAL;
	pose_t               offset     = pose_identity;
	switch (mode) {
	case origin_mode_local: {
		// The origin needs to be at the user's head. Not all runtimes do this
		// exactly, some of them have variant behavior. However, this _should_
		// be fairly straightforward to enforce.
		base_space = has_unbounded ? XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT : XR_REFERENCE_SPACE_TYPE_LOCAL;

		// If head is offset from the origin, then the runtime's implementation
		// varies from StereoKit's promise. Here we set the space offset to be
		// the inverse of whatever that offset is.
		if (!openxr_space_try_get_offset(session, time, XR_REFERENCE_SPACE_TYPE_VIEW, base_space, &offset))
			return false;
		offset.orientation.x = 0;
		offset.orientation.z = 0;
		offset.orientation   = quat_normalize(offset.orientation);
	} break;
	case origin_mode_stage: {
		if      (has_stage)       { base_space = XR_REFERENCE_SPACE_TYPE_STAGE;           }
		else if (has_local_floor) { base_space = XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT; }
		else {
			base_space = has_unbounded ? XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT : XR_REFERENCE_SPACE_TYPE_LOCAL;
			offset.position.y = -1.5f;
		}
	} break;
	case origin_mode_floor: {
		if      (has_local_floor) base_space = XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT;
		else if (has_stage) {
			base_space = XR_REFERENCE_SPACE_TYPE_STAGE;

			// Stage will have a different orientation and XZ location from a
			// LOCAL_FLOOR, so we need to calculate those. This should be a
			// perfect fallback from LOCAL_FLOOR
			if (!openxr_space_try_get_offset(session, time, XR_REFERENCE_SPACE_TYPE_VIEW, XR_REFERENCE_SPACE_TYPE_STAGE, &offset))
				return false;
			offset.orientation.x = 0;
			offset.orientation.z = 0;
			offset.orientation   = quat_normalize(offset.orientation);
			offset.position      = { offset.position.x, 0, offset.position.z };
		} else {
			base_space = has_unbounded ? XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT : XR_REFERENCE_SPACE_TYPE_LOCAL;
			offset.position.y = -1.5f;
		}
	} break;
	}

	bool32_t result = false;
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.referenceSpaceType = base_space;
	memcpy(&ref_space.poseInReferenceSpace.position,    &offset.position,    sizeof(offset.position));
	memcpy(&ref_space.poseInReferenceSpace.orientation, &offset.orientation, sizeof(offset.orientation));
	XrSpace app_space = XR_NULL_HANDLE;
	XrResult err = xrCreateReferenceSpace(session, &ref_space, &app_space);
	if (XR_FAILED(err)) {
		result = false;
		*out_app_space = XR_NULL_HANDLE;
	} else {
		result = true;
		*out_app_space = app_space;

		const char *space_name = "";
		if      (base_space == XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT)  space_name = "unbounded";
		else if (base_space == XR_REFERENCE_SPACE_TYPE_LOCAL)           space_name = "local";
		else if (base_space == XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT) space_name = "local floor";
		else if (base_space == XR_REFERENCE_SPACE_TYPE_STAGE)           space_name = "stage";
		log_diagf("Created <~grn>%s<~clr> app space at an offset of <~wht>(%.2g,%.2g,%.2g) (%.2g,%.2g,%.2g,%.2g)<~clr>",
			space_name,
			offset.position   .x, offset.position   .y, offset.position   .z,
			offset.orientation.x, offset.orientation.y, offset.orientation.z, offset.orientation.w);
	}

	if (out_space_type != nullptr)
		*out_space_type = base_space;
	if (out_space_offset != nullptr)
		*out_space_offset = offset;

	return result;
}

///////////////////////////////////////////

void openxr_set_origin_offset(pose_t offset) {
	// Update our app's space to use the new offset
	if (xr_app_space != XR_NULL_HANDLE) {
		xrDestroySpace(xr_app_space);
		xr_app_space = {};
	}
	XrReferenceSpaceCreateInfo space_info = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	space_info.referenceSpaceType = xr_app_space_type;
	memcpy(&space_info.poseInReferenceSpace.position,    &offset.position,    sizeof(XrVector3f));
	memcpy(&space_info.poseInReferenceSpace.orientation, &offset.orientation, sizeof(XrQuaternionf));
	XrResult err = xrCreateReferenceSpace(xr_session, &space_info, &xr_app_space);
	if (XR_FAILED(err)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(err));
	}

	xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);
}

///////////////////////////////////////////

void openxr_cleanup() {
	if (xr_instance) {

		openxr_views_destroy();

		// Release all the other OpenXR resources that we've created!
		// What gets allocated, must get deallocated!
		if (xr_head_space != XR_NULL_HANDLE) { xrDestroySpace   (xr_head_space); xr_head_space = {}; }
		if (xr_app_space  != XR_NULL_HANDLE) { xrDestroySpace   (xr_app_space ); xr_app_space  = {}; }
		if (xr_session    != XR_NULL_HANDLE) { xrDestroySession (xr_session   ); xr_session    = {}; }
		if (xr_instance   != XR_NULL_HANDLE) { xrDestroyInstance(xr_instance  ); xr_instance   = {}; }
	}
}

///////////////////////////////////////////

void openxr_shutdown() {
	ext_management_cleanup();

	openxr_cleanup();

	xr_system_created = false;
	xr_system_success = false;
}

///////////////////////////////////////////

void openxr_step_begin() {
	openxr_poll_events();

	// Poll sk_app events for keyboard, mouse, file dialogs, etc.
	// OpenXR may not have a window, but can still receive input events
	// and native file dialogs on some systems (e.g., Android)
	ska_event_t evt;
	while (ska_event_poll(&evt)) {
		ska_handle_event(&evt);
	}

	ext_management_evt_step_begin();
	input_step();

	// Begin the render frame early so that any graphics operations the
	// application performs during step are captured.
	render_pipeline_begin_frame();
}

///////////////////////////////////////////

void openxr_step_end() {
	ext_management_evt_step_end();

	if (xr_has_session) { openxr_render_frame(); }
	else                { render_clear(); render_pipeline_skip_present(); ska_time_sleep(33); }

	xr_extension_structs_clear();

	// If the OpenXR state is idling, the device is likely in some sort of
	// standby. Either way, the session isn't available, so there's little
	// point in stepping the application. Instead, we block the thread and
	// just poll OpenXR until we leave the state.
	//
	// This happens at the end of step_end so that the app still can receive a
	// message about the app going into a hidden state.
	if (xr_session_state == XR_SESSION_STATE_IDLE && sk_is_running()) {
		const sk_settings_t* settings = sk_get_settings_ref();
		if (settings->standby_mode == standby_mode_pause) {
			log_diagf("Sleeping until OpenXR session wakes");
			xr_ext_android_thread_set_type(xr_thread_type_app_work);

			// Add a small delay before pausing audio since the sleeping path can
			// be triggered by a regular shutdown, and it would be a waste to stop
			// and resume audio when we're just going to shut down later.
			int32_t       timer      = 0;
			const int32_t timer_time = 5; // timer_time * 100ms == 500ms

			while (xr_session_state == XR_SESSION_STATE_IDLE && sk_is_running()) {
				if (timer == timer_time) audio_pause();
				timer += 1;

				ska_time_sleep(100);
				openxr_poll_events();
			}
			if (timer > timer_time) audio_resume();

			xr_ext_android_thread_set_type(xr_thread_type_render_main);
			log_diagf("Resuming from sleep");
		} else if (settings->standby_mode == standby_mode_slow) {
			ska_time_sleep(77);
		}
	}
}

///////////////////////////////////////////

const char* openxr_state_name(XrSessionState state) {
	switch (state) {
	case XR_SESSION_STATE_IDLE:         return "idle";
	case XR_SESSION_STATE_READY:        return "ready";
	case XR_SESSION_STATE_SYNCHRONIZED: return "synchronized";
	case XR_SESSION_STATE_VISIBLE:      return "visible";
	case XR_SESSION_STATE_FOCUSED:      return "focused";
	case XR_SESSION_STATE_STOPPING:     return "stopping";
	case XR_SESSION_STATE_EXITING:      return "exiting";
	case XR_SESSION_STATE_LOSS_PENDING: return "loss pending";
	default:                            return "unknown";
	}
}

///////////////////////////////////////////

bool openxr_poll_events() {
	XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	bool result = true;

	while (xrPollEvent(xr_instance, &event_buffer) == XR_SUCCESS) {
		switch (event_buffer.type) {
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged *changed = (XrEventDataSessionStateChanged*)&event_buffer;
			log_diagf("OpenXR state: <~WHT>%s<~BLK> -> <~WHT>%s<~clr>", openxr_state_name(xr_session_state), openxr_state_name(changed->state));
			xr_session_state = changed->state;

			if      (xr_session_state == XR_SESSION_STATE_FOCUSED) sk_set_app_focus(app_focus_active);
			else if (xr_session_state == XR_SESSION_STATE_VISIBLE) sk_set_app_focus(app_focus_background);
			else                                                   sk_set_app_focus(app_focus_hidden);

			// Session state change is where we can begin and end sessions, as well as find quit messages!
			switch (xr_session_state) {
			// The runtime should never return unknown: https://www.khronos.org/registry/OpenXR/specs/1.0/man/html/XrSessionState.html
			case XR_SESSION_STATE_UNKNOWN: abort(); break;
			case XR_SESSION_STATE_IDLE:             break; // Wait till we get XR_SESSION_STATE_READY.
			case XR_SESSION_STATE_READY: {
				// Get the session started!
				XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
				begin_info.primaryViewConfigurationType = XR_PRIMARY_CONFIG;

				// Let extensions have a go at the session begin parameters!
				// Extensions such as XR_MSFT_first_person_observer use this.
				ext_management_evt_session_begin(&begin_info);

				XrResult xresult = xrBeginSession(xr_session, &begin_info);
				if (XR_FAILED(xresult)) {
					log_errf("xrBeginSession failed [%s]", openxr_string(xresult));
					sk_quit(quit_reason_session_lost);
					result = false;
				} else {
					xr_has_session = true;
					log_diag("OpenXR session began.");

					// FoV normally updates right before drawing, but we need it to
					// be available as soon as the session begins, for apps that
					// are listening to sk_app_focus changing to determine if FoV
					// is ready.
					openxr_views_update_fov();
				}
			} break;
			case XR_SESSION_STATE_SYNCHRONIZED: break; // We're connected to a session, but not visible to users yet.
			case XR_SESSION_STATE_VISIBLE:      break; // We're visible to users, but not in focus or receiving input. Modal OS dialogs could be visible here.
			case XR_SESSION_STATE_FOCUSED:      break; // We're visible and focused. This is the "normal" operating state of an app.
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_has_session = false; result = false; break; // We should not render in this state. We may be minimized, suspended, or otherwise out of action for the moment.
			case XR_SESSION_STATE_EXITING:      sk_quit(quit_reason_user);                        result = false; break; // Runtime wants us to terminate the app, usually from a user's request.
			case XR_SESSION_STATE_LOSS_PENDING: sk_quit(quit_reason_session_lost);                result = false; break; // The OpenXR runtime may have had some form of failure. It is theoretically possible to recover from this state by occasionally attempting to xrGetSystem, but we don't currently do this.
			default: break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING         : sk_quit(quit_reason_session_lost); result = false; break;
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
			XrEventDataReferenceSpaceChangePending *pending   = (XrEventDataReferenceSpaceChangePending*)&event_buffer;

			// Update the main app space. In particular, some fallback spaces
			// may require recalculation.
			XrSpace new_space = {};
			if (openxr_try_get_app_space(xr_session, sk_get_settings_ref()->origin, pending->changeTime, &xr_app_space_type, &world_origin_offset, &new_space)) {
				if (xr_app_space) xrDestroySpace(xr_app_space);
				xr_app_space = new_space;
			}

			xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, pending->changeTime);
		} break;
		default: break;
		}

		ext_management_evt_poll_event(&event_buffer);

		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
	return result;
}

///////////////////////////////////////////

bool32_t openxr_get_space(XrSpace space, pose_t *out_pose, XrTime time) {
	if (time == 0)
		time = xr_time;

	XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
	XrResult        res            = xrLocateSpace(space, xr_app_space, time, &space_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
		memcpy(&out_pose->position,    &space_location.pose.position,    sizeof(vec3));
		memcpy(&out_pose->orientation, &space_location.pose.orientation, sizeof(quat));
		return true;
	}
	return false;
}

///////////////////////////////////////////

xr_runtime_ openxr_get_known_runtime() {
	return xr_known_runtime;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_instance() {
	if (backend_xr_get_type() != backend_xr_type_openxr) 
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return (openxr_handle_t)xr_instance;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_session() {
	if (backend_xr_get_type() != backend_xr_type_openxr) 
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return (openxr_handle_t)xr_session;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_system_id() {
	if (backend_xr_get_type() != backend_xr_type_openxr)
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return (openxr_handle_t)xr_system_id;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_space() {
	if (backend_xr_get_type() != backend_xr_type_openxr) 
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return (openxr_handle_t)xr_app_space;
}

///////////////////////////////////////////

int64_t backend_openxr_get_time() {
	if (backend_xr_get_type() != backend_xr_type_openxr) 
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return xr_time;
}

///////////////////////////////////////////

int64_t backend_openxr_get_eyes_sample_time() {
	if (backend_xr_get_type() != backend_xr_type_openxr)
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return xr_eyes_sample_time;
}

///////////////////////////////////////////

void *backend_openxr_get_function(const char *function_name) {
	if (backend_xr_get_type() != backend_xr_type_openxr)
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	if (xr_instance == XR_NULL_HANDLE)
		log_err("backend_openxr_get_function must be called after StereoKit initialization!");

	void   (*fn)()  = nullptr;
	XrResult result = xrGetInstanceProcAddr(xr_instance, function_name, (PFN_xrVoidFunction*)&fn);
	if (XR_FAILED(result)) {
		log_errf("Failed to find OpenXR function '%s' [%s]", function_name, openxr_string(result));
		return nullptr;
	}

	return (void*)fn;
}


} // namespace sk

#endif
