#include "demo_mixed_reality.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

///////////////////////////////////////////

mesh_t mr_sphere;

///////////////////////////////////////////

void demo_mixed_reality_init() {
	mr_sphere = mesh_find(default_id_mesh_sphere);

	world_set_refresh_type    (world_refresh_timer);
	world_set_refresh_interval(2);
	world_set_refresh_radius  (5);
}

///////////////////////////////////////////

void demo_mixed_reality_update() {
	static pose_t window_pose =
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	ui_window_begin("Mixed Reality Settings", &window_pose);

	// Display blend mode
	ui_label("Display Blend");

	ui_push_enabled(device_display_valid_blend(display_blend_opaque));
	if (ui_button("Opaque"))
		device_display_set_blend(display_blend_opaque);
	ui_pop_enabled();
	ui_sameline();

	ui_push_enabled(device_display_valid_blend(display_blend_blend));
	if (ui_button("Blend"))
		device_display_set_blend(display_blend_blend);
	ui_pop_enabled();
	ui_sameline();

	ui_push_enabled(device_display_valid_blend(display_blend_additive));
	if (ui_button("Additive"))
		device_display_set_blend(display_blend_additive);
	ui_pop_enabled();

	// World occlusion controls
	ui_hseparator();
	occlusion_caps_ available = world_occlusion_capabilities();
	occlusion_caps_ current   = world_get_occlusion();

	ui_label(available == occlusion_caps_none ? "Occlusion isn't available on this system" : "world_occlusion_set");

	ui_push_enabled(available & occlusion_caps_depth);
	bool32_t depth_occlusion = current & occlusion_caps_depth;
	if (ui_toggle("occlusion_caps_depth", depth_occlusion))
		world_set_occlusion(depth_occlusion ? (current | occlusion_caps_depth) : (current & ~occlusion_caps_depth));
	ui_pop_enabled();
	ui_sameline();

	ui_push_enabled(available & occlusion_caps_hands);
	current = world_get_occlusion();
	bool32_t hand_occlusion = current & occlusion_caps_hands;
	if (ui_toggle("occlusion_caps_hands", hand_occlusion))
		world_set_occlusion(hand_occlusion ? (current | occlusion_caps_hands) : (current & ~occlusion_caps_hands));
	ui_pop_enabled();

	ui_push_enabled(available & occlusion_caps_mesh);
	current = world_get_occlusion();
	bool32_t mesh_occlusion = current & occlusion_caps_mesh;
	if (ui_toggle("occlusion_caps_mesh", mesh_occlusion))
		world_set_occlusion(mesh_occlusion ? (current | occlusion_caps_mesh) : (current & ~occlusion_caps_mesh));
	ui_pop_enabled();

	// Raycasting controls
	ui_hseparator();
	ui_label(sk_system_info().world_raycast_present ? "world_raycast_enabled" : "World raycasting isn't available on this system");
	ui_push_enabled(sk_system_info().world_raycast_present);

	bool32_t raycast = world_get_raycast_enabled();
	if (ui_toggle("Enable Raycast", raycast))
		world_set_raycast_enabled(raycast);

	ui_pop_enabled();

	ui_window_end();

	// Raycast finger visualization
	for (int h = 0; h < handed_max; h++) {
		const hand_t *hand = input_hand((handed_)h);
		if (!(hand->tracked_state & button_state_active)) continue;

		const hand_joint_t *tip = &hand->fingers[finger_id_index][joint_id_tip];
		ray_t finger_ray = { tip->position, tip->orientation * vec3_forward };
		ray_t intersection;
		if (world_raycast(finger_ray, &intersection))
			render_add_mesh(mr_sphere, material_find(default_id_material), matrix_trs(intersection.pos, quat_identity, vec3_one * 0.03f));
	}

	// Play boundary visualization
	if (world_has_bounds()) {
		vec2   s    = vec2{ world_get_bounds_size().x / 2, world_get_bounds_size().y / 2 };
		matrix pose = pose_matrix(world_get_bounds_pose());
		vec3   tl   = matrix_transform_pt(pose, vec3{ s.x, 0,  s.y});
		vec3   br   = matrix_transform_pt(pose, vec3{-s.x, 0, -s.y});
		vec3   tr   = matrix_transform_pt(pose, vec3{-s.x, 0,  s.y});
		vec3   bl   = matrix_transform_pt(pose, vec3{ s.x, 0, -s.y});

		color32 white = {255, 255, 255, 255};
		line_add(tl, tr, white, white, 1.5f*cm2m);
		line_add(bl, br, white, white, 1.5f*cm2m);
		line_add(tl, bl, white, white, 1.5f*cm2m);
		line_add(tr, br, white, white, 1.5f*cm2m);
	}
}

///////////////////////////////////////////

void demo_mixed_reality_shutdown() {
	mesh_release(mr_sphere);
}
