/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 * Copyright (c) 2023 Austin Hale
 */

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#include "openxr.h"
#include "openxr_platform.h"
#include "extensions/fb_colorspace.h"
#include "extensions/composition_depth.h"
#include "extensions/meta_environment_depth.h"
#include "extensions/view_config_views_change.h"

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../device.h"
#include "../sk_memory.h"
#include "../log.h"
#include "../asset_types/texture.h"
#include "../asset_types/texture_.h"
#include "../systems/render.h"
#include "../systems/frame_pacer.h"
#include "../systems/render_pipeline.h"
#include "../systems/input.h"
#include "../systems/system.h"
#include "../libraries/sokol_time.h"
#include "../libraries/profiler.h"

#include <sk_renderer.h>
#include <stdio.h>

namespace sk {

///////////////////////////////////////////

// Counter mirrors sk_renderer's frame counter so the ring slot we read
// matches the frame whose timing skr_renderer_get_cpu_time_us reports. Both
// counters advance exclusively at skr_renderer_frame_end, so once both have
// started ticking, their modular phase is fixed.
static uint64_t xr_cpu_dead_ring [SKR_MAX_FRAMES_IN_FLIGHT];  // ticks per completed frame
static uint32_t xr_cpu_dead_frame;                            // mirrors _skr_vk.frame
static uint64_t xr_cpu_dead_accum_ticks;                      // accumulator for current frame

void openxr_cpu_dead_commit() {
	xr_cpu_dead_ring[xr_cpu_dead_frame % SKR_MAX_FRAMES_IN_FLIGHT] = xr_cpu_dead_accum_ticks;
	xr_cpu_dead_accum_ticks = 0;
	xr_cpu_dead_frame++;
}

uint64_t openxr_cpu_dead_time_us() {
	// Match sk_renderer's read offset: it reads (flight_idx + 1) % N, which
	// is the same slot index our (frame + 1) % N selects.
	return (uint64_t)stm_us(xr_cpu_dead_ring[(xr_cpu_dead_frame + 1) % SKR_MAX_FRAMES_IN_FLIGHT]);
}

///////////////////////////////////////////

struct swapchain_t {
	XrSwapchain          handle;
	int32_t              width;
	int32_t              height;
	uint32_t             backbuffer_count; // The number of backbuffer surfaces the swapchain has
	XrSwapchainImage    *backbuffers;
	tex_t               *textures;
	bool32_t             acquired;
};
// Callers wait on render_gpu_future first, since xrDestroySwapchain needs all
// GPU work on it done, including the runtime's barriers on our queue.
void swapchain_delete(swapchain_t *swapchain) {
	if (swapchain->textures) {
		for (size_t s = 0; s < swapchain->backbuffer_count; s++)
			tex_release(swapchain->textures[s]);
	}
	if (swapchain->handle)
		xrDestroySwapchain(swapchain->handle);
	sk_free(swapchain->backbuffers  );
	sk_free(swapchain->textures);
	*swapchain = {};
}

// One of these exists per view when the runtime won't take a layered
// swapchain, see device_display_t::per_eye.
struct display_eye_t {
	swapchain_t         swapchain_color;
	swapchain_t         swapchain_depth;
	pipeline_surface_id render_surface;
	int32_t             render_surface_tex;
};

struct device_display_t {
	XrViewConfigurationType      type;
	XrEnvironmentBlendMode       blend;
	display_blend_               valid_blends;
	XrCompositionLayerProjection projection_layer;
	bool32_t                     active;
	float                        render_scale;
	int32_t                      multisample;

	swapchain_t swapchain_color;
	swapchain_t swapchain_depth;
	pipeline_surface_id              render_surface;
	int32_t                          render_surface_tex;

	// Some runtimes (notably the Android XR emulator) reject layered
	// swapchains, where one array layer per view is the normal layout. In that
	// case we fall back to one single layer swapchain per view, and this is
	// the state that tracks it.
	bool32_t       per_eye;
	int32_t        eye_count;
	display_eye_t *eyes;

	uint32_t                          view_cap;
	XrView                           *view_xr;
	XrViewConfigurationView          *view_configs;
	XrCompositionLayerProjectionView *view_layers;
	XrCompositionLayerDepthInfoKHR   *view_depths;
	matrix                           *view_transforms;
	matrix                           *view_projections;
};
void device_display_delete(device_display_t *display) {
	if (display->render_surface >= 0)
		render_pipeline_surface_destroy(display->render_surface);
	for (int32_t i = 0; i < display->eye_count; i++) {
		if (display->eyes[i].render_surface >= 0)
			render_pipeline_surface_destroy(display->eyes[i].render_surface);
	}
	future_wait(render_gpu_future());
	swapchain_delete(&display->swapchain_color);
	swapchain_delete(&display->swapchain_depth);
	for (int32_t i = 0; i < display->eye_count; i++) {
		swapchain_delete(&display->eyes[i].swapchain_color);
		swapchain_delete(&display->eyes[i].swapchain_depth);
	}
	display->eye_count = 0;
	sk_free(display->eyes);
	sk_free(display->view_xr);
	sk_free(display->view_configs);
	sk_free(display->view_layers);
	sk_free(display->view_depths);
	sk_free(display->view_transforms);
	sk_free(display->view_projections);
	*display = {};
}

///////////////////////////////////////////

int32_t    xr_display_primary_idx    = -1;
system_t*  xr_render_sys             = nullptr;
int64_t    xr_preferred_color_format = -1;
int64_t    xr_preferred_depth_format = -1;

array_t<device_display_t>                          xr_displays           = {};
array_t<device_display_t>                          xr_displays_2nd       = {};
array_t<XrSecondaryViewConfigurationStateMSFT>     xr_display_2nd_states = {};
array_t<XrSecondaryViewConfigurationLayerInfoMSFT> xr_display_2nd_layers = {};

array_t<uint8_t> xr_end_frame_chain_bytes  = {};
array_t<size_t>  xr_end_frame_chain_offset = {};

array_t<uint8_t> xr_compositor_bytes      = {};
array_t<size_t>  xr_compositor_layers     = {};
array_t<int32_t> xr_compositor_layer_sort = {};

array_t<XrCompositionLayerBaseHeader*> xr_compositor_layer_ptrs = {};
array_t<XrCompositionLayerBaseHeader*> xr_compositor_2nd_layer_ptrs = {};

// Runtimes are expected to support one array layer per view in a swapchain,
// but a few don't. The Android XR emulator's gralloc can't allocate layered
// hardware buffers at all, so it rejects any arraySize > 1 swapchain with
// XR_ERROR_FEATURE_UNSUPPORTED. -1 is unknown, 0 unsupported, 1 supported.
static int32_t xr_layered_swapchains = -1;

///////////////////////////////////////////

// The number of pipeline surfaces a display is currently rendering through.
static int32_t display_surface_count(device_display_t *display) {
	return display->per_eye ? display->eye_count : 1;
}
static pipeline_surface_id display_surface(device_display_t *display, int32_t index) {
	return display->per_eye ? display->eyes[index].render_surface : display->render_surface;
}
// Dimensions of the swapchains a display is currently built with, in either
// mode. Both are 0 before the first build.
static void display_swapchain_size(device_display_t *display, int32_t *out_w, int32_t *out_h) {
	if (display->per_eye && display->eye_count > 0) {
		*out_w = display->eyes[0].swapchain_color.width;
		*out_h = display->eyes[0].swapchain_color.height;
	} else {
		*out_w = display->swapchain_color.width;
		*out_h = display->swapchain_color.height;
	}
}
// True once a display has every pipeline surface it needs to render with.
static bool32_t display_surfaces_valid(device_display_t *display) {
	if (display->per_eye) {
		if (display->eye_count <= 0) return false;
		for (int32_t i = 0; i < display->eye_count; i++) {
			if (display->eyes[i].render_surface < 0) return false;
		}
		return true;
	}
	return display->render_surface >= 0;
}

///////////////////////////////////////////

bool openxr_create_swapchain (swapchain_t *out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height);
static XrResult openxr_create_swapchain_raw(swapchain_t *out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height);
void openxr_preferred_format (int64_t *out_color, int64_t *out_depth);
bool openxr_preferred_blend  (XrViewConfigurationType view_type, display_blend_ preference, display_blend_* out_valid, XrEnvironmentBlendMode* out_blend);

bool openxr_display_create           (XrViewConfigurationType view_type, device_display_t* out_display);
bool openxr_display_locate           (device_display_t* display, XrTime at_time);
bool openxr_display_swapchain_update (device_display_t* display);
bool openxr_display_swapchain_acquire(device_display_t* display, color128 color, render_layer_ render_filter);
void openxr_display_swapchain_release(device_display_t* display);

///////////////////////////////////////////

void backend_openxr_composition_layer(void *XrCompositionLayerBaseHeader, int32_t layer_size, int32_t sort_order) {
	int32_t start = xr_compositor_bytes.count;
	xr_compositor_bytes.add_range((uint8_t*)XrCompositionLayerBaseHeader, layer_size);

	int32_t id = xr_compositor_layer_sort.binary_search(sort_order);
	if (id < 0) id = ~id;
	xr_compositor_layer_sort.insert(id, sort_order);
	xr_compositor_layers    .insert(id, start);
}

///////////////////////////////////////////

const array_t<XrCompositionLayerBaseHeader *> *compositor_layers_get() {
	xr_compositor_layer_ptrs.clear();
	for (int32_t i = 0; i < xr_compositor_layers.count; i++) {
		xr_compositor_layer_ptrs.add((XrCompositionLayerBaseHeader *)(xr_compositor_bytes.data + xr_compositor_layers[i]));
	}
	return &xr_compositor_layer_ptrs;
}

///////////////////////////////////////////

void xr_extension_structs_clear() {
	xr_compositor_bytes     .clear();
	xr_compositor_layers    .clear();
	xr_compositor_layer_sort.clear();

	xr_end_frame_chain_bytes .clear();
	xr_end_frame_chain_offset.clear();
}

///////////////////////////////////////////

void xr_extension_structs_free() {
	xr_compositor_bytes     .free();
	xr_compositor_layers    .free();
	xr_compositor_layer_sort.free();
	xr_compositor_layer_ptrs.free();

	xr_end_frame_chain_bytes .free();
	xr_end_frame_chain_offset.free();
}

///////////////////////////////////////////

void backend_openxr_end_frame_chain(void* XrBaseHeader, int32_t data_size) {
	int32_t start = xr_end_frame_chain_bytes.count;
	xr_end_frame_chain_bytes.add_range((uint8_t*)XrBaseHeader, data_size);
	xr_end_frame_chain_offset.add(start);
}

///////////////////////////////////////////

void xr_chain_insert_extensions(XrBaseHeader *to_type, array_t<uint8_t> ext_bytes, array_t<size_t> ext_offsets) {
	for (int32_t i = 0; i < ext_offsets.count; i++) {
		xr_insert_next(to_type, (XrBaseHeader*)&ext_bytes.data[ext_offsets[i]]);
	}
}

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

const char *openxr_blend_name(XrEnvironmentBlendMode blend) {
	switch (blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      return "Opaque";
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    return "Additive";
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: return "Alpha Blend";
	default: return "N/A";
	}
}

///////////////////////////////////////////

bool32_t xr_blend_valid(display_blend_ blend) {
	if (xr_display_primary_idx == -1) return false;
	display_blend_ valid = xr_displays[xr_display_primary_idx].valid_blends;

	if (blend == display_blend_any_transparent) {
		if      (valid & display_blend_additive) { blend = display_blend_additive; }
		else if (valid & display_blend_blend   ) { blend = display_blend_blend;    }
		else                                     { blend = display_blend_none;     }
	}

	return (blend & valid) != 0;
}

///////////////////////////////////////////

bool32_t xr_set_blend(display_blend_ blend) {
	if (xr_display_primary_idx == -1) return false;
	device_display_t* display = &xr_displays[xr_display_primary_idx];
	display_blend_    valid   = display->valid_blends;

	if (blend == display_blend_any_transparent) {
		if      (valid & display_blend_additive) { blend = display_blend_additive; }
		else if (valid & display_blend_blend   ) { blend = display_blend_blend;    }
		else                                     { blend = display_blend_none;     }
	}

	if ((blend & valid) == 0) {
		log_err("Set display blend to an invalid mode!");
		return false;
	}
	device_data.display_blend = blend;

	switch (blend) {
	case display_blend_additive: display->blend = XR_ENVIRONMENT_BLEND_MODE_ADDITIVE;    break;
	case display_blend_blend:    display->blend = XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND; break;
	case display_blend_opaque:   display->blend = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;      break;
	default: abort(); break; // Should not be reachable
	}

	return true;
}

///////////////////////////////////////////

bool openxr_views_create() {
	xr_render_sys          = systems_find("FrameRender");
	xr_display_primary_idx = -1;

	// OpenXR has a preferred swapchain format, this'll find one that matches
	// with formats we support.
	openxr_preferred_format(&xr_preferred_color_format, &xr_preferred_depth_format);

	// Tell OpenXR what sort of color space we're rendering in
	xr_ext_fb_colorspace_set_for(tex_get_tex_format(xr_preferred_color_format));

	// Find all the valid view configurations
	uint32_t count = 0;
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, 0, &count, nullptr), "xrEnumerateViewConfigurations");
	XrViewConfigurationType *types = sk_malloc_t(XrViewConfigurationType, count);
	xr_check(xrEnumerateViewConfigurations(xr_instance, xr_system_id, count, &count, types), "xrEnumerateViewConfigurations");

	// Initialize each valid view configuration
	for (uint32_t t = 0; t < count; t++) {
		switch (types[t]) {
		case XR_PRIMARY_CONFIG: {
			// A primary display surface, this is what the user will be seeing
			// in headset. Swapchain creation is deferred to
			// openxr_views_create_swapchains so that extensions like depth
			// composition are available.
			device_display_t display = {};
			if (!openxr_display_create(types[t], &display))
				return false;

			int32_t display_idx = xr_displays.add(display);
			if (xr_display_primary_idx == -1)
				xr_display_primary_idx = display_idx;
		} break;
		case XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT: {
			// A HoloLens recording view, this is usually surface that is drawn
			// for compositing with the camera feed.
			device_display_t display = {};
			if (!openxr_display_create(types[t], &display))
				return false;
			display.active = false;

			XrSecondaryViewConfigurationStateMSFT state = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT };
			state.active                = false;
			state.viewConfigurationType = types[t];

			xr_displays_2nd      .add(display);
			xr_display_2nd_states.add(state);
		} break;
		default:break;
		}
	}
	sk_free(types);

	if (xr_display_primary_idx == -1) {
		log_info("No primary display configuration was found!");
		return false;
	}

	// Update the display info right away, some of this gets updated each draw,
	// but most users will want this info as soon as the session begins. If we
	// skip doing this here, then there will be a single frame delay where the
	// information isn't present.
	switch (xr_displays[xr_display_primary_idx].blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      device_data.display_blend = display_blend_opaque;   break;
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    device_data.display_blend = display_blend_additive; break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: device_data.display_blend = display_blend_blend;    break;
	default:                                    device_data.display_blend = display_blend_none;     break;
	}

	return true;
}

///////////////////////////////////////////

bool openxr_views_create_swapchains() {
	for (int32_t i = 0; i < xr_displays.count; i++) {
		if (!openxr_display_swapchain_update(&xr_displays[i])) {
			log_fail_reason(80, log_error, "Couldn't create OpenXR display swapchains!");
			return false;
		}
	}

	display_swapchain_size(&xr_displays[xr_display_primary_idx], &device_data.display_width, &device_data.display_height);

	return true;
}

///////////////////////////////////////////

bool32_t xr_view_type_valid(XrViewConfigurationType type) {
	for (int32_t i = 0; i < xr_displays.count; i++) {
		if (xr_displays[i].type == type)
			return true;
	}
	for (int32_t i = 0; i < xr_displays_2nd.count; i++) {
		if (xr_displays_2nd[i].type == type)
			return true;
	}
	return false;
}

///////////////////////////////////////////

void openxr_views_destroy() {
	openxr_step_time_reset();
	xr_layered_swapchains = -1;

	for (int32_t i = 0; i < xr_displays.count; i++) {
		device_display_delete(&xr_displays[i]);
	}
	for (int32_t i = 0; i < xr_displays_2nd.count; i++) {
		device_display_delete(&xr_displays_2nd[i]);
	}
	xr_display_primary_idx = -1;

	xr_displays          .free();
	xr_displays_2nd      .free();
	xr_display_2nd_states.free();
	xr_display_2nd_layers.free();
	xr_compositor_2nd_layer_ptrs.free();
	xr_extension_structs_free();
}

///////////////////////////////////////////

bool openxr_display_create(XrViewConfigurationType view_type, device_display_t *out_display) {
	*out_display = {};

	// Get the surface format information before we create surfaces!
	if (!openxr_preferred_blend(view_type, sk_get_settings_ref()->blend_preference, &out_display->valid_blends, &out_display->blend)) return false;

	// Debug print the view and format info
	tex_format_ color_fmt = tex_get_tex_format(xr_preferred_color_format);
	tex_format_ depth_fmt = tex_get_tex_format(xr_preferred_depth_format);
	log_diagf("Creating view: <~grn>%s<~clr> color:<~grn>%s<~clr> depth:<~grn>%s<~clr> blend:<~grn>%s<~clr>",
		openxr_view_name(view_type),
		render_fmt_name(color_fmt),
		render_fmt_name(depth_fmt),
		openxr_blend_name(out_display->blend));

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	xr_check(xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, view_type, 0, &out_display->view_cap, nullptr),
		"xrEnumerateViewConfigurationViews");

	// Extract information from the views we got
	out_display->type             = view_type;
	out_display->active           = true;
	out_display->render_surface   = -1;
	out_display->projection_layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	out_display->view_xr          = sk_malloc_t(XrView,                           out_display->view_cap);
	out_display->view_configs     = sk_malloc_t(XrViewConfigurationView,          out_display->view_cap);
	out_display->view_layers      = sk_malloc_t(XrCompositionLayerProjectionView, out_display->view_cap);
	out_display->view_depths      = sk_malloc_t(XrCompositionLayerDepthInfoKHR,   out_display->view_cap);
	out_display->view_projections = sk_malloc_t(matrix,                           out_display->view_cap);
	out_display->view_transforms  = sk_malloc_t(matrix,                           out_display->view_cap);
	out_display->eyes             = sk_malloc_t(display_eye_t,                    out_display->view_cap);
	for (uint32_t i = 0; i < out_display->view_cap; i++) {
		out_display->view_xr         [i] = { XR_TYPE_VIEW };
		out_display->view_configs    [i] = { XR_TYPE_VIEW_CONFIGURATION_VIEW };
		out_display->view_depths     [i] = { XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR };
		out_display->view_layers     [i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		out_display->view_projections[i] = matrix_identity;
		out_display->view_transforms [i] = matrix_identity;
		out_display->eyes            [i] = {};
		out_display->eyes            [i].render_surface = -1;
	}
	if (view_type == XR_PRIMARY_CONFIG) {
		out_display->multisample  = render_get_multisample();
		out_display->render_scale = render_get_scaling();
	} else {
		out_display->multisample  = 1;
		out_display->render_scale = 1;
	}

	return true;
}

///////////////////////////////////////////

// Wraps a swapchain's native images in tex_t objects, so the renderer can draw
// into them. `array_count` is how many views share this swapchain.
static void openxr_swapchain_textures_create(swapchain_t *sc_color, swapchain_t *sc_depth, bool has_depth_sc, uint32_t array_count) {
	// Create texture objects if we don't have 'em
	if (sc_color->textures == nullptr) {
		sc_color->textures = sk_malloc_t(tex_t, (size_t)sc_color->backbuffer_count);
		memset(sc_color->textures, 0, sizeof(tex_t) * sc_color->backbuffer_count);
		if (has_depth_sc) {
			sc_depth->textures = sk_malloc_t(tex_t, (size_t)sc_depth->backbuffer_count);
			memset(sc_depth->textures, 0, sizeof(tex_t) * sc_depth->backbuffer_count);
		}

		for (uint32_t i = 0; i < sc_color->backbuffer_count; i++) {
			sc_color->textures[i] = tex_create(tex_type_rendertarget, tex_get_tex_format(xr_preferred_color_format));

			char           name[64];
			static int32_t target_index = 0;
			target_index++;
			snprintf(name, sizeof(name), "sk/render/colortarget_%d", target_index);
			tex_set_id(sc_color->textures[i], name);

			if (has_depth_sc) {
				sc_depth->textures[i] = tex_create(tex_type_zbuffer, tex_get_tex_format(xr_preferred_depth_format));
				snprintf(name, sizeof(name), "sk/render/depthtarget_%d", target_index);
				tex_set_id(sc_depth->textures[i], name);
			}
		}
	}

	// Update or set the native textures
	for (uint32_t back = 0; back < sc_color->backbuffer_count; back++) {
		void *native_surface_col = (void*)sc_color->backbuffers[back].image;
		tex_set_surface(sc_color->textures[back], native_surface_col, (tex_type_)(tex_type_rendertarget | tex_type_attachment_internal), xr_preferred_color_format, sc_color->width, sc_color->height, array_count, 1);

		if (has_depth_sc) {
			void *native_surface_depth = (void*)sc_depth->backbuffers[back].image;
			tex_set_surface(sc_depth->textures[back], native_surface_depth, (tex_type_)(tex_type_zbuffer | tex_type_attachment_internal), xr_preferred_depth_format, sc_depth->width, sc_depth->height, array_count, 1);
			tex_set_zbuffer(sc_color->textures[back], sc_depth->textures[back]);
		} else {
			// Provide a write-only depth buffer for z-testing when
			// there's no XR depth swapchain. MSAA paths create their
			// own depth through the pipeline surface.
			tex_add_zbuffer(sc_color->textures[back], tex_get_tex_format(xr_preferred_depth_format));
		}
	}
}

///////////////////////////////////////////

// Creates the pipeline surface a swapchain renders through if it doesn't exist
// yet, then updates it for the current dimensions and MSAA. Returns the
// surface id, which the caller is expected to store back.
static pipeline_surface_id openxr_swapchain_surface_update(pipeline_surface_id surface, swapchain_t *sc_color, uint32_t array_count, int32_t msaa) {
	// Create pipeline surface if it doesn't exist yet. This persists
	// across swapchain recreations.
	if (surface < 0) {
		surface = render_pipeline_surface_create(
			tex_get_tex_format(xr_preferred_color_format),
			tex_get_tex_format(xr_preferred_depth_format),
			array_count);
	}

	// Update pipeline surface for current MSAA / dimensions. When switching
	// between MSAA and direct modes, we need to clear the old tex first so
	// resize doesn't try to modify a swapchain backbuffer it doesn't own.
	if (msaa > 1) {
		render_pipeline_surface_set_tex(surface, nullptr);
		render_pipeline_surface_resize (surface, sc_color->width, sc_color->height, msaa);
	} else {
		render_pipeline_surface_set_tex(surface, sc_color->textures[0]);
	}
	return surface;
}

///////////////////////////////////////////

bool openxr_display_swapchain_update(device_display_t *display) {
	swapchain_t *sc_color = &display->swapchain_color;
	swapchain_t *sc_depth = &display->swapchain_depth;

	// Check if the latest configuration is different from what we've already
	// set up.
	uint32_t view_count = 0;
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, display->type, display->view_cap, &view_count, display->view_configs);
	int32_t w = (int32_t)(display->view_configs[0].recommendedImageRectWidth  * display->render_scale);
	int32_t h = (int32_t)(display->view_configs[0].recommendedImageRectHeight * display->render_scale);
	int32_t s = display->multisample;
	if (display->render_scale != 1.0f) {
		const int32_t quantize = 4;
		w = (w / quantize) * quantize;
		h = (h / quantize) * quantize;
	}
	if (w > (int32_t)display->view_configs[0].maxImageRectWidth ) w = display->view_configs[0].maxImageRectWidth;
	if (h > (int32_t)display->view_configs[0].maxImageRectHeight) h = display->view_configs[0].maxImageRectHeight;

	int32_t array_count = display->view_cap;
	bool has_depth_sc = xr_ext_composition_depth_available();

	// Recreate the swapchains when the dimensions changed, when the runtime
	// made us switch between layered and per-view swapchains, or when we don't
	// have any yet. MSAA is handled entirely through intermediate render
	// pipeline surfaces, so multisample changes don't require recreation.
	int32_t have_w, have_h;
	display_swapchain_size(display, &have_w, &have_h);
	bool want_per_eye  = xr_layered_swapchains == 0;
	bool have_surfaces = display->per_eye ? display->eye_count > 0 : display->render_surface >= 0;
	if (have_w != w || have_h != h || display->per_eye != want_per_eye || have_surfaces == false) {
		// Recreating deletes the old swapchains, see swapchain_delete
		if (have_surfaces) future_wait(render_gpu_future());

		if (want_per_eye == false) {
			// One array layer per view is the layout runtimes are expected
			// to support, so that's what we try first.
			XrResult result = openxr_create_swapchain_raw(sc_color, display->type, true, array_count, xr_preferred_color_format, w, h);
			if (XR_FAILED(result)) {
				// Some runtimes can't allocate layered buffers at all, the
				// Android XR emulator's gralloc for example, and reject
				// this with XR_ERROR_FEATURE_UNSUPPORTED. One swapchain per
				// view is the fallback.
				log_infof("Runtime rejected a layered swapchain (arraySize %d) [%s], using one swapchain per view instead.",
					array_count, openxr_string(result));
				xr_layered_swapchains = 0;
				want_per_eye          = true;
			} else {
				// Only a multi-view swapchain tells us anything about
				// layered support, a single view one works either way.
				if (array_count > 1) xr_layered_swapchains = 1;
			}
		}

		if (want_per_eye) {
			// Each view gets its own single layer swapchain, surface, and
			// draw. This is more draw calls than a layered swapchain, but
			// it's the only layout these runtimes accept.
			if (display->eye_count != array_count) {
				for (int32_t i = 0; i < display->eye_count; i++) {
					if (display->eyes[i].render_surface >= 0)
						render_pipeline_surface_destroy(display->eyes[i].render_surface);
					display->eyes[i].render_surface = -1;
				}
				display->eye_count = array_count;
			}
			display->per_eye = true;
			for (int32_t i = 0; i < display->eye_count; i++) {
				display_eye_t *eye = &display->eyes[i];
				if (!openxr_create_swapchain(&eye->swapchain_color, display->type, true,  1, xr_preferred_color_format, w, h)) return false;
				if (has_depth_sc) {
					if (!openxr_create_swapchain(&eye->swapchain_depth, display->type, false, 1, xr_preferred_depth_format, w, h)) return false;
				}
				openxr_swapchain_textures_create(&eye->swapchain_color, &eye->swapchain_depth, has_depth_sc, 1);

				eye->render_surface_tex = -1;
				eye->render_surface     = openxr_swapchain_surface_update(eye->render_surface, &eye->swapchain_color, 1, s);
			}
			log_diagf("Set swapchain to %d<~BLK>x<~clr>%d<~BLK>x<~clr>%d for <~grn>%s<~clr> (per-view)",
				w, h, display->eye_count, openxr_view_name(display->type));
		} else {
			// If we ever came from per-view mode, tear those down first.
			if (display->eye_count > 0) {
				for (int32_t i = 0; i < display->eye_count; i++) {
					if (display->eyes[i].render_surface >= 0)
						render_pipeline_surface_destroy(display->eyes[i].render_surface);
					display->eyes[i].render_surface = -1;
					swapchain_delete(&display->eyes[i].swapchain_color);
					swapchain_delete(&display->eyes[i].swapchain_depth);
				}
				display->eye_count = 0;
			}
			display->per_eye = false;
			if (!openxr_create_swapchain(sc_color, display->type, true, array_count, xr_preferred_color_format, w, h)) return false;
			if (has_depth_sc) {
				if (!openxr_create_swapchain(sc_depth, display->type, false, array_count, xr_preferred_depth_format, w, h)) return false;
			}

			log_diagf("Set swapchain to %d<~BLK>x<~clr>%d for <~grn>%s<~clr>", w, h, openxr_view_name(display->type));

			openxr_swapchain_textures_create(sc_color, sc_depth, has_depth_sc, array_count);

			display->render_surface_tex = -1;
			display->render_surface     = openxr_swapchain_surface_update(display->render_surface, sc_color, array_count, s);
		}

		if (display->type == XR_PRIMARY_CONFIG) {
			device_data.display_width  = w;
			device_data.display_height = h;
		}
	} else if (display->per_eye) {
		for (int32_t i = 0; i < display->eye_count; i++)
			display->eyes[i].render_surface = openxr_swapchain_surface_update(display->eyes[i].render_surface, &display->eyes[i].swapchain_color, 1, s);
	} else {
		display->render_surface = openxr_swapchain_surface_update(display->render_surface, sc_color, array_count, s);
	}

	return true;
}

///////////////////////////////////////////

// Creates the swapchain and enumerates its images. Returns the XrResult of the
// first call that failed, and leaves the swapchain empty in that case, so the
// caller can retry with a different configuration.
static XrResult openxr_create_swapchain_raw(swapchain_t *out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height) {
	swapchain_delete(out_swapchain);

	// Create a swapchain for this viewpoint! A swapchain is a set of texture
	// buffers used for displaying to screen, typically this is a backbuffer
	// and a front buffer, one for rendering data to, and one for displaying
	// on-screen. MSAA is handled separately via intermediate render pipeline
	// surfaces and in-tile resolve, so the swapchain is always sampleCount=1.
	XrSwapchainCreateInfo swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
	XrSwapchain           handle         = {};
	swapchain_info.arraySize   = array_size;
	swapchain_info.mipCount    = 1;
	swapchain_info.faceCount   = 1;
	swapchain_info.format      = format;
	swapchain_info.width       = width;
	swapchain_info.height      = height;
	swapchain_info.sampleCount = 1;
	swapchain_info.usageFlags  = color
		? XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT
		: XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	// If it's a secondary view, let OpenXR know
	XrSecondaryViewConfigurationSwapchainCreateInfoMSFT secondary = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT };
	if (type == XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT) {
		secondary.viewConfigurationType = type;
		swapchain_info.next = &secondary;
	}

	XrResult result = xrCreateSwapchain(xr_session, &swapchain_info, &handle);
	if (XR_FAILED(result)) return result;

	// Find out how many textures were generated for the swapchain
	uint32_t backbuffer_count = 0;
	result = xrEnumerateSwapchainImages(handle, 0, &backbuffer_count, nullptr);
	if (XR_FAILED(result)) { xrDestroySwapchain(handle); return result; }

	// We'll want to track our own information about the swapchain, so we can
	// draw stuff onto it!
	out_swapchain->width  = swapchain_info.width;
	out_swapchain->height = swapchain_info.height;
	out_swapchain->handle = handle;
	if (out_swapchain->backbuffer_count != backbuffer_count) {
		out_swapchain->backbuffer_count = backbuffer_count;
		out_swapchain->backbuffers      = sk_malloc_t(XrSwapchainImage, backbuffer_count);
	}
	for (uint32_t i=0; i<backbuffer_count; i++) {
		out_swapchain->backbuffers[i] = { XR_TYPE_SWAPCHAIN_IMAGE };
	}

	result = xrEnumerateSwapchainImages(out_swapchain->handle, backbuffer_count, &backbuffer_count, (XrSwapchainImageBaseHeader *)out_swapchain->backbuffers);
	if (XR_FAILED(result)) {
		// No textures have been created yet, so this can't go through
		// swapchain_delete.
		xrDestroySwapchain(out_swapchain->handle);
		sk_free(out_swapchain->backbuffers);
		*out_swapchain = {};
		return result;
	}

	return XR_SUCCESS;
}

///////////////////////////////////////////

bool openxr_create_swapchain(swapchain_t *out_swapchain, XrViewConfigurationType type, bool color, uint32_t array_size, int64_t format, int32_t width, int32_t height) {
	xr_check(openxr_create_swapchain_raw(out_swapchain, type, color, array_size, format, width, height),
		"xrCreateSwapchain");
	return true;
}

///////////////////////////////////////////

void openxr_preferred_format(int64_t *out_color_dx, int64_t *out_depth_dx) {
	int64_t pixel_formats[] = {
		tex_fmt_to_native(tex_format_rgba32),
		tex_fmt_to_native(tex_format_bgra32),
		tex_fmt_to_native(tex_format_rgb10a2),
		tex_fmt_to_native(tex_format_rgba32_linear),
		tex_fmt_to_native(tex_format_bgra32_linear) };

	int64_t depth_formats[] = {
		tex_fmt_to_native(tex_format_depth16),
		tex_fmt_to_native(tex_format_depth32),
		tex_fmt_to_native(tex_format_depthstencil)};

	// Get the list of formats OpenXR would like
	uint32_t count = 0;
	xrEnumerateSwapchainFormats(xr_session, 0, &count, nullptr);
	int64_t *formats = sk_malloc_t(int64_t, count);
	xrEnumerateSwapchainFormats(xr_session, count, &count, formats);

	log_diag("<~BLK>____________________<~clr>");
	log_diag("<~BLK>| <~YLW>Swapchain Formats<~clr> ");
	log_diag("<~BLK>|-------------------<~clr>");
	for (uint32_t i = 0; i < count; i++) {
		bool found = false;
		for (int32_t f = 0; !found && f < _countof(pixel_formats); f++) found = formats[i] == pixel_formats[f];
		for (int32_t f = 0; !found && f < _countof(depth_formats); f++) found = formats[i] == depth_formats[f];

		const char* name = render_fmt_name(tex_get_tex_format(formats[i]));
		if (strcmp(name, "none"   ) == 0) continue;
		if (strcmp(name, "Unknown") == 0) log_diagf("<~BLK>|<~clr> %s0x%X<~clr>", found?"+":" <~BLK>", formats[i]);
		else                              log_diagf("<~BLK>|<~clr> %s%s<~clr>",   found?"+":" <~BLK>", name);
	}
	log_diag("<~BLK>|___________________<~clr>");

	// According to the OpenXR spec, formats should be ordered by the runtime's
	// preference for that format. This means that if we can, we should choose
	// the first format from the list, if we can use it! Mobile platforms will
	// usually have Depth16 listed before Depth32, and PC platforms will have 
	// Depth32 listed before Depth16, etc.

	// Check those against our formats, prefer OpenXR's pick for color format
	*out_color_dx = 0;
	for (uint32_t i=0; i<count; i++) {
		for (int32_t f=0; *out_color_dx == 0 && f<_countof(pixel_formats); f++) {
			if (formats[i] == pixel_formats[f]) {
				*out_color_dx = pixel_formats[f];
				break;
			}
		}
	}
	// If the user specified a color format we can check if it's present, and if
	// so, overwrite OpenXR's preference.
	if (sk_get_settings().color_format != tex_format_none) {
		int64_t native_color = tex_fmt_to_native(sk_get_settings().color_format);
		bool    found        = false;
		for (uint32_t i = 0; i < count; i++) {
			if (native_color == formats[i]) {
				*out_color_dx = formats[i];
				found         = true;
				break;
			}
		}
		if (!found) log_infof("Couldn't find explicit %s format '%s' for swapchains. Falling back to runtime's preference.", "color", render_fmt_name(sk_get_settings().color_format));
	}

	// Check those against our formats, prefer OpenXR's pick for depth format.
	*out_depth_dx = 0;
	for (uint32_t i = 0; i < count; i++) {
		for (int32_t f = 0; *out_depth_dx == 0 && f < _countof(depth_formats); f++) {
			if (formats[i] == depth_formats[f]) {
				*out_depth_dx = depth_formats[f];
				break;
			}
		}
	}
	// If the user specified a depth mode we can check if it's present, and if
	// so, overwrite OpenXR's preference.
	if (sk_get_settings().depth_mode != depth_mode_default) {
		int64_t native_depth = tex_fmt_to_native(render_preferred_depth_fmt());
		bool    found        = false;
		for (uint32_t i = 0; i < count; i++) {
			if (native_depth == formats[i]) {
				*out_depth_dx = formats[i];
				found         = true;
				break;
			}
		}
		if (!found) log_infof("Couldn't find explicit %s format '%s' for swapchains. Falling back to runtime's preference.", "depth", render_fmt_name(sk_get_settings().color_format));
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
		"xrEnumerateEnvironmentBlendModes");
	blend_modes = sk_malloc_t(XrEnvironmentBlendMode, blend_count);
	xr_check(xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, view_type, blend_count, &blend_count, blend_modes),
		"xrEnumerateEnvironmentBlendModes");
	
	*out_blend = XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;
	*out_valid = display_blend_none;
	for (uint32_t i = 0; i < blend_count; i++) {
		display_blend_ curr = display_blend_none;
		switch (blend_modes[i]) {
		case XR_ENVIRONMENT_BLEND_MODE_OPAQUE:      curr = display_blend_opaque;   break;
		case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE:    curr = display_blend_additive; break;
		case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: curr = display_blend_blend;    break;
		default: break;
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

// The app step runs before its frame's xrWaitFrame, so the next frame steps
// by the gap between the last two reported display times, one frame behind
// the display. A skipped frame shows up as a double step on the frame after.
static XrTime xr_pace_prev = 0;  // predictedDisplayTime from the report before this one

void openxr_step_time_reset() {
	xr_pace_prev = 0;
}

///////////////////////////////////////////

bool openxr_render_frame() {
	profiler_zone();

	// Block until the previous frame is finished displaying, and is ready for
	// another one. Also returns a prediction of when the next frame will be
	// displayed, for use with predicting locations of controllers, viewpoints,
	// etc.
	XrFrameWaitInfo                            wait_info        = { XR_TYPE_FRAME_WAIT_INFO };
	XrFrameState                               frame_state      = { XR_TYPE_FRAME_STATE };
	XrSecondaryViewConfigurationFrameStateMSFT secondary_states = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT };
	if (xr_display_2nd_states.count > 0) {
		secondary_states.viewConfigurationCount  = xr_display_2nd_states.count;
		secondary_states.viewConfigurationStates = xr_display_2nd_states.data;
		frame_state.next = &secondary_states;
	}

	{
		profiler_zone_name("xrWaitFrame");

		uint64_t dead_start = stm_now();
		XrResult xrWaitFrameResult = xrWaitFrame(xr_session, &wait_info, &frame_state);
		xr_cpu_dead_accum_ticks += stm_diff(stm_now(), dead_start);

		if (xrWaitFrameResult == XR_ERROR_SESSION_LOST) {
			sk_quit(quit_reason_session_lost);
		}
		xr_check(xrWaitFrameResult, "xrWaitFrame");
	}

	// Don't track sync time, start the frame timer after xrWaitFrame
	xr_render_sys->profile_frame_start = stm_now();

	// Check each secondary display to see if it's active or not. Also refresh
	// any active display whose recommended view config values the runtime has
	// reported as changed (XR_EXT_view_configuration_views_change).
	for (int32_t i = 0; i < xr_displays_2nd.count; i++) {
		bool became_active = false;
		if (xr_displays_2nd[i].active != (bool32_t)xr_display_2nd_states[i].active) {
			xr_displays_2nd[i].active = (bool32_t)xr_display_2nd_states[i].active;
			became_active = xr_displays_2nd[i].active;
		}
		bool views_changed = xr_ext_view_config_views_change_consume(xr_displays_2nd[i].type);
		if (xr_displays_2nd[i].active && (became_active || views_changed)) {
			openxr_display_swapchain_update(&xr_displays_2nd[i]);
		}
	}
	device_display_t* primary = &xr_displays[xr_display_primary_idx];
	if (primary->render_scale != render_get_scaling() || primary->multisample != render_get_multisample() ||
		xr_ext_view_config_views_change_consume(primary->type)) {
		primary->render_scale = render_get_scaling();
		primary->multisample  = render_get_multisample();
		openxr_display_swapchain_update(primary);
	}

	// Must be called before any rendering is done! This can return some
	// interesting flags, like XR_SESSION_VISIBILITY_UNAVAILABLE, which means
	// we could skip rendering this frame and call xrEndFrame right away.
	XrFrameBeginInfo begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	{
		profiler_zone_name("xrBeginFrame");

		uint32_t queue_family = skr_get_vk_graphics_queue_family();
		skr_vk_queue_lock(queue_family);
		XrResult begin_result = xrBeginFrame(xr_session, &begin_info);
		skr_vk_queue_unlock(queue_family);
		xr_check(begin_result,
			"xrBeginFrame");
	}

	xr_time = frame_state.predictedDisplayTime;
	if (frame_state.predictedDisplayPeriod > 0)
		device_data.display_refresh_rate = 1e9f / (float)frame_state.predictedDisplayPeriod;
	frame_pacer_step((uint64_t)frame_state.predictedDisplayTime, (uint64_t)xr_pace_prev);
	xr_pace_prev = frame_state.predictedDisplayTime;

	// Meta's environment depth images are only valid when acquired during a running
	// OpenXR frame (between xrBeginFrame and xrEndFrame)
	xr_ext_meta_environment_depth_update_frame(xr_time);

	// Execute any code that's dependent on the predicted time, such as
	// updating the location of controller models. This often includes drawing,
	// which means this must come before our render usage check.
	input_step_late();

	// If there's nothing to render, we may want to totally skip all projection
	// layers entirely.
	bool render_displays =
		(xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED) &&
		(sk_get_settings_ref()->omit_empty_frames == false || render_list_item_count(render_get_primary_list()) != 0);
	if (render_displays) {
		profiler_zone_name("Render Setup");
		// Set up the primary displays
		for (int32_t i = 0; i < xr_displays.count; i++) {
			device_display_t* display = &xr_displays[i];
			if (!display_surfaces_valid(display)) continue;
			for (int32_t e = 0; e < display_surface_count(display); e++)
				render_pipeline_surface_set_enabled(display_surface(display, e), display->active);
			if (!display->active) continue;

			for (int32_t e = 0; e < display_surface_count(display); e++)
				render_pipeline_surface_set_viewport_scale(display_surface(display, e), render_get_viewport_scaling());

			if (!openxr_display_locate           (display, xr_time) ||
				!openxr_display_swapchain_acquire(display, render_get_clear_color_ln(), render_get_filter()))
			{
				for (int32_t e = 0; e < display_surface_count(display); e++)
					render_pipeline_surface_set_enabled(display_surface(display, e), false);
				continue;
			}
			;

			if (i == xr_display_primary_idx) {
				device_data.display_fov.right  = display->view_xr[0].fov.angleRight * rad2deg;
				device_data.display_fov.left   = display->view_xr[0].fov.angleLeft  * rad2deg;
				device_data.display_fov.top    = display->view_xr[0].fov.angleUp    * rad2deg;
				device_data.display_fov.bottom = display->view_xr[0].fov.angleDown  * rad2deg;
			}
			backend_openxr_composition_layer(&display->projection_layer, sizeof(XrCompositionLayerProjection), 0);
		}

		// Set up any secondary displays
		xr_display_2nd_layers.clear();
		xr_compositor_2nd_layer_ptrs.clear();
		for (int32_t i = 0; i < xr_displays_2nd.count; i++) {
			device_display_t* display = &xr_displays_2nd[i];
			if (!display_surfaces_valid(display)) continue;
			for (int32_t e = 0; e < display_surface_count(display); e++)
				render_pipeline_surface_set_enabled(display_surface(display, e), display->active);
			if (!display->active) continue;

			for (int32_t e = 0; e < display_surface_count(display); e++)
				render_pipeline_surface_set_viewport_scale(display_surface(display, e), render_get_viewport_scaling());

			if (!openxr_display_locate           (display, xr_time) ||
				!openxr_display_swapchain_acquire(display, render_get_clear_color_ln(), render_get_capture_filter()))
			{
				for (int32_t e = 0; e < display_surface_count(display); e++)
					render_pipeline_surface_set_enabled(display_surface(display, e), false);
				continue;
			}

			xr_compositor_2nd_layer_ptrs.add((XrCompositionLayerBaseHeader*)&display->projection_layer);

			XrSecondaryViewConfigurationLayerInfoMSFT layer2nd = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT };
			layer2nd.viewConfigurationType = display->type;
			layer2nd.environmentBlendMode  = display->blend;
			layer2nd.layerCount            = 1;
			layer2nd.layers                = xr_compositor_2nd_layer_ptrs.data + (xr_compositor_2nd_layer_ptrs.count - 1);
			xr_display_2nd_layers.add(layer2nd);
		}
		if (xr_display_2nd_layers.count > 0) {
			XrSecondaryViewConfigurationFrameEndInfoMSFT end_second = { XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT };
			end_second.viewConfigurationCount      = xr_display_2nd_layers.count;
			end_second.viewConfigurationLayersInfo = xr_display_2nd_layers.data;
			backend_openxr_end_frame_chain(&end_second, sizeof(end_second));
		}
	} else {
		// Disable all surfaces
		for (int32_t i = 0; i < xr_displays.count; i++) {
			if (!display_surfaces_valid(&xr_displays[i])) continue;
			for (int32_t e = 0; e < display_surface_count(&xr_displays[i]); e++)
				render_pipeline_surface_set_enabled(display_surface(&xr_displays[i], e), false);
		}
	}

	{
		profiler_zone_name("Render");

		render_pipeline_draw();
	}

	// Release the swapchains for all active displays
	// Note: MSAA resolve and depth buffer discard are handled by sk_renderer
	// through Vulkan render pass operations (in-tile resolve for MSAA).
	if (render_displays) {
		for (int32_t i = 0; i < xr_displays    .count; i++) openxr_display_swapchain_release(&xr_displays    [i]);
		for (int32_t i = 0; i < xr_displays_2nd.count; i++) openxr_display_swapchain_release(&xr_displays_2nd[i]);
	}

	// End the frame without swapchain presentation - OpenXR manages its own
	// swapchains externally via xrReleaseSwapchainImage/xrEndFrame.
	render_pipeline_skip_present();

	// We're finished with rendering our layer, so send it off for display!
	const array_t<XrCompositionLayerBaseHeader*> *composition_layers = compositor_layers_get();
	XrFrameEndInfo end_info = { XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_displays[xr_display_primary_idx].blend;
	end_info.layerCount           = (uint32_t)composition_layers->count;
	end_info.layers               = composition_layers->data;
	xr_chain_insert_extensions((XrBaseHeader*)&end_info, xr_end_frame_chain_bytes, xr_end_frame_chain_offset);

	{
		profiler_zone_name("xrEndFrame");

		uint32_t queue_family = skr_get_vk_graphics_queue_family();
		skr_vk_queue_lock(queue_family);
		XrResult end_result = xrEndFrame(xr_session, &end_info);
		skr_vk_queue_unlock(queue_family);
		xr_check(end_result,
			"xrEndFrame");
	}
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

bool openxr_display_locate(device_display_t* display, XrTime at_time) {
	// Find the state and location of each viewpoint at the predicted time
	uint32_t         view_count  = 0;
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = display->type;
	locate_info.displayTime           = at_time;
	locate_info.space                 = xr_app_space;
	xr_check(xrLocateViews(xr_session, &locate_info, &view_state, display->view_cap, &view_count, display->view_xr),
		"xrLocateViews");

	// And now we'll iterate through each viewpoint, and render it!
	vec2 clip_planes = render_get_clip();
	for (uint32_t i = 0; i < view_count; i++) {
		int32_t array_idx    = 0;
		int32_t view_rect[4] = {};

		// Per-view swapchains are single layer, so each view draws into
		// array layer 0 of its own swapchain.
		swapchain_t *sc_color = &display->swapchain_color;
		swapchain_t *sc_depth = &display->swapchain_depth;
		if (display->per_eye) {
			int32_t eye = display->eye_count > 0 && (int32_t)i < display->eye_count ? (int32_t)i : 0;
			sc_color = &display->eyes[eye].swapchain_color;
			sc_depth = &display->eyes[eye].swapchain_depth;
			render_pipeline_surface_get_surface_info(display->eyes[eye].render_surface, 0, &array_idx, view_rect);
		} else {
			render_pipeline_surface_get_surface_info(display->render_surface, i, &array_idx, view_rect);
		}

		// Set up our rendering information for the viewpoint we're using right
		// now!
		XrCompositionLayerProjectionView *view = &display->view_layers[i];
		*view = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		view->pose = display->view_xr[i].pose;
		view->fov  = display->view_xr[i].fov;
		view->subImage.swapchain        = sc_color->handle;
		view->subImage.imageArrayIndex  = array_idx;
		view->subImage.imageRect.offset = { view_rect[0], view_rect[1] };
		view->subImage.imageRect.extent = { view_rect[2], view_rect[3] };

		if (xr_ext_composition_depth_available()) {
			XrCompositionLayerDepthInfoKHR *depth = &display->view_depths[i];
			depth->minDepth = 0;
			depth->maxDepth = 1;
			depth->nearZ    = clip_planes.x;
			depth->farZ     = clip_planes.y;
			depth->subImage.swapchain        = sc_depth->handle;
			depth->subImage.imageArrayIndex  = array_idx;
			depth->subImage.imageRect.offset = { view_rect[0], view_rect[1] };
			depth->subImage.imageRect.extent = { view_rect[2], view_rect[3] };
			view->next = depth;
		}

		float xr_projection[16];
		openxr_projection(view->fov, clip_planes.x, clip_planes.y, xr_projection);
		memcpy(&display->view_projections[i], xr_projection, sizeof(float) * 16);
		matrix view_tr = matrix_trs((vec3 &)view->pose.position, (quat &)view->pose.orientation, vec3_one);
		view_tr = view_tr * render_get_cam_final();
		matrix_inverse(view_tr, display->view_transforms[i]);
	}
	display->projection_layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	display->projection_layer.space      = xr_app_space;
	display->projection_layer.viewCount  = view_count;
	display->projection_layer.views      = view_count == 0 ? nullptr : display->view_layers;
	display->projection_layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;

	if (display->per_eye) {
		// Each surface renders exactly one view, so it only needs that
		// view's matrices.
		for (uint32_t i = 0; i < view_count && i < (uint32_t)display->eye_count; i++)
			render_pipeline_surface_set_perspective(display->eyes[i].render_surface,
				&display->view_transforms[i],
				&display->view_projections[i],
				1);
	} else {
		render_pipeline_surface_set_perspective(display->render_surface,
			display->view_transforms,
			display->view_projections,
			display->view_cap);
	}

	return true;
}

///////////////////////////////////////////

// Acquires and waits for a swapchain pair's images, returning false if the
// runtime couldn't hand one over.
static bool openxr_swapchain_acquire(swapchain_t *sc_color, swapchain_t *sc_depth, uint32_t *out_color_id) {
	XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t                    color_id     = 0;

	uint32_t queue_family = skr_get_vk_graphics_queue_family();
	skr_vk_queue_lock(queue_family);
	XrResult acquire_result = xrAcquireSwapchainImage(sc_color->handle, &acquire_info, &color_id);
	if (XR_SUCCEEDED(acquire_result)) {
		sc_color->acquired = true;
		if (sc_depth->handle) {
			uint32_t depth_id;
			acquire_result = xrAcquireSwapchainImage(sc_depth->handle, &acquire_info, &depth_id);
			if (XR_SUCCEEDED(acquire_result))
				sc_depth->acquired = true;
		}
	}
	skr_vk_queue_unlock(queue_family);
	if (XR_FAILED(acquire_result)) return false;

	// Wait until the image is available to render to. The compositor could
	// still be reading from it.
	XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	wait_info.timeout = XR_INFINITE_DURATION;
	if (XR_FAILED(xrWaitSwapchainImage(sc_color->handle, &wait_info))) return false;
	if (sc_depth->handle) {
		if (XR_FAILED(xrWaitSwapchainImage(sc_depth->handle, &wait_info))) return false;
	}

	*out_color_id = color_id;
	return true;
}

///////////////////////////////////////////

// Points a pipeline surface at an acquired swapchain image, either directly or
// as an MSAA resolve target.
static void openxr_surface_set_target(pipeline_surface_id surface, swapchain_t *sc_color, uint32_t color_id, int32_t multisample, color128 color, render_layer_ render_filter) {
	if (multisample <= 1) {
		render_pipeline_surface_set_tex(surface, sc_color->textures[color_id]);
	} else {
		// Set the swapchain image as the MSAA resolve target for in-tile resolve.
		// The render pass will automatically resolve the MSAA target to this image
		// during end_pass, which is more efficient on tile-based GPUs (Android).
		render_pipeline_surface_set_resolve_target(surface, &sc_color->textures[color_id]->gpu_tex);
	}
	render_pipeline_surface_set_clear(surface, color, render_sky_covers(render_filter));
	render_pipeline_surface_set_layer(surface, render_filter);
}

///////////////////////////////////////////

bool openxr_display_swapchain_acquire(device_display_t* display, color128 color, render_layer_ render_filter) {
	// We need to ask which swapchain image to use for rendering! Which one
	// will we get? Who knows! It's up to the runtime to decide.
	uint64_t dead_start = stm_now();

	if (display->per_eye) {
		for (int32_t i = 0; i < display->eye_count; i++) {
			display_eye_t *eye      = &display->eyes[i];
			uint32_t       color_id = 0;
			if (!openxr_swapchain_acquire(&eye->swapchain_color, &eye->swapchain_depth, &color_id)) return false;
			openxr_surface_set_target(eye->render_surface, &eye->swapchain_color, color_id, display->multisample, color, render_filter);
			eye->render_surface_tex = color_id;
		}
		xr_cpu_dead_accum_ticks += stm_diff(stm_now(), dead_start);
		return true;
	}

	uint32_t color_id = 0;
	if (!openxr_swapchain_acquire(&display->swapchain_color, &display->swapchain_depth, &color_id)) return false;
	xr_cpu_dead_accum_ticks += stm_diff(stm_now(), dead_start);

	openxr_surface_set_target(display->render_surface, &display->swapchain_color, color_id, display->multisample, color, render_filter);
	display->render_surface_tex = color_id;

	return true;
}

///////////////////////////////////////////

void openxr_display_swapchain_release(device_display_t *display) {
	// And tell OpenXR we're done with rendering to this one!
	XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	uint32_t queue_family = skr_get_vk_graphics_queue_family();
	skr_vk_queue_lock(queue_family);
	if (display->per_eye) {
		for (int32_t i = 0; i < display->eye_count; i++) {
			display_eye_t *eye = &display->eyes[i];
			if (eye->swapchain_color.acquired) xrReleaseSwapchainImage(eye->swapchain_color.handle, &release_info);
			if (eye->swapchain_depth.acquired) xrReleaseSwapchainImage(eye->swapchain_depth.handle, &release_info);
			eye->swapchain_color.acquired = false;
			eye->swapchain_depth.acquired = false;
		}
	} else {
		if (display->swapchain_color.acquired) xrReleaseSwapchainImage(display->swapchain_color.handle, &release_info);
		if (display->swapchain_depth.acquired) xrReleaseSwapchainImage(display->swapchain_depth.handle, &release_info);
		display->swapchain_color.acquired = false;
		display->swapchain_depth.acquired = false;
	}
	skr_vk_queue_unlock(queue_family);

	// Clear the MSAA resolve target for the next frame
	if (display->per_eye) {
		for (int32_t i = 0; i < display->eye_count; i++)
			render_pipeline_surface_set_resolve_target(display->eyes[i].render_surface, nullptr);
	} else {
		render_pipeline_surface_set_resolve_target(display->render_surface, nullptr);
	}
}

///////////////////////////////////////////

void openxr_views_update_fov(XrTime time) {
	if (xr_display_primary_idx == -1) return;
	device_display_t* disp = &xr_displays[xr_display_primary_idx];

	// Find the state and location of each viewpoint at the predicted time
	uint32_t         view_count  = 0;
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = disp->type;
	locate_info.displayTime           = time;
	locate_info.space                 = xr_head_space; // We don't need app space here, and app space may not be valid yet
	if (XR_FAILED(xrLocateViews(xr_session, &locate_info, &view_state, disp->view_cap, &view_count, disp->view_xr)))
		return;

	// Copy over the FoV so it's available to the users
	if (view_count > 0) {
		device_data.display_fov.right  = disp->view_xr[0].fov.angleRight * rad2deg;
		device_data.display_fov.left   = disp->view_xr[0].fov.angleLeft  * rad2deg;
		device_data.display_fov.top    = disp->view_xr[0].fov.angleUp    * rad2deg;
		device_data.display_fov.bottom = disp->view_xr[0].fov.angleDown  * rad2deg;
	}
}

} // namespace sk
#endif