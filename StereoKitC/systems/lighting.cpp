// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#include "lighting.h"
#include "render.h"
#include "defaults.h"
#include "../stereokit.h"
#include "../sk_math.h"
#include "../device.h"
#include "../spherical_harmonics.h"
#include "../asset_types/mesh.h"
#include "../asset_types/material.h"
#include "../asset_types/texture.h"
#include "../asset_types/shader.h"
#include "../xr_backends/extensions/light_estimation.h"
#include "../log.h"

namespace sk {

///////////////////////////////////////////

struct lighting_state_t {
	mesh_t                  sky_mesh;
	material_t              sky_mat;
	material_t              sky_mat_default;
	bool32_t                sky_show;
	tex_t                   sky_pending_tex;

	lighting_mode_          mode;
	vec4                    ambient[9];
	spherical_harmonics_t   ambient_src;
	vec3                    directional_dir;
	color128                directional_color;
	tex_t                   reflection;
	tex_t                   reflection_pending_tex;

	bool                    pending_scene_permission;
};
static lighting_state_t local = {};

///////////////////////////////////////////

static void _lighting_set_reflection(tex_t ibl_cubemap);
static void _lighting_set_ambient   (const spherical_harmonics_t& ambient_lighting);

///////////////////////////////////////////

static tex_t check_pending_cubemap(tex_t* ref_pending, const char* description) {
	if (*ref_pending == nullptr) return nullptr;

	asset_state_ state = tex_asset_state(*ref_pending);

	// Check if it's errored out
	if (state < 0) {
		tex_release(*ref_pending);
		*ref_pending = nullptr;
		return nullptr;
	}

	// Check if it's a valid asset, we'll know for sure once the metadata is
	// loaded.
	if (state >= asset_state_loaded_meta && ((*ref_pending)->type & tex_type_cubemap) == 0) {
		log_warnf("%s must be a cubemap, ignoring %s", description, tex_get_id(*ref_pending));
		tex_release(*ref_pending);
		*ref_pending = nullptr;
		return nullptr;
	}

	// Check if it's loaded, transfer ownership out
	if (state >= asset_state_loaded) {
		tex_t result = *ref_pending;
		*ref_pending = nullptr;
		return result;
	}

	return nullptr;
}

///////////////////////////////////////////

static void check_pending_skytex() {
	tex_t tex = check_pending_cubemap(&local.sky_pending_tex, "Skytex");
	if (tex != nullptr) {
		render_global_texture(render_skytex_register, tex);
		tex_release(tex);
	}
}

///////////////////////////////////////////

static void check_pending_reflection() {
	tex_t tex = check_pending_cubemap(&local.reflection_pending_tex, "Reflection texture");
	if (tex != nullptr) {
		if (local.reflection != nullptr) tex_release(local.reflection);
		local.reflection = tex;
	}
}

///////////////////////////////////////////

void lighting_check_pending() {
	check_pending_skytex();
	check_pending_reflection();
}

///////////////////////////////////////////

bool lighting_init() {
	// Create a default skybox mesh
	local.sky_mesh = mesh_create();
	vind_t inds [] = {2,1,0, 3,2,0};
	vert_t verts[] = {
		vert_t{ {-1, 1,1}, {0,0,1}, {0,0}, {255,255,255,255} },
		vert_t{ { 1, 1,1}, {0,0,1}, {1,0}, {255,255,255,255} },
		vert_t{ { 1,-1,1}, {0,0,1}, {1,1}, {255,255,255,255} },
		vert_t{ {-1,-1,1}, {0,0,1}, {0,1}, {255,255,255,255} }, };
	mesh_set_data(local.sky_mesh, verts, _countof(verts), inds, _countof(inds));
	mesh_set_id  (local.sky_mesh, "sk/lighting/skybox_mesh");

	// Create a default skybox material
	shader_t shader_sky = shader_find(default_id_shader_sky);
	local.sky_mat_default = material_create(shader_sky);
	material_set_id          (local.sky_mat_default, "sk/lighting/skybox_material");
	material_set_queue_offset(local.sky_mat_default, 100);
	material_set_depth_write (local.sky_mat_default, false);
	material_set_depth_test  (local.sky_mat_default, depth_test_less_or_eq);
	render_set_skymaterial(local.sky_mat_default);
	shader_release(shader_sky);

	// Create a default skybox texture
	tex_t sky_cubemap = tex_find(default_id_cubemap);
	
	render_set_skytex   (sky_cubemap);
	render_enable_skytex(true);

	lighting_set_reflection (sky_cubemap);
	lighting_set_ambient    (sk_default_lighting);
	lighting_set_directional(sh_dominant_dir(sk_default_lighting), {0,0,0,1});
	lighting_set_mode       (lighting_mode_auto);
	
	tex_release(sky_cubemap);

	return true;
}

///////////////////////////////////////////

void lighting_step() {
	lighting_check_pending();

	// If we asked for permission to do world lighting, check on the status of
	// that.
	if (local.pending_scene_permission) {
		permission_state_ state = permission_state(permission_type_scene);
		if (state == permission_state_granted) {
			local.pending_scene_permission = false;
			lighting_set_mode(lighting_mode_world);
		} else if (state == permission_state_unavailable) {
			local.pending_scene_permission = false;
			lighting_set_mode(lighting_mode_manual);
		}
	}

	if (local.mode == lighting_mode_world) {
		spherical_harmonics_t sh;
		if (xr_ext_light_estimation_update_sh(&sh)) {
			_lighting_set_ambient(sh);
			
			tex_t reflection = tex_gen_cubemap_sh(local.ambient_src, 32, 0.2f, 2.0f);
			_lighting_set_reflection(reflection);
			tex_release(reflection);
		}
	}

	if (local.sky_show && device_display_get_blend() == display_blend_opaque) {
		render_add_mesh(local.sky_mesh, local.sky_mat, matrix_identity, {1,1,1,1}, render_layer_vfx);
	}
}

///////////////////////////////////////////

void lighting_shutdown() {
	tex_release     (local.sky_pending_tex);
	tex_release     (local.reflection);
	tex_release     (local.reflection_pending_tex);
	material_release(local.sky_mat_default);
	material_release(local.sky_mat);
	mesh_release    (local.sky_mesh);

	local = {};
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture == nullptr) return;

	tex_addref(sky_texture);
	if (local.sky_pending_tex != nullptr) tex_release(local.sky_pending_tex);
	local.sky_pending_tex = sky_texture;


	// This is also checked every step, but if the texture is already valid, we
	// can set it up right away and avoid any potential frame delays.
	check_pending_skytex();
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	if (local.sky_pending_tex != nullptr) {
		tex_addref(local.sky_pending_tex);
		return local.sky_pending_tex;
	}

	// Fall back to what's in the global texture slot
	return render_get_global_texture(render_skytex_register);
}

///////////////////////////////////////////

void render_set_skymaterial(material_t sky_material) {
	// Don't allow null, fall back to the default sky material on null.
	if (sky_material == nullptr) {
		sky_material = local.sky_mat_default;
	}

	// Safe swap the material reference
	material_addref(sky_material);
	if (local.sky_mat != nullptr) material_release(local.sky_mat);
	local.sky_mat = sky_material;
}

///////////////////////////////////////////

material_t render_get_skymaterial(void) {
	material_addref(local.sky_mat);
	return local.sky_mat;
}

///////////////////////////////////////////

void render_set_skylight(const spherical_harmonics_t &light_info) {
	lighting_set_ambient(light_info);
}

///////////////////////////////////////////

spherical_harmonics_t render_get_skylight() {
	return lighting_get_ambient();
}

///////////////////////////////////////////

void render_enable_skytex(bool32_t show_sky) {
	local.sky_show = show_sky;
}

///////////////////////////////////////////

bool32_t render_enabled_skytex() {
	return local.sky_show;
}

///////////////////////////////////////////

const vec4* lighting_get_lighting() {
	return local.ambient;
}

///////////////////////////////////////////

bool32_t lighting_mode_available(lighting_mode_ mode) {
	switch (mode) {
	case lighting_mode_auto:   return true;
	case lighting_mode_manual: return true;
	case lighting_mode_world:  return xr_ext_android_light_estimation_available();
	default:                   return false;
	}
}

///////////////////////////////////////////

bool32_t lighting_set_mode(lighting_mode_ mode) {
	// In auto mode, select the best mode based on current conditions
	if (mode == lighting_mode_auto) {
		display_blend_ blend = device_display_get_blend();
		return lighting_set_mode((blend & display_blend_any_transparent) > 0 && lighting_mode_available(lighting_mode_world) 
			? lighting_mode_world
			: lighting_mode_manual);
	}

	if (local.mode == mode) return true;

	if (mode == lighting_mode_world) {
		// Request permission if we need it
		permission_state_ perms = permission_state(permission_type_scene);
		if (perms == permission_state_capable) {
			permission_request(permission_type_scene);
			// Permissions may be granted immediately
			perms = permission_state(permission_type_scene);
		}

		if (perms != permission_state_granted) {
			// If we're still waiting on permission, set a flag
			if (perms == permission_state_capable)
				local.pending_scene_permission = true;
			mode = lighting_mode_manual;
		}
	} else {
		local.pending_scene_permission = false;
	}

	// Stop light estimation if we're leaving world mode
	if (local.mode == lighting_mode_world) {
		xr_ext_light_estimation_stop();
	}

	local.mode = mode;

	// Start light estimation if we're entering world mode
	if (mode == lighting_mode_world) {
		if (!xr_ext_light_estimation_start()) {
			local.mode = lighting_mode_manual;
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////

lighting_mode_ lighting_get_mode(void) {
	return local.pending_scene_permission ? lighting_mode_world : local.mode;
}

///////////////////////////////////////////

static void _lighting_set_ambient(const spherical_harmonics_t& ambient_lighting) {
	local.ambient_src = ambient_lighting;
	sh_to_fast(ambient_lighting, local.ambient);
}

///////////////////////////////////////////

void lighting_set_ambient(const spherical_harmonics_t& ambient_lighting) {
	if (local.mode == lighting_mode_world) return;
	_lighting_set_ambient(ambient_lighting);
}

///////////////////////////////////////////

spherical_harmonics_t lighting_get_ambient(void) {
	return local.ambient_src;
}

///////////////////////////////////////////

void lighting_set_directional(vec3 dir, color128 color_linear) {
	if (local.mode == lighting_mode_world) return;
	local.directional_dir   = dir;
	local.directional_color = color_linear;
}

///////////////////////////////////////////

void lighting_get_directional(vec3* out_dir, color128* out_color_linear) {
	if (out_dir)          *out_dir          = local.directional_dir;
	if (out_color_linear) *out_color_linear = local.directional_color;
}

///////////////////////////////////////////

static void _lighting_set_reflection(tex_t ibl_cubemap) {
	if (ibl_cubemap == nullptr) return;

	tex_addref(ibl_cubemap);
	if (local.reflection_pending_tex != nullptr) tex_release(local.reflection_pending_tex);
	local.reflection_pending_tex = ibl_cubemap;

	// This is also checked every step, but if the texture is already valid, we
	// can set it up right away and avoid any potential frame delays.
	check_pending_reflection();
}

///////////////////////////////////////////

void lighting_set_reflection(tex_t ibl_cubemap) {
	if (local.mode == lighting_mode_world) return;
	_lighting_set_reflection(ibl_cubemap);
}

///////////////////////////////////////////

tex_t lighting_get_reflection(void) {
	if (local.reflection_pending_tex != nullptr) {
		tex_addref(local.reflection_pending_tex);
		return local.reflection_pending_tex;
	}

	if (local.reflection != nullptr) tex_addref(local.reflection);
	return local.reflection;
}

} // namespace sk
