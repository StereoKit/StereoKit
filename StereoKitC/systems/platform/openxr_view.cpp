
#include "openxr.h"
#include "openxr_input.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../../systems/render.h"
#include "../../systems/input.h"
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
	XrCompositionLayerProjection projection_data;
	bool32_t                     active;
	
	swapchain_t swapchain_color;
	swapchain_t swapchain_depth;

	uint32_t view_count;
	uint32_t view_cap;
	XrView                           *views;
	XrCompositionLayerProjectionView *view_layers;
	matrix                           *view_transforms;
	matrix                           *view_projections;
};
void device_display_delete(device_display_t &display) {
	swapchain_delete(display.swapchain_color);
	swapchain_delete(display.swapchain_depth);

	free(display.views           ); display.views            = nullptr;
	free(display.view_transforms ); display.view_transforms  = nullptr;
	free(display.view_layers     ); display.view_layers      = nullptr;
	free(display.view_projections); display.view_projections = nullptr;
}

///////////////////////////////////////////

XrViewConfigurationType xr_request_displays[] = {
	XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
};
XrViewConfigurationType xr_display_primary = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

uint32_t                      xr_display_count      = 0;
device_display_t             *xr_displays           = nullptr;
XrViewConfigurationType      *xr_display_types      = nullptr;
XrCompositionLayerProjection**xr_composition_layers = nullptr;

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t &out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count);
void openxr_preferred_format(DXGI_FORMAT &out_color, DXGI_FORMAT &out_depth);
bool openxr_preferred_blend (XrViewConfigurationType view_type, XrEnvironmentBlendMode &out_blend);

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
				arrput(xr_display_types, types[t]);
				arrput(xr_displays,  device_display_t{});
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
}

///////////////////////////////////////////

void openxr_views_destroy() {
	for (size_t i = 0; i < arrlen(xr_displays); i++) {
		device_display_delete(xr_displays[i]);
	}
	arrfree(xr_displays);
	arrfree(xr_display_types);
	arrfree(xr_composition_layers);
}

///////////////////////////////////////////

bool openxr_create_view(XrViewConfigurationType view_type, device_display_t &out_view) {
	// Get the surface format information before we create surfaces!
	DXGI_FORMAT color_format, depth_format;
	openxr_preferred_format(color_format, depth_format);

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	uint32_t                 config_ct = 0;
	XrViewConfigurationView *configs   = nullptr;
	xr_check(xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, view_type, 0, &config_ct, nullptr),
		"openxr_views_create can't find any valid view configurations");

	configs = (XrViewConfigurationView *)malloc(sizeof(XrViewConfigurationView) * config_ct);
	for (uint32_t i = 0; i < config_ct; i++) configs[i] = { XR_TYPE_VIEW_CONFIGURATION_VIEW };

	xr_check(xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, view_type, config_ct, &config_ct, configs),
		"xrEnumerateViewConfigurationViews failed [%s]");

	// Extract information from the views we got
	out_view.type             = view_type;
	out_view.view_cap         = config_ct;
	out_view.view_count       = 0;
	out_view.views            = (XrView*)malloc(sizeof(XrView) * config_ct);
	out_view.view_layers      = (XrCompositionLayerProjectionView*)malloc(sizeof(XrCompositionLayerProjectionView) * config_ct);
	out_view.view_transforms  = (matrix*)malloc(sizeof(matrix) * config_ct);
	out_view.view_projections = (matrix*)malloc(sizeof(matrix) * config_ct);
	
	int32_t w = configs[0].recommendedImageRectWidth;
	int32_t h = configs[0].recommendedImageRectHeight;
	int32_t s = configs[0].recommendedSwapchainSampleCount;
	free(configs); configs = nullptr;
	
	if (!openxr_preferred_blend (view_type, out_view.blend)) return false;
	if (!openxr_create_swapchain(out_view.swapchain_color, view_type, true,  config_ct, color_format, w, h, s)) return false;
	if (!openxr_create_swapchain(out_view.swapchain_depth, view_type, false, config_ct, depth_format, w, h, s)) return false;

	// Turn swapchains into StereoKit assets
	for (uint32_t s = 0; s < out_view.swapchain_color.surface_count; s++) {
		out_view.swapchain_color.textures[s] = tex_create(tex_type_rendertarget, tex_format_rgba32);
		out_view.swapchain_depth.textures[s] = tex_create(tex_type_depth, tex_get_tex_format(depth_format));

		char           name[64];
		static int32_t target_index = 0;
		target_index++;
		sprintf_s(name, "renderer/colortarget_%d", target_index);
		tex_set_id(out_view.swapchain_color.textures[s], name);
		sprintf_s(name, "renderer/depthtarget_%d", target_index);
		tex_set_id(out_view.swapchain_depth.textures[s], name);

		tex_setsurface (out_view.swapchain_color.textures[s], out_view.swapchain_color.images[s].texture, color_format);
		tex_setsurface (out_view.swapchain_depth.textures[s], out_view.swapchain_depth.images[s].texture, depth_format);
		tex_set_zbuffer(out_view.swapchain_color.textures[s], out_view.swapchain_depth.textures[s]);
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
	out_swapchain = {};
	out_swapchain.width         = swapchain_info.width;
	out_swapchain.height        = swapchain_info.height;
	out_swapchain.handle        = handle;
	out_swapchain.surface_count = surface_count;
	out_swapchain.images        = (XrSwapchainImageD3D11KHR*)malloc(sizeof(XrSwapchainImageD3D11KHR) * surface_count);
	out_swapchain.textures      = (tex_t                   *)malloc(sizeof(tex_t                   ) * surface_count);
	for (size_t i = 0; i < surface_count; i++) {
		out_swapchain.images[i] = { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR };
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
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_D16_UNORM,
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

void openxr_render_frame() {
	// Block until the previous frame is finished displaying, and is ready for another one.
	// Also returns a prediction of when the next frame will be displayed, for use with predicting
	// locations of controllers, viewpoints, etc.
	XrFrameWaitInfo wait_info   = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState    frame_state = { XR_TYPE_FRAME_STATE };
	xrWaitFrame (xr_session, &wait_info, &frame_state);
	// Must be called before any rendering is done! This can return some interesting flags, like 
	// XR_SESSION_VISIBILITY_UNAVAILABLE, which means we could skip rendering this frame and call
	// xrEndFrame right away.
	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	xrBeginFrame(xr_session, &begin_info);

	// Timing also needs some work, may be best as some sort of anchor system
	xr_time = frame_state.predictedDisplayTime;

	// Execute any code that's dependant on the predicted time, such as updating the location of
	// controller models.
	input_update_predicted();

	// Render all the views for the application, then clear out the render queue
	openxr_views_render(frame_state.predictedDisplayTime);
	render_clear();

	XrFrameEndSecondaryViewConfigurationInfoMSFT end_second = {};
	end_second.viewConfigurationCount = 

	// We're finished with rendering our layer, so send it off for display!
	XrFrameEndInfo end_info{ XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_displays[0].blend;
	end_info.layerCount           = xr_displays[0].active ? 1 : 0;
	end_info.layers               = (XrCompositionLayerBaseHeader*)&xr_displays[0].projection_data;
	xrEndFrame(xr_session, &end_info);
}

///////////////////////////////////////////

void openxr_views_render(XrTime time) {
	// If the session is active, lets render our layer in the compositor!
	arrsetlen(xr_composition_layers, 0);
	bool session_active = 
		xr_session_state == XR_SESSION_STATE_VISIBLE || 
		xr_session_state == XR_SESSION_STATE_FOCUSED;
	if (session_active) {
		for (size_t i = 0; i < xr_display_count; i++) {
			arrput(xr_composition_layers, &xr_displays[i].projection_data);
			openxr_render_layer(time, xr_displays[i]);
		}
	}
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
	uint32_t                    img_id;
	XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(xr_swapchains.handle, &acquire_info, &img_id);

	// Wait until the image is available to render to. The compositor could still be
	// reading from it.
	XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wait_info.timeout = XR_INFINITE_DURATION;
	xrWaitSwapchainImage(xr_swapchains.handle, &wait_info);

	// And now we'll iterate through each viewpoint, and render it!
	for (uint32_t i = 0; i < layer.view_count; i++) {
		// Set up our rendering information for the viewpoint we're using right now!
		XrCompositionLayerProjectionView &view = layer.view_layers[i];
		view = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		view.pose = layer.views[i].pose;
		view.fov  = layer.views[i].fov;
		view.subImage.imageArrayIndex  = i;
		view.subImage.swapchain        = xr_swapchains.handle;
		view.subImage.imageRect.offset = { 0, 0 };
		view.subImage.imageRect.extent = { xr_swapchains.width, xr_swapchains.height };

		float xr_projection[16];
		openxr_projection(view.fov, 0.1f, 50, xr_projection);
		memcpy(&layer.view_projections[i], xr_projection, sizeof(float) * 16);
		matrix_inverse(matrix_trs((vec3&)view.pose.position, (quat&)view.pose.orientation, vec3_one), layer.view_transforms[i]);
	}

	// Call the rendering callback with our view and swapchain info
	tex_t target = xr_swapchains.surface_data[img_id];
	tex_rtarget_clear(target, sk_info.display_type == display_opaque 
		? color32{ 0,0,0,255 } 
		: color32{ 0,0,0,0   });
	tex_rtarget_set_active(target);
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)xr_swapchains.width, (float)xr_swapchains.height);
	d3d_context->RSSetViewports(1, &viewport);

	render_draw_matrix(layer.view_transforms, layer.view_projections, layer.view_count);

	// And tell OpenXR we're done with rendering to this one!
	XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(xr_swapchains.handle, &release_info);

	layer.projection_data = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	layer.projection_data.space      = xr_app_space;
	layer.projection_data.viewCount  = layer.view_count;
	layer.projection_data.views      = layer.view_layers;
	layer.projection_data.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	return true;
}

} // namespace sk