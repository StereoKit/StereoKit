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
#include "render.h"

#include <float.h>

namespace sk {

// These values are set by the XR backends.
origin_mode_ world_origin_mode;
pose_t       world_origin_offset;
material_t   world_occlusion_material = {};

///////////////////////////////////////////

bool world_init() {
	profiler_zone();

	world_occlusion_material = material_copy_id(default_id_material_unlit);
	material_set_id   (world_occlusion_material, "sk/world/material");
	material_set_color(world_occlusion_material, "color", { 0,0,0,0 });

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
	material_release(world_occlusion_material);
}

///////////////////////////////////////////

void world_step() {
	profiler_zone();
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

void world_set_occlusion_enabled(bool32_t enabled) {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: oxr_su_set_occlusion_enabled(enabled); return;
#endif
	default: return;
	}
}

///////////////////////////////////////////

bool32_t world_get_occlusion_enabled() {
	switch (backend_xr_get_type()) {
#if defined(SK_XR_OPENXR)
	case backend_xr_type_openxr: return oxr_su_get_occlusion_enabled();
#endif
	default: return false;
	}
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
		(asset_header_t**)&world_occlusion_material,
		(asset_header_t*)material);
}

///////////////////////////////////////////

material_t world_get_occlusion_material() {
	if (world_occlusion_material)
		material_addref(world_occlusion_material);
	return world_occlusion_material;
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