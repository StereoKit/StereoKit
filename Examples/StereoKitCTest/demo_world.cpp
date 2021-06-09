#include "demo_world.h"

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"
using namespace sk;

///////////////////////////////////////////

mesh_t world_sphere;
material_t world_material;

///////////////////////////////////////////

void demo_world_init() {
	world_sphere   = mesh_find(default_id_mesh_sphere);
	world_material = material_find(default_id_material);
}

///////////////////////////////////////////

void demo_world_update() {
	static pose_t window_pose =
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	ui_window_begin("Main", window_pose);
	ui_window_end();

	for (size_t h = 0; h < handed_max; h++) {
		const hand_t *hand = input_hand((handed_)h);
		ray_t hand_ray = {hand->palm.position, hand->palm.orientation * vec3_forward};
		ray_t intersection;
		if (world_raycast(hand_ray, intersection)) {
			render_add_mesh(world_sphere, world_material, matrix_trs(intersection.pos, quat_identity, vec3_one*0.02f));
		}
	}
	
}

///////////////////////////////////////////

void demo_world_shutdown() {
	mesh_release    (world_sphere);
	material_release(world_material);
}