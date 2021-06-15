// #pragma comment(lib,"openxr_loader.lib")
// Windows thing? ^^

#include "openxr.h"
#include "openxr_input.h"
#include "openxr_view.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../sk_memory.h"
#include "../../log.h"
#include "../../asset_types/texture.h"
#include "../render.h"
#include "../input.h"
#include "../hand/input_hand.h"
#include "android.h"
#include "linux.h"
#include "uwp.h"
#include "win32.h"
#include "platform_utils.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

XrFormFactor xr_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
const char *xr_request_layers[] = {
	"",
};
bool xr_has_depth_lsr         = false;
bool xr_has_articulated_hands = false;
bool xr_has_hand_meshes       = false;

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

bool   xr_check_bounds = false;
bool   xr_has_bounds   = false;
vec2   xr_bounds_size  = {};
pose_t xr_bounds_pose  = { {}, quat_identity };

XrDebugUtilsMessengerEXT xr_debug = {};
XrReferenceSpaceType     xr_refspace;

///////////////////////////////////////////

XrReferenceSpaceType openxr_preferred_space();
bool                 openxr_preferred_extensions(uint32_t &out_extension_count, const char **out_extensions);
void                 openxr_preferred_layers(uint32_t &out_layer_count, const char **out_layers);

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

	XrExtent2Df bounds;
	if (XR_FAILED(xrGetReferenceSpaceBoundsRect(xr_session, XR_REFERENCE_SPACE_TYPE_STAGE, &bounds)))
		return false;
	if (!openxr_get_space(xr_stage_space, out_pose, time))
		return false;
	
	out_size->x = bounds.width;
	out_size->y = bounds.height;

	log_diagf("Bounds updated: %.2f<~BLK>x<~clr>%.2f at (%.1f,%.1f,%.1f) (%.2f,%.2f,%.2f,%.2f)", 
		out_size->x, out_size->y, 
		out_pose->position.x, out_pose->position.y, out_pose->position.z,
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

int64_t openxr_get_time() {
#ifdef XR_USE_TIMESPEC
	struct timespec time;
	xr_extensions.xrConvertTimeToTimespecTimeKHR(xr_instance, xr_time, &time);
	return time.tv_sec*1000000000 + time.tv_nsec;
#else
	LARGE_INTEGER time;
	xr_extensions.xrConvertTimeToWin32PerformanceCounterKHR(xr_instance, xr_time, &time);
	return time.QuadPart;
#endif
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

	array_t<const char *> extensions = openxr_list_extensions([](const char *ext) {log_diagf("available: %s", ext);});
	extensions.each([](const char *&ext) { log_diagf("REQUESTED: <~grn>%s<~clr>", ext); });

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
	free(layers);

	// Check if OpenXR is on this system, if this is null here, the user needs to install an
	// OpenXR runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		log_fail_reasonf(90, log_warning, "Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", openxr_string(result));
		openxr_shutdown();
		return false;
	}

	// Create links to the extension functions
	xr_extensions = xrCreateExtensionTable(xr_instance);

#ifdef _DEBUG
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
		log_fail_reasonf(90, log_warning, "Couldn't find our desired MR form factor, no MR device attached/ready? [%s]", openxr_string(result));
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
	xr_has_articulated_hands          = xr_ext_available.EXT_hand_tracking        && properties_tracking.supportsHandTracking;
	xr_has_hand_meshes                = xr_ext_available.MSFT_hand_tracking_mesh  && properties_handmesh.supportsHandTrackingMesh;
	sk_info.eye_tracking_present      = xr_ext_available.EXT_eye_gaze_interaction && properties_gaze    .supportsEyeGazeInteraction;
	sk_info.perception_bridge_present = xr_ext_available.MSFT_perception_anchor_interop;
	sk_info.spatial_bridge_present    = xr_ext_available.MSFT_spatial_graph_bridge;
	xr_has_depth_lsr                  = xr_ext_available.KHR_composition_layer_depth;

	if (xr_has_articulated_hands)          log_diag("OpenXR articulated hands ext enabled!");
	if (xr_has_hand_meshes)                log_diag("OpenXR hand mesh ext enabled!");
	if (xr_has_depth_lsr)                  log_diag("OpenXR depth LSR ext enabled!");
	if (sk_info.eye_tracking_present)      log_diag("OpenXR gaze ext enabled!");
	if (sk_info.spatial_bridge_present)    log_diag("OpenXR spatial bridge ext enabled!");
	if (sk_info.perception_bridge_present) log_diag("OpenXR perception anchor interop ext enabled!");

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
	gfx_binding.glxDrawable = (GLXDrawable)platform._glx_drawable;
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
		log_fail_reasonf(90, log_warning, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
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
		xr_stage_space = nullptr;
	}

	if (!openxr_views_create() || !oxri_init()) {
		openxr_shutdown();
		return false;
	}

	if (sk_info.overlay_app) {
		log_diag("Starting as an overlay app, display blend mode switched to blend.");
		sk_info.display_type = display_blend;
	}

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

	free(layers);
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

	free(refspace_types);

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

	// Check if the bounds have changed. This needs a valid xr_time, which is
	// why we have this on a flag delay. xr_time is set in openxr_render_frame
	if (xr_check_bounds) {
		xr_check_bounds = false;
		xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose, xr_time);
	}
}

///////////////////////////////////////////

void openxr_poll_events() {
	XrEventDataBuffer event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };

	while (xrPollEvent(xr_instance, &event_buffer) == XR_SUCCESS) {
		switch (event_buffer.type) {
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged *changed = (XrEventDataSessionStateChanged*)&event_buffer;
			xr_session_state = changed->state;
			sk_focused       = xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED;

			// Session state change is where we can begin and end sessions, as well as find quit messages!
			switch (xr_session_state) {
			case XR_SESSION_STATE_READY: {
				XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
				begin_info.primaryViewConfigurationType = xr_display_types[0];

				XrSecondaryViewConfigurationSessionBeginInfoMSFT secondary = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT };
				if (xr_display_types.count > 1) {
					secondary.next                          = nullptr;
					secondary.viewConfigurationCount        = (int32_t)xr_display_types.count-1;
					secondary.enabledViewConfigurationTypes = &xr_display_types[1];
					begin_info.next = &secondary;
				}

				xrBeginSession(xr_session, &begin_info);

				xr_running = true;
				log_diag("OpenXR session begin.");
			} break;
			case XR_SESSION_STATE_SYNCHRONIZED: {
				xr_check_bounds = true;
			} break;
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_running = false; break;
			case XR_SESSION_STATE_EXITING:      sk_run = false;              break;
			case XR_SESSION_STATE_LOSS_PENDING: sk_run = false;              break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
			XrEventDataInteractionProfileChanged *changed = (XrEventDataInteractionProfileChanged*)&event_buffer;
			if (changed->session == xr_session) {
				oxri_update_interaction_profile();
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: sk_run = false; return;
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
			XrEventDataReferenceSpaceChangePending *pending = (XrEventDataReferenceSpaceChangePending*)&event_buffer;
			xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose, pending->changeTime);
			break;
		}
		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
}

///////////////////////////////////////////

void openxr_poll_actions() {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED || xr_time == 0)
		return;

	// Track the head location
	openxr_get_space(xr_head_space, &input_head_pose_local);
	matrix root = render_get_cam_final();
	input_head_pose_world.position    = matrix_mul_point   (root, input_head_pose_local.position);
	input_head_pose_world.orientation = matrix_mul_rotation(root, input_head_pose_local.orientation);

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

pose_t world_from_spatial_graph(uint8_t spatial_graph_node_id[16]) {
	if (!xr_session) {
		log_warn("No OpenXR session available for converting spatial graph nodes!");
		return { {0,0,0}, {0,0,0,1} };
	}
	if (!sk_info.spatial_bridge_present) {
		log_warn("This system doesn't support the spatial bridge! Check SK.System.spatialBridgePresent.");
		return { {0,0,0}, {0,0,0,1} };
	}

	XrSpace                               space;
	pose_t                                result     = {};
	XrSpatialGraphNodeSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT };
	space_info.nodeType = XR_SPATIAL_GRAPH_NODE_TYPE_STATIC_MSFT;
	space_info.pose     = { {0,0,0,1}, {0,0,0} };
	memcpy(space_info.nodeId, spatial_graph_node_id, sizeof(space_info.nodeId));

	xr_extensions.xrCreateSpatialGraphNodeSpaceMSFT(xr_session, &space_info, &space);

	openxr_get_space(space, &result);
	return result;
}

///////////////////////////////////////////

pose_t world_from_perception_anchor(void *perception_spatial_anchor) {
#if defined(SK_OS_WINDOWS_UWP)
	if (!xr_session) {
		log_warn("No OpenXR session available for converting perception anchors!");
		return { {0,0,0}, {0,0,0,1} };
	}
	if (!sk_info.perception_bridge_present) {
		log_warn("This system doesn't support the perception bridge! Check SK.System.perceptionBridgePresent.");
		return { {0,0,0}, {0,0,0,1} };
	}

	// Create an anchor from what the user gave us
	XrSpatialAnchorMSFT anchor = {};
	xr_extensions.xrCreateSpatialAnchorFromPerceptionAnchorMSFT(xr_session, (IUnknown*)perception_spatial_anchor, &anchor);

	// Create a Space from the anchor
	XrSpace                            space;
	XrSpatialAnchorSpaceCreateInfoMSFT info = { XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT };
	info.anchor            = anchor;
	info.poseInAnchorSpace = { {0,0,0,1}, {0,0,0} };
	xr_extensions.xrCreateSpatialAnchorSpaceMSFT(xr_session, &info, &space);

	// Convert the space into a pose
	pose_t result;
	openxr_get_space(space, &result);

	// Release the anchor, and return the resulting pose!
	xr_extensions.xrDestroySpatialAnchorMSFT(anchor);
	return result;
#else
	log_warn("world_from_perception_anchor not available outside of Windows UWP!");
	return { {0,0,0}, {0,0,0,1} };
#endif
}

} // namespace sk
