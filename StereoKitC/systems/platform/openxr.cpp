#pragma comment(lib,"openxr_loader.lib")

#include "openxr.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../systems/d3d.h"
#include "../../systems/render.h"
#include "../../systems/input.h"
#include "../../systems/hand/input_hand.h"
#include "../../systems/hand/hand_leap.h"
#include "../../asset_types/texture.h"

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;

namespace sk {

///////////////////////////////////////////

struct swapchain_t {
	XrSwapchain handle;
	int32_t     width;
	int32_t     height;
	vector<XrSwapchainImageD3D11KHR> surface_images;
	vector<tex_t>                    surface_data;
};

///////////////////////////////////////////

XrFormFactor            app_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
XrViewConfigurationType app_config_view = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

XrInstance     xr_instance      = {};
XrSession      xr_session       = {};
XrExtTable     xr_extensions    = {};
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
bool           xr_running       = false;
XrSpace        xr_app_space     = {};
XrSpace        xr_head_space    = {};
XrSystemId     xr_system_id     = XR_NULL_SYSTEM_ID;
XrTime         xr_time;
bool           xr_depth_lsr = false;
bool           xr_depth_lsr_ext = false;
bool           xr_articulated_hands = false;
bool           xr_articulated_hands_ext = false;

XrEnvironmentBlendMode xr_blend;
XrReferenceSpaceType   xr_refspace;

vector<matrix>                  xr_viewpt_view;
vector<matrix>                  xr_viewpt_proj;
vector<XrView>                  xr_views;
vector<XrViewConfigurationView> xr_config_views;
swapchain_t                     xr_swapchains;
xr_hand_state_                  xr_hand_state = xr_hand_state_uncertain;

///////////////////////////////////////////

bool openxr_render_layer(XrTime predictedTime, vector<XrCompositionLayerProjectionView> &projectionViews, XrCompositionLayerProjection &layer);

void                 openxr_preferred_format(DXGI_FORMAT &out_pixel_format, tex_format_ &out_depth_format);
XrReferenceSpaceType openxr_preferred_space();
void                 openxr_preferred_extensions(uint32_t &out_extension_count, const char **out_extensions);

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

	XrInstanceCreateInfo createInfo   = { XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = extension_count;
	createInfo.enabledExtensionNames = extensions;
	createInfo.applicationInfo.applicationVersion = 1;

	// Use a version Id formatted as 0xMMMiiPPP
	createInfo.applicationInfo.engineVersion = 
		(SK_VERSION_MAJOR << 20)              |
		(SK_VERSION_MINOR << 12 & 0x000FF000) |
		(SK_VERSION_PATCH & 0x00000FFF);

	createInfo.applicationInfo.apiVersion         = XR_CURRENT_API_VERSION;
	strcpy_s(createInfo.applicationInfo.applicationName, app_name);
	strcpy_s(createInfo.applicationInfo.engineName, "StereoKit");
	XrResult result = xrCreateInstance(&createInfo, &xr_instance);

	free(extensions);

	// Check if OpenXR is on this system, if this is null here, the user needs to install an
	// OpenXR runtime and ensure it's active!
	if (XR_FAILED(result) || xr_instance == XR_NULL_HANDLE) {
		log_infof("Couldn't create OpenXR instance [%s], is OpenXR installed and set as the active runtime?", openxr_string(result));
		return false;
	}

	// Create links to the extension functions
	xr_extensions = xrCreateExtensionTable(xr_instance);

	// Request a form factor from the device (HMD, Handheld, etc.)
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = app_config_form;
	result = xrGetSystem(xr_instance, &systemInfo, &xr_system_id);
	if (XR_FAILED(result)) {
		log_infof("xrGetSystem failed [%s]", openxr_string(result));
		return false;
	}

	// Figure out what this device is capable of!
	XrSystemProperties properties = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemHandTrackingPropertiesMSFT tracking_properties = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_MSFT };
	properties.next = &tracking_properties;
	result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_infof("xrGetSystemProperties failed [%s]", openxr_string(result));
		return false;
	}
	log_diagf("Using system: %s", properties.systemName);
	xr_articulated_hands = xr_articulated_hands_ext && tracking_properties.supportsHandTracking;
	xr_depth_lsr         = xr_depth_lsr_ext; // TODO: Find out how to verify this one

	// OpenXR wants to ensure apps are using the correct LUID, so this MUST be called before xrCreateSession
	XrGraphicsRequirementsD3D11KHR requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	result = xr_extensions.xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement);
	if (XR_FAILED(result)) {
		log_infof("xrGetD3D11GraphicsRequirementsKHR failed [%s]", openxr_string(result));
		return false;
	}
	if (!d3d_init(&requirement.adapterLuid))
		return false;

	// Check what blend mode is valid for this device (opaque vs transparent displays)
	// We'll just take the first one available!
	uint32_t                       blend_count = 0;
	vector<XrEnvironmentBlendMode> blend_modes;
	result = xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, 0, &blend_count, nullptr);
	blend_modes.resize(blend_count);
	result = xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, blend_count, &blend_count, blend_modes.data());
	if (XR_FAILED(result)) {
		log_infof("xrEnumerateEnvironmentBlendModes failed [%s]", openxr_string(result));
		return false;
	}

	for (size_t i = 0; i < blend_count; i++) {
		if (blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_OPAQUE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND) {
			xr_blend = blend_modes[i];
			break;
		}
	}
	
	// register dispay type with the system
	switch (xr_blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE: {
		sk_info.display_type = display_opaque;
	}break;
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE: {
		sk_info.display_type = display_additive;
	}break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: {
		sk_info.display_type = display_passthrough;
	}break;
	}

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
		log_infof("Couldn't create an OpenXR session, no MR device attached/ready? [%s]", openxr_string(result));
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
	
	// Get the surface format information before we create surfaces!
	DXGI_FORMAT color_format;
	tex_format_ depth_format;
	openxr_preferred_format(color_format, depth_format);

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	uint32_t view_count = 0;
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, 0, &view_count, nullptr);
	xr_config_views.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	xr_views       .resize(view_count, { XR_TYPE_VIEW });
	xr_viewpt_view .resize(view_count, { });
	xr_viewpt_proj .resize(view_count, { });
	result = xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, view_count, &view_count, xr_config_views.data());
	if (XR_FAILED(result)) {
		log_infof("xrEnumerateViewConfigurationViews failed [%s]", openxr_string(result));
		return false;
	}

	// Create a swapchain for this viewpoint! A swapchain is a set of texture buffers used for displaying to screen,
	// typically this is a backbuffer and a front buffer, one for rendering data to, and one for displaying on-screen.
	XrViewConfigurationView &view           = xr_config_views[0];
	XrSwapchainCreateInfo    swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
	XrSwapchain              handle         = {};
	swapchain_info.arraySize   = view_count;
	swapchain_info.mipCount    = 1;
	swapchain_info.faceCount   = 1;
	swapchain_info.format      = color_format;
	swapchain_info.width       = view.recommendedImageRectWidth;
	swapchain_info.height      = view.recommendedImageRectHeight;
	swapchain_info.sampleCount = view.recommendedSwapchainSampleCount;
	swapchain_info.usageFlags  = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
	result = xrCreateSwapchain(xr_session, &swapchain_info, &handle);
	if (XR_FAILED(result)) {
		log_infof("xrCreateSwapchain failed [%s]", openxr_string(result));
		return false;
	}

	// Find out how many textures were generated for the swapchain
	uint32_t surface_count = 0;
	xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr);

	// We'll want to track our own information about the swapchain, so we can draw stuff onto it! We'll also create
	// a depth buffer for each generated texture here as well with make_surfacedata.
	xr_swapchains = {};
	xr_swapchains.width  = swapchain_info.width;
	xr_swapchains.height = swapchain_info.height;
	xr_swapchains.handle = handle;
	xr_swapchains.surface_images.resize(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR } );
	xr_swapchains.surface_data  .resize(surface_count, {});
	result = xrEnumerateSwapchainImages(xr_swapchains.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader*)xr_swapchains.surface_images.data());
	if (XR_FAILED(result)) {
		log_infof("xrEnumerateSwapchainImages failed [%s]", openxr_string(result));
		return false;
	}

	for (uint32_t s = 0; s < surface_count; s++) {
		char name[64];
		sprintf_s(name, 64, "stereokit/system/rendertarget_%d", s);
		xr_swapchains.surface_data[s] = tex_create(tex_type_rendertarget, tex_format_rgba32);
		tex_set_id     (xr_swapchains.surface_data[s], name);
		tex_setsurface (xr_swapchains.surface_data[s], xr_swapchains.surface_images[s].texture, color_format);
		tex_add_zbuffer(xr_swapchains.surface_data[s], depth_format);
	}
	
	openxr_make_actions();

	return true;
}

///////////////////////////////////////////

void openxr_preferred_extensions(uint32_t &out_extension_count, const char **out_extensions) {
	const char *extensions[] = {
		XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
		XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME,
		XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME,
		XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME,
		XR_MSFT_HAND_TRACKING_PREVIEW_EXTENSION_NAME,
		XR_MSFT_SPATIAL_GRAPH_BRIDGE_PREVIEW_EXTENSION_NAME, };

	// Find what extensions are available on this system!
	uint32_t ext_count = 0;
	xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
	XrExtensionProperties *exts = (XrExtensionProperties *)malloc(sizeof(XrExtensionProperties) * ext_count);
	for (uint32_t i = 0; i < ext_count; i++) exts[i] = { XR_TYPE_EXTENSION_PROPERTIES };
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, exts);

	// Count how many there are, and copy them out
	out_extension_count = 0;
	for (int32_t e = 0; e < _countof(extensions); e++) {
		for (uint32_t i = 0; i < ext_count; i++) {
			if (strcmp(exts[i].extensionName, extensions[e]) == 0) {
				if (out_extensions != nullptr)
					out_extensions[out_extension_count] = extensions[e];
				out_extension_count += 1;
				break;
			}
		}
	}

	// Flag any extensions the app will need to know about
	if (out_extensions != nullptr) {
		for (uint32_t i = 0; i < out_extension_count; i++) {
			if (strcmp(out_extensions[i], XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME) == 0)
				xr_depth_lsr_ext = true;
			else if (strcmp(out_extensions[i], XR_MSFT_HAND_TRACKING_PREVIEW_EXTENSION_NAME) == 0)
				xr_articulated_hands_ext = true;
		}
	}

	free(exts);
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

void openxr_preferred_format(DXGI_FORMAT &out_pixel_format, tex_format_ &out_depth_format) {
	DXGI_FORMAT pixel_formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,};
	DXGI_FORMAT depth_formats[] = {
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D24_UNORM_S8_UINT,};

	// Get the list of formats OpenXR would like
	uint32_t count = 0;
	xrEnumerateSwapchainFormats(xr_session, 0, &count, nullptr);
	int64_t *formats = (int64_t *)malloc(sizeof(int64_t) * count);
	xrEnumerateSwapchainFormats(xr_session, count, &count, formats);

	// Check those against our color formats
	out_pixel_format = DXGI_FORMAT_UNKNOWN;
	for (uint32_t i = 0; i < count; i++) {
		for (int32_t f = 0; f < _countof(pixel_formats); f++) {
			if (formats[i] == pixel_formats[f]) {
				out_pixel_format = pixel_formats[f];
				break;
			}
		}
		if (out_pixel_format != DXGI_FORMAT_UNKNOWN)
			break;
	}

	// Check those against our depth formats
	DXGI_FORMAT depth_format = DXGI_FORMAT_UNKNOWN;
	for (uint32_t i = 0; i < count; i++) {
		for (int32_t f = 0; f < _countof(depth_formats); f++) {
			if (formats[i] == depth_formats[f]) {
				depth_format = depth_formats[f];
				break;
			}
		}
		if (depth_format != DXGI_FORMAT_UNKNOWN)
			break;
	}

	// Depth maps directly to SK tex types, so we'll return that instead!
	out_depth_format = tex_format_depth16;
	switch (depth_format) {
	case DXGI_FORMAT_D16_UNORM:         out_depth_format = tex_format_depth16; break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: out_depth_format = tex_format_depthstencil; break;
	case DXGI_FORMAT_D32_FLOAT:         out_depth_format = tex_format_depth32; break;
	}

	// Release memory
	free(formats);
}

///////////////////////////////////////////

void openxr_shutdown() {
	// We used a graphics API to initialize the swapchain data, so we'll
	// give it a chance to release anything here!
	for (size_t s = 0; s < xr_swapchains.surface_data.size(); s++) {
		tex_release(xr_swapchains.surface_data[s]);
	}
	xrDestroySwapchain(xr_swapchains.handle);

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
				begin_info.primaryViewConfigurationType = app_config_view;
				xrBeginSession(xr_session, &begin_info);
				xr_running = true;
				log_diag("OpenXR session begin.");
			} break;
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_running = false; break;
			case XR_SESSION_STATE_EXITING:      sk_run = false;              break;
			case XR_SESSION_STATE_LOSS_PENDING: sk_run = false;              break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: sk_run = false; return;
		}
		event_buffer = { XR_TYPE_EVENT_DATA_BUFFER };
	}
}

///////////////////////////////////////////

void openxr_render_frame() {
	// Block until the previous frame is finished displaying, and is ready for another one.
	// Also returns a prediction of when the next frame will be displayed, for use with predicting
	// locations of controllers, viewpoints, etc.
	XrFrameState frame_state = { XR_TYPE_FRAME_STATE };
	xrWaitFrame (xr_session, nullptr, &frame_state);
	// Must be called before any rendering is done! This can return some interesting flags, like 
	// XR_SESSION_VISIBILITY_UNAVAILABLE, which means we could skip rendering this frame and call
	// xrEndFrame right away.
	xrBeginFrame(xr_session, nullptr);

	// Timing also needs some work, may be best as some sort of anchor system
	xr_time = frame_state.predictedDisplayTime + frame_state.predictedDisplayPeriod;

	// Execute any code that's dependant on the predicted time, such as updating the location of
	// controller models.
	input_update_predicted();

	// If the session is active, lets render our layer in the compositor!
	XrCompositionLayerBaseHeader            *layer      = nullptr;
	XrCompositionLayerProjection             layer_proj = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	vector<XrCompositionLayerProjectionView> views;
	bool session_active = xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED;
	if (session_active && openxr_render_layer(frame_state.predictedDisplayTime, views, layer_proj)) {
		layer = (XrCompositionLayerBaseHeader*)&layer_proj;
	}

	// We're finished with rendering our layer, so send it off for display!
	XrFrameEndInfo end_info{ XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_blend;
	end_info.layerCount           = layer == nullptr ? 0 : 1;
	end_info.layers               = &layer;
	xrEndFrame(xr_session, &end_info);

	render_clear();
}

///////////////////////////////////////////

void openxr_projection(XrFovf fov, float clip_near, float clip_far, float *result) {
	// Mix of XMMatrixPerspectiveFovRH from DirectXMath and XrMatrix4x4f_CreateProjectionFov from xr_linear.h
	const float tanLeft        = tanf(fov.angleLeft);
	const float tanRight       = tanf(fov.angleRight);
	const float tanDown        = tanf(fov.angleDown);
	const float tanUp          = tanf(fov.angleUp);
	const float tanAngleWidth  = tanRight - tanLeft;
	const float tanAngleHeight = tanUp - tanDown;
	const float range          = clip_far / (clip_near - clip_far);

	// [row][column]
	memset(result, 0, sizeof(float) * 16);
	result[0]  = 2 / tanAngleWidth;                    // [0][0] Different, DX uses: Width (Height / AspectRatio);
	result[5]  = 2 / tanAngleHeight;                   // [1][1] Same as DX's: Height (CosFov / SinFov)
	result[8]  = (tanRight + tanLeft) / tanAngleWidth; // [2][0] Only present in xr's
	result[9]  = (tanUp + tanDown) / tanAngleHeight;   // [2][1] Only present in xr's
	result[10] = range;                               // [2][2] Same as xr's: -(farZ + offsetZ) / (farZ - nearZ)
	result[11] = -1;                                  // [2][3] Same
	result[14] = range * clip_near;                   // [3][2] Same as xr's: -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);
}

///////////////////////////////////////////

bool openxr_render_layer(XrTime predictedTime, vector<XrCompositionLayerProjectionView> &views, XrCompositionLayerProjection &layer) {

	// Find the state and location of each viewpoint at the predicted time
	uint32_t         view_count  = 0;
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = app_config_view;
	locate_info.displayTime           = predictedTime;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, (uint32_t)xr_views.size(), &view_count, xr_views.data());

	// We need to ask which swapchain image to use for rendering! Which one will we get?
	// Who knows! It's up to the runtime to decide.
	uint32_t                    img_id;
	XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(xr_swapchains.handle, &acquire_info, &img_id);

	// Wait until the image is available to render to. The compositor could still be
	// reading from it.
	XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wait_info.timeout = XR_INFINITE_DURATION;
	xrWaitSwapchainImage(xr_swapchains.handle, &wait_info);

	// And now we'll iterate through each viewpoint, and render it!
	views.resize(view_count);
	for (uint32_t i = 0; i < view_count; i++) {
		// Set up our rendering information for the viewpoint we're using right now!
		views[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		views[i].pose = xr_views[i].pose;
		views[i].fov  = xr_views[i].fov;
		views[i].subImage.imageArrayIndex  = i;
		views[i].subImage.swapchain        = xr_swapchains.handle;
		views[i].subImage.imageRect.offset = { 0, 0 };
		views[i].subImage.imageRect.extent = { xr_swapchains.width, xr_swapchains.height };

		float xr_projection[16];
		openxr_projection(views[i].fov, 0.1f, 50, xr_projection);
		memcpy(&xr_viewpt_proj[i], xr_projection, sizeof(float) * 16);
		matrix_inverse(matrix_trs((vec3&)views[i].pose.position, (quat&)views[i].pose.orientation, vec3_one), xr_viewpt_view[i]);
	}

	// Call the rendering callback with our view and swapchain info
	tex_t target = xr_swapchains.surface_data[img_id];
	tex_rtarget_clear(target, sk_info.display_type == display_opaque ? color32{0,0,0,255} : color32{0, 0, 0, 0});
	tex_rtarget_set_active(target);
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)xr_swapchains.width, (float)xr_swapchains.height);
	d3d_context->RSSetViewports(1, &viewport);

	render_draw_matrix(&xr_viewpt_view[0], &xr_viewpt_proj[0], view_count);

	// And tell OpenXR we're done with rendering to this one!
	XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(xr_swapchains.handle, &release_info);

	layer.space      = xr_app_space;
	layer.viewCount  = (uint32_t)views.size();
	layer.views      = views.data();
	layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	return true;
}

///////////////////////////////////////////

void openxr_make_actions() {
	
}

///////////////////////////////////////////

void openxr_poll_actions() {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED)
		return;

	// Track the head location
	openxr_get_space(xr_head_space, input_head_pose);
}

///////////////////////////////////////////

bool32_t openxr_get_space(XrSpace space, pose_t &out_pose, XrTime time) {
	if (time == 0)
		time = xr_time;

	XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
	XrResult        res            = xrLocateSpace(space, xr_app_space, time, &space_location);
	if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
		memcpy(&out_pose.position,    &space_location.pose.position,    sizeof(vec3));
		memcpy(&out_pose.orientation, &space_location.pose.orientation, sizeof(quat));
		return true;
	}
	return false;
}

} // namespace sk