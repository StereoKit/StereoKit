#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_extensions.h"
#include "openxr_input.h"

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../sk_memory.h"
#include "../log.h"
#include "../asset_types/texture_.h"
#include "../asset_types/texture.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/system.h"
#include "../libraries/sokol_time.h"
#include "../libraries/sk_gpu.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct swapchain_t {
	XrSwapchain handle;
	int32_t     width;
	int32_t     height;
	int32_t     multisample;
	uint32_t    surface_count;
	uint32_t    surface_layers;
	XrSwapchainImage *images;
	tex_t            *textures;
};
void swapchain_delete(swapchain_t &swapchain) {
	for (size_t s = 0; s < swapchain.surface_count * swapchain.surface_layers; s++) {
		tex_release(swapchain.textures[s]);
		swapchain.textures[s] = nullptr;
	}
	if (swapchain.handle) {
		xrDestroySwapchain(swapchain.handle);
		swapchain.handle = XR_NULL_HANDLE;
	}
	sk_free(swapchain.images  ); swapchain.images   = nullptr;
	sk_free(swapchain.textures); swapchain.textures = nullptr;

	swapchain.surface_count = 0;
	swapchain.surface_layers = 0;
}

///////////////////////////////////////////

struct device_display_t {
	XrViewConfigurationType      type;
	XrEnvironmentBlendMode       blend;
	XrCompositionLayerProjection*projection_data;
	bool32_t                     active;
	int64_t color_format;
	int64_t depth_format;
	float   render_scale;
	int32_t multisample;

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

	sk_free(display.projection_data );
	sk_free(display.views           );
	sk_free(display.view_transforms );
	sk_free(display.view_layers     );
	sk_free(display.view_depths     );
	sk_free(display.view_projections);
	sk_free(display.view_configs    );
}

///////////////////////////////////////////

array_t<uint8_t> xr_compositor_bytes      = {};
array_t<size_t>  xr_compositor_layers     = {};
array_t<int32_t> xr_compositor_layer_sort = {};

void backend_openxr_composition_layer(void *XrCompositionLayerBaseHeader, int32_t layer_size, int32_t sort_order) {
	int32_t start = xr_compositor_bytes.count;
	xr_compositor_bytes.add_range((uint8_t*)XrCompositionLayerBaseHeader, layer_size);

	int32_t id = xr_compositor_layer_sort.binary_search(sort_order);
	if (id < 0) id = ~id;
	xr_compositor_layer_sort.insert(id, sort_order);
	xr_compositor_layers    .insert(id, start);
}

///////////////////////////////////////////

array_t<XrCompositionLayerBaseHeader*> xr_compositor_layer_ptrs = {};
const array_t<XrCompositionLayerBaseHeader *> *compositor_layers_get() {
	xr_compositor_layer_ptrs.clear();
	for (int32_t i = 0; i < xr_compositor_layers.count; i++) {
		xr_compositor_layer_ptrs.add((XrCompositionLayerBaseHeader *)(xr_compositor_bytes.data + xr_compositor_layers[i]));
	}
	return &xr_compositor_layer_ptrs;
}

///////////////////////////////////////////

void xr_compositor_layers_clear() {
	xr_compositor_bytes     .clear();
	xr_compositor_layers    .clear();
	xr_compositor_layer_sort.clear();
}

///////////////////////////////////////////

void compositor_layers_free() {
	xr_compositor_bytes     .free();
	xr_compositor_layers    .free();
	xr_compositor_layer_sort.free();
	xr_compositor_layer_ptrs.free();
}

///////////////////////////////////////////

XrViewConfigurationType xr_request_displays[] = {
	XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
	XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
};
XrViewConfigurationType xr_display_primary = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
system_t               *xr_render_sys      = nullptr;

array_t<device_display_t>                          xr_displays           = {};
array_t<XrViewConfigurationType>                   xr_display_types      = {};
array_t<XrSecondaryViewConfigurationStateMSFT>     xr_display_2nd_states = {};
array_t<XrSecondaryViewConfigurationLayerInfoMSFT> xr_display_2nd_layers = {};

///////////////////////////////////////////

bool openxr_create_view      (XrViewConfigurationType view_type, device_display_t &out_view);
bool openxr_create_swapchain (swapchain_t &out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count);
void openxr_preferred_format (int64_t &out_color, int64_t &out_depth);
bool openxr_preferred_blend  (XrViewConfigurationType view_type, display_blend_ preference, display_blend_* out_valid, XrEnvironmentBlendMode* out_blend);
bool openxr_update_swapchains(device_display_t &display);
bool openxr_render_layer     (XrTime predictedTime, device_display_t &layer, render_layer_ render_filter);

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

bool32_t xr_blend_valid(display_blend_ blend) {
	if (blend == display_blend_any_transparent) {
		if (xr_valid_blends & display_blend_additive) {
			blend = display_blend_additive;
		} else if (xr_valid_blends & display_blend_blend) {
			blend = display_blend_additive;
		} else {
			blend = display_blend_none;
		}
	}

	return (blend & xr_valid_blends) != 0;
}

///////////////////////////////////////////

bool32_t xr_set_blend(display_blend_ blend) {
	if (blend == display_blend_any_transparent) {
		if (xr_valid_blends & display_blend_additive) {
			blend = display_blend_additive;
		} else if (xr_valid_blends & display_blend_blend) {
			blend = display_blend_additive;
		} else {
			blend = display_blend_none;
		}
	}

	if ((blend & xr_valid_blends) == 0) {
		log_err("Set display blend to an invalid mode!");
		return false;
	}
	device_data.display_blend = blend;

	for (int32_t i = 0; i < xr_displays.count; i++) {
		if (xr_displays[i].type != xr_display_primary) continue;
		switch (blend) {
		case display_blend_additive: xr_displays[i].blend = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE; break;
		case display_blend_blend:    xr_displays[i].blend = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND; break;
		case display_blend_opaque:   xr_displays[i].blend = XR_ENVIRONMENT_BLEND_MODE_OPAQUE; break;
		}
	}

	return true;
}

///////////////////////////////////////////

bool openxr_views_create() {
	xr_render_sys = systems_find("FrameRender");

	// Find all the valid view configurations
	uint32_t count = 0;
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, 0, &count, nullptr),
		"xrEnumerateViewConfigurations failed [%s]");
	XrViewConfigurationType *types = sk_malloc_t(XrViewConfigurationType, count);
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
					xr_display_2nd_states.add(state);
				}
				xr_display_types.add(types[t]);
				xr_displays     .add(device_display_t{});
				if (!openxr_create_view(types[t], xr_displays.last()))
					return false;
			}
		}
	}
	sk_free(types);

	if (xr_displays.count == 0) {
		log_info("No valid display configurations were found!");
		return false;
	}

	// Register dispay type with the system
	switch (xr_displays[0].blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      sk_info.display_type = display_opaque;      break;
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    sk_info.display_type = display_additive;    break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: sk_info.display_type = display_passthrough; break;
	// Just max_enum
	default: break;
	}

	return true;
}

///////////////////////////////////////////

void openxr_views_destroy() {
	for (int32_t i = 0; i < xr_displays.count; i++) {
		device_display_delete(xr_displays[i]);
	}
	xr_displays          .free();
	xr_display_types     .free();
	xr_display_2nd_states.free();
	xr_display_2nd_layers.free();
	compositor_layers_free();
}

///////////////////////////////////////////

bool openxr_create_view(XrViewConfigurationType view_type, device_display_t &out_view) {
	out_view = {};

	// Get the surface format information before we create surfaces!
	openxr_preferred_format(out_view.color_format, out_view.depth_format);
	if (!openxr_preferred_blend(view_type, sk_settings.blend_preference, &xr_valid_blends, &out_view.blend)) return false;

	// Tell OpenXR what sort of color space we're rendering in
	if (xr_ext_available.FB_color_space) {
		const char    *colorspace_str = "XR_COLOR_SPACE_REC709_FB";
		XrColorSpaceFB colorspace     =  XR_COLOR_SPACE_REC709_FB;
		skg_tex_fmt_   fmt            = skg_tex_fmt_from_native(out_view.color_format);

		// Maybe?
		if (fmt != skg_tex_fmt_bgra32 && fmt != skg_tex_fmt_rgba32) {
			colorspace_str = "XR_COLOR_SPACE_REC2020_FB";
			colorspace     =  XR_COLOR_SPACE_REC2020_FB;
		}

		if (XR_SUCCEEDED(xr_extensions.xrSetColorSpaceFB(xr_session, colorspace)))
			log_diagf("Set color space to <~grn>%s<~clr>", colorspace_str);
	}

	// Debug print the view and format info
	const char *blend_mode_str = "NA";
	switch (out_view.blend) {
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    blend_mode_str = "Additive";    break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: blend_mode_str = "Alpha Blend"; break;
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      blend_mode_str = "Opaque";      break; 
	// Just max_enum
	default: break;
	}
	log_diagf("Creating view: <~grn>%s<~clr> color:<~grn>%s<~clr> depth:<~grn>%s<~clr> blend:<~grn>%s<~clr>",
		openxr_view_name(view_type),
		render_fmt_name((tex_format_)skg_tex_fmt_from_native(out_view.color_format)),
		render_fmt_name((tex_format_)skg_tex_fmt_from_native(out_view.depth_format)),
		blend_mode_str);

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	xr_check(xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, view_type, 0, &out_view.view_cap, nullptr),
		"openxr_views_create can't find any valid view configurations");

	out_view.view_configs = sk_malloc_t(XrViewConfigurationView, out_view.view_cap);
	for (uint32_t i = 0; i < out_view.view_cap; i++) out_view.view_configs[i] = { XR_TYPE_VIEW_CONFIGURATION_VIEW };

	// Extract information from the views we got
	out_view.type             = view_type;
	out_view.active           = true;
	out_view.projection_data  = sk_malloc_t(XrCompositionLayerProjection, 1);
	out_view.view_count       = 0;
	out_view.views            = sk_malloc_t(XrView, out_view.view_cap);
	out_view.view_layers      = sk_malloc_t(XrCompositionLayerProjectionView, out_view.view_cap);
	out_view.view_depths      = sk_malloc_t(XrCompositionLayerDepthInfoKHR,   out_view.view_cap);
	out_view.view_transforms  = sk_malloc_t(matrix, out_view.view_cap);
	out_view.view_projections = sk_malloc_t(matrix, out_view.view_cap);
	for (uint32_t i = 0; i < out_view.view_cap; i++) {
		out_view.views      [i] = { XR_TYPE_VIEW };
		out_view.view_layers[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		out_view.view_depths[i] = { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
	}
	if (view_type == xr_display_primary) {
		out_view.multisample  = render_get_multisample();
		out_view.render_scale = render_get_scaling();
	} else {
		out_view.multisample  = 1;
		out_view.render_scale = 1;
	}

	bool needs_swapchain_now = view_type != XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT;
	if (needs_swapchain_now && !openxr_update_swapchains(out_view)) {
		log_fail_reason(80, log_error, "Couldn't create OpenXR view swapchains!");
		return false;
	}

	// Update the display info right away, some of this gets updated each draw,
	// but most users will want this info as soon as the session begins. If we
	// skip doing this here, then there will be a single frame delay where the
	// information isn't present.
	if (view_type == xr_display_primary) {
		switch (out_view.blend) {
		case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    device_data.display_blend = display_blend_additive; break;
		case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: device_data.display_blend = display_blend_blend;    break;
		case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      device_data.display_blend = display_blend_opaque;   break;
		default:                                    device_data.display_blend = display_blend_none;     break;
		}
		device_data.display_width  = out_view.swapchain_color.width;
		device_data.display_height = out_view.swapchain_color.height;
	}
	return true;
}

///////////////////////////////////////////

bool openxr_update_swapchains(device_display_t &display) {
	// Check if the latest configuration is different from what we've already
	// set up.
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, display.type, display.view_cap, &display.view_cap, display.view_configs);
	int32_t w = (int32_t)(display.view_configs[0].recommendedImageRectWidth  * display.render_scale);
	int32_t h = (int32_t)(display.view_configs[0].recommendedImageRectHeight * display.render_scale);
	int32_t s = display.multisample;
	if (display.render_scale != 1.0f) {
		const int32_t quantize = 4;
		w = (w / quantize) * quantize;
		h = (h / quantize) * quantize;
	}
	if (w > (int32_t)display.view_configs[0].maxImageRectWidth      ) w = display.view_configs[0].maxImageRectWidth;
	if (h > (int32_t)display.view_configs[0].maxImageRectHeight     ) h = display.view_configs[0].maxImageRectHeight;
	if (s > (int32_t)display.view_configs[0].maxSwapchainSampleCount) s = display.view_configs[0].maxSwapchainSampleCount;

	if (   w == display.swapchain_color.width
		&& h == display.swapchain_color.height
		&& s == display.swapchain_color.multisample) {
		return true;
	}

	// Create the new swapchains for the current size
	if (!openxr_create_swapchain(display.swapchain_color, display.type, true,  display.view_cap, display.color_format, w, h, s)) return false;
	if (!openxr_create_swapchain(display.swapchain_depth, display.type, false, display.view_cap, display.depth_format, w, h, s)) return false;

	log_diagf("Set view: <~grn>%s<~clr> to %d<~BLK>x<~clr>%d<~BLK>@<~clr>%d<~BLK>msaa<~clr>", openxr_view_name(display.type), display.swapchain_color.width, display.swapchain_color.height, display.swapchain_color.multisample);

	// If shaders can't select layers from a texture array, we'll have to
	// separate the layers into individual render targets.
	if (skg_capability(skg_cap_tex_layer_select) && xr_has_single_pass) {
		display.swapchain_color.surface_layers = 1;
		display.swapchain_depth.surface_layers = 1;
	} else {
		display.swapchain_color.surface_layers = display.view_cap;
		display.swapchain_depth.surface_layers = display.view_cap;
	}

	// Create texture objects if we don't have 'em
	if (display.swapchain_color.textures == nullptr) {
		display.swapchain_color.textures = sk_malloc_t(tex_t, (size_t)display.swapchain_color.surface_count * display.swapchain_color.surface_layers);
		display.swapchain_depth.textures = sk_malloc_t(tex_t, (size_t)display.swapchain_depth.surface_count * display.swapchain_depth.surface_layers);
		memset(display.swapchain_color.textures, 0, sizeof(tex_t) * display.swapchain_color.surface_count * display.swapchain_color.surface_layers);
		memset(display.swapchain_depth.textures, 0, sizeof(tex_t) * display.swapchain_depth.surface_count * display.swapchain_depth.surface_layers);

		for (uint32_t surf = 0; surf < display.swapchain_color.surface_count; surf++) {
			for (uint32_t layer = 0; layer < display.swapchain_color.surface_layers; layer++) {
				int32_t index = layer*display.swapchain_color.surface_count + surf;

				display.swapchain_color.textures[index] = tex_create(tex_type_rendertarget, tex_get_tex_format(display.color_format));
				display.swapchain_depth.textures[index] = tex_create(tex_type_depth,        tex_get_tex_format(display.depth_format));

				char           name[64];
				static int32_t target_index = 0;
				target_index++;
				snprintf(name, sizeof(name), "renderer/colortarget_%d", target_index);
				tex_set_id(display.swapchain_color.textures[index], name);
				snprintf(name, sizeof(name), "renderer/depthtarget_%d", target_index);
				tex_set_id(display.swapchain_depth.textures[index], name);
			}
		}
	}

	// Update or set the native textures
	for (uint32_t surf = 0; surf < display.swapchain_color.surface_count; surf++) {
		// Update our textures with the new swapchain display surfaces
		void *native_surface_col   = nullptr;
		void *native_surface_depth = nullptr;
#if defined(XR_USE_GRAPHICS_API_D3D11)
		native_surface_col   = display.swapchain_color.images[surf].texture;
		native_surface_depth = display.swapchain_depth.images[surf].texture;
#elif defined(XR_USE_GRAPHICS_API_OPENGL) || defined(XR_USE_GRAPHICS_API_OPENGL_ES)
		native_surface_col   = (void*)(uint64_t)display.swapchain_color.images[surf].image;
		native_surface_depth = (void*)(uint64_t)display.swapchain_depth.images[surf].image;
#endif
		if (display.swapchain_color.surface_layers == 1) {
			tex_set_surface(display.swapchain_color.textures[surf], native_surface_col,   tex_type_rendertarget, display.color_format, display.swapchain_color.width, display.swapchain_color.height, display.view_cap);
			tex_set_surface(display.swapchain_depth.textures[surf], native_surface_depth, tex_type_depth,        display.depth_format, display.swapchain_depth.width, display.swapchain_depth.height, display.view_cap);
			tex_set_zbuffer(display.swapchain_color.textures[surf], display.swapchain_depth.textures[surf]);
		} else {
			for (uint32_t layer = 0; layer < display.swapchain_color.surface_layers; layer++) {
				int32_t index = layer * display.swapchain_color.surface_count + surf;
				tex_set_surface_layer(display.swapchain_color.textures[index], native_surface_col,   tex_type_rendertarget, display.color_format, display.swapchain_color.width, display.swapchain_color.height, layer);
				tex_set_surface_layer(display.swapchain_depth.textures[index], native_surface_depth, tex_type_depth,        display.depth_format, display.swapchain_depth.width, display.swapchain_depth.height, layer);
				tex_set_zbuffer(display.swapchain_color.textures[index], display.swapchain_depth.textures[index]);
			}
		}
	}

	if (display.type == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO) {
		sk_info.display_width  = w;
		sk_info.display_height = h;
		device_data.display_width  = w;
		device_data.display_height = h;
	}

	return true;
}

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t &out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height, int32_t sample_count) {
	swapchain_delete(out_swapchain);

	// Create a swapchain for this viewpoint! A swapchain is a set of texture
	// buffers used for displaying to screen, typically this is a backbuffer
	// and a front buffer, one for rendering data to, and one for displaying
	// on-screen.
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
	XrSecondaryViewConfigurationSwapchainCreateInfoMSFT secondary = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT };
	if (type != xr_display_primary) {
		secondary.viewConfigurationType = type;
		swapchain_info.next = &secondary;
	}

	xr_check(xrCreateSwapchain(xr_session, &swapchain_info, &handle),
		"xrCreateSwapchain failed [%s]");

	// Find out how many textures were generated for the swapcha in
	uint32_t surface_count = 0;
	xr_check(xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr),
		"xrEnumerateSwapchainImages failed [%s]");

	// We'll want to track our own information about the swapchain, so we can
	// draw stuff onto it! We'll also create a depth buffer for each generated
	// texture here as well with make_surfacedata.
	out_swapchain.width         = swapchain_info.width;
	out_swapchain.height        = swapchain_info.height;
	out_swapchain.multisample   = swapchain_info.sampleCount;
	out_swapchain.handle        = handle;
	if (out_swapchain.surface_count != surface_count) {
		out_swapchain.surface_count = surface_count;
		out_swapchain.images        = sk_malloc_t(XrSwapchainImage, surface_count);
	}
	for (uint32_t i=0; i<surface_count; i++) {
		out_swapchain.images[i] = { XR_TYPE_SWAPCHAIN_IMAGE };
	}

	xr_check(xrEnumerateSwapchainImages(out_swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader *)out_swapchain.images),
		"xrEnumerateSwapchainImages failed [%s]");

	return true;
}

///////////////////////////////////////////

void openxr_preferred_format(int64_t &out_color_dx, int64_t &out_depth_dx) {
	int64_t pixel_formats[] = {
		skg_tex_fmt_to_native(skg_tex_fmt_rgba32),
		skg_tex_fmt_to_native(skg_tex_fmt_bgra32),
		skg_tex_fmt_to_native(skg_tex_fmt_rg11b10),
		skg_tex_fmt_to_native(skg_tex_fmt_rgb10a2),
		skg_tex_fmt_to_native(skg_tex_fmt_rgba32_linear),
		skg_tex_fmt_to_native(skg_tex_fmt_bgra32_linear) };

	int64_t depth_formats[] = {
		skg_tex_fmt_to_native((skg_tex_fmt_)render_preferred_depth_fmt()),
		skg_tex_fmt_to_native(skg_tex_fmt_depth16),
		skg_tex_fmt_to_native(skg_tex_fmt_depth32),
		skg_tex_fmt_to_native(skg_tex_fmt_depthstencil)};

	// Get the list of formats OpenXR would like
	uint32_t count = 0;
	xrEnumerateSwapchainFormats(xr_session, 0, &count, nullptr);
	int64_t *formats = sk_malloc_t(int64_t, count);
	xrEnumerateSwapchainFormats(xr_session, count, &count, formats);

	// Check those against our formats, prefer OpenXR's pick for color format
	out_color_dx = 0;
	for (uint32_t i=0; i<count; i++) {
		for (int32_t f=0; out_color_dx == 0 && f<_countof(pixel_formats); f++) {
			if (formats[i] == pixel_formats[f]) {
				out_color_dx = pixel_formats[f];
				break;
			}
		}
	}

	// For depth, prefer our top pick over OpenXR's top pick, since we have
	// some extra qualifications to our selection.
	out_depth_dx = 0;
	for (int32_t f=0;  f<_countof(depth_formats); f++) {
		for (uint32_t i=0; out_depth_dx == 0 && i<count; i++) {
			if (formats[i] == depth_formats[f]) {
				out_depth_dx = depth_formats[f];
				break;
			}
		}
	}

	// Release memory
	sk_free(formats);
}

///////////////////////////////////////////

bool openxr_preferred_blend(XrViewConfigurationType view_type, display_blend_ preference, display_blend_ *out_valid, XrEnvironmentBlendMode *out_blend) {
	// Check what blend mode is valid for this device (opaque vs transparent
	// displays)
	uint32_t                blend_count = 0;
	XrEnvironmentBlendMode *blend_modes;
	xr_check(xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, view_type, 0, &blend_count, nullptr),
		"xrEnumerateEnvironmentBlendModes failed [%s]");
	blend_modes = sk_malloc_t(XrEnvironmentBlendMode, blend_count);
	xr_check(xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, view_type, blend_count, &blend_count, blend_modes),
		"xrEnumerateEnvironmentBlendModes failed [%s]");
	
	*out_blend = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
	*out_valid = display_blend_none;
	for (uint32_t i = 0; i < blend_count; i++) {
		display_blend_ curr = display_blend_none;
		switch (blend_modes[i]) {
		case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      curr = display_blend_opaque;   break;
		case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    curr = display_blend_additive; break;
		case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: curr = display_blend_blend;    break;
		}
		// Accumulate all valid blend modes
		*out_valid |= curr;

		// Store the first user preference
		if (*out_blend == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM && (curr & preference) > 0)
			*out_blend = blend_modes[i];
	}
	// If none matched user preference, just use the first one the runtime
	// reports.
	if (*out_blend == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM)
		*out_blend = blend_modes[0];

	sk_free(blend_modes);
	return true;
}


///////////////////////////////////////////

bool openxr_render_frame() {
	// Block until the previous frame is finished displaying, and is ready for
	// another one. Also returns a prediction of when the next frame will be
	// displayed, for use with predicting locations of controllers, viewpoints,
	// etc.
	XrFrameWaitInfo                            wait_info        = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState                               frame_state      = { XR_TYPE_FRAME_STATE };
	XrSecondaryViewConfigurationFrameStateMSFT secondary_states = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT };
	if (xr_displays.count > 1) {
		secondary_states.viewConfigurationCount  = (uint32_t)xr_displays.count - 1;
		secondary_states.viewConfigurationStates = &xr_display_2nd_states[0];
		frame_state.next = &secondary_states;
	}
	xr_check(xrWaitFrame(xr_session, &wait_info, &frame_state),
		"xrWaitFrame [%s]");

	// Don't track sync time, start the frame timer after xrWaitFrame
	xr_render_sys->profile_frame_start = stm_now();

	// Check active for the primary display
	bool session_active =
		xr_session_state == XR_SESSION_STATE_VISIBLE ||
		xr_session_state == XR_SESSION_STATE_FOCUSED;
	xr_displays[0].active = session_active;

	// Check each secondary display to see if it's active or not
	for (int32_t i = 1; i < xr_displays.count; i++) {
		if (xr_displays[i].active != (bool32_t)xr_display_2nd_states[i - 1].active) {
			xr_displays[i].active  = (bool32_t)xr_display_2nd_states[i - 1].active;

			if (xr_displays[i].active) {
				openxr_update_swapchains(xr_displays[i]);
			}
		}
	}
	if (xr_displays.count > 0 && (xr_displays[0].render_scale != render_get_scaling() || xr_displays[0].multisample != render_get_multisample())) {
		xr_displays[0].render_scale = render_get_scaling();
		xr_displays[0].multisample  = render_get_multisample();
		openxr_update_swapchains(xr_displays[0]);
	}

	// Must be called before any rendering is done! This can return some
	// interesting flags, like XR_SESSION_VISIBILITY_UNAVAILABLE, which means
	// we could skip rendering this frame and call xrEndFrame right away.
	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	xr_check(xrBeginFrame(xr_session, &begin_info),
		"xrBeginFrame [%s]");

	// Timing also needs some work, may be best as some sort of anchor system
	xr_time = frame_state.predictedDisplayTime;

	// Execute any code that's dependent on the predicted time, such as
	// updating the location of controller models.
	input_update_poses(true);

	// Render all the views for the application, then clear out the render
	// queue. If the session is active, lets render our layer in the
	// compositor!
	xr_display_2nd_layers.clear();

	skg_draw_begin();
	for (int32_t i = 0; i < xr_displays.count; i++) {
		if (!xr_displays[i].active) continue;

		render_layer_ filter = xr_displays[i].type == xr_display_primary
			? render_get_filter()
			: render_get_capture_filter();
		openxr_render_layer(xr_time, xr_displays[i], filter);

		if (xr_displays[i].type != xr_display_primary) {
			XrSecondaryViewConfigurationLayerInfoMSFT layer2nd = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT };
			layer2nd.viewConfigurationType = xr_displays[i].type;
			layer2nd.environmentBlendMode  = xr_displays[i].blend;
			layer2nd.layerCount            = 1;
			layer2nd.layers                = (XrCompositionLayerBaseHeader**)&xr_displays[i].projection_data;
			xr_display_2nd_layers.add(layer2nd);
		} else {
			backend_openxr_composition_layer(xr_displays[i].projection_data, sizeof(XrCompositionLayerProjection), 0);
		}
	}

	// We're finished with rendering our layer, so send it off for display!
	const array_t<XrCompositionLayerBaseHeader*> *comp_layers = compositor_layers_get();
	XrFrameEndInfo end_info = { XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_displays[0].blend;
	end_info.layerCount           = (uint32_t)comp_layers->count;
	end_info.layers               = comp_layers->data;

	XrSecondaryViewConfigurationFrameEndInfoMSFT end_second = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT };
	end_second.viewConfigurationCount      = (uint32_t)xr_display_2nd_layers.count;
	end_second.viewConfigurationLayersInfo = xr_display_2nd_layers.data;
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
	result[10] = range;                                // [2][2] Same as xr's: -(farZ + offsetZ) / (farZ - nearZ)
	result[11] = -1;                                   // [2][3] Same
	result[14] = range * clip_near;                    // [3][2] Same as xr's: -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);
}

///////////////////////////////////////////

bool openxr_render_layer(XrTime predictedTime, device_display_t &layer, render_layer_ render_filter) {

	// Find the state and location of each viewpoint at the predicted time
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = layer.type;
	locate_info.displayTime           = predictedTime;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, layer.view_cap, &layer.view_count, layer.views);

	// Copy over the FoV so it's available to the users
	if (layer.view_count > 0 && layer.type != xr_display_primary) {
		device_data.display_fov.right  = layer.views[0].fov.angleRight * rad2deg;
		device_data.display_fov.left   = layer.views[0].fov.angleLeft  * rad2deg;
		device_data.display_fov.top    = layer.views[0].fov.angleUp    * rad2deg;
		device_data.display_fov.bottom = layer.views[0].fov.angleDown  * rad2deg;
	}

	// We need to ask which swapchain image to use for rendering! Which one
	// will we get? Who knows! It's up to the runtime to decide.
	uint32_t                    color_id, depth_id;
	XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	xrAcquireSwapchainImage(layer.swapchain_color.handle, &acquire_info, &color_id);
	xrAcquireSwapchainImage(layer.swapchain_depth.handle, &acquire_info, &depth_id);

	// Wait until the image is available to render to. The compositor could
	// still be reading from it.
	XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wait_info.timeout = XR_INFINITE_DURATION;
	xrWaitSwapchainImage(layer.swapchain_color.handle, &wait_info);
	xrWaitSwapchainImage(layer.swapchain_depth.handle, &wait_info);

	// And now we'll iterate through each viewpoint, and render it!
	vec2 clip_planes = render_get_clip();
	for (uint32_t i = 0; i < layer.view_count; i++) {
		// Set up our rendering information for the viewpoint we're using right
		// now!
		XrCompositionLayerProjectionView &view = layer.view_layers[i];
		view = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		view.pose = layer.views[i].pose;
		view.fov  = layer.views[i].fov;
		view.subImage.imageArrayIndex  = i;
		view.subImage.swapchain        = layer.swapchain_color.handle;
		view.subImage.imageRect.offset = { 0, 0 };
		view.subImage.imageRect.extent = { layer.swapchain_color.width, layer.swapchain_color.height };

		if (xr_ext_available.KHR_composition_layer_depth) {
			XrCompositionLayerDepthInfoKHR &depth = layer.view_depths[i];
			depth.minDepth = 0;
			depth.maxDepth = 1;
			depth.nearZ    = clip_planes.x;
			depth.farZ     = clip_planes.y;
			depth.subImage.imageArrayIndex  = i;
			depth.subImage.swapchain        = layer.swapchain_depth.handle;
			depth.subImage.imageRect.offset = { 0, 0 };
			depth.subImage.imageRect.extent = { layer.swapchain_depth.width, layer.swapchain_depth.height };
			view.next = &depth;
		}

		float xr_projection[16];
		openxr_projection(view.fov, clip_planes.x, clip_planes.y, xr_projection);
		memcpy(&layer.view_projections[i], xr_projection, sizeof(float) * 16);
		matrix view_tr = matrix_trs((vec3 &)view.pose.position, (quat &)view.pose.orientation, vec3_one);
		view_tr = view_tr * render_get_cam_final();
		matrix_inverse(view_tr, layer.view_transforms[i]);
	}

	for (uint32_t s_layer = 0; s_layer < layer.swapchain_color.surface_layers; s_layer++) {
		int32_t index = s_layer*layer.swapchain_color.surface_count + color_id;

		// Call the rendering callback with our view and swapchain info
		tex_t    target = layer.swapchain_color.textures[index];
		color128 col    = sk_info.display_type == display_opaque
			? render_get_clear_color_ln()
			: color128{ 0,0,0,0 };
		skg_tex_target_bind(&target->tex);
		skg_target_clear(true, &col.r);

		render_draw_matrix(&layer.view_transforms[s_layer], &layer.view_projections[s_layer], layer.view_count / layer.swapchain_color.surface_layers, render_filter);
	}

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

///////////////////////////////////////////

void openxr_views_update_fov() {
	device_display_t* disp = &xr_displays[0];
	for (int32_t i = 0; i < xr_displays.count; i++) {
		if (xr_displays[i].type == xr_display_primary) {
			disp = &xr_displays[i];
			break;
		}
	}

	// Find the state and location of each viewpoint at the predicted time
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = disp->type;
	locate_info.displayTime           = xr_time;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, disp->view_cap, &disp->view_count, disp->views);

	// Copy over the FoV so it's available to the users
	if (disp->view_count > 0) {
		device_data.display_fov.right  = disp->views[0].fov.angleRight * rad2deg;
		device_data.display_fov.left   = disp->views[0].fov.angleLeft  * rad2deg;
		device_data.display_fov.top    = disp->views[0].fov.angleUp    * rad2deg;
		device_data.display_fov.bottom = disp->views[0].fov.angleDown  * rad2deg;
	}
}

} // namespace sk
#endif