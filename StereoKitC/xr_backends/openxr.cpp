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
#include "openxr_extensions.h"
#include "openxr_input.h"
#include "openxr_view.h"

#include "../sk_memory.h"
#include "../log.h"
#include "../device.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../systems/render.h"
#include "../systems/audio.h"
#include "../systems/input.h"
#include "../systems/world.h"
#include "../asset_types/anchor.h"

#include <sk_gpu.h>

#include <openxr/openxr.h>
#include <openxr/openxr_reflection.h>
#include <openxr/openxr_platform.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(SK_OS_ANDROID)
#include <unistd.h> // gettid
#endif

#if defined(SK_OS_ANDROID) || defined(SK_OS_LINUX)
#include <time.h>
#endif

namespace sk {

///////////////////////////////////////////

typedef struct context_callback_t {
	void (*callback)(void* context);
	void *context;
} context_callback_t;

typedef struct poll_event_callback_t {
	void (*callback)(void* context, void* XrEventDataBuffer);
	void *context;
} poll_event_callback_t;

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
xr_ext_table_t       xr_extensions        = {};
xr_ext_info_t        xr_ext               = {};
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

array_t<const char*> xr_exts_user         = {};
array_t<const char*> xr_exts_exclude      = {};
array_t<uint64_t>    xr_exts_loaded       = {};
bool32_t             xr_minimum_exts      = false;

bool                 xr_has_bounds        = false;
vec2                 xr_bounds_size       = {};
pose_t               xr_bounds_pose       = pose_identity;
pose_t               xr_bounds_pose_local = pose_identity;

array_t<context_callback_t>    xr_callbacks_pre_session_create = {};
array_t<poll_event_callback_t> xr_callbacks_poll_event         = {};

XrDebugUtilsMessengerEXT xr_debug = {};
XrReferenceSpaceType     xr_refspace;

///////////////////////////////////////////

bool32_t openxr_try_get_app_space   (XrSession session, origin_mode_ mode, XrTime time, XrReferenceSpaceType *out_space_type, pose_t* out_space_offset, XrSpace *out_app_space);
void     openxr_list_layers    (array_t<const char*>* ref_available_layers, array_t<const char*>* ref_request_layers);
XrTime   openxr_acquire_time        ();
bool     openxr_blank_frame         ();
bool     is_ext_explicitly_requested(const char* extension_name);

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

void openxr_show_ext_table(array_t<const char*> exts_request, array_t<const char*> exts_available, array_t<const char*> layers_request, array_t<const char*> layers_available) {
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

#if defined(SK_DEBUG)
XrBool32 XRAPI_PTR openxr_debug_messenger_callback(XrDebugUtilsMessageSeverityFlagsEXT severity,
                                                   XrDebugUtilsMessageTypeFlagsEXT,
                                                   const XrDebugUtilsMessengerCallbackDataEXT *msg,
                                                   void*) {
	// Print the debug message we got! There's a bunch more info we could
	// add here too, but this is a pretty good start, and you can always
	// add a breakpoint this line!
	log_ level = log_diagnostic;
	if      (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  ) level = log_error;
	else if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) level = log_warning;
	log_writef(level, "[<~mag>xr<~clr>] %s: %s", msg->functionName, msg->message);

	// Returning XR_TRUE here will force the calling function to fail
	return (XrBool32)XR_FALSE;
}
#endif

bool openxr_create_system() {
	if (xr_system_created == true) return xr_system_success;
	xr_system_success = false;
	xr_system_created = true;

#if defined(SK_OS_ANDROID)
	PFN_xrInitializeLoaderKHR ext_xrInitializeLoaderKHR;
	xrGetInstanceProcAddr(
		XR_NULL_HANDLE,
		"xrInitializeLoaderKHR",
		(PFN_xrVoidFunction *)(&ext_xrInitializeLoaderKHR));

	XrLoaderInitInfoAndroidKHR init_android = { XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
	init_android.applicationVM      = backend_android_get_java_vm ();
	init_android.applicationContext = backend_android_get_activity();
	if (XR_FAILED(ext_xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&init_android))) {
		log_fail_reasonf(90, log_warning, "Failed to initialize OpenXR loader");
		return false;
	}
#endif

	array_t<const char*> exts_request   = {};
	array_t<const char*> exts_available = {};
	openxr_list_extensions(xr_exts_user, xr_exts_exclude, xr_minimum_exts, &exts_available, &exts_request);

	array_t<const char*> layers_request   = {};
	array_t<const char*> layers_available = {};
	openxr_list_layers(&layers_available, &layers_request);

	for (int32_t i = 0; i < exts_request.count; i++)
		xr_exts_loaded.add(hash_fnv64_string(exts_request[i]));

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
#if defined(SK_OS_ANDROID)
	XrInstanceCreateInfoAndroidKHR create_android = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
	create_android.applicationVM       = backend_android_get_java_vm ();
	create_android.applicationActivity = backend_android_get_activity();
	create_info.next = (void*)&create_android;
#endif
	XrResult result = xrCreateInstance(&create_info, &xr_instance);

	// If we succeeded, log some infor about our setup.
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
	}

	// Always log the extension table, this may contain information about why
	// we failed to load.
	openxr_show_ext_table(exts_request, exts_available, layers_request, layers_available);
	for(int32_t e=0; e<exts_available.count; e+=1) _sk_free((void*)exts_available[e]);
	exts_request  .free();
	exts_available.free();
	for(int32_t e=0; e<layers_available.count; e+=1) _sk_free((void*)layers_available[e]);
	layers_request  .free();
	layers_available.free();

	// If the instance is null here, the user needs to install an OpenXR
	// runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	// Create links to the extension functions
	xr_extensions = openxr_create_extension_table(xr_instance);

#if defined(SK_DEBUG)
	// Set up a really verbose debug log! Great for dev, but turn this off or
	// down for final builds. WMR doesn't produce much output here, but it
	// may be more useful for other runtimes?
	// Here's some extra information about the message types and severities:
	// https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#debug-message-categorization
	XrDebugUtilsMessengerCreateInfoEXT debug_info = { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debug_info.messageTypes =
		XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
		XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
		XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
	debug_info.messageSeverities =
		//XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debug_info.userCallback = openxr_debug_messenger_callback;
	// Start up the debug utils!
	if (xr_ext.EXT_debug_utils == xr_ext_active)
		xr_extensions.xrCreateDebugUtilsMessengerEXT(xr_instance, &debug_info, &xr_debug);
#endif

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

XrGraphicsRequirements luid_requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
void *openxr_get_luid() {
	
#if defined(XR_USE_GRAPHICS_API_D3D11)
	if (!openxr_create_system()) return nullptr;

	// Get an extension function, and check it for our requirements
	PFN_xrGetGraphicsRequirementsKHR xrGetGraphicsRequirementsKHR;
	if (XR_FAILED(xrGetInstanceProcAddr(xr_instance, NAME_xrGetGraphicsRequirementsKHR, (PFN_xrVoidFunction *)(&xrGetGraphicsRequirementsKHR))) ||
		XR_FAILED(xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &luid_requirement))) {
		xrDestroyInstance(xr_instance);
		return nullptr;
	}

	return (void *)&luid_requirement.adapterLuid;
#else
	return nullptr;
#endif
}

///////////////////////////////////////////

XrTime openxr_acquire_time() {
	XrTime result = {};
#ifdef XR_USE_TIMESPEC
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) != 0 ||
		XR_FAILED(xr_extensions.xrConvertTimespecTimeToTimeKHR(xr_instance, &time, &result))) {
		log_warn("openxr_acquire_time failed to get current time!");
	}
#else
	LARGE_INTEGER time;
	if (!QueryPerformanceCounter(&time) ||
		XR_FAILED(xr_extensions.xrConvertWin32PerformanceCounterToTimeKHR(xr_instance, &time, &result))) {
		log_warn("openxr_acquire_time failed to get current time!");
	}
#endif
	return result;
}

///////////////////////////////////////////

bool openxr_init() {
	if (!openxr_create_system())
		return false;

	device_data.display_type = display_type_stereo;

	// A number of other items also use the xr_time, so lets get this ready
	// right away.
	xr_time = openxr_acquire_time();

	// We would use backend_openxr_ext_enabled, but openxr isn't full ready
	// yet, so it throws errors into the logs.
	if (xr_exts_loaded.index_of(hash_fnv64_string(XR_GFX_EXTENSION)) < 0) {
		log_infof("Couldn't load required extension [%s]", XR_GFX_EXTENSION);
		openxr_cleanup();
		return false;
	}

	system_info_t* sys_info = sk_get_info_ref();

	// The Session gets created before checking capabilities! In certain
	// contexts, such as Holographic Remoting, the system won't know about its
	// capabilities until the session is ready. Holographic Remoting knows to
	// make the session ready immediately after xrCreateSession, so we don't
	// need to wait all the way until the OpenXR message loop tells us.
	// See here for more info on Holographic Remoting's lifecycle:
	// https://learn.microsoft.com/en-us/windows/mixed-reality/develop/native/holographic-remoting-create-remote-openxr?source=recommendations#connect-to-the-device

	// Before we call xrCreateSession, lets fire an event for anyone that needs
	// to set things up!
	for (int32_t i = 0; i < xr_callbacks_pre_session_create.count; i++) {
		xr_callbacks_pre_session_create[i].callback(xr_callbacks_pre_session_create[i].context);
	}
	xr_callbacks_pre_session_create.free();

	// OpenXR wants to ensure apps are using the correct LUID, so this MUST be
	// called before xrCreateSession
	XrGraphicsRequirements requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
	xr_check(xr_extensions.xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement),
		"xrGetGraphicsRequirementsKHR");
	void *luid = nullptr;
#ifdef XR_USE_GRAPHICS_API_D3D11
	luid = (void *)&requirement.adapterLuid;
#elif defined(XR_USE_GRAPHICS_API_OPENGL_ES)
	log_diagf("OpenXR requires GLES v%d.%d.%d - v%d.%d.%d",
		XR_VERSION_MAJOR(requirement.minApiVersionSupported), XR_VERSION_MINOR(requirement.minApiVersionSupported), XR_VERSION_PATCH(requirement.minApiVersionSupported),
		XR_VERSION_MAJOR(requirement.maxApiVersionSupported), XR_VERSION_MINOR(requirement.maxApiVersionSupported), XR_VERSION_PATCH(requirement.maxApiVersionSupported));
#endif

	// A session represents this application's desire to display things! This
	// is where we hook up our graphics API. This does not start the session,
	// for that, you'll need a call to xrBeginSession, which we do in
	// openxr_poll_events
	XrGraphicsBinding gfx_binding = { XR_TYPE_GRAPHICS_BINDING };
	skg_platform_data_t platform = skg_get_platform_data();
#if defined(XR_USE_PLATFORM_XLIB)
	gfx_binding.xDisplay    = (Display*  )platform._x_display;
	gfx_binding.visualid    = *(uint32_t *)platform._visual_id;
	gfx_binding.glxFBConfig = (GLXFBConfig)platform._glx_fb_config;
	gfx_binding.glxDrawable = *((GLXDrawable*)platform._glx_drawable);
	gfx_binding.glxContext  = (GLXContext )platform._glx_context;
#elif defined(XR_USE_GRAPHICS_API_OPENGL)
	gfx_binding.hDC   = (HDC  )platform._gl_hdc;
	gfx_binding.hGLRC = (HGLRC)platform._gl_hrc;
#elif defined(XR_USE_GRAPHICS_API_OPENGL_ES)
	#if defined (SK_OS_LINUX)
		gfx_binding.getProcAddress = eglGetProcAddress;
	#endif
	gfx_binding.display = (EGLDisplay)platform._egl_display;
	gfx_binding.config  = (EGLConfig )platform._egl_config;
	gfx_binding.context = (EGLContext)platform._egl_context;
#elif defined(XR_USE_GRAPHICS_API_D3D11)
	gfx_binding.device = (ID3D11Device*)platform._d3d11_device;
#endif
	XrSessionCreateInfo session_info = { XR_TYPE_SESSION_CREATE_INFO };
	session_info.next     = &gfx_binding;
	session_info.systemId = xr_system_id;
	XrSessionCreateInfoOverlayEXTX overlay_info = {XR_TYPE_SESSION_CREATE_INFO_OVERLAY_EXTX};
	const sk_settings_t *settings = sk_get_settings_ref();
	if (xr_ext.EXTX_overlay == xr_ext_active && settings->overlay_app) {
		overlay_info.sessionLayersPlacement = settings->overlay_priority;
		gfx_binding.next = &overlay_info;
		sys_info->overlay_app = true;
	}
	XrResult result = xrCreateSession(xr_instance, &session_info, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (XR_FAILED(result) || xr_session == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
		openxr_cleanup();
		return false;
	}

		// On Android, tell OpenXR what kind of thread this is. This can be
	// important on Android systems so we don't get treated as a low priority
	// thread by accident.
#if defined(SK_OS_ANDROID)
	if (xr_ext.KHR_android_thread_settings == xr_ext_active) {
		xr_extensions.xrSetAndroidApplicationThreadKHR(xr_session, XR_ANDROID_THREAD_TYPE_RENDERER_MAIN_KHR, gettid());
	}
#endif

	// Fetch the runtime name/info, for logging and for a few other checks
	XrInstanceProperties inst_properties = { XR_TYPE_INSTANCE_PROPERTIES };
	xr_check(xrGetInstanceProperties(xr_instance, &inst_properties),
		"xrGetInstanceProperties");

	// Figure out what this device is capable of!
	XrSystemProperties                      properties          = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT       properties_tracking = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	XrSystemHandTrackingMeshPropertiesMSFT  properties_handmesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
	XrSystemEyeGazeInteractionPropertiesEXT properties_gaze     = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
	// Validation layer does not seem to like 'next' chaining beyond a depth of
	// 1, so we'll just call these one at a time.
	xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties");
	if (xr_ext.EXT_hand_tracking == xr_ext_active) {
		properties.next = &properties_tracking;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties");
	}
	if (xr_ext.MSFT_hand_tracking_mesh == xr_ext_active) {
		properties.next = &properties_handmesh;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties");
	}
	if (xr_ext.EXT_eye_gaze_interaction == xr_ext_active) {
		properties.next = &properties_gaze;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties");
	}

	log_diagf("System name: <~grn>%s<~clr>", properties.systemName);
	device_data.name = string_copy(properties.systemName);

	if (xr_ext.EXT_hand_tracking        == xr_ext_active && properties_tracking.supportsHandTracking       == false) xr_ext.EXT_hand_tracking        = xr_ext_disabled;
	if (xr_ext.MSFT_hand_tracking_mesh  == xr_ext_active && properties_handmesh.supportsHandTrackingMesh   == false) xr_ext.MSFT_hand_tracking_mesh  = xr_ext_disabled;
	if (xr_ext.EXT_eye_gaze_interaction == xr_ext_active && properties_gaze    .supportsEyeGazeInteraction == false) xr_ext.EXT_eye_gaze_interaction = xr_ext_disabled;
	sys_info->perception_bridge_present = xr_ext.MSFT_perception_anchor_interop == xr_ext_active;
	sys_info->spatial_bridge_present    = xr_ext.MSFT_spatial_graph_bridge      == xr_ext_active;

	// Exception for the articulated Vive Index hand simulation. This
	// simulation is insufficient to treat it like true articulated hands.
	//
	// We can skip this exception if Ultraleap's hand tracking layer is
	// present.
	//
	// TODO: Remove this when the hand tracking data source extension is more
	// generally available.
#if defined(_M_X64) && (defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP))
	if (xr_ext.EXT_hand_tracking_data_source != xr_ext_active &&
		(strcmp(device_get_runtime(), "Windows Mixed Reality Runtime") == 0 || strcmp(device_get_runtime(), "SteamVR/OpenXR") == 0)) {

		// We don't need to ask for the Ultraleap layer above so we don't have it
		// stored anywhere. Gotta check it again.
		bool     has_leap_layer = false;
		uint32_t xr_layer_count = 0;
		xrEnumerateApiLayerProperties(0, &xr_layer_count, nullptr);
		XrApiLayerProperties *xr_layers = sk_malloc_t(XrApiLayerProperties, xr_layer_count);
		for (uint32_t i = 0; i < xr_layer_count; i++) xr_layers[i] = { XR_TYPE_API_LAYER_PROPERTIES };
		xrEnumerateApiLayerProperties(xr_layer_count, &xr_layer_count, xr_layers);
		for (uint32_t i = 0; i < xr_layer_count; i++) {
			if (strcmp(xr_layers[i].layerName, "XR_APILAYER_ULTRALEAP_hand_tracking") == 0) {
				has_leap_layer = true;
				break;
			}
		}
		sk_free(xr_layers);

		// The Leap hand tracking layer seems to supercede the built-in extensions.
		if (has_leap_layer == false && xr_ext.EXT_hand_tracking == xr_ext_active && is_ext_explicitly_requested("XR_EXT_hand_tracking") == false) {
			log_diag("XR_EXT_hand_tracking - Rejected - Incompatible implementations on WMR and SteamVR.");
			xr_ext.EXT_hand_tracking = xr_ext_rejected;
		}
		if (has_leap_layer == false && xr_ext.MSFT_hand_tracking_mesh == xr_ext_active && is_ext_explicitly_requested("XR_MSFT_hand_tracking_mesh") == false) {
			log_diag("XR_MSFT_hand_tracking_mesh - Rejected - Incompatible implementations on WMR and SteamVR.");
			xr_ext.MSFT_hand_tracking_mesh = xr_ext_rejected;
		}
	}
#endif

	// Snapdragon Spaces advertises the palm pose extension, but provides bad
	// data for it. Only enable it if it's explicitly requested.
	if (strstr(device_get_runtime(), "Snapdragon") != nullptr && is_ext_explicitly_requested("XR_EXT_palm_pose") == false) {
		xr_ext.EXT_palm_pose = xr_ext_rejected;
		log_diag("XR_EXT_palm_pose - Rejected - Not fully implemented on Snapdragon Spaces.");
	}

	// Quest has a menu button that is always shown when hand tracking, but the
	// hand interaction EXTs don't support actions for it. This can lead to a
	// mismatch where users see the menu button, but SK _can't_ react to the
	// button events. hand_interaction EXTs are disabled on Quest so that input
	// falls back to the simple_controller interaction profile. We will only
	// enable it if it's explicitly requested.
	//
	// Quest does not implement XR_EXT_hand_interaction, so we only need to do
	// this for the MSFT one.
	if (strstr(device_get_runtime(), "Oculus") != nullptr && is_ext_explicitly_requested("XR_MSFT_hand_interaction") == false) {
		xr_ext.MSFT_hand_interaction = xr_ext_rejected;
		log_diag("XR_MSFT_hand_interaction - Rejected - Hides menu button events on Quest.");
	}

	// SK's hand_interaction implementations use XR_EXT_hand_tracking for some
	// data, so we can't rely on these interaction profiles unless
	// XR_EXT_hand_tracking is available.
	if (xr_ext.EXT_hand_interaction == xr_ext_active && is_ext_explicitly_requested("XR_EXT_hand_interaction") == false && xr_ext.EXT_hand_tracking != xr_ext_active) {
		log_diag("XR_EXT_hand_interaction - Disabled - Dependant on XR_EXT_hand_tracking.");
		xr_ext.EXT_hand_interaction = xr_ext_disabled;
	}
	if (xr_ext.MSFT_hand_interaction == xr_ext_active && is_ext_explicitly_requested("XR_MSFT_hand_interaction") == false && xr_ext.EXT_hand_tracking != xr_ext_active) {
		log_diag("XR_MSFT_hand_interaction - Disabled - Dependant on XR_EXT_hand_tracking.");
		xr_ext.MSFT_hand_interaction = xr_ext_disabled;
	}

	device_data.has_hand_tracking = xr_ext.EXT_hand_tracking == xr_ext_active;
	device_data.tracking          = device_tracking_none;
	if      (properties.trackingProperties.positionTracking)    device_data.tracking = device_tracking_6dof;
	else if (properties.trackingProperties.orientationTracking) device_data.tracking = device_tracking_3dof;


	if (xr_ext.EXT_hand_tracking        == xr_ext_active) log_diag("XR_EXT_hand_tracking - Ready.");
	if (xr_ext.MSFT_hand_tracking_mesh  == xr_ext_active) log_diag("XR_MSFT_hand_tracking_mesh - Ready.");
	if (xr_ext.EXT_eye_gaze_interaction == xr_ext_active) log_diag("XR_EXT_eye_gaze_interaction - Ready.");

	// Check scene understanding features, these may be dependant on Session
	// creation in the context of Holographic Remoting.
	if (xr_ext.MSFT_scene_understanding == xr_ext_active) {
		uint32_t count = 0;
		xr_extensions.xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, 0, &count, nullptr);
		XrSceneComputeFeatureMSFT *features = sk_malloc_t(XrSceneComputeFeatureMSFT, count);
		xr_extensions.xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, count, &count, features);
		for (uint32_t i = 0; i < count; i++) {
			if      (features[i] == XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT  ) sys_info->world_occlusion_present = true;
			else if (features[i] == XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT) sys_info->world_raycast_present   = true;
		}
		sk_free(features);
	}
	if (sys_info->world_occlusion_present) log_diag("XR_MSFT_scene_understanding - Supports world occlusion.");
	if (sys_info->world_raycast_present)   log_diag("XR_MSFT_scene_understanding - Supports world raycast.");

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
		platform_sleep(33);
		if (!openxr_poll_events()) { log_infof("Exit event during initialization"); openxr_cleanup(); return false; }
	}
	// Blank frames should only be submitted when the session is READY
	while (xr_session_state == XR_SESSION_STATE_READY) {
		openxr_blank_frame();
		if (!openxr_poll_events()) { log_infof("Exit event during initialization"); openxr_cleanup(); return false; }
	}

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

	if (!oxri_init()) {
		openxr_cleanup();
		return false;
	}

	// If this is an overlay app, and the user has not explicitly requested a
	// blend mode, then we'll auto-switch to 'blend', as that's likely the most
	// appropriate mode for the app.
	if (sys_info->overlay_app) log_diag("Starting as an overlay app.");
	if (sys_info->overlay_app && sk_get_settings_ref()->blend_preference == display_blend_none) {
		log_diag("Overlay app defaulting to 'blend' display_blend.");
		device_data.display_blend = display_blend_blend;
	}

	xr_has_bounds  = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);
	xr_bounds_pose = matrix_transform_pose(render_get_cam_final(), xr_bounds_pose_local);

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (xr_ext.OCULUS_audio_device_guid == xr_ext_active) {
		wchar_t device_guid[128];
		if (XR_SUCCEEDED(xr_extensions.xrGetAudioOutputDeviceGuidOculus(xr_instance, device_guid))) {
			audio_set_default_device_out(device_guid);
		}
		if (XR_SUCCEEDED(xr_extensions.xrGetAudioInputDeviceGuidOculus(xr_instance, device_guid))) {
			audio_set_default_device_in(device_guid);
		}
	}
#endif

	if (xr_ext.MSFT_spatial_anchor == xr_ext_active)
		anchors_init(anchor_system_openxr_msft);

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

void openxr_list_layers(array_t<const char*>* ref_available_layers, array_t<const char*>* ref_request_layers) {
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
		case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:  has_unbounded   = xr_ext.MSFT_unbounded_reference_space == xr_ext_active; break;
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

	xr_has_bounds  = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);
	xr_bounds_pose = matrix_transform_pose  (render_get_cam_final(), xr_bounds_pose_local);
}

///////////////////////////////////////////

void openxr_cleanup() {
	if (xr_instance) {
		// Shut down the input!
		oxri_shutdown();

		openxr_views_destroy();

		// Release all the other OpenXR resources that we've created!
		// What gets allocated, must get deallocated!
		if (xr_debug      != XR_NULL_HANDLE) { xr_extensions.xrDestroyDebugUtilsMessengerEXT(xr_debug); xr_debug = {}; }
		if (xr_head_space != XR_NULL_HANDLE) { xrDestroySpace   (xr_head_space); xr_head_space = {}; }
		if (xr_app_space  != XR_NULL_HANDLE) { xrDestroySpace   (xr_app_space ); xr_app_space  = {}; }
		if (xr_session    != XR_NULL_HANDLE) { xrDestroySession (xr_session   ); xr_session    = {}; }
		if (xr_instance   != XR_NULL_HANDLE) { xrDestroyInstance(xr_instance  ); xr_instance   = {}; }
	}
}

///////////////////////////////////////////

void openxr_shutdown() {
	anchors_shutdown();

	openxr_cleanup();

	xr_minimum_exts   = false;
	xr_system_created = false;
	xr_system_success = false;

	xr_exts_loaded .free();
	xr_exts_user   .free();
	xr_exts_exclude.free();
	xr_callbacks_pre_session_create.free();
}

///////////////////////////////////////////

void openxr_step_begin() {
	openxr_poll_events();
	if (xr_has_session)
		openxr_poll_actions();
	input_step();
	
	anchors_step_begin();
}

///////////////////////////////////////////

void openxr_step_end() {
	anchors_step_end();

	if (xr_has_session) { openxr_render_frame(); }
	else                { render_clear(); platform_sleep(33); }

	xr_extension_structs_clear();

	// If the OpenXR state is idling, the device is likely in some sort of
	// standby. Either way, the session isn't available, so there's little
	// point in stepping the application. Instead, we block the thread and
	// just poll OpenXR until we leave the state.
	//
	// This happens at the end of step_end so that the app still can receive a
	// message about the app going into a hidden state.
	if (xr_session_state == XR_SESSION_STATE_IDLE && sk_is_running()) {
		log_diagf("Sleeping until OpenXR session wakes");
#if defined(SK_OS_ANDROID)
		if (xr_ext.KHR_android_thread_settings == xr_ext_active) {
			xr_extensions.xrSetAndroidApplicationThreadKHR(xr_session, XR_ANDROID_THREAD_TYPE_APPLICATION_WORKER_KHR, gettid());
		}
#endif
		// Add a small delay before pausing audio since the sleeping path can
		// be triggered by a regular shutdown, and it would be a waste to stop
		// and resume audio when we're just going to shut down later.
		int32_t       timer      = 0;
		const int32_t timer_time = 5; // timer_time * 100ms == 500ms

		while (xr_session_state == XR_SESSION_STATE_IDLE && sk_is_running()) {
			if (timer == timer_time) audio_pause();
			timer += 1;

			platform_sleep(100);
			openxr_poll_events();
		}
		if (timer > timer_time) audio_resume();

#if defined(SK_OS_ANDROID)
		if (xr_ext.KHR_android_thread_settings == xr_ext_active) {
			xr_extensions.xrSetAndroidApplicationThreadKHR(xr_session, XR_ANDROID_THREAD_TYPE_RENDERER_MAIN_KHR, gettid());
		}
#endif
		log_diagf("Resuming from sleep");
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

				// If the XR_MSFT_first_person_observer extension is present,
				// we may have a secondary display we need to enable. This is
				// typically the HoloLens video recording or streaming feature.
				XrSecondaryViewConfigurationSessionBeginInfoMSFT secondary      = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT };
				XrViewConfigurationType                          secondary_type = XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT;
				if (xr_ext.MSFT_first_person_observer == xr_ext_active && xr_view_type_valid(secondary_type)) {
					secondary.viewConfigurationCount        = 1;
					secondary.enabledViewConfigurationTypes = &secondary_type;
					begin_info.next = &secondary;
				}

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
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged *changed = (XrEventDataInteractionProfileChanged*)&event_buffer;
			if (changed->session == xr_session) {
				oxri_update_interaction_profile();
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

			xr_has_bounds  = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, pending->changeTime);
			xr_bounds_pose = matrix_transform_pose(render_get_cam_final(), xr_bounds_pose_local);
		} break;
		default: break;
		}

		for (int32_t i = 0; i < xr_callbacks_poll_event.count; i++) {
			xr_callbacks_poll_event[i].callback(xr_callbacks_poll_event[i].context, &event_buffer);
		}

		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
	return result;
}

///////////////////////////////////////////

void openxr_poll_actions() {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED)
		return;

	oxri_update_frame();
}

///////////////////////////////////////////

bool32_t openxr_get_space(XrSpace space, pose_t *out_pose, XrTime time) {
	if (time == 0) time = xr_time;

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

bool32_t openxr_get_gaze_space(pose_t* out_pose, XrTime &out_gaze_sample_time, XrTime time) {
	if (time == 0) time = xr_time;

	XrEyeGazeSampleTimeEXT gaze_sample_time = { XR_TYPE_EYE_GAZE_SAMPLE_TIME_EXT };
	XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION, &gaze_sample_time };
	XrResult        res = xrLocateSpace(xr_gaze_space, xr_app_space, time, &space_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
		memcpy(&out_pose->position, &space_location.pose.position, sizeof(vec3));
		memcpy(&out_pose->orientation, &space_location.pose.orientation, sizeof(quat));
		out_gaze_sample_time = gaze_sample_time.time;
		return true;
	}
	return false;
}

///////////////////////////////////////////

pose_t world_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time) {
	if (!xr_session) {
		log_warn("No OpenXR session available for converting spatial graph nodes!");
		return pose_identity;
	}
	if (!sk_get_info_ref()->spatial_bridge_present) {
		log_warn("This system doesn't support the spatial bridge! Check SK.System.spatialBridgePresent.");
		return pose_identity;
	}

	XrSpace                               space;
	XrSpatialGraphNodeSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT };
	space_info.nodeType = dynamic ? XR_SPATIAL_GRAPH_NODE_TYPE_DYNAMIC_MSFT : XR_SPATIAL_GRAPH_NODE_TYPE_STATIC_MSFT;
	space_info.pose     = { {0,0,0,1}, {0,0,0} };
	memcpy(space_info.nodeId, spatial_graph_node_id, sizeof(space_info.nodeId));

	if (XR_FAILED(xr_extensions.xrCreateSpatialGraphNodeSpaceMSFT(xr_session, &space_info, &space))) {
		log_warn("world_from_spatial_graph: xrCreateSpatialGraphNodeSpaceMSFT call failed, maybe a bad spatial node?");
		return pose_identity;
	}

	XrTime time = 0;
#if defined(SK_OS_WINDOWS_UWP) || defined(SK_OS_WINDOWS)
	if (qpc_time > 0) {
		LARGE_INTEGER li;
		li.QuadPart = qpc_time;
		xr_extensions.xrConvertWin32PerformanceCounterToTimeKHR(xr_instance, &li, &time);
	}
#endif

	pose_t result = {};
	if (!openxr_get_space(space, &result, time)) {
		log_warn("world_from_spatial_graph: openxr_get_space call failed, maybe a bad spatial node?");
		return pose_identity;
	}
	return result;
}

///////////////////////////////////////////

bool32_t world_try_from_spatial_graph(uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t *out_pose) {
	if (!xr_session) {
		log_warn("No OpenXR session available for converting spatial graph nodes!");
		*out_pose = pose_identity;
		return false;
	}
	if (!sk_get_info_ref()->spatial_bridge_present) {
		log_warn("This system doesn't support the spatial bridge! Check SK.System.spatialBridgePresent.");
		*out_pose = pose_identity;
		return false;
	}

	XrSpace                               space;
	XrSpatialGraphNodeSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT };
	space_info.nodeType = dynamic ? XR_SPATIAL_GRAPH_NODE_TYPE_DYNAMIC_MSFT : XR_SPATIAL_GRAPH_NODE_TYPE_STATIC_MSFT;
	space_info.pose     = { {0,0,0,1}, {0,0,0} };
	memcpy(space_info.nodeId, spatial_graph_node_id, sizeof(space_info.nodeId));

	if (XR_FAILED(xr_extensions.xrCreateSpatialGraphNodeSpaceMSFT(xr_session, &space_info, &space))) {
		*out_pose = pose_identity;
		return false;
	}

	XrTime time = 0;
#if defined(SK_OS_WINDOWS_UWP) || defined(SK_OS_WINDOWS)
	if (qpc_time > 0) {
		LARGE_INTEGER li;
		li.QuadPart = qpc_time;
		xr_extensions.xrConvertWin32PerformanceCounterToTimeKHR(xr_instance, &li, &time);
	}
#endif

	if (!openxr_get_space(space, out_pose, time)) {
		*out_pose = pose_identity;
		return false;
	}
	return true;
}

///////////////////////////////////////////

pose_t world_from_perception_anchor(void *perception_spatial_anchor) {
#if defined(SK_OS_WINDOWS_UWP)
	if (!xr_session) {
		log_warn("No OpenXR session available for converting perception anchors!");
		return pose_identity;
	}
	if (!sk_get_info_ref()->perception_bridge_present) {
		log_warn("This system doesn't support the perception bridge! Check SK.System.perceptionBridgePresent.");
		return pose_identity;
	}

	// Create an anchor from what the user gave us
	XrSpatialAnchorMSFT anchor = {};
	xr_extensions.xrCreateSpatialAnchorFromPerceptionAnchorMSFT(xr_session, (IUnknown*)perception_spatial_anchor, &anchor);

	// Create a Space from the anchor
	XrSpace                            space;
	XrSpatialAnchorSpaceCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	info.anchor            = anchor;
	info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	if (XR_FAILED(xr_extensions.xrCreateSpatialAnchorSpaceMSFT(xr_session, &info, &space))) {
		log_warn("world_from_perception_anchor: xrCreateSpatialAnchorSpaceMSFT call failed, possibly a bad anchor?");
		return pose_identity;
	}

	// Convert the space into a pose
	pose_t result;
	if (!openxr_get_space(space, &result)) {
		xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
		return pose_identity;
	}

	// Release the anchor, and return the resulting pose!
	xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
	return result;
#else
	log_warn("world_from_perception_anchor not available outside of Windows UWP!");
	return pose_identity;
#endif
}

///////////////////////////////////////////

bool32_t world_try_from_perception_anchor(void *perception_spatial_anchor, pose_t *out_pose) {
#if defined(SK_OS_WINDOWS_UWP)
	if (!xr_session) {
		log_warn("No OpenXR session available for converting perception anchors!");
		*out_pose = pose_identity;
		return false;
	}
	if (!sk_get_info_ref()->perception_bridge_present) {
		log_warn("This system doesn't support the perception bridge! Check SK.System.perceptionBridgePresent.");
		*out_pose = pose_identity;
		return false;
	}

	// Create an anchor from what the user gave us
	XrSpatialAnchorMSFT anchor = {};
	xr_extensions.xrCreateSpatialAnchorFromPerceptionAnchorMSFT(xr_session, (IUnknown*)perception_spatial_anchor, &anchor);

	// Create a Space from the anchor
	XrSpace                            space;
	XrSpatialAnchorSpaceCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	info.anchor            = anchor;
	info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	if (XR_FAILED(xr_extensions.xrCreateSpatialAnchorSpaceMSFT(xr_session, &info, &space))) {
		log_warn("world_from_perception_anchor: xrCreateSpatialAnchorSpaceMSFT call failed, possibly a bad anchor?");
		*out_pose = pose_identity;
		return false;
	}

	// Convert the space into a pose
	if (!openxr_get_space(space, out_pose)) {
		*out_pose = pose_identity;
		xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
		return false;
	}

	// Release the anchor, and return the resulting pose!
	xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
	return true;
#else
	log_warn("world_from_perception_anchor not available outside of Windows UWP!");
	*out_pose = pose_identity;
	return false;
#endif
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

///////////////////////////////////////////

bool32_t backend_openxr_ext_enabled(const char *extension_name) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
		return false;
	}
	uint64_t hash = hash_fnv64_string(extension_name);
	return xr_exts_loaded.index_of(hash) >= 0;
}

///////////////////////////////////////////

void backend_openxr_ext_request(const char *extension_name) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_exts_user.add(string_copy(extension_name));
}

///////////////////////////////////////////

bool is_ext_explicitly_requested(const char* extension_name) {
	for (int32_t i = 0; i < xr_exts_user.count; i++) {
		if (strcmp(xr_exts_user[i], extension_name) == 0) return true;
	}
	return false;
}

///////////////////////////////////////////

void backend_openxr_ext_exclude(const char* extension_name) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_exts_exclude.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_use_minimum_exts(bool32_t use_minimum_exts) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_minimum_exts = use_minimum_exts;
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_ pre_session must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_callbacks_pre_session_create.add({ on_pre_session_create, context });
}

///////////////////////////////////////////

void backend_openxr_add_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer), void* context) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
		return;
	}

	xr_callbacks_poll_event.add({ on_poll_event, context });
}

///////////////////////////////////////////

void backend_openxr_remove_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer)) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
		return;
	}

	for (int32_t i = 0; i < xr_callbacks_poll_event.count; i++) {
		if (xr_callbacks_poll_event[i].callback == on_poll_event || on_poll_event == nullptr) {
			xr_callbacks_poll_event.remove(i);
			return;
		}
	}
}

} // namespace sk

#endif
