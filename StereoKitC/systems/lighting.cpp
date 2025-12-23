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
#include "../log.h"

namespace sk {

///////////////////////////////////////////

struct lighting_state_t {
	mesh_t                  sky_mesh;
	material_t              sky_mat;
	material_t              sky_mat_default;
	bool32_t                sky_show;
	tex_t                   sky_pending_tex;
	vec4                    lighting[9];
	spherical_harmonics_t   lighting_src;
};
static lighting_state_t local = {};

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
	render_set_skylight (sk_default_lighting);
	render_enable_skytex(true);
	tex_release(sky_cubemap);

	return true;
}

///////////////////////////////////////////

void lighting_step() {
	render_check_pending_skytex();

	if (local.sky_show && device_display_get_blend() == display_blend_opaque) {
		render_add_mesh(local.sky_mesh, local.sky_mat, matrix_identity, {1,1,1,1}, render_layer_vfx);
	}
}

///////////////////////////////////////////

void lighting_shutdown() {
	tex_release     (local.sky_pending_tex);
	material_release(local.sky_mat_default);
	material_release(local.sky_mat);
	mesh_release    (local.sky_mesh);

	local = {};
}

///////////////////////////////////////////

void render_check_pending_skytex() {
	if (local.sky_pending_tex == nullptr) return;

	asset_state_ state = tex_asset_state(local.sky_pending_tex);

	// Check if it's errored out
	if (state < 0) {
		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}

	// Check if it's a valid asset, we'll know for sure once the metadata is
	// loaded.
	if (state >= asset_state_loaded_meta && (local.sky_pending_tex->type & tex_type_cubemap) == 0) {
		log_warnf("Skytex must be a cubemap, ignoring %s", tex_get_id(local.sky_pending_tex));

		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}

	// Check if it's loaded
	if (state >= asset_state_loaded) {
		render_global_texture(11, local.sky_pending_tex); // render_skytex_register = 11

		tex_release(local.sky_pending_tex);
		local.sky_pending_tex = nullptr;
		return;
	}
}

///////////////////////////////////////////

void render_set_skytex(tex_t sky_texture) {
	if (sky_texture == nullptr) return;

	tex_addref(sky_texture);
	local.sky_pending_tex = sky_texture;

	// This is also checked every step, but if the texture is already valid, we
	// can set it up right away and avoid any potential frame delays.
	render_check_pending_skytex();
}

///////////////////////////////////////////

tex_t render_get_skytex() {
	if (local.sky_pending_tex != nullptr) {
		tex_addref(local.sky_pending_tex);
		return local.sky_pending_tex;
	}

	// Fall back to what's in the global texture slot
	// This requires accessing render's global textures - for now return pending or null
	return nullptr;
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
	local.lighting_src = light_info;
	sh_to_fast(light_info, local.lighting);
}

///////////////////////////////////////////

spherical_harmonics_t render_get_skylight() {
	return local.lighting_src;
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
	return local.lighting;
}

} // namespace sk
