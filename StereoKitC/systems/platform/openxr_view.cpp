
#include "openxr.h"
#include "openxr_input.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../../systems/render.h"

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

struct projection_layer_t {
	swapchain_t swapchain_color;
	swapchain_t swapchain_depth;

	uint32_t view_count;
	XrView  *views;
	matrix  *view_transforms;
	matrix  *projections;
};
void projection_layer_delete(projection_layer_t &layer) {
	swapchain_delete(layer.swapchain_color);
	swapchain_delete(layer.swapchain_depth);

	free(layer.views          ); layer.views           = nullptr;
	free(layer.view_transforms); layer.view_transforms = nullptr;
	free(layer.projections    ); layer.projections     = nullptr;
}

///////////////////////////////////////////

XrViewConfigurationType xr_request_views[] = {
	XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
};

int32_t             xr_view_count;
projection_layer_t *xr_view_layers;

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t &out_swapchain, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count);
void openxr_preferred_format(DXGI_FORMAT &out_pixel_format_dx, tex_format_ &out_pixel_format_sk, DXGI_FORMAT &out_depth_format_dx, tex_format_ &out_depth_format_sk);

///////////////////////////////////////////

bool openxr_views_create() {
	// Find all the valid view configurations
	uint32_t count = 0;
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, 0, &count, nullptr),
		"xrEnumerateViewConfigurations failed [%s]");
	XrViewConfigurationType *types = (XrViewConfigurationType *)malloc(sizeof(XrViewConfigurationType) * count);
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, count, &count, types),
		"xrEnumerateViewConfigurations failed [%s]");

	// Count how many view configurations are valid that we've requested
	xr_view_count = 0;
	for (uint32_t t = 0; t < count; t++) {
		for (uint32_t r = 0; r < _countof(xr_request_views); r++) {
			if (types[t] == xr_request_views[r])
				xr_view_count++;
		}
	}

	// Now initialize each valid view configurations
	xr_view_layers = (projection_layer_t*)malloc(sizeof(projection_layer_t) * xr_view_count);
	xr_view_count  = 0;
	for (uint32_t t = 0; t < count; t++) {
		for (uint32_t r = 0; r < _countof(xr_request_views); r++) {
			if (types[t] == xr_request_views[r]) {
				openxr_create_view(types[t], xr_view_layers[xr_view_count]);
				xr_view_count++;
			}
		}
	}

	free(types);
}

///////////////////////////////////////////

void openxr_views_destroy() {
	for (size_t i = 0; i < xr_view_count; i++)
		projection_layer_delete(xr_view_layers[i]);
	free(xr_view_layers);
}

///////////////////////////////////////////

int32_t xr_target_index = 0;
bool openxr_create_view(XrViewConfigurationType view_type, projection_layer_t &out_view) {
	// Get the surface format information before we create surfaces!
	DXGI_FORMAT color_format,    depth_format;
	tex_format_ color_format_sk, depth_format_sk;
	openxr_preferred_format(color_format, color_format_sk, depth_format, depth_format_sk);

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

	int32_t w = configs[0].recommendedImageRectWidth;
	int32_t h = configs[0].recommendedImageRectHeight;
	int32_t s = configs[0].recommendedSwapchainSampleCount;
	free(configs); configs = nullptr;
	
	openxr_create_swapchain(out_view.swapchain_color, true,  config_ct, color_format, w, h, s);
	openxr_create_swapchain(out_view.swapchain_depth, false, config_ct, depth_format, w, h, s);

	// Turn swapchains into StereoKit assets
	for (uint32_t s = 0; s < out_view.swapchain_color.surface_count; s++) {
		out_view.swapchain_color.textures[s] = tex_create(tex_type_rendertarget, tex_format_rgba32);
		out_view.swapchain_depth.textures[s] = tex_create(tex_type_depth, depth_format_sk);

		char name[64];
		sprintf_s(name, "renderer/colortarget_%d", xr_target_index);
		tex_set_id(out_view.swapchain_color.textures[s], name);
		sprintf_s(name, "renderer/depthtarget_%d", xr_target_index);
		tex_set_id(out_view.swapchain_depth.textures[s], name);

		tex_setsurface (out_view.swapchain_color.textures[s], out_view.swapchain_color.images[s].texture, color_format);
		tex_setsurface (out_view.swapchain_depth.textures[s], out_view.swapchain_depth.images[s].texture, depth_format);
		tex_set_zbuffer(out_view.swapchain_color.textures[s], out_view.swapchain_depth.textures[s]);

		xr_target_index++;
	}
	
	return true;
}

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t &out_swapchain, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count) {
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
	for (size_t i = 0; i < surface_count; i++)
		out_swapchain.images[i] = { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR };
	
	xr_check(xrEnumerateSwapchainImages(out_swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader *)out_swapchain.images),
		"xrEnumerateSwapchainImages failed [%s]");

	return true;
}

///////////////////////////////////////////

void openxr_preferred_format(DXGI_FORMAT &out_pixel_format_dx, tex_format_ &out_pixel_format_sk, DXGI_FORMAT &out_depth_format_dx, tex_format_ &out_depth_format_sk) {
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
	out_pixel_format_dx = DXGI_FORMAT_UNKNOWN;
	for (uint32_t i = 0; i < count; i++) {
		for (int32_t f = 0; f < _countof(pixel_formats); f++) {
			if (formats[i] == pixel_formats[f]) {
				out_pixel_format_dx = pixel_formats[f];
				break;
			}
		}
		if (out_pixel_format_dx != DXGI_FORMAT_UNKNOWN)
			break;
	}

	// Find the matching StereoKit format
	out_pixel_format_sk = tex_format_rgba32;
	switch (out_depth_format_dx) {
	case DXGI_FORMAT_R8G8B8A8_UNORM:      out_pixel_format_sk = tex_format_rgba32_linear; break;
	case DXGI_FORMAT_B8G8R8A8_UNORM:      out_pixel_format_sk = tex_format_rgba32_linear; break;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: out_pixel_format_sk = tex_format_rgba32;        break;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: out_pixel_format_sk = tex_format_rgba32;        break;
	}

	// Check those against our depth formats
	out_depth_format_dx = DXGI_FORMAT_UNKNOWN;
	for (uint32_t i = 0; i < count; i++) {
		for (int32_t f = 0; f < _countof(depth_formats); f++) {
			if (formats[i] == depth_formats[f]) {
				out_depth_format_dx = depth_formats[f];
				break;
			}
		}
		if (out_depth_format_dx != DXGI_FORMAT_UNKNOWN)
			break;
	}

	// Depth maps directly to SK tex types, so we'll return that instead!
	out_depth_format_sk = tex_format_depth16;
	switch (out_depth_format_dx) {
	case DXGI_FORMAT_D16_UNORM:         out_depth_format_sk = tex_format_depth16;      break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: out_depth_format_sk = tex_format_depthstencil; break;
	case DXGI_FORMAT_D32_FLOAT:         out_depth_format_sk = tex_format_depth32;      break;
	}

	// Release memory
	free(formats);
}

///////////////////////////////////////////

void openxr_views_render() {
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
	locate_info.viewConfigurationType = xr_view_primary;
	locate_info.displayTime           = predictedTime;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, (uint32_t)xr_views_count.size(), &view_count, xr_views.data());

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
	tex_rtarget_clear(target, sk_info.display_type == display_opaque 
		? color32{ 0,0,0,255 } 
		: color32{ 0,0,0,0   });
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

} // namespace sk