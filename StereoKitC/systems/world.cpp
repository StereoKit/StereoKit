#include "world.h"
#include "../platforms/platform.h"

#include "../stereokit.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/assets.h"
#include "../asset_types/mesh_.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/simulator.h"
#include "../xr_backends/extensions/msft_scene_understanding.h"
#include "../libraries/profiler.h"
#include "defaults.h"
#include "sensor_depth.h"
#include "render.h"

#include <float.h>
#include <math.h>

namespace sk {

///////////////////////////////////////////
// Depth-based occlusion
///////////////////////////////////////////

static material_t depth_prepass_mat      = {};
static mesh_t     depth_grid_mesh        = {};
static uint32_t   depth_grid_w           = 0;
static uint32_t   depth_grid_h           = 0;
static bool       depth_started_by_world = false;
static bool       depth_enabled          = false;

static matrix depth_build_vp(const sensor_depth_view_t *eye, float near_z, float far_z) {
	// View matrix: transforms world -> depth eye space
	matrix view_mat = pose_matrix_inv(eye->pose);

	// Projection matrix from asymmetric FOV tangents
	float l = tanf(eye->fov.left   * deg2rad);
	float r = tanf(eye->fov.right  * deg2rad);
	float t = tanf(eye->fov.top    * deg2rad);
	float b = tanf(eye->fov.bottom * deg2rad);

	float n      = near_z;
	float width  = r - l;
	float height = t - b;

	matrix proj;
	if (isinf(far_z)) {
		// Infinite far plane
		proj = { {
			{2.0f / width,    0,                 0,      0},
			{0,               2.0f / height,     0,      0},
			{(r + l) / width, (t + b) / height, -1,     -1},
			{0,               0,                -2 * n,  0},
		} };
	} else {
		float f = far_z;
		proj = { {
			{2.0f / width,    0,                0,                   0},
			{0,               2.0f / height,    0,                   0},
			{(r + l) / width, (t + b) / height, -(f + n) / (f - n), -1},
			{0,               0,                -2*f*n / (f - n),    0},
		} };
	}

	return view_mat * proj;
}

static void depth_ensure_grid(uint32_t w, uint32_t h) {
	if (w == depth_grid_w && h == depth_grid_h && depth_grid_mesh != nullptr)
		return;

	if (depth_grid_mesh != nullptr)
		mesh_release(depth_grid_mesh);

	depth_grid_w = w;
	depth_grid_h = h;

	// Index-only mesh — shader derives position from SV_VertexID.
	depth_grid_mesh = mesh_create();
	mesh_set_id       (depth_grid_mesh, "default/mesh_depth_grid");
	mesh_set_keep_data(depth_grid_mesh, false);

	int32_t  ind_count = (int32_t)(6 * (w - 1) * (h - 1));
	vind_t  *inds      = sk_malloc_t(vind_t, ind_count);

	// Build triangle grid — vertex IDs map to texel grid positions.
	// Alternate diagonal direction per quad for more uniform artifacts.
	int32_t idx = 0;
	for (uint32_t y = 0; y < h - 1; y++) {
		for (uint32_t x = 0; x < w - 1; x++) {
			vind_t v00 = (vind_t)(y       * w + x);
			vind_t v10 = (vind_t)(y       * w + x + 1);
			vind_t v01 = (vind_t)((y + 1) * w + x);
			vind_t v11 = (vind_t)((y + 1) * w + x + 1);
			if ((x + y) % 2 == 0) {
				inds[idx++] = v00; inds[idx++] = v10; inds[idx++] = v01;
				inds[idx++] = v10; inds[idx++] = v11; inds[idx++] = v01;
			} else {
				inds[idx++] = v00; inds[idx++] = v10; inds[idx++] = v11;
				inds[idx++] = v00; inds[idx++] = v11; inds[idx++] = v01;
			}
		}
	}

	mesh_set_inds(depth_grid_mesh, inds, ind_count);
	sk_free(inds);
}

static void depth_init() {
	depth_prepass_mat = material_create(sk_default_shader_depth_prepass);
	material_set_id          (depth_prepass_mat, "default/material_depth_prepass");
	material_set_depth_test  (depth_prepass_mat, depth_test_always);
	material_set_depth_write (depth_prepass_mat, true);
	material_set_cull        (depth_prepass_mat, cull_none);
	material_set_queue_offset(depth_prepass_mat, -200);
}

static void depth_shutdown() {
	if (depth_started_by_world && sensor_depth_running()) {
		sensor_depth_stop();
		depth_started_by_world = false;
	}
	depth_enabled = false;
	material_release(depth_prepass_mat);
	depth_prepass_mat = {};
	mesh_release(depth_grid_mesh);
	depth_grid_mesh = {};
	depth_grid_w    = 0;
	depth_grid_h    = 0;
}

static void depth_step() {
	if (!depth_enabled || depth_prepass_mat == nullptr)
		return;

	sensor_depth_frame_t frame = {};
	if (!sensor_depth_try_get_latest_frame(&frame))
		return;

	tex_t depth_tex = sensor_depth_get_texture();
	if (depth_tex == nullptr)
		return;

	depth_ensure_grid(frame.width, frame.height);
	if (depth_grid_mesh == nullptr)
		return;

	material_set_texture(depth_prepass_mat, "depth_tex", depth_tex);
	material_set_vector4(depth_prepass_mat, "depth_dims", {
		(float)frame.width, (float)frame.height,
		1.0f / (float)frame.width, 1.0f / (float)frame.height });

	matrix depth_vp_l = depth_build_vp(&frame.views[0], frame.near_z, frame.far_z);
	matrix depth_vp_r = depth_build_vp(&frame.views[1], frame.near_z, frame.far_z);
	material_set_matrix(depth_prepass_mat, "depth_view_proj_inv_l", matrix_invert(depth_vp_l));
	material_set_matrix(depth_prepass_mat, "depth_view_proj_inv_r", matrix_invert(depth_vp_r));

	render_add_mesh(depth_grid_mesh, depth_prepass_mat, matrix_identity, {1,1,1,1}, render_layer_0);
}

static void depth_set_occlusion(occlusion_caps_ flags, occlusion_caps_ old_flags) {
	bool depth_on  = (flags     & occlusion_caps_depth) != 0;
	bool depth_was = (old_flags & occlusion_caps_depth) != 0;

	if (depth_on && !depth_was) {
		if (!sensor_depth_running()) {
			sensor_depth_start();
			depth_started_by_world = true;
		}
	} else if (!depth_on && depth_was) {
		if (depth_started_by_world) {
			sensor_depth_stop();
			depth_started_by_world = false;
		}
	}
	depth_enabled = depth_on;

	if (depth_on) {
		bool hands_on = (flags & occlusion_caps_hands) != 0;
		sensor_depth_set_capabilities(hands_on ? sensor_depth_caps_none : sensor_depth_caps_hand_removal);
	}
}

static occlusion_caps_ depth_capabilities() {
	occlusion_caps_ caps = occlusion_caps_none;
	if (sensor_depth_available()) {
		caps = caps | occlusion_caps_depth;
		if ((sensor_depth_get_capabilities() & sensor_depth_caps_hand_removal) != 0)
			caps = caps | occlusion_caps_hands;
	}
	return caps;
}

///////////////////////////////////////////
// Mesh-based occlusion
///////////////////////////////////////////

static material_t mesh_occlusion_material = {};

static void mesh_init() {
	mesh_occlusion_material = material_copy_id(default_id_material_unlit);
	material_set_id   (mesh_occlusion_material, "sk/world/material");
	material_set_color(mesh_occlusion_material, "color", { 0,0,0,0 });
}

static void mesh_shutdown() {
	material_release(mesh_occlusion_material);
	mesh_occlusion_material = {};
}

static void mesh_set_occlusion(occlusion_caps_ flags, occlusion_caps_ old_flags) {
	bool mesh_on  = (flags     & occlusion_caps_mesh) != 0;
	bool mesh_was = (old_flags & occlusion_caps_mesh) != 0;
	if (mesh_on != mesh_was) {
		switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
		case backend_xr_type_openxr: oxr_su_set_occlusion_enabled(mesh_on); break;
#endif
		default: break;
		}
	}
}

static occlusion_caps_ mesh_capabilities() {
	occlusion_caps_ caps = occlusion_caps_none;
	if (sk_get_info_ref()->world_occlusion_present)
		caps = caps | occlusion_caps_mesh;
	return caps;
}

///////////////////////////////////////////
// General world state
///////////////////////////////////////////

origin_mode_    world_origin_mode;
pose_t          world_origin_offset;
occlusion_caps_ world_occlusion_flags = occlusion_caps_none;

///////////////////////////////////////////

bool world_init() {
	profiler_zone();

	mesh_init();
	depth_init();

	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: {
		xr_has_bounds = openxr_get_stage_bounds(&xr_bounds_size, &xr_bounds_pose_local, xr_time);
	}break;
#endif
	default: break;
	}

	return true;
}

///////////////////////////////////////////

void world_shutdown() {
	depth_shutdown();
	mesh_shutdown();
	world_occlusion_flags = occlusion_caps_none;
}

///////////////////////////////////////////

void world_step() {
	profiler_zone();
	depth_step();
}

///////////////////////////////////////////

bool32_t world_raycast(ray_t ray, ray_t *out_intersection) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_raycast(ray, out_intersection);
#endif
	default: *out_intersection = {}; return false;
	}
}

///////////////////////////////////////////

void world_set_occlusion(occlusion_caps_ flags) {
	occlusion_caps_ old_flags = world_occlusion_flags;
	world_occlusion_flags = flags;

	mesh_set_occlusion (flags, old_flags);
	depth_set_occlusion(flags, old_flags);
}

///////////////////////////////////////////

occlusion_caps_ world_get_occlusion() {
	return world_occlusion_flags;
}

///////////////////////////////////////////

occlusion_caps_ world_occlusion_capabilities() {
	return mesh_capabilities() | depth_capabilities();
}

///////////////////////////////////////////

void world_set_occlusion_enabled(bool32_t enabled) {
	if (enabled)
		world_set_occlusion(world_occlusion_flags | occlusion_caps_mesh);
	else
		world_set_occlusion(world_occlusion_flags & ~occlusion_caps_mesh);
}

///////////////////////////////////////////

bool32_t world_get_occlusion_enabled() {
	return (world_occlusion_flags & occlusion_caps_mesh) != 0;
}

///////////////////////////////////////////

void world_set_raycast_enabled(bool32_t enabled) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: oxr_su_set_raycast_enabled(enabled); return;
#endif
	default: return;
	}
}

///////////////////////////////////////////

bool32_t world_get_raycast_enabled() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_get_raycast_enabled();
#endif
	default: return false;
	}
}

///////////////////////////////////////////

void world_set_occlusion_material(material_t material) {
	assets_safeswap_ref(
		(asset_header_t**)&mesh_occlusion_material,
		(asset_header_t*)material);
}

///////////////////////////////////////////

material_t world_get_occlusion_material() {
	if (mesh_occlusion_material)
		material_addref(mesh_occlusion_material);
	return mesh_occlusion_material;
}

///////////////////////////////////////////

void world_set_refresh_type(world_refresh_ refresh_type) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: oxr_su_set_refresh_type(refresh_type); return;
#endif
	default: return;
	}
}

///////////////////////////////////////////

world_refresh_ world_get_refresh_type() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_get_refresh_type();
#endif
	default: return world_refresh_area;
	}
}

///////////////////////////////////////////

void world_set_refresh_radius(float radius_meters) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: oxr_su_set_refresh_radius(radius_meters); return;
#endif
	default: return;
	}
}

///////////////////////////////////////////

float world_get_refresh_radius() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_get_refresh_radius();
#endif
	default: return 0;
	}
}

///////////////////////////////////////////

void world_set_refresh_interval(float every_seconds) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: oxr_su_set_refresh_interval(every_seconds); return;
#endif
	default: return;
	}
}

///////////////////////////////////////////

float world_get_refresh_interval() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_get_refresh_interval();
#endif
	default: return 0;
	}
}

///////////////////////////////////////////

bool32_t world_has_bounds() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr:    return xr_has_bounds;
#endif
	case backend_xr_type_simulator: return true;
	default:                        return false;
	}
}

///////////////////////////////////////////

vec2 world_get_bounds_size() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr:    return xr_bounds_size;
#endif
	case backend_xr_type_simulator: return simulator_bounds_size();
	default:                        return vec2_zero;
	}
}

///////////////////////////////////////////

pose_t world_get_bounds_pose() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr:    return render_cam_final_transform(xr_bounds_pose_local);
#endif
	case backend_xr_type_simulator: return simulator_bounds_pose();
	default:                        return pose_identity;
	}
}

button_state_ world_get_tracked() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr:    return openxr_space_tracked();
#endif
	case backend_xr_type_simulator: return button_state_active;
	default:                        return button_state_active;
	}
}

///////////////////////////////////////////

origin_mode_ world_get_origin_mode() {
	return world_origin_mode;
}

///////////////////////////////////////////

pose_t world_get_origin_offset() {
	return world_origin_offset;
}

///////////////////////////////////////////

void world_set_origin_offset(pose_t offset) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr:    openxr_set_origin_offset   (offset); break;
#endif
	case backend_xr_type_simulator: simulator_set_origin_offset(offset); break;
	default: abort(); break;
	}
	world_origin_offset = offset;
}

///////////////////////////////////////////

// This function is for backwards compatibility.
pose_t world_from_perception_anchor(void* perception_spatial_anchor) {
	log_warnf("%s is no longer supported.", "world_from_perception_anchor");
	return pose_identity;
}

///////////////////////////////////////////

// This function is for backwards compatibility.
bool32_t world_try_from_perception_anchor(void* perception_spatial_anchor, pose_t* out_pose) {
	log_warnf("%s is no longer supported.", "world_try_from_perception_anchor");
	*out_pose = pose_identity;
	return false;
}

} // namespace sk