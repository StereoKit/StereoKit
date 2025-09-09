#include "demo_world.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

///////////////////////////////////////////

mesh_t     world_sphere;
material_t world_material;
material_t world_old_material;
bool       world_occlusion_available;
bool       world_raycast_available;

///////////////////////////////////////////

void demo_world_init() {
	world_sphere   = mesh_find       (default_id_mesh_sphere);
	world_material = material_copy_id(default_id_material);

	world_old_material = world_get_occlusion_material();
	world_set_occlusion_material(world_material);

	world_occlusion_available = sk_system_info().world_occlusion_present;
	world_raycast_available   = sk_system_info().world_raycast_present;
}

///////////////////////////////////////////

void demo_world_update() {
	static pose_t window_pose =
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	ui_window_begin("Main", &window_pose);
	if (world_occlusion_available) {
		bool32_t occlusion = world_get_occlusion_enabled();
		if (ui_toggle("Occlusion Enabled", occlusion))
			world_set_occlusion_enabled(occlusion);

		bool32_t wire = material_get_wireframe(world_material);
		if (ui_toggle("Wireframe", wire)) 
			material_set_wireframe(world_material, wire);
	} else {
		ui_label("This system is not capable of occlusion.");
	}

	ui_hseparator();

	if (world_raycast_available) {
		bool32_t raycast = world_get_raycast_enabled();
		if (ui_toggle("Raycast Enabled", raycast))
			world_set_raycast_enabled(raycast);
	} else {
		ui_label("This system is not capable of world raycasting.");
	}
	ui_window_end();

	for (size_t h = 0; h < handed_max; h++) {
		const hand_t *hand = input_hand((handed_)h);
		if (!(hand->tracked_state & button_state_active)) continue;

		ray_t hand_ray = {hand->palm.position, hand->palm.orientation * vec3_forward};
		ray_t intersection;
		line_add(hand_ray.pos, hand_ray.pos + hand_ray.dir * 0.1f, {0,128,0,255}, {0,255,0,255}, 0.01f);
		if (world_raycast(hand_ray, &intersection)) {
			render_add_mesh(world_sphere, world_material, matrix_trs(intersection.pos, quat_identity, vec3_one*0.02f));
		}
	}
}

///////////////////////////////////////////

void demo_world_shutdown() {
	world_set_occlusion_material(world_old_material);
	material_release(world_old_material);

	mesh_release    (world_sphere);
	material_release(world_material);
}