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
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/sk_gpu.h"
#include "../systems/render.h"
#include "../systems/audio.h"
#include "../systems/input.h"
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

XrFormFactor xr_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
const char *xr_request_layers[] = {
	"",
};
bool xr_has_depth_lsr         = false;
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
XrSpace        xr_stage_space   = {};
XrSpace        xr_head_space    = {};
XrSystemId     xr_system_id     = XR_NULL_SYSTEM_ID;
XrTime         xr_time          = 0;
XrTime         xr_eyes_sample_time = 0;

array_t<const char*> xr_exts_user   = {};
array_t<uint64_t>    xr_exts_loaded = {};

array_t<context_callback_t> xr_callbacks_pre_session_create = {};

bool   xr_has_bounds        = false;
vec2   xr_bounds_size       = {};
pose_t xr_bounds_pose       = pose_identity;
pose_t xr_bounds_pose_local = pose_identity;

XrDebugUtilsMessengerEXT xr_debug = {};
XrReferenceSpaceType     xr_refspace;

///////////////////////////////////////////

XrReferenceSpaceType openxr_preferred_space ();
void                 openxr_preferred_layers(uint32_t &out_layer_count, const char **out_layers);
XrTime               openxr_acquire_time    ();

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
	}

	log_diagf("Bounds updated: %.2f<~BLK>x<~clr>%.2f at (%.1f,%.1f,%.1f) (%.2f,%.2f,%.2f,%.2f)",
		out_size->x, out_size->y,
		out_pose->position   .x, out_pose->position   .y, out_pose->position   .z,
		out_pose->orientation.x, out_pose->orientation.y, out_pose->orientation.z, out_pose->orientation.w);
	return true;
}

///////////////////////////////////////////

XrGraphicsRequirements luid_requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS };
void *openxr_get_luid() {
#if defined(XR_USE_GRAPHICS_API_D3D11)
	const char *extensions[] = { XR_GFX_EXTENSION };

	XrInstanceCreateInfo create_info = { XR_TYPE_INSTANCE_CREATE_INFO };
	create_info.enabledExtensionCount      = 1;
	create_info.enabledExtensionNames      = extensions;
	create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	snprintf(create_info.applicationInfo.applicationName, sizeof(create_info.applicationInfo.applicationName), "%s", sk_app_name);
	snprintf(create_info.applicationInfo.engineName,      sizeof(create_info.applicationInfo.engineName     ), "StereoKit");
	XrResult result = xrCreateInstance(&create_info, &xr_instance);

	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		return nullptr;
	}

	// Request a form factor from the device (HMD, Handheld, etc.)
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = xr_config_form;
	result = xrGetSystem(xr_instance, &systemInfo, &xr_system_id);
	if (XR_FAILED(result)) {
		xrDestroyInstance(xr_instance);
		return nullptr;
	}

	// Get an extension function, and check it for our requirements
	PFN_xrGetGraphicsRequirementsKHR xrGetGraphicsRequirementsKHR;
	if (XR_FAILED(xrGetInstanceProcAddr(xr_instance, NAME_xrGetGraphicsRequirementsKHR, (PFN_xrVoidFunction *)(&xrGetGraphicsRequirementsKHR))) ||
		XR_FAILED(xrGetGraphicsRequirementsKHR(xr_instance, xr_system_id, &luid_requirement))) {
		xrDestroyInstance(xr_instance);
		return nullptr;
	}

	xrDestroyInstance(xr_instance);
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

	array_t<const char *> extensions = openxr_list_extensions(xr_exts_user, [](const char *ext) {log_diagf("available: %s", ext);});
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
		openxr_shutdown();
		return false;
	}

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
		openxr_shutdown();
		return false;
	}

	// Figure out what this device is capable of!
	XrSystemProperties                      properties          = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT       properties_tracking = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	XrSystemHandTrackingMeshPropertiesMSFT  properties_handmesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
	XrSystemEyeGazeInteractionPropertiesEXT properties_gaze     = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
	properties         .next = &properties_tracking;
	properties_tracking.next = &properties_handmesh;
	properties_handmesh.next = &properties_gaze;

	xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties),
		"xrGetSystemProperties failed [%s]");
	log_diagf("Using system: <~grn>%s<~clr>", properties.systemName);
	xr_has_single_pass                = true;
	xr_has_articulated_hands          = xr_ext_available.EXT_hand_tracking        && properties_tracking.supportsHandTracking;
	xr_has_hand_meshes                = xr_ext_available.MSFT_hand_tracking_mesh  && properties_handmesh.supportsHandTrackingMesh;
	xr_has_depth_lsr                  = xr_ext_available.KHR_composition_layer_depth;
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

	// Get the runtime name so we're less likely to invalidate something we
	// don't kow about.
	XrInstanceProperties inst_properties = { XR_TYPE_INSTANCE_PROPERTIES };
	xr_check(xrGetInstanceProperties(xr_instance, &inst_properties),
		"xrGetInstanceProperties failed [%s]");

	// The Leap hand tracking layer seems to supercede the built-in extensions.
	if (has_leap_layer == false) {
		if (strcmp(inst_properties.runtimeName, "Windows Mixed Reality Runtime") == 0 ||
			strcmp(inst_properties.runtimeName, "SteamVR/OpenXR") == 0) {
			log_diag("Rejecting OpenXR's provided hand tracking extension due to the suspicion that it is inadequate for StereoKit.");
			xr_has_articulated_hands = false;
			xr_has_hand_meshes       = false;
		}
	}
#endif

	if (xr_has_articulated_hands)          log_diag("OpenXR articulated hands ext enabled!");
	if (xr_has_hand_meshes)                log_diag("OpenXR hand mesh ext enabled!");
	if (xr_has_depth_lsr)                  log_diag("OpenXR depth LSR ext enabled!");
	if (sk_info.eye_tracking_present)      log_diag("OpenXR gaze ext enabled!");
	if (sk_info.spatial_bridge_present)    log_diag("OpenXR spatial bridge ext enabled!");
	if (sk_info.perception_bridge_present) log_diag("OpenXR perception anchor interop ext enabled!");

	// Check scene understanding features
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

	// Before we call xrCreateSession, lets fire an event for anyone that needs
	// to set things up!
	for (size_t i = 0; i < xr_callbacks_pre_session_create.count; i++) {
		xr_callbacks_pre_session_create[i].callback(xr_callbacks_pre_session_create[i].context);
	}
	xr_callbacks_pre_session_create.free();

	// OpenXR wants to ensure apps are using the correct LUID, so this MUST be called before xrCreateSession
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

	// A session represents this application's desire to display things! This is where we hook up our graphics API.
	// This does not start the session, for that, you'll need a call to xrBeginSession, which we do in openxr_poll_events
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
	result = xrCreateSession(xr_instance, &session_info, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (XR_FAILED(result) || xr_session == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_inform, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
		openxr_shutdown();
		return false;
	}

	// Create reference spaces! So we can find stuff relative to them :)
	xr_refspace = openxr_preferred_space();

	// The space for our application
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = xr_refspace;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_app_space);
	if (XR_FAILED(result)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(result));
		openxr_shutdown();
		return false;
	}

	// The space for our head
	ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_VIEW;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_head_space);
	if (XR_FAILED(result)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(result));
		openxr_shutdown();
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

	if (!openxr_views_create() || !oxri_init()) {
		openxr_shutdown();
		return false;
	}

	if (sk_info.overlay_app) {
		log_diag("Starting as an overlay app, display blend mode switched to blend.");
		sk_info.display_type = display_blend;
	}

	xr_time        = openxr_acquire_time();
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

XrReferenceSpaceType openxr_preferred_space() {

	// OpenXR uses a couple different types of reference frames for 
	// positioning content, we need to choose one for displaying our content!
	// STAGE would be relative to the center of your guardian system's 
	// bounds, and LOCAL would be relative to your device's starting location.

	XrReferenceSpaceType refspace_priority[] = {
		XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT,
		XR_REFERENCE_SPACE_TYPE_LOCAL,
		XR_REFERENCE_SPACE_TYPE_STAGE, };

	// Find the spaces OpenXR has access to on this device
	uint32_t refspace_count = 0;
	xrEnumerateReferenceSpaces(xr_session, 0, &refspace_count, nullptr);
	XrReferenceSpaceType *refspace_types = sk_malloc_t(XrReferenceSpaceType, refspace_count);
	xrEnumerateReferenceSpaces(xr_session, refspace_count, &refspace_count, refspace_types);

	// Find which one we prefer!
	XrReferenceSpaceType result = (XrReferenceSpaceType)0;
	for (int32_t p = 0; p < _countof(refspace_priority); p++) {
		for (uint32_t i = 0; i < refspace_count; i++) {
			if (refspace_types[i] == refspace_priority[p]) {
				result = refspace_types[i];
				break;
			}
		}
		if (result != 0)
			break;
	}

	// TODO: UNBOUNDED_MSFT and STAGE have very different behavior, but
	// STAGE behavior is preferred. So it would be nice to make some considerations
	// here to change that?

	sk_free(refspace_types);

	return result;
}

///////////////////////////////////////////

void openxr_shutdown() {
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

void openxr_step_begin() {
	openxr_poll_events();
	if (xr_running)
		openxr_poll_actions();
}

///////////////////////////////////////////

void openxr_step_end() {
	if (xr_running)
		openxr_render_frame();

	xr_compositor_layers_clear();
	render_clear();
}

///////////////////////////////////////////

void openxr_poll_events() {
	XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };

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

				xr_running = true;
				log_diag("OpenXR session begin.");
			} break;
			case XR_SESSION_STATE_SYNCHRONIZED: break;
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_running = false; break;
			case XR_SESSION_STATE_VISIBLE: break; // In this case, we can't recieve input. For now pretend it's not happening.
			case XR_SESSION_STATE_FOCUSED: break; // This is probably the normal case, so everything can continue!
			case XR_SESSION_STATE_LOSS_PENDING: sk_running = false;              break;
			case XR_SESSION_STATE_EXITING:      sk_running = false;              break;
			default: break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged *changed = (XrEventDataInteractionProfileChanged*)&event_buffer;
			if (changed->session == xr_session) {
				oxri_update_interaction_profile();
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: sk_running = false; return;
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
			XrEventDataReferenceSpaceChangePending *pending = (XrEventDataReferenceSpaceChangePending*)&event_buffer;
			xr_has_bounds  = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, pending->changeTime);
			xr_bounds_pose = matrix_transform_pose(render_get_cam_final(), xr_bounds_pose_local);
		}
			break;
		default: break;
		}
		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
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
	openxr_get_space(space, &result);

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
	openxr_get_space(space, out_pose);

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
	XrResult result = xrGetInstanceProcAddr(xr_instance, function_name, &fn);
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
		log_err("backend_openxr_ext_request must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_exts_user.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (sk_initialized) {
		log_err("backend_openxr_ext_request must be called BEFORE StereoKit initialization!");
		return;
	}

	xr_callbacks_pre_session_create.add({ on_pre_session_create, context });
}

} // namespace sk

#endif