
#include "openxr.h"
#include "openxr_input.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../../systems/render.h"
#include "../../systems/input.h"
#include "../../systems/d3d.h"
#include "../../libraries/stb_ds.h"

#include <openxr/openxr.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct swapchain_t {
	XrSwapchain handle;
	int32_t     width;
	int32_t     height;
	int32_t     surface_count;
	XrSwapchainImageD3D11KHR *images;
	tex_t                    *textures;
};
void swapchain_delete(swapchain_t &swapchain) {
	for (size_t s = 0; s < swapchain.surface_count; s++) {
		tex_release(swapchain.textures[s]);
	}
	xrDestroySwapchain(swapchain.handle);
	free(swapchain.images);
	free(swapchain.textures);
}

///////////////////////////////////////////

struct device_display_t {
	XrViewConfigurationType      type;
	XrEnvironmentBlendMode       blend;
	XrCompositionLayerProjection*projection_data;
	bool32_t                     active;
	DXGI_FORMAT color_format;
	DXGI_FORMAT depth_format;
	
	swapchain_t swapchain_color;
	swapchain_t swapchain_depth;

	uint32_t view_count;
	uint32_t view_cap;
	XrView                           *views;
	XrCompositionLayerProjectionView *view_layers;
	XrCompositionLayerDepthInfoKHR   *view_depths;
	matrix                           *view_transforms;
	matrix                           *view_projections;
	XrViewConfigurationView          *view_configs;
};
void device_display_delete(device_display_t &display) {
	swapchain_delete(display.swapchain_color);
	swapchain_delete(display.swapchain_depth);

	free(display.projection_data );
	free(display.views           ); display.views            = nullptr;
	free(display.view_transforms ); display.view_transforms  = nullptr;
	free(display.view_layers     ); display.view_layers      = nullptr;
	free(display.view_depths     ); display.view_depths      = nullptr;
	free(display.view_projections); display.view_projections = nullptr;
	free(display.view_configs    ); display.view_configs     = nullptr;
}

///////////////////////////////////////////

XrViewConfigurationType xr_request_displays[] = {
	XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
};
XrViewConfigurationType xr_display_primary = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

uint32_t                                   xr_display_count      = 0;
device_display_t                          *xr_displays           = nullptr;
XrViewConfigurationType                   *xr_display_types      = nullptr;
XrSecondaryViewConfigurationStateMSFT     *xr_display_2nd_states = nullptr;
XrSecondaryViewConfigurationLayerInfoMSFT *xr_display_2nd_layers = nullptr;

///////////////////////////////////////////

bool openxr_create_view      (XrViewConfigurationType view_type, device_display_t &out_view);
bool openxr_create_swapchain (swapchain_t &out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count);
void openxr_preferred_format (DXGI_FORMAT &out_color, DXGI_FORMAT &out_depth);
bool openxr_preferred_blend  (XrViewConfigurationType view_type, XrEnvironmentBlendMode &out_blend);
bool openxr_update_swapchains(device_display_t &display);
bool openxr_render_layer     (XrTime predictedTime, device_display_t &layer);

///////////////////////////////////////////

const char *openxr_view_name(XrViewConfigurationType type) {
	switch (type) {
	case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO:                              return "PrimaryMono";
	case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO:                            return "PrimaryStereo";
	case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO:                        return "PrimaryQuad_Varjo";
	case XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT: return "SecondaryMonoFPO_Msft";
	default: return "N/A";
	}
}

///////////////////////////////////////////

const char *openxr_fmt_name(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM:      return "rgba8_linear";
	case DXGI_FORMAT_B8G8R8A8_UNORM:      return "bgra8_linear";
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "rgba8_srgb";
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "bgra8_srgb";
	case DXGI_FORMAT_D16_UNORM:           return "d16";
	case DXGI_FORMAT_D32_FLOAT:           return "d32"; 
	case DXGI_FORMAT_D24_UNORM_S8_UINT:   return "d24_s8";
	default: return "N/A";
	}
}

///////////////////////////////////////////

bool openxr_views_create() {
	// Find all the valid view configurations
	uint32_t count = 0;
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, 0, &count, nullptr),
		"xrEnumerateViewConfigurations failed [%s]");
	XrViewConfigurationType *types = (XrViewConfigurationType *)malloc(sizeof(XrViewConfigurationType) * count);
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, count, &count, types),
		"xrEnumerateViewConfigurations failed [%s]");

	// Initialize each valid view configuration
	for (uint32_t t = 0; t < count; t++) {
		for (uint32_t r = 0; r < _countof(xr_request_displays); r++) {
			if (types[t] == xr_request_displays[r]) {
				if (types[t] != xr_display_primary) {
					XrSecondaryViewConfigurationStateMSFT state = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT };
					state.active                = false;
					state.viewConfigurationType = types[t];
					arrput(xr_display_2nd_states, state);
				}
				arrput(xr_display_types, types[t]);
				arrput(xr_displays,      device_display_t{});
				openxr_create_view(types[t], arrlast(xr_displays));
			}
		}
	}
	free(types);
	xr_display_count = arrlen(xr_displays);

	if (xr_display_count == 0) {
		log_info("No valid display configurations were found!");
		return false;
	}

	// Register dispay type with the system
	switch (xr_displays[0].blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      sk_info.display_type = display_opaque;      break;
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    sk_info.display_type = display_additive;    break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: sk_info.display_type = display_passthrough; break;
	}

	return true;
}

///////////////////////////////////////////

void openxr_views_destroy() {
	for (size_t i = 0; i < arrlen(xr_displays); i++) {
		device_display_delete(xr_displays[i]);
	}
	arrfree(xr_displays);
	arrfree(xr_display_types);
	arrfree(xr_display_2nd_states);
	arrfree(xr_display_2nd_layers);
}

///////////////////////////////////////////

bool openxr_create_view(XrViewConfigurationType view_type, device_display_t &out_view) {
	out_view = {};

	// Get the surface format information before we create surfaces!
	openxr_preferred_format(out_view.color_format, out_view.depth_format);
	if (!openxr_preferred_blend (view_type, out_view.blend)) return false;

	// Debug print the view and format info
	log_diagf("Creating view: %s color:%s depth:%s", 
		openxr_view_name(view_type), 
		openxr_fmt_name (out_view.color_format),  
		openxr_fmt_name (out_view.depth_format));

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	xr_check(xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, view_type, 0, &out_view.view_cap, nullptr),
		"openxr_views_create can't find any valid view configurations");

	out_view.view_configs = (XrViewConfigurationView *)malloc(sizeof(XrViewConfigurationView) * out_view.view_cap);
	for (uint32_t i = 0; i < out_view.view_cap; i++) out_view.view_configs[i] = { XR_TYPE_VIEW_CONFIGURATION_VIEW };

	// Extract information from the views we got
	out_view.type             = view_type;
	out_view.active           = true;
	out_view.projection_data  = (XrCompositionLayerProjection*)malloc(sizeof(XrCompositionLayerProjection));
	out_view.view_count       = 0;
	out_view.views            = (XrView*)malloc(sizeof(XrView) * out_view.view_cap);
	out_view.view_layers      = (XrCompositionLayerProjectionView*)malloc(sizeof(XrCompositionLayerProjectionView) * out_view.view_cap);
	out_view.view_depths      = (XrCompositionLayerDepthInfoKHR  *)malloc(sizeof(XrCompositionLayerDepthInfoKHR)   * out_view.view_cap);
	out_view.view_transforms  = (matrix*)malloc(sizeof(matrix) * out_view.view_cap);
	out_view.view_projections = (matrix*)malloc(sizeof(matrix) * out_view.view_cap);
	for (int32_t i = 0; i < out_view.view_cap; i++) {
		out_view.views      [i] = { XR_TYPE_VIEW };
		out_view.view_layers[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		out_view.view_depths[i] = { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
	}

	if (!openxr_update_swapchains(out_view)) {
		log_warnf("Couldn't create OpenXR view swapchains!");
		return false;
	}
	
	return true;
}

///////////////////////////////////////////

bool openxr_update_swapchains(device_display_t &display) {
	// Check if the latest configuration is different from what we've already
	// set up.
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, display.type, display.view_cap, &display.view_cap, display.view_configs);
	int w = display.view_configs[0].recommendedImageRectWidth;
	int h = display.view_configs[0].recommendedImageRectHeight;
	if (   w == display.swapchain_color.width
		&& h == display.swapchain_color.height) {
		return true;
	}
	log_diagf("Setting view: %s to %dx%d", openxr_view_name(display.type), w, h);

	// Create the new swapchaines for the current size
	int samples = display.view_configs[0].recommendedSwapchainSampleCount;
	if (!openxr_create_swapchain(display.swapchain_color, display.type, true,  display.view_cap, display.color_format, w, h, samples)) return false;
	if (!openxr_create_swapchain(display.swapchain_depth, display.type, false, display.view_cap, display.depth_format, w, h, samples)) return false;

	for (uint32_t s = 0; s < display.swapchain_color.surface_count; s++) {
		// If we don't have textures for the swapchain, create them
		if (display.swapchain_color.textures[s] == nullptr) {
			display.swapchain_color.textures[s] = tex_create(tex_type_rendertarget, tex_format_rgba32);
			display.swapchain_depth.textures[s] = tex_create(tex_type_depth,        tex_get_tex_format(display.depth_format));

			char           name[64];
			static int32_t target_index = 0;
			target_index++;
			sprintf_s(name, "renderer/colortarget_%d", target_index);
			tex_set_id(display.swapchain_color.textures[s], name);
			sprintf_s(name, "renderer/depthtarget_%d", target_index);
			tex_set_id(display.swapchain_depth.textures[s], name);
		}

		// Update our textures with the new swapchain display surfaces
		tex_setsurface (display.swapchain_color.textures[s], display.swapchain_color.images[s].texture, display.color_format);
		tex_setsurface (display.swapchain_depth.textures[s], display.swapchain_depth.images[s].texture, display.depth_format);
		tex_set_zbuffer(display.swapchain_color.textures[s], display.swapchain_depth.textures[s]);
	}

	return true;
}

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t &out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count) {
	// Create a swapchain for this viewpoint! A swapchain is a set of texture buffers used for displaying to screen,
	// typically this is a backbuffer and a front buffer, one for rendering data to, and one for displaying on-screen.
	XrSwapchainCreateInfo swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
	XrSwapchain           handle         = {};
	swapchain_info.arraySize   = array_size;
	swapchain_info.mipCount    = 1;
	swapchain_info.faceCount   = 1;
	swapchain_info.format      = format;
	swapchain_info.width       = width;
	swapchain_info.height      = height;
	swapchain_info.sampleCount = sample_count;
	swapchain_info.usageFlags  = color 
		? XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT 
		: XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	// If it's a secondary view, let OpenXR know
	XrSwapchainSecondaryViewConfigurationCreateInfoMSFT secondary = { XR_TYPE_SWAPCHAIN_SECONDARY_VIEW_CONFIGURATION_CREATE_INFO_MSFT };
	if (type != xr_display_primary) {
		secondary.viewConfigurationType = type;
		swapchain_info.next = &secondary;
	}

	xr_check(xrCreateSwapchain(xr_session, &swapchain_info, &handle), 
		"xrCreateSwapchain failed [%s]");

	// Find out how many textures were generated for the swapchain
	uint32_t surface_count = 0;
	xr_check(xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr), 
		"xrEnumerateSwapchainImages failed [%s]");

	// We'll want to track our own information about the swapchain, so we can draw stuff onto it! We'll also create
	// a depth buffer for each generated texture here as well with make_surfacedata.
	out_swapchain.width         = swapchain_info.width;
	out_swapchain.height        = swapchain_info.height;
	out_swapchain.handle        = handle;
	if (out_swapchain.surface_count != surface_count) {
		out_swapchain.surface_count = surface_count;
		out_swapchain.images        = (XrSwapchainImageD3D11KHR*)malloc(sizeof(XrSwapchainImageD3D11KHR) * surface_count);
		out_swapchain.textures      = (tex_t                   *)malloc(sizeof(tex_t                   ) * surface_count);
	}
	for (uint32_t i=0; i<surface_count; i++) {
		out_swapchain.images  [i] = { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR };
		out_swapchain.textures[i] = nullptr;
	}
	
	xr_check(xrEnumerateSwapchainImages(out_swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader *)out_swapchain.images),
		"xrEnumerateSwapchainImages failed [%s]");

	return true;
}

///////////////////////////////////////////

void openxr_preferred_format(DXGI_FORMAT &out_color_dx, DXGI_FORMAT &out_depth_dx) {
	DXGI_FORMAT pixel_formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,};
	DXGI_FORMAT depth_formats[] = {
		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,};

	// Get the list of formats OpenXR would like
	uint32_t count = 0;
	xrEnumerateSwapchainFormats(xr_session, 0, &count, nullptr);
	int64_t *formats = (int64_t *)malloc(sizeof(int64_t) * count);
	xrEnumerateSwapchainFormats(xr_session, count, &count, formats);

	// Check those against our formats
	out_color_dx = DXGI_FORMAT_UNKNOWN;
	out_depth_dx = DXGI_FORMAT_UNKNOWN;
	for (uint32_t i=0; i<count; i++) {
		for (int32_t f=0; out_color_dx == DXGI_FORMAT_UNKNOWN && f<_countof(pixel_formats); f++) {
			if (formats[i] == pixel_formats[f]) {
				out_color_dx = pixel_formats[f];
				break;
			}
		}
		for (int32_t f=0; out_depth_dx == DXGI_FORMAT_UNKNOWN && f<_countof(depth_formats); f++) {
			if (formats[i] == depth_formats[f]) {
				out_depth_dx = depth_formats[f];
				break;
			}
		}
	}

	// Release memory
	free(formats);
}

///////////////////////////////////////////

bool openxr_preferred_blend(XrViewConfigurationType view_type, XrEnvironmentBlendMode &out_blend) {
	// Check what blend mode is valid for this device (opaque vs transparent displays)
	// We'll just take the first one available!
	uint32_t                blend_count = 0;
	XrEnvironmentBlendMode *blend_modes;
	xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, view_type, 0, &blend_count, nullptr);
	blend_modes = (XrEnvironmentBlendMode*)malloc(sizeof(XrEnvironmentBlendMode) * blend_count);
	xr_check(xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, view_type, blend_count, &blend_count, blend_modes),
		"xrEnumerateEnvironmentBlendModes failed [%s]");

	for (size_t i = 0; i < blend_count; i++) {
		if (blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_OPAQUE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND) {
			out_blend = blend_modes[i];
			return true;
		}
	}

	log_info("openxr_preferred_blend couldn't find a valid blend mode!");
	return false;
}


///////////////////////////////////////////

bool openxr_render_frame() {
	// Block until the previous frame is finished displaying, and is ready for another one.
	// Also returns a prediction of when the next frame will be displayed, for use with predicting
	// locations of controllers, viewpoints, etc.
	XrFrameWaitInfo                             wait_info        = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState                                frame_state      = { XR_TYPE_FRAME_STATE };
	XrFrameSecondaryViewConfigurationsStateMSFT secondary_states = { XR_TYPE_FRAME_SECONDARY_VIEW_CONFIGURATIONS_STATE_MSFT };
	if (xr_display_count > 1) {
		secondary_states.viewConfigurationCount = xr_display_count - 1;
		secondary_states.states                 = xr_display_2nd_states;
		frame_state.next = &secondary_states;
	}
	xr_check(xrWaitFrame(xr_session, &wait_info, &frame_state),
		"xrWaitFrame [%s]");

	// Check active for the primary display
	bool session_active = 
		xr_session_state == XR_SESSION_STATE_VISIBLE || 
		xr_session_state == XR_SESSION_STATE_FOCUSED;
	xr_displays[0].active = session_active;

	// Check each secondary display to see if it's active or not
	for (uint32_t i = 1; i < xr_display_count; i++) {
		if (xr_displays[i].active != xr_display_2nd_states[i - 1].active) {
			xr_displays[i].active  = xr_display_2nd_states[i - 1].active;

			if (xr_displays[i].active) {
				openxr_update_swapchains(xr_displays[i]);
			}
		}
	}

	// Must be called before any rendering is done! This can return some interesting flags, like 
	// XR_SESSION_VISIBILITY_UNAVAILABLE, which means we could skip rendering this frame and call
	// xrEndFrame right away.
	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	xr_check(xrBeginFrame(xr_session, &begin_info),
		"xrBeginFrame [%s]");

	// Timing also needs some work, may be best as some sort of anchor system
	xr_time = frame_state.predictedDisplayTime;

	// Execute any code that's dependant on the predicted time, such as updating the location of
	// controller models.
	input_update_predicted();

	// Render all the views for the application, then clear out the render queue
	// If the session is active, lets render our layer in the compositor!
	arrsetlen(xr_display_2nd_layers, 0);
	
	for (size_t i = 0; i < xr_display_count; i++) {
		if (!xr_displays[i].active) continue;

		if (xr_displays[i].type != xr_display_primary) {
			XrSecondaryViewConfigurationLayerInfoMSFT layer = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT };
			layer.viewConfigurationType = xr_displays[i].type;
			layer.environmentBlendMode  = xr_displays[i].blend;
			layer.layerCount            = 1;
			layer.layers                = (XrCompositionLayerBaseHeader**)&xr_displays[i].projection_data;
			arrput(xr_display_2nd_layers, layer);
		}
		openxr_render_layer(xr_time, xr_displays[i]);
	}

	render_clear();

	XrFrameEndSecondaryViewConfigurationInfoMSFT end_second = { XR_TYPE_FRAME_END_SECONDARY_VIEW_CONFIGURATION_INFO_MSFT };
	end_second.viewConfigurationLayersInfo = xr_display_2nd_layers;
	end_second.viewConfigurationCount      = arrlen(xr_display_2nd_layers);
	
	// We're finished with rendering our layer, so send it off for display!
	XrCompositionLayerBaseHeader *layer    = (XrCompositionLayerBaseHeader*)&xr_displays[0].projection_data[0];
	XrFrameEndInfo                end_info = { XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_displays[0].blend;
	end_info.layerCount           = xr_displays[0].active ? 1 : 0;
	end_info.layers               = &layer;
	if (end_second.viewConfigurationCount > 0)
		end_info.next = &end_second;
	xr_check(xrEndFrame(xr_session, &end_info),
		"xrEndFrame [%s]");
	return true;
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

bool openxr_render_layer(XrTime predictedTime, device_display_t &layer) {

	// Find the state and location of each viewpoint at the predicted time
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = layer.type;
	locate_info.displayTime           = predictedTime;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, layer.view_cap, &layer.view_count, layer.views);

	// We need to ask which swapchain image to use for rendering! Which one will we get?
	// Who knows! It's up to the runtime to decide.
	uint32_t                    color_id, depth_id;
	XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(layer.swapchain_color.handle, &acquire_info, &color_id);
	xrAcquireSwapchainImage(layer.swapchain_depth.handle, &acquire_info, &depth_id);

	// Wait until the image is available to render to. The compositor could still be
	// reading from it.
	XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wait_info.timeout = XR_INFINITE_DURATION;
	xrWaitSwapchainImage(layer.swapchain_color.handle, &wait_info);
	xrWaitSwapchainImage(layer.swapchain_depth.handle, &wait_info);

	// And now we'll iterate through each viewpoint, and render it!
	for (uint32_t i = 0; i < layer.view_count; i++) {
		// Set up our rendering information for the viewpoint we're using right now!
		XrCompositionLayerProjectionView &view = layer.view_layers[i];
		view = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		view.pose = layer.views[i].pose;
		view.fov  = layer.views[i].fov;
		view.subImage.imageArrayIndex  = i;
		view.subImage.swapchain        = layer.swapchain_color.handle;
		view.subImage.imageRect.offset = { 0, 0 };
		view.subImage.imageRect.extent = { layer.swapchain_color.width, layer.swapchain_color.height };

		if (xr_depth_lsr) {
			XrCompositionLayerDepthInfoKHR &depth = layer.view_depths[i];
			depth.minDepth = 0;
			depth.maxDepth = 1;
			depth.nearZ    = 0.1f;
			depth.farZ     = 50;
			depth.subImage.imageArrayIndex  = i;
			depth.subImage.swapchain        = layer.swapchain_depth.handle;
			depth.subImage.imageRect.offset = { 0, 0 };
			depth.subImage.imageRect.extent = { layer.swapchain_depth.width, layer.swapchain_depth.height };
			view.next = &depth;
		}

		float xr_projection[16];
		openxr_projection(view.fov, 0.1f, 50, xr_projection);
		memcpy(&layer.view_projections[i], xr_projection, sizeof(float) * 16);
		matrix_inverse(matrix_trs((vec3&)view.pose.position, (quat&)view.pose.orientation, vec3_one), layer.view_transforms[i]);
	}

	// Call the rendering callback with our view and swapchain info
	tex_t target = layer.swapchain_color.textures[color_id];
	tex_rtarget_clear(target, sk_info.display_type == display_opaque 
		? color32{ 0,0,0,255 } 
		: color32{ 0,0,0,0   });
	tex_rtarget_set_active(target);
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)layer.swapchain_color.width, (float)layer.swapchain_color.height);
	d3d_context->RSSetViewports(1, &viewport);

	render_draw_matrix(layer.view_transforms, layer.view_projections, layer.view_count);

	// And tell OpenXR we're done with rendering to this one!
	XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(layer.swapchain_color.handle, &release_info);
	xrReleaseSwapchainImage(layer.swapchain_depth.handle, &release_info);

	layer.projection_data[0] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	layer.projection_data[0].space      = xr_app_space;
	layer.projection_data[0].viewCount  = layer.view_count;
	layer.projection_data[0].views      = layer.view_count == 0 ? nullptr : layer.view_layers;
	layer.projection_data[0].layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	return true;
}

} // namespace sk