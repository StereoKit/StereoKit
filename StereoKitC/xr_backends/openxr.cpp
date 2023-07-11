#include "../stereokit.h"
#include "../_stereokit.h"

#include "../platforms/platform_utils.h"

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
#include "../libraries/sk_gpu.h"
#include "../systems/render.h"
#include "../systems/audio.h"
#include "../systems/input.h"
#include "../systems/world.h"
#include "../hands/input_hand.h"
#include "../platforms/android.h"
#include "../platforms/linux.h"
#include "../platforms/uwp.h"
#include "../platforms/win32.h"

#include <openxr/openxr.h>
#include <openxr/openxr_reflection.h>
#include <openxr/openxr_platform.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
bool xr_has_articulated_hands = false;
bool xr_has_hand_meshes       = false;
bool xr_has_single_pass       = false;

XrInstance     xr_instance      = {};
XrSession      xr_session       = {};
XrExtTable     xr_extensions    = {};
XrExtInfo      xr_ext_available = {};
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
bool           xr_running       = false;
XrSpace        xr_app_space     = {};
XrReferenceSpaceType xr_app_space_type = {};
XrSpace        xr_stage_space   = {};
XrSpace        xr_head_space    = {};
XrSystemId     xr_system_id     = XR_NULL_SYSTEM_ID;
XrTime         xr_time          = 0;
XrTime         xr_eyes_sample_time = 0;
display_blend_ xr_valid_blends   = display_blend_none;
bool           xr_system_created = false;
bool           xr_system_success = false;

array_t<const char*> xr_exts_user    = {};
array_t<const char*> xr_exts_exclude = {};
array_t<uint64_t>    xr_exts_loaded  = {};
bool32_t             xr_minimum_exts = false;

array_t<context_callback_t>    xr_callbacks_pre_session_create = {};
array_t<poll_event_callback_t> xr_callbacks_poll_event         = {};

bool   xr_has_bounds        = false;
vec2   xr_bounds_size       = {};
pose_t xr_bounds_pose       = pose_identity;
pose_t xr_bounds_pose_local = pose_identity;

XrDebugUtilsMessengerEXT xr_debug = {};
XrReferenceSpaceType     xr_refspace;

///////////////////////////////////////////

bool32_t             openxr_try_get_app_space(XrSession session, origin_mode_ mode, XrTime time, XrReferenceSpaceType *out_space_type, pose_t* out_space_offset, XrSpace *out_app_space);
void                 openxr_preferred_layers (uint32_t &out_layer_count, const char **out_layers);
XrTime               openxr_acquire_time     ();
bool                 openxr_blank_frame      ();

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
	init_android.applicationVM      = android_vm;
	init_android.applicationContext = android_activity;
	if (XR_FAILED(ext_xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR *)&init_android))) {
		log_fail_reasonf(90, log_warning, "Failed to initialize OpenXR loader");
		return false;
	}
#endif

	array_t<const char *> extensions = openxr_list_extensions(xr_exts_user, xr_exts_exclude, xr_minimum_exts, [](const char *ext) {log_diagf("available: %s", ext);});
	extensions.each([](const char *&ext) { 
		log_diagf("REQUESTED: <~grn>%s<~clr>", ext);
		xr_exts_loaded.add(hash_fnv64_string(ext));
	});

	uint32_t layer_count = 0;
	openxr_preferred_layers(layer_count, nullptr);
	const char **layers = sk_malloc_t(const char *, layer_count);
	openxr_preferred_layers(layer_count, layers);

	XrInstanceCreateInfo create_info = { XR_TYPE_INSTANCE_CREATE_INFO };
	create_info.enabledExtensionCount = (uint32_t)extensions.count;
	create_info.enabledExtensionNames = extensions.data;
	create_info.enabledApiLayerCount  = layer_count;
	create_info.enabledApiLayerNames  = layers;
	create_info.applicationInfo.applicationVersion = 1;

	// Use a version Id formatted as 0xMMMiiPPP
	create_info.applicationInfo.engineVersion =
		(SK_VERSION_MAJOR << 20)              |
		(SK_VERSION_MINOR << 12 & 0x000FF000) |
		(SK_VERSION_PATCH       & 0x00000FFF);

	create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	snprintf(create_info.applicationInfo.applicationName, sizeof(create_info.applicationInfo.applicationName), "%s", sk_app_name);
	snprintf(create_info.applicationInfo.engineName,      sizeof(create_info.applicationInfo.engineName     ), "StereoKit");
#if defined(SK_OS_ANDROID)
	XrInstanceCreateInfoAndroidKHR create_android = { XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR };
	create_android.applicationVM       = android_vm;
	create_android.applicationActivity = android_activity;
	create_info.next = (void*)&create_android;
#endif
	XrResult result = xrCreateInstance(&create_info, &xr_instance);

	extensions.free();
	sk_free(layers);

	// Check if OpenXR is on this system, if this is null here, the user needs
	// to install an OpenXR runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		const char *err_name = openxr_string(result);

		log_fail_reasonf(90, log_inform, "Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", err_name);
		openxr_cleanup();
		return false;
	}

	// Fetch the runtime name/info, for logging and for a few other checks
	XrInstanceProperties inst_properties = { XR_TYPE_INSTANCE_PROPERTIES };
	xr_check(xrGetInstanceProperties(xr_instance, &inst_properties),
		"xrGetInstanceProperties failed [%s]");

	log_diagf("Found OpenXR runtime: <~grn>%s<~clr> %u.%u.%u",
		inst_properties.runtimeName,
		XR_VERSION_MAJOR(inst_properties.runtimeVersion),
		XR_VERSION_MINOR(inst_properties.runtimeVersion),
		XR_VERSION_PATCH(inst_properties.runtimeVersion));

	// Create links to the extension functions
	xr_extensions = xrCreateExtensionTable(xr_instance);

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
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_info.userCallback = [](XrDebugUtilsMessageSeverityFlagsEXT severity, XrDebugUtilsMessageTypeFlagsEXT, const XrDebugUtilsMessengerCallbackDataEXT *msg, void*) {
		// Print the debug message we got! There's a bunch more info we could
		// add here too, but this is a pretty good start, and you can always
		// add a breakpoint this line!
		log_ level = log_diagnostic;
		if      (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  ) level = log_error;
		else if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) level = log_warning;
		log_writef(level, "%s: %s", msg->functionName, msg->message);

		// Returning XR_TRUE here will force the calling function to fail
		return (XrBool32)XR_FALSE;
	};
	// Start up the debug utils!
	if (xr_ext_available.EXT_debug_utils)
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
		"xrGetGraphicsRequirementsKHR failed [%s]");
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
	if (xr_ext_available.EXTX_overlay && sk_settings.overlay_app) {
		overlay_info.sessionLayersPlacement = sk_settings.overlay_priority;
		gfx_binding.next = &overlay_info;
		sk_info.overlay_app = true;
	}
	XrResult result = xrCreateSession(xr_instance, &session_info, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (XR_FAILED(result) || xr_session == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
		openxr_cleanup();
		return false;
	}

	// Fetch the runtime name/info, for logging and for a few other checks
	XrInstanceProperties inst_properties = { XR_TYPE_INSTANCE_PROPERTIES };
	xr_check(xrGetInstanceProperties(xr_instance, &inst_properties),
		"xrGetInstanceProperties failed [%s]");

	// Figure out what this device is capable of!
	XrSystemProperties                      properties          = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT       properties_tracking = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	XrSystemHandTrackingMeshPropertiesMSFT  properties_handmesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
	XrSystemEyeGazeInteractionPropertiesEXT properties_gaze     = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
	// Validation layer does not seem to like 'next' chaining beyond a depth of
	// 1, so we'll just call these one at a time.
	xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties failed [%s]");
	if (xr_ext_available.EXT_hand_tracking) {
		properties.next = &properties_tracking;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties failed [%s]");
	}
	if (xr_ext_available.MSFT_hand_tracking_mesh) {
		properties.next = &properties_handmesh;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties failed [%s]");
	}
	if (xr_ext_available.EXT_eye_gaze_interaction) {
		properties.next = &properties_gaze;
		xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties), "xrGetSystemProperties failed [%s]");
	}

	log_diagf("System name: <~grn>%s<~clr>", properties.systemName);
	device_data.name = string_copy(properties.systemName);

	xr_has_single_pass                = true;
	xr_has_articulated_hands          = xr_ext_available.EXT_hand_tracking        && properties_tracking.supportsHandTracking;
	xr_has_hand_meshes                = xr_ext_available.MSFT_hand_tracking_mesh  && properties_handmesh.supportsHandTrackingMesh;
	sk_info.eye_tracking_present      = xr_ext_available.EXT_eye_gaze_interaction && properties_gaze    .supportsEyeGazeInteraction;
	sk_info.perception_bridge_present = xr_ext_available.MSFT_perception_anchor_interop;
	sk_info.spatial_bridge_present    = xr_ext_available.MSFT_spatial_graph_bridge;

	if (skg_capability(skg_cap_tex_layer_select) && xr_has_single_pass) log_diagf("Platform supports single-pass rendering");
	else                                                                log_diagf("Platform does not support single-pass rendering");

	// Exception for the articulated WMR hand simulation, and the Vive Index
	// controller equivalent. These simulations are insufficient to treat them
	// like true articulated hands.
	// 
	// Key indicators are Windows+x64+(WMR or SteamVR), and skip if Ultraleap's hand
	// tracking layer is present.
	//
	// TODO: Remove this when the hand tracking extension is improved.
#if defined(_M_X64) && (defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP))
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
	if (xr_ext_available.EXT_hand_tracking && has_leap_layer == false) {
		if (strcmp(inst_properties.runtimeName, "Windows Mixed Reality Runtime") == 0 ||
			strcmp(inst_properties.runtimeName, "SteamVR/OpenXR") == 0) {
			log_diag("Rejecting OpenXR's provided hand tracking extension due to the suspicion that it is inadequate for StereoKit.");
			xr_has_articulated_hands = false;
			xr_has_hand_meshes       = false;
		}
	}
#endif

	device_data.has_eye_gaze      = sk_info.eye_tracking_present;
	device_data.has_hand_tracking = xr_has_articulated_hands;
	device_data.tracking          = device_tracking_none;
	if      (properties.trackingProperties.positionTracking)    device_data.tracking = device_tracking_6dof;
	else if (properties.trackingProperties.orientationTracking) device_data.tracking = device_tracking_3dof;


	if (xr_has_articulated_hands)          log_diag("OpenXR articulated hands ext enabled!");
	if (xr_has_hand_meshes)                log_diag("OpenXR hand mesh ext enabled!");
	if (sk_info.eye_tracking_present)      log_diag("OpenXR gaze ext enabled!");
	if (sk_info.spatial_bridge_present)    log_diag("OpenXR spatial bridge ext enabled!");
	if (sk_info.perception_bridge_present) log_diag("OpenXR perception anchor interop ext enabled!");

	// Check scene understanding features, these may be dependant on Session
	// creation in the context of Holographic Remoting.
	if (xr_ext_available.MSFT_scene_understanding) {
		uint32_t count = 0;
		xr_extensions.xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, 0, &count, nullptr);
		XrSceneComputeFeatureMSFT *features = sk_malloc_t(XrSceneComputeFeatureMSFT, count);
		xr_extensions.xrEnumerateSceneComputeFeaturesMSFT(xr_instance, xr_system_id, count, &count, features);
		for (uint32_t i = 0; i < count; i++) {
			if      (features[i] == XR_SCENE_COMPUTE_FEATURE_VISUAL_MESH_MSFT  ) sk_info.world_occlusion_present = true;
			else if (features[i] == XR_SCENE_COMPUTE_FEATURE_COLLIDER_MESH_MSFT) sk_info.world_raycast_present   = true;
		}
		sk_free(features);
	}
	if (sk_info.world_occlusion_present) log_diag("OpenXR world occlusion enabled! (Scene Understanding)");
	if (sk_info.world_raycast_present)   log_diag("OpenXR world raycast enabled! (Scene Understanding)");

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
	while (openxr_poll_events() && (xr_session_state == XR_SESSION_STATE_IDLE || xr_session_state == XR_SESSION_STATE_READY)) {
		openxr_blank_frame();
	}

	// Create reference spaces! So we can find stuff relative to them :) Some
	// platforms still take time after session start before the spaces provide
	// valid data, so we'll wait for that here.
	int32_t ref_space_tries = 10;
	while (openxr_try_get_app_space(xr_session, sk_settings.origin, xr_time, &xr_app_space_type, &world_origin_offset, &xr_app_space) == false && openxr_poll_events() && ref_space_tries > 0) {
		openxr_blank_frame();
		ref_space_tries--;
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

	if (sk_info.overlay_app) {
		log_diag("Starting as an overlay app, display blend mode switched to blend.");
		sk_info.display_type = display_blend;
	}

	xr_has_bounds  = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);
	xr_bounds_pose = matrix_transform_pose(render_get_cam_final(), xr_bounds_pose_local);

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (xr_ext_available.OCULUS_audio_device_guid) {
		wchar_t device_guid[128];
		if (XR_SUCCEEDED(xr_extensions.xrGetAudioOutputDeviceGuidOculus(xr_instance, device_guid))) {
			audio_set_default_device_out(device_guid);
		}
		if (XR_SUCCEEDED(xr_extensions.xrGetAudioInputDeviceGuidOculus(xr_instance, device_guid))) {
			audio_set_default_device_in(device_guid);
		}
	}
#endif

	return true;
}

///////////////////////////////////////////

bool openxr_blank_frame() {
	XrFrameWaitInfo wait_info   = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState    frame_state = { XR_TYPE_FRAME_STATE };
	xr_check(xrWaitFrame(xr_session, &wait_info, &frame_state),
		"blank xrWaitFrame [%s]");

	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	xr_check(xrBeginFrame(xr_session, &begin_info),
		"blank xrBeginFrame [%s]");

	XrFrameEndInfo end_info = { XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	xr_check(xrEndFrame(xr_session, &end_info),
		"blank xrEndFrame [%s]");

	return true;
}

///////////////////////////////////////////

void openxr_preferred_layers(uint32_t &out_layer_count, const char **out_layers) {
	// Find what extensions are available on this system!
	uint32_t layer_count = 0;
	xrEnumerateApiLayerProperties(0, &layer_count, nullptr);
	XrApiLayerProperties *layers = sk_malloc_t(XrApiLayerProperties, layer_count);
	for (uint32_t i = 0; i < layer_count; i++) layers[i] = { XR_TYPE_API_LAYER_PROPERTIES };
	xrEnumerateApiLayerProperties(layer_count, &layer_count, layers);

	if (out_layers == nullptr) {
		for (uint32_t i = 0; i < layer_count; i++) {
			log_diagf("OpenXR layer found: %s", layers[i].layerName);
		}
	}

	// Count how many there are, and copy them out
	out_layer_count = 0;
	for (int32_t e = 0; e < _countof(xr_request_layers); e++) {
		for (uint32_t i = 0; i < layer_count; i++) {
			if (strcmp(layers[i].layerName, xr_request_layers[e]) == 0) {
				if (out_layers != nullptr)
					out_layers[out_layer_count] = xr_request_layers[e];
				out_layer_count += 1;
				break;
			}
		}
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
		case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:  has_unbounded   = true; break;
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

void openxr_shutdown() {
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
	if (xr_running)
		openxr_poll_actions();
}

///////////////////////////////////////////

void openxr_step_end() {
	if (xr_running)
		openxr_render_frame();

	xr_extension_structs_clear();
	render_clear();
}

///////////////////////////////////////////

bool openxr_poll_events() {
	XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	bool result = true;

	while (xrPollEvent(xr_instance, &event_buffer) == XR_SUCCESS) {
		switch (event_buffer.type) {
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged *changed = (XrEventDataSessionStateChanged*)&event_buffer;
			xr_session_state = changed->state;

			if      (xr_session_state == XR_SESSION_STATE_FOCUSED) sk_focus = app_focus_active;
			else if (xr_session_state == XR_SESSION_STATE_VISIBLE) sk_focus = app_focus_background;
			else                                                   sk_focus = app_focus_hidden;

			// Session state change is where we can begin and end sessions, as well as find quit messages!
			switch (xr_session_state) {
			// The runtime should never return this: https://www.khronos.org/registry/OpenXR/specs/1.0/man/html/XrSessionState.html
			case XR_SESSION_STATE_UNKNOWN: log_errf("Runtime gave us a XR_SESSION_STATE_UNKNOWN! Bad runtime!"); break;
			case XR_SESSION_STATE_IDLE: break; // Wait till we get XR_SESSION_STATE_READY.
			case XR_SESSION_STATE_READY: {
				// Get the session started!
				XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
				begin_info.primaryViewConfigurationType = xr_display_types[0];

				XrSecondaryViewConfigurationSessionBeginInfoMSFT secondary = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT };
				if (xr_display_types.count > 1) {
					secondary.next                          = nullptr;
					secondary.viewConfigurationCount        = xr_display_types.count-1;
					secondary.enabledViewConfigurationTypes = &xr_display_types[1];
					begin_info.next = &secondary;
				}

				xrBeginSession(xr_session, &begin_info);

				// FoV normally updates right before drawing, but we need it to
				// be available as soon as the session begins, for apps that
				// are listening to sk_app_focus changing to determine if FoV
				// is ready.
				openxr_views_update_fov();

				xr_running = true;
				log_diag("OpenXR session begin.");
			} break;
			case XR_SESSION_STATE_SYNCHRONIZED: break;
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_running = false; result = false; break;
			case XR_SESSION_STATE_VISIBLE: break; // In this case, we can't recieve input. For now pretend it's not happening.
			case XR_SESSION_STATE_FOCUSED: break; // This is probably the normal case, so everything can continue!
			case XR_SESSION_STATE_LOSS_PENDING: sk_running = false; result = false; break;
			case XR_SESSION_STATE_EXITING:      sk_running = false; result = false; break;
			default: break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged *changed = (XrEventDataInteractionProfileChanged*)&event_buffer;
			if (changed->session == xr_session) {
				oxri_update_interaction_profile();
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: sk_running = false; result = false; break;
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
			XrEventDataReferenceSpaceChangePending *pending = (XrEventDataReferenceSpaceChangePending*)&event_buffer;
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
	if (!sk_info.spatial_bridge_present) {
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
	if (!sk_info.spatial_bridge_present) {
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
	if (!sk_info.perception_bridge_present) {
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
	if (!sk_info.perception_bridge_present) {
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
	if (sk_initialized) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_exts_user.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_ext_exclude(const char* extension_name) {
	if (sk_initialized) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_exts_exclude.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_use_minimum_exts(bool32_t use_minimum_exts) {
	if (sk_initialized) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_minimum_exts = use_minimum_exts;
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (sk_initialized) {
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
