#pragma comment(lib,"openxr_loader.lib")

#include "openxr.h"
#include "openxr_input.h"
#include "openxr_view.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../log.h"
#include "../../systems/d3d.h"
#include "../../systems/render.h"
#include "../../systems/input.h"
#include "../../systems/hand/input_hand.h"
#include "../../asset_types/texture.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <string.h>
#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

XrFormFactor xr_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

const char *xr_request_extensions[] = {
	XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
	XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME,
	XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME,
	XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME,
	XR_MSFT_HAND_INTERACTION_EXTENSION_NAME,
	XR_EXT_HAND_TRACKING_EXTENSION_NAME,
	XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME,
	XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME,
	XR_MSFT_FIRST_PERSON_OBSERVER_EXTENSION_NAME,
};
const char *xr_request_layers[] = {
	"",
};
bool xr_depth_lsr     = false;
bool xr_depth_lsr_ext = false;
bool xr_articulated_hands     = false;
bool xr_articulated_hands_ext = false;
bool xr_spatial_bridge_ext = false;

XrInstance     xr_instance      = {};
XrSession      xr_session       = {};
XrExtTable     xr_extensions    = {};
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
bool           xr_running       = false;
XrSpace        xr_app_space     = {};
XrSpace        xr_head_space    = {};
XrSystemId     xr_system_id     = XR_NULL_SYSTEM_ID;
XrTime         xr_time          = 0;

XrEnvironmentBlendMode xr_blend;
XrReferenceSpaceType   xr_refspace;
xr_hand_state_         xr_hand_state = xr_hand_state_uncertain;

///////////////////////////////////////////

XrReferenceSpaceType openxr_preferred_space();
void                 openxr_preferred_extensions(uint32_t &out_extension_count, const char **out_extensions);
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

int64_t openxr_get_time() {
	LARGE_INTEGER time;
	xr_extensions.xrConvertTimeToWin32PerformanceCounterKHR(xr_instance, xr_time, &time);
	return time.QuadPart;
}

///////////////////////////////////////////

bool openxr_init(const char *app_name) {
	uint32_t extension_count = 0;
	openxr_preferred_extensions(extension_count, nullptr);
	const char **extensions = (const char**)malloc(sizeof(char *) * extension_count);
	openxr_preferred_extensions(extension_count, extensions);

	uint32_t layer_count = 0;
	openxr_preferred_layers(layer_count, nullptr);
	const char **layers = (const char**)malloc(sizeof(char *) * layer_count);
	openxr_preferred_layers(layer_count, layers);

	XrInstanceCreateInfo createInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = extension_count;
	createInfo.enabledExtensionNames = extensions;
	createInfo.enabledApiLayerCount = layer_count;
	createInfo.enabledApiLayerNames = layers;
	createInfo.applicationInfo.applicationVersion = 1;

	// Use a version Id formatted as 0xMMMiiPPP
	createInfo.applicationInfo.engineVersion = 
		(SK_VERSION_MAJOR << 20)              |
		(SK_VERSION_MINOR << 12 & 0x000FF000) |
		(SK_VERSION_PATCH       & 0x00000FFF);

	createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	strcpy_s(createInfo.applicationInfo.applicationName, app_name);
	strcpy_s(createInfo.applicationInfo.engineName, "StereoKit");
	XrResult result = xrCreateInstance(&createInfo, &xr_instance);

	free(extensions);
	free(layers);

	// Check if OpenXR is on this system, if this is null here, the user needs to install an
	// OpenXR runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		log_fail_reasonf(90, "Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", openxr_string(result));
		return false;
	}

	// Create links to the extension functions
	xr_extensions = xrCreateExtensionTable(xr_instance);

	// Request a form factor from the device (HMD, Handheld, etc.)
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = xr_config_form;
	result = xrGetSystem(xr_instance, &systemInfo, &xr_system_id);
	if (XR_FAILED(result)) {
		log_fail_reasonf(90, "Couldn't find our desired MR form factor, no MR device attached/ready? [%s]", openxr_string(result));
		return false;
	}

	// Figure out what this device is capable of!
	XrSystemProperties                properties          = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesEXT tracking_properties = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	properties.next = &tracking_properties;
	xr_check(xrGetSystemProperties(xr_instance, xr_system_id, &properties),
		"xrGetSystemProperties failed [%s]");
	log_diagf("Using system: %s", properties.systemName);
	xr_articulated_hands = xr_articulated_hands_ext && tracking_properties.supportsHandTracking;
	xr_depth_lsr         = xr_depth_lsr_ext;

	if (xr_articulated_hands)   log_diag("OpenXR articulated hands ext enabled!");
	if (xr_depth_lsr)           log_diag("OpenXR depth LSR ext enabled!");
	if (sk_info.spatial_bridge) log_diag("OpenXR spatial bridge ext enabled!");

	// OpenXR wants to ensure apps are using the correct LUID, so this MUST be called before xrCreateSession
	XrGraphicsRequirementsD3D11KHR requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	xr_check(xr_extensions.xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement),
		"xrGetD3D11GraphicsRequirementsKHR failed [%s]");
	if (!d3d_init(&requirement.adapterLuid))
		return false;

	// A session represents this application's desire to display things! This is where we hook up our graphics API.
	// This does not start the session, for that, you'll need a call to xrBeginSession, which we do in openxr_poll_events
	XrGraphicsBindingD3D11KHR d3d_binding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	d3d_binding.device = d3d_device;
	XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
	sessionInfo.next     = &d3d_binding;
	sessionInfo.systemId = xr_system_id;
	xrCreateSession(xr_instance, &sessionInfo, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (XR_FAILED(result) || xr_session == XR_NULL_HANDLE) {
		log_fail_reasonf(90, "Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
		return false;
	}

	// Create reference spaces! So we can find stuff relative to them :)
	xr_refspace = openxr_preferred_space();

	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = xr_refspace;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_app_space);
	if (XR_FAILED(result)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(result));
		return false;
	}

	ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_VIEW;
	result = xrCreateReferenceSpace(xr_session, &ref_space, &xr_head_space);
	if (XR_FAILED(result)) {
		log_infof("xrCreateReferenceSpace failed [%s]", openxr_string(result));
		return false;
	}

	if (!openxr_views_create()) return false;
	if (!oxri_init()) return false;
	return true;
}

///////////////////////////////////////////

void openxr_preferred_extensions(uint32_t &out_extension_count, const char **out_extensions) {
	// Find what extensions are available on this system!
	uint32_t ext_count = 0;
	xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
	XrExtensionProperties *exts = (XrExtensionProperties *)malloc(sizeof(XrExtensionProperties) * ext_count);
	for (uint32_t i = 0; i < ext_count; i++) exts[i] = { XR_TYPE_EXTENSION_PROPERTIES };
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, exts);

	// Count how many there are, and copy them out
	out_extension_count = 0;
	for (int32_t e = 0; e < _countof(xr_request_extensions); e++) {
		for (uint32_t i = 0; i < ext_count; i++) {
			if (strcmp(exts[i].extensionName, xr_request_extensions[e]) == 0) {
				if (out_extensions != nullptr)
					out_extensions[out_extension_count] = xr_request_extensions[e];
				out_extension_count += 1;
				break;
			}
		}
	}

	// Flag any extensions the app will need to know about
	if (out_extensions != nullptr) {
		for (uint32_t i = 0; i < out_extension_count; i++) {
			if      (strcmp(out_extensions[i], XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME) == 0) xr_depth_lsr_ext         = true;
			else if (strcmp(out_extensions[i], XR_EXT_HAND_TRACKING_EXTENSION_NAME          ) == 0) xr_articulated_hands_ext = true;
			else if (strcmp(out_extensions[i], XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME  ) == 0) sk_info.spatial_bridge   = true;
		}
	}

	free(exts);
}

///////////////////////////////////////////

void openxr_preferred_layers(uint32_t &out_layer_count, const char **out_layers) {
	// Find what extensions are available on this system!
	uint32_t layer_count = 0;
	xrEnumerateApiLayerProperties(0, &layer_count, nullptr);
	XrApiLayerProperties *layers = (XrApiLayerProperties *)malloc(sizeof(XrApiLayerProperties) * layer_count);
	for (uint32_t i = 0; i < layer_count; i++) layers[i] = { XR_TYPE_API_LAYER_PROPERTIES };
	xrEnumerateApiLayerProperties(layer_count, &layer_count, layers);

	if (out_layers == nullptr) {
		for (uint32_t i = 0; i < layer_count; i++) {
			log_diagf("oxr layer found: %s", layers[i].layerName);
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

	// OpenXR uses a couple different types of reference frames for positioning content, we need to choose one for
	// displaying our content! STAGE would be relative to the center of your guardian system's bounds, and LOCAL
	// would be relative to your device's starting location.

	XrReferenceSpaceType refspace_priority[] = {
		XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT,
		XR_REFERENCE_SPACE_TYPE_LOCAL,
		XR_REFERENCE_SPACE_TYPE_STAGE, };

	// Find the spaces OpenXR has access to on this device
	uint32_t refspace_count = 0;
	xrEnumerateReferenceSpaces(xr_session, 0, &refspace_count, nullptr);
	XrReferenceSpaceType *refspace_types = (XrReferenceSpaceType *)malloc(sizeof(XrReferenceSpaceType) * refspace_count);
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
	// Shut down the input!
	oxri_shutdown();

	openxr_views_destroy();

	// Release all the other OpenXR resources that we've created!
	// What gets allocated, must get deallocated!
	if (xr_head_space != XR_NULL_HANDLE) xrDestroySpace   (xr_head_space);
	if (xr_app_space  != XR_NULL_HANDLE) xrDestroySpace   (xr_app_space);
	if (xr_session    != XR_NULL_HANDLE) xrDestroySession (xr_session);
	if (xr_instance   != XR_NULL_HANDLE) xrDestroyInstance(xr_instance);

	d3d_shutdown();
}

///////////////////////////////////////////

void openxr_step_begin() {
	d3d_update();
	openxr_poll_events();
	if (xr_running)
		openxr_poll_actions();
}

///////////////////////////////////////////

void openxr_step_end() {
	if (xr_running)
		openxr_render_frame();
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
					secondary.viewConfigurationCount        = xr_display_types.count-1;
					secondary.enabledViewConfigurationTypes = &xr_display_types[1];
					begin_info.next = &secondary;
				}

				xrBeginSession(xr_session, &begin_info);
				xr_running = true;
				log_diag("OpenXR session begin.");
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
		}
		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
}

///////////////////////////////////////////

void openxr_poll_actions() {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED || xr_time == 0)
		return;

	// Track the head location
	pose_t head_pose;
	openxr_get_space(xr_head_space, head_pose);
	matrix root = render_get_cam_root();
	input_head_pose.position    = matrix_mul_point   (root, head_pose.position);
	input_head_pose.orientation = matrix_mul_rotation(root, head_pose.orientation);
}

///////////////////////////////////////////

bool32_t openxr_get_space(XrSpace space, pose_t &out_pose, XrTime time) {
	if (time == 0) time = xr_time;

	XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
	XrResult        res            = xrLocateSpace(space, xr_app_space, time, &space_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
		memcpy(&out_pose.position,    &space_location.pose.position,    sizeof(vec3));
		memcpy(&out_pose.orientation, &space_location.pose.orientation, sizeof(quat));
		return true;
	}
	return false;
}

///////////////////////////////////////////

pose_t pose_from_spatial(uint8_t spatial_graph_node_id[16]) {
	if (!sk_info.spatial_bridge) {
		log_warn("This system doesn't support the spatial bridge! Check StereoKitApp.System.spatialBridge.");
		return { {0,0,0}, {0,0,0,1} };
	}

	XrSpace                               space;
	pose_t                                result     = {};
	XrSpatialGraphNodeSpaceCreateInfoMSFT space_info = { XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT };
	space_info.nodeType = XR_SPATIAL_GRAPH_NODE_TYPE_STATIC_MSFT;
	space_info.pose     = { {0,0,0,1}, {0,0,0} };
	memcpy(space_info.nodeId, spatial_graph_node_id, sizeof(space_info.nodeId));

	xr_extensions.xrCreateSpatialGraphNodeSpaceMSFT(xr_session, &space_info, &space);

	openxr_get_space(space, result);
	return result;
}

} // namespace sk
