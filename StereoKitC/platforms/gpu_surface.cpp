// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "gpu_surface.h"

#include "../log.h"

namespace sk {

///////////////////////////////////////////

// Sizes the swapchain where the surface reports no extent of its own: Wayland
// on Vulkan, and WebGPU everywhere. Ignored otherwise.
skr_vec2i_t gpu_surface_drawable_size(ska_window_t* window) {
	skr_vec2i_t size = {};
	ska_window_get_drawable_size(window, &size.x, &size.y);
	return size;
}

///////////////////////////////////////////

#if defined(SKR_WEBGPU)

bool gpu_surface_create(ska_window_t* window, skr_surface_t* out_surface) {
	WGPUSurface wgpu_surface = nullptr;
	if (!ska_wgpu_create_surface(window, skr_get_wgpu_instance(), &wgpu_surface)) {
		log_errf("Failed to create WebGPU surface: %s", ska_error_get());
		return false;
	}
	if (skr_surface_create(wgpu_surface, gpu_surface_drawable_size(window), out_surface) != skr_err_success) {
		log_err("Failed to create renderer surface");
		wgpuSurfaceRelease(wgpu_surface);
		return false;
	}
	return true;
}

tex_format_ gpu_surface_color_format(const skr_surface_t* surface) {
	// view_format, not format: on the web those differ, and the frame views are
	// what a resolve target gets matched against.
	return (tex_format_)skr_tex_fmt_from_native(surface->view_format);
}

#else

bool gpu_surface_create(ska_window_t* window, skr_surface_t* out_surface) {
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	if (!ska_vk_create_surface(window, skr_get_vk_instance(), &vk_surface)) {
		log_errf("Failed to create Vulkan surface: %s", ska_error_get());
		return false;
	}
	if (skr_surface_create(vk_surface, gpu_surface_drawable_size(window), out_surface) != skr_err_success) {
		log_err("Failed to create renderer surface");
		vkDestroySurfaceKHR(skr_get_vk_instance(), vk_surface, nullptr);
		return false;
	}
	return true;
}

tex_format_ gpu_surface_color_format(const skr_surface_t* surface) {
	// The swapchain images carry the format here, there's no separate field
	if (surface->image_count == 0) return tex_format_bgra32;
	return (tex_format_)skr_tex_get_format(&surface->images[0]);
}

#endif

} // namespace sk
